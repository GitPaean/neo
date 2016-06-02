#ifndef PSCPRECONDITIONER_H
#define PSCPRECONDITIONER_H

#include <deal.II/base/timer.h>
#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_tools.h>
#include <deal.II/base/work_stream.h>
#include <deal.II/distributed/tria.h>
#include <deal.II/grid/grid_generator.h>

#include <functional>

#include <GenericLinearAlgebra.h>
#include <DDHandler.h>
#include <MFOperator.h>

template <int dim=2, typename VectorType=LA::MPI::Vector, class number=double, bool same_diagonal=false>
class PSCPreconditioner final
{
public:
  typedef typename dealii::FullMatrix<double> Matrix;
  class AdditionalData;

  PSCPreconditioner();
  PSCPreconditioner (const PSCPreconditioner &) = delete ;
  PSCPreconditioner &operator = (const PSCPreconditioner &) = delete;

  // interface for MGSmootherPrecondition but global_operator is not used
  template <class GlobalOperatorType>
  void initialize(const GlobalOperatorType &global_operator,
                  const AdditionalData &data);
  void clear();

  void vmult(VectorType &dst, const VectorType &src) const;

  void Tvmult(VectorType &dst, const VectorType &src) const;

  void vmult_add(VectorType &dst, const VectorType &src) const;

  void Tvmult_add(VectorType &dst, const VectorType &src) const;

  static dealii::TimerOutput *timer;

protected:
  AdditionalData data;

private:
  void build_matrix
  (const std::vector<typename dealii::DoFHandler<dim>::level_cell_iterator> &cell_range,
   const std::vector<dealii::types::global_dof_index> &global_dofs_on_subdomain,
   const std::map<dealii::types::global_dof_index, unsigned int> &all_to_unique,
   dealii::FullMatrix<double> &matrix);

  std::vector<Matrix *> patch_inverses;
  std::vector<Matrix> real_patch_inverses;

  dealii::MeshWorker::IntegrationInfoBox<dim> info_box;
  std::unique_ptr<dealii::MeshWorker::DoFInfo<dim> >  dof_info;

  MatrixIntegrator<dim,same_diagonal>                 matrix_integrator;

  unsigned int level;
};

template <int dim, typename VectorType, class number, bool same_diagonal>
class PSCPreconditioner<dim, VectorType, number, same_diagonal>::AdditionalData
{
public:
  AdditionalData() : ddh(0), weight(1.0), mapping(0) {}

  const DDHandlerBase<dim> *ddh;
  double weight;
  const dealii::Mapping<dim> *mapping;
};

template <int dim, typename VectorType, class number, bool same_diagonal>
template <class GlobalOperatorType>
void PSCPreconditioner<dim, VectorType, number, same_diagonal>::initialize(const GlobalOperatorType & /*global_operator*/,
    const AdditionalData &data)
{
  Assert(data.ddh != 0, dealii::ExcInternalError());
  Assert(data.mapping != 0, dealii::ExcInternalError());

  this->data = data;

  level = data.ddh->get_level();

  const dealii::FiniteElement<dim> &fe = data.ddh->get_dofh().get_fe();
  const dealii::DoFHandler<dim> &dof_handler =data.ddh->get_dofh();

  const unsigned int n_gauss_points = fe.degree+1;
  info_box.initialize_gauss_quadrature(n_gauss_points,
                                       n_gauss_points,
                                       n_gauss_points);
  info_box.initialize_update_flags();
  dealii::UpdateFlags update_flags = dealii::update_JxW_values |
                                     dealii::update_quadrature_points |
                                     dealii::update_values |
                                     dealii::update_gradients;
  info_box.add_update_flags(update_flags, true, true, true, true);
  info_box.cell_selector.add("src", true, true, false);
  info_box.boundary_selector.add("src", true, true, false);
  info_box.face_selector.add("src", true, true, false);
  info_box.initialize(fe, *(data.mapping), &(dof_handler.block_info()));
  dof_info.reset(new dealii::MeshWorker::DoFInfo<dim> (dof_handler.block_info()));

  patch_inverses.resize(data.ddh->global_dofs_on_subdomain.size());
  //setup local matrices/inverses
  {
    timer->enter_subsection("LO::build_matrices");
    if (same_diagonal)
      {
        real_patch_inverses.resize(1);
        const unsigned int n = fe.n_dofs_per_cell();
        real_patch_inverses[0]=dealii::FullMatrix<double>(n);
        dealii::Triangulation<dim> local_triangulation;
        dealii::DoFHandler<dim> local_dof_handler(local_triangulation);
        if (level == 0)
          dealii::GridGenerator::hyper_cube (local_triangulation,0., 1.);
        else
          dealii::GridGenerator::hyper_cube (local_triangulation,0., 2./std::pow(2., level));
        if (level != 0) local_triangulation.refine_global(1);

        local_dof_handler.distribute_dofs (fe);
        local_dof_handler.initialize_local_block_info();
        dealii::MeshWorker::IntegrationInfoBox<dim> local_info_box;
        const unsigned int local_n_gauss_points = local_dof_handler.get_fe().degree+1;
        local_info_box.initialize_gauss_quadrature(local_n_gauss_points,
                                                   local_n_gauss_points,
                                                   local_n_gauss_points);
        local_info_box.initialize_update_flags();
        dealii::UpdateFlags local_update_flags = dealii::update_quadrature_points |
                                                 dealii::update_values |
                                                 dealii::update_gradients;
        local_info_box.add_update_flags(local_update_flags, true, true, true, true);
        local_info_box.initialize(fe, *(data.mapping), &(local_dof_handler.block_info()));
        dealii::MeshWorker::DoFInfo<dim> local_dof_info(local_dof_handler.block_info());
        dealii::FullMatrix<double> dummy_matrix(local_dof_handler.n_dofs(),local_dof_handler.n_dofs());
        dealii::MeshWorker::Assembler::MatrixSimple<dealii::FullMatrix<double> > local_assembler;
        local_assembler.initialize(dummy_matrix);
        MatrixIntegrator<dim,false> local_integrator ;
        dealii::MeshWorker::integration_loop<dim, dim>
        (local_dof_handler.begin_active(),
         local_dof_handler.end(),
         local_dof_info, local_info_box,
         local_integrator,local_assembler);
        for (unsigned int i = 0; i < n; ++i)
          for (unsigned int j = 0; j < n; ++j)
            {
              real_patch_inverses[0](i, j) = dummy_matrix(i, j);
            }
        //assign to the smoother
        for (unsigned int i=0; i<data.ddh->subdomain_to_global_map.size(); ++i)
          patch_inverses[i] = &real_patch_inverses[0];
      }
    else
      {
        real_patch_inverses.resize(data.ddh->subdomain_to_global_map.size());
        for (unsigned int i=0; i<data.ddh->subdomain_to_global_map.size(); ++i)
          {
            build_matrix(data.ddh->subdomain_to_global_map[i],
                         data.ddh->global_dofs_on_subdomain[i],
                         data.ddh->all_to_unique[i],
                         real_patch_inverses[i]);
            patch_inverses[i] = &real_patch_inverses[i];
          }
      }
    timer->leave_subsection();
  }

  // now invert actually
  for (unsigned int i=0; i<real_patch_inverses.size(); ++i)
    {
      //invert patch_matrix
      /*std::cout << "level: " << data.ddh->get_level() << " "
                << "original cell matrix " << i << ": "<< std::endl;
      real_patch_inverses[i].print(std::cout);*/
      real_patch_inverses[i].gauss_jordan();
    }
}

template <int dim, typename VectorType, class number, bool same_diagonal>
void PSCPreconditioner<dim, VectorType, number, same_diagonal>::build_matrix
(const std::vector<typename dealii::DoFHandler<dim>::level_cell_iterator> &cell_range,
 const std::vector<dealii::types::global_dof_index> &global_dofs_on_subdomain,
 const std::map<dealii::types::global_dof_index, unsigned int> &all_to_unique,
 dealii::FullMatrix<double> &matrix)
{

  dealii::MGLevelObject<dealii::FullMatrix<double> > mg_matrix ;
  mg_matrix.resize(level,level);

  mg_matrix[level] = std::move(dealii::FullMatrix<double>(global_dofs_on_subdomain.size()));

  Assembler::MGMatrixSimpleMapped<dealii::FullMatrix<double> > assembler;
  assembler.initialize(mg_matrix);
#ifdef CG
  assembler.initialize(constraints);
#endif
  assembler.initialize(all_to_unique);

  //now assemble everything
  dealii::MeshWorker::LoopControl lctrl;
  //assemble faces from both sides
  lctrl.faces_to_ghost = dealii::MeshWorker::LoopControl::both;
  lctrl.ghost_cells = true;

  dealii::integration_loop<dim, dim> (cell_range, *dof_info, info_box,
                                      matrix_integrator, assembler, lctrl);

  matrix.copy_from(mg_matrix[level]);
}


template <int dim, typename VectorType, class number, bool same_diagonal>
void PSCPreconditioner<dim, VectorType, number, same_diagonal>::clear()
{}

template <int dim, typename VectorType, class number, bool same_diagonal>
dealii::TimerOutput *
PSCPreconditioner<dim, VectorType, number, same_diagonal>::timer;

#ifdef HEADER_IMPLEMENTATION
#include <PSCPreconditioner.cc>
#endif

#endif
