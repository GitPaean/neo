#ifndef MGMatrixSimpleMappedMAPPED_H
#define MGMatrixSimpleMappedMAPPED_H

#include <deal.II/meshworker/simple.h>

namespace Assembler
{

  template <typename MatrixType, bool use_mapping = false>
  class MGMatrixSimpleMapped  final
  {
  public:
    /**
     * Constructor, initializing the #threshold, which limits how small
     * dealii::numbers may be to be entered into the matrix.
     */
    MGMatrixSimpleMapped(double threshold = 1.e-12);

    /**
     * Store the result matrix for later assembling.
     */
    void initialize(dealii::MGLevelObject<MatrixType> &m);

    /**
     * Initialize the multilevel constraints.
     */
    void initialize(const dealii::MGConstrainedDoFs &mg_constrained_dofs);

    void initialize(const std::map<dealii::types::global_dof_index, unsigned int> &d);

    /**
     * Initialize the matrices #flux_up and #flux_down used for local
     * refinement with discontinuous Galerkin methods.
     */
    void initialize_fluxes(dealii::MGLevelObject<MatrixType> &flux_up,
                           dealii::MGLevelObject<MatrixType> &flux_down);

    /**
     * Initialize the matrices #interface_in and #interface_out used for
     * local refinement with continuous Galerkin methods.
     */
    void initialize_interfaces(dealii::MGLevelObject<MatrixType> &interface_in,
                               dealii::MGLevelObject<MatrixType> &interface_out);
    /**
     * Initialize the local data in the DoFInfo object used later for
     * assembling.
     *
     * The info object refers to a cell if <code>!face</code>, or else to an
     * interior or boundary face.
     */
    template <class DOFINFO>
    void initialize_info(DOFINFO &info, bool face) const;

    /**
     * Assemble the matrix DoFInfo::M1[0] into the global matrix.
     */
    template<class DOFINFO>
    void assemble(const DOFINFO &info);

    /**
     * Assemble both local matrices in the info objects into the global
     * matrices.
     */
    template<class DOFINFO>
    void assemble(const DOFINFO &info1,
                  const DOFINFO &info2);
  private:
    /**
     * Get mapped DoFs if there is a dof_mapping or return the identity otherwise
     */
    dealii::types::global_dof_index mapped_dof(dealii::types::global_dof_index);


    /**
     * Assemble a single matrix into a global matrix.
     */
    void assemble(MatrixType &G,
                  const dealii::FullMatrix<double> &M,
                  const std::vector<dealii::types::global_dof_index> &i1,
                  const std::vector<dealii::types::global_dof_index> &i2);

    /**
     * Assemble a single matrix into a global matrix.
     */
    void assemble(MatrixType &G,
                  const dealii::FullMatrix<double> &M,
                  const std::vector<dealii::types::global_dof_index> &i1,
                  const std::vector<dealii::types::global_dof_index> &i2,
                  const unsigned int level);

    /**
     * Assemble a single matrix into a global matrix.
     */

    void assemble_up(MatrixType &G,
                     const dealii::FullMatrix<double> &M,
                     const std::vector<dealii::types::global_dof_index> &i1,
                     const std::vector<dealii::types::global_dof_index> &i2,
                     const unsigned int level = dealii::numbers::invalid_unsigned_int);
    /**
     * Assemble a single matrix into a global matrix.
     */

    void assemble_down(MatrixType &G,
                       const dealii::FullMatrix<double> &M,
                       const std::vector<dealii::types::global_dof_index> &i1,
                       const std::vector<dealii::types::global_dof_index> &i2,
                       const unsigned int level = dealii::numbers::invalid_unsigned_int);

    /**
     * Assemble a single matrix into a global matrix.
     */

    void assemble_in(MatrixType &G,
                     const dealii::FullMatrix<double> &M,
                     const std::vector<dealii::types::global_dof_index> &i1,
                     const std::vector<dealii::types::global_dof_index> &i2,
                     const unsigned int level = dealii::numbers::invalid_unsigned_int);

    /**
     * Assemble a single matrix into a global matrix.
     */

    void assemble_out(MatrixType &G,
                      const dealii::FullMatrix<double> &M,
                      const std::vector<dealii::types::global_dof_index> &i1,
                      const std::vector<dealii::types::global_dof_index> &i2,
                      const unsigned int level = dealii::numbers::invalid_unsigned_int);

    /**
     * The global matrix being assembled.
     */
    dealii::SmartPointer<dealii::MGLevelObject<MatrixType>,MGMatrixSimpleMapped<MatrixType, use_mapping> > matrix;

    /**
     * The matrix used for face flux terms across the refinement edge,
     * coupling coarse to fine.
     */
    dealii::SmartPointer<dealii::MGLevelObject<MatrixType>,MGMatrixSimpleMapped<MatrixType, use_mapping> > flux_up;

    /**
     * The matrix used for face flux terms across the refinement edge,
     * coupling fine to coarse.
     */
    dealii::SmartPointer<dealii::MGLevelObject<MatrixType>,MGMatrixSimpleMapped<MatrixType, use_mapping> > flux_down;

    /**
     * The matrix used for face contributions for continuous elements across
     * the refinement edge, coupling coarse to fine.
     */
    dealii::SmartPointer<dealii::MGLevelObject<MatrixType>,MGMatrixSimpleMapped<MatrixType, use_mapping> > interface_in;

    /**
     * The matrix used for face contributions for continuous elements across
     * the refinement edge, coupling fine to coarse.
     */
    dealii::SmartPointer<dealii::MGLevelObject<MatrixType>,MGMatrixSimpleMapped<MatrixType, use_mapping> > interface_out;
    /**
     * A pointer to the object containing constraints.
     */
    dealii::SmartPointer<const dealii::MGConstrainedDoFs,MGMatrixSimpleMapped<MatrixType, use_mapping> > mg_constrained_dofs;

    const std::map<dealii::types::global_dof_index, unsigned int> *dof_mapping;

    /**
     * The smallest positive number that will be entered into the global
     * matrix. All smaller absolute values will be treated as zero and will
     * not be assembled.
     */
    const double threshold;

  };


  template <typename MatrixType, bool use_mapping>
  inline
  MGMatrixSimpleMapped<MatrixType, use_mapping>::MGMatrixSimpleMapped(double threshold)
    :
    threshold(threshold)
  {}


  template <typename MatrixType, bool use_mapping>
  inline void
  MGMatrixSimpleMapped<MatrixType, use_mapping>::initialize(dealii::MGLevelObject<MatrixType> &m)
  {
    matrix = &m;
  }

  template <typename MatrixType, bool use_mapping>
  inline void
  MGMatrixSimpleMapped<MatrixType, use_mapping>::initialize(const dealii::MGConstrainedDoFs &c)
  {
    mg_constrained_dofs = &c;
  }

  template <typename MatrixType, bool use_mapping>
  inline void
  MGMatrixSimpleMapped<MatrixType, use_mapping>::initialize(const std::map<dealii::types::global_dof_index, unsigned int> &d)
  {
    AssertThrow (use_mapping == true, dealii::ExcMessage("You try to set a dof_mapping but 'use_mapping' is false!"));
    dof_mapping = &d;
  }


  template <typename MatrixType, bool use_mapping>
  inline void
  MGMatrixSimpleMapped<MatrixType, use_mapping>::initialize_fluxes(dealii::MGLevelObject<MatrixType> &up,
      dealii::MGLevelObject<MatrixType> &down)
  {
    flux_up = &up;
    flux_down = &down;
  }


  template <typename MatrixType, bool use_mapping>
  inline void
  MGMatrixSimpleMapped<MatrixType, use_mapping>::initialize_interfaces
  (dealii::MGLevelObject<MatrixType> &in, dealii::MGLevelObject<MatrixType> &out)
  {
    interface_in = &in;
    interface_out = &out;
  }


  template <typename MatrixType, bool use_mapping>
  template <class DOFINFO>
  inline void
  MGMatrixSimpleMapped<MatrixType, use_mapping>::initialize_info(DOFINFO &info, bool face) const
  {
    const unsigned int n = info.indices_by_block.size();

    if (n == 0)
      info.initialize_matrices(1, face);
    else
      {
        info.initialize_matrices(n*n, face);
        unsigned int k=0;
        for (unsigned int i=0; i<n; ++i)
          for (unsigned int j=0; j<n; ++j,++k)
            {
              info.matrix(k,false).row = i;
              info.matrix(k,false).column = j;
              if (face)
                {
                  info.matrix(k,true).row = i;
                  info.matrix(k,true).column = j;
                }
            }
      }
  }

  template <typename MatrixType, bool use_mapping>
  inline dealii::types::global_dof_index
  MGMatrixSimpleMapped<MatrixType, use_mapping>::mapped_dof(dealii::types::global_dof_index i)
  {    
    return (use_mapping == false)?i:dof_mapping->at(i);
  }


  template <typename MatrixType, bool use_mapping>
  inline void
  MGMatrixSimpleMapped<MatrixType, use_mapping>::assemble
  (MatrixType                                 &G,
   const dealii::FullMatrix<double>                   &M,
   const std::vector<dealii::types::global_dof_index> &i1,
   const std::vector<dealii::types::global_dof_index> &i2)
  {
    AssertDimension(M.m(), i1.size());
    AssertDimension(M.n(), i2.size());
    Assert(mg_constrained_dofs == 0, dealii::ExcInternalError());
//TODO: Possibly remove this function all together

    for (unsigned int j=0; j<i1.size(); ++j)
      for (unsigned int k=0; k<i2.size(); ++k)
        if (std::fabs(M(j,k)) >= threshold)
          G.add(mapped_dof(i1[j]), mapped_dof(i2[k]), M(j,k));
  }


  template <typename MatrixType, bool use_mapping>
  inline void
  MGMatrixSimpleMapped<MatrixType, use_mapping>::assemble
  (MatrixType                                 &G,
   const dealii::FullMatrix<double>                   &M,
   const std::vector<dealii::types::global_dof_index> &i1,
   const std::vector<dealii::types::global_dof_index> &i2,
   const unsigned int                          level)
  {
    AssertDimension(M.m(), i1.size());
    AssertDimension(M.n(), i2.size());

    if (mg_constrained_dofs == 0)
      {
        for (unsigned int j=0; j<i1.size(); ++j)
          for (unsigned int k=0; k<i2.size(); ++k)
            if (std::fabs(M(j,k)) >= threshold)
              G.add(mapped_dof(i1[j]), mapped_dof(i2[k]), M(j,k));
      }
    else
      {
        for (unsigned int j=0; j<i1.size(); ++j)
          for (unsigned int k=0; k<i2.size(); ++k)
            {
              // Only enter the local values into the global matrix,
              //  if the value is larger than the threshold
              if (std::fabs(M(j,k)) < threshold)
                continue;

              // Do not enter, if either the row or the column
              // corresponds to an index on the refinement edge. The
              // level problems are solved with homogeneous
              // Dirichlet boundary conditions, therefore we
              // eliminate these rows and columns. The corresponding
              // matrix entries are entered by assemble_in() and
              // assemble_out().
              if (mg_constrained_dofs->at_refinement_edge(level, mapped_dof(i1[j])) ||
                  mg_constrained_dofs->at_refinement_edge(level, mapped_dof(i2[k])))
                continue;

              // At the boundary, only enter the term on the
              // diagonal, but not the coupling terms
              if ((mg_constrained_dofs->is_boundary_index(level, mapped_dof(i1[j])) ||
                   mg_constrained_dofs->is_boundary_index(level, mapped_dof(i2[k]))) &&
                  (mapped_dof(i1[j]) != mapped_dof(i2[k])))
                continue;

              G.add(mapped_dof(i1[j]), mapped_dof(i2[k]), M(j,k));
            }
      }
  }


  template <typename MatrixType, bool use_mapping>
  inline void
  MGMatrixSimpleMapped<MatrixType, use_mapping>::assemble_up
  (MatrixType                                 &G,
   const dealii::FullMatrix<double>                   &M,
   const std::vector<dealii::types::global_dof_index> &i1,
   const std::vector<dealii::types::global_dof_index> &i2,
   const unsigned int                          level)
  {
    AssertDimension(M.n(), i1.size());
    AssertDimension(M.m(), i2.size());

    if (mg_constrained_dofs == 0)
      {
        for (unsigned int j=0; j<i1.size(); ++j)
          for (unsigned int k=0; k<i2.size(); ++k)
            if (std::fabs(M(k,j)) >= threshold)
              G.add(mapped_dof(i1[j]), mapped_dof(i2[k]), M(k,j));
      }
    else
      {
        for (unsigned int j=0; j<i1.size(); ++j)
          for (unsigned int k=0; k<i2.size(); ++k)
            if (std::fabs(M(k,j)) >= threshold)
              if (!mg_constrained_dofs->at_refinement_edge(level, mapped_dof(i2[k])))
                G.add(mapped_dof(i1[j]), mapped_dof(i2[k]), M(k,j));
      }
  }

  template <typename MatrixType, bool use_mapping>
  inline void
  MGMatrixSimpleMapped<MatrixType, use_mapping>::assemble_down
  (MatrixType                                 &G,
   const dealii::FullMatrix<double>                   &M,
   const std::vector<dealii::types::global_dof_index> &i1,
   const std::vector<dealii::types::global_dof_index> &i2,
   const unsigned int                          level)
  {
    AssertDimension(M.m(), i1.size());
    AssertDimension(M.n(), i2.size());

    if (mg_constrained_dofs == 0)
      {
        for (unsigned int j=0; j<i1.size(); ++j)
          for (unsigned int k=0; k<i2.size(); ++k)
            if (std::fabs(M(j,k)) >= threshold)
              G.add(mapped_dof(i1[j]), mapped_dof(i2[k]), M(j,k));
      }
    else
      {
        for (unsigned int j=0; j<i1.size(); ++j)
          for (unsigned int k=0; k<i2.size(); ++k)
            if (std::fabs(M(j,k)) >= threshold)
              if (!mg_constrained_dofs->at_refinement_edge(level, mapped_dof(i2[k])))
                G.add(mapped_dof(i1[j]), mapped_dof(i2[k]), M(j,k));
      }
  }

  template <typename MatrixType, bool use_mapping>
  inline void
  MGMatrixSimpleMapped<MatrixType, use_mapping>::assemble_in
  (MatrixType                                 &G,
   const dealii::FullMatrix<double>                   &M,
   const std::vector<dealii::types::global_dof_index> &i1,
   const std::vector<dealii::types::global_dof_index> &i2,
   const unsigned int                          level)
  {
    AssertDimension(M.m(), i1.size());
    AssertDimension(M.n(), i2.size());
    Assert(mg_constrained_dofs != 0, dealii::ExcInternalError());

    for (unsigned int j=0; j<i1.size(); ++j)
      for (unsigned int k=0; k<i2.size(); ++k)
        if (std::fabs(M(j,k)) >= threshold)
          // Enter values into matrix only if j corresponds to a
          // degree of freedom on the refinement edge, k does
          // not, and both are not on the boundary. This is part
          // the difference between the complete matrix with no
          // boundary condition at the refinement edge and and
          // the matrix assembled above by assemble().

          // Thus the logic is: enter the row if it is
          // constrained by hanging node constraints (actually,
          // the whole refinement edge), but not if it is
          // constrained by a boundary constraint.
          if (mg_constrained_dofs->at_refinement_edge(level, mapped_dof(i1[j])) &&
              !mg_constrained_dofs->at_refinement_edge(level, mapped_dof(i2[k])))
            {
              if ((!mg_constrained_dofs->is_boundary_index(level, mapped_dof(i1[j])) &&
                   !mg_constrained_dofs->is_boundary_index(level, mapped_dof(i2[k])))
                  ||
                  (mg_constrained_dofs->is_boundary_index(level, mapped_dof(i1[j])) &&
                   mg_constrained_dofs->is_boundary_index(level, mapped_dof(i2[k])) &&
                   mapped_dof(i1[j]) == mapped_dof(i2[k])))
                G.add(mapped_dof(i1[j]), mapped_dof(i2[k]), M(j,k));
            }
  }


  template <typename MatrixType, bool use_mapping>
  inline void
  MGMatrixSimpleMapped<MatrixType, use_mapping>::assemble_out
  (MatrixType                                 &G,
   const dealii::FullMatrix<double>                   &M,
   const std::vector<dealii::types::global_dof_index> &i1,
   const std::vector<dealii::types::global_dof_index> &i2,
   const unsigned int                          level)
  {
    AssertDimension(M.n(), i1.size());
    AssertDimension(M.m(), i2.size());
    Assert(mg_constrained_dofs != 0, dealii::ExcInternalError());

    for (unsigned int j=0; j<i1.size(); ++j)
      for (unsigned int k=0; k<i2.size(); ++k)
        if (std::fabs(M(k,j)) >= threshold)
          if (mg_constrained_dofs->at_refinement_edge(level, mapped_dof(i1[j])) &&
              !mg_constrained_dofs->at_refinement_edge(level, mapped_dof(i2[k])))
            {
              if ((!mg_constrained_dofs->is_boundary_index(level, mapped_dof(i1[j])) &&
                   !mg_constrained_dofs->is_boundary_index(level, mapped_dof(i2[k])))
                  ||
                  (mg_constrained_dofs->is_boundary_index(level, mapped_dof(i1[j])) &&
                   mg_constrained_dofs->is_boundary_index(level, mapped_dof(i2[k])) &&
                   mapped_dof(i1[j]) == mapped_dof(i2[k])))
                G.add(mapped_dof(i1[j]), mapped_dof(i2[k]), M(k,j));
            }
  }


  template <typename MatrixType, bool use_mapping>
  template <class DOFINFO>
  inline void
  MGMatrixSimpleMapped<MatrixType, use_mapping>::assemble(const DOFINFO &info)
  {
    Assert(info.level_cell, dealii::ExcMessage("Cell must access level dofs"));
    const unsigned int level = info.cell->level();

    if (info.indices_by_block.size() == 0)
      {
        if (matrix != 0)
          assemble((*matrix)[level], info.matrix(0,false).matrix,
                   info.indices, info.indices, level);
        if (mg_constrained_dofs != 0)
          {
            assemble_in((*interface_in)[level], info.matrix(0,false).matrix,
                        info.indices, info.indices, level);
            assemble_out((*interface_out)[level],info.matrix(0,false).matrix,
                         info.indices, info.indices, level);
          }
      }
    else
      for (unsigned int k=0; k<info.n_matrices(); ++k)
        {
          const unsigned int row = info.matrix(k,false).row;
          const unsigned int column = info.matrix(k,false).column;

          if (matrix != 0)
            assemble((*matrix)[level], info.matrix(k,false).matrix,
                     info.indices_by_block[row], info.indices_by_block[column], level);

          if (mg_constrained_dofs != 0)
            {
              assemble_in((*interface_in)[level], info.matrix(k,false).matrix,
                          info.indices_by_block[row], info.indices_by_block[column], level);
              assemble_out((*interface_out)[level],info.matrix(k,false).matrix,
                           info.indices_by_block[column], info.indices_by_block[row], level);
            }
        }
  }


  template <typename MatrixType, bool use_mapping>
  template <class DOFINFO>
  inline void
  MGMatrixSimpleMapped<MatrixType, use_mapping>::assemble(const DOFINFO &info1,
                                                          const DOFINFO &info2)
  {
    Assert(info1.level_cell, dealii::ExcMessage("Cell must access level dofs"));
    Assert(info2.level_cell, dealii::ExcMessage("Cell must access level dofs"));
    const unsigned int level1 = info1.cell->level();
    const unsigned int level2 = info2.cell->level();

    if (info1.indices_by_block.size() == 0)
      {
        if (level1 == level2)
          {
            if (matrix != 0)
              {
                assemble((*matrix)[level1], info1.matrix(0,false).matrix, info1.indices, info1.indices, level1);
                assemble((*matrix)[level1], info1.matrix(0,true).matrix, info1.indices, info2.indices, level1);
                assemble((*matrix)[level1], info2.matrix(0,false).matrix, info2.indices, info2.indices, level1);
                assemble((*matrix)[level1], info2.matrix(0,true).matrix, info2.indices, info1.indices, level1);
              }
          }
        else
          {
            Assert(level1 > level2, dealii::ExcInternalError());
            // Do not add info2.M1,
            // which is done by
            // the coarser cell
            if (matrix != 0)
              assemble((*matrix)[level1], info1.matrix(0,false).matrix, info1.indices, info1.indices, level1);
            if (level1>0)
              {
                assemble_up((*flux_up)[level1],info1.matrix(0,true).matrix, info2.indices, info1.indices, level1);
                assemble_down((*flux_down)[level1], info2.matrix(0,true).matrix, info2.indices, info1.indices, level1);
              }
          }
      }
    else
      for (unsigned int k=0; k<info1.n_matrices(); ++k)
        {
          const unsigned int row = info1.matrix(k,false).row;
          const unsigned int column = info1.matrix(k,false).column;

          if (level1 == level2)
            {
              if (matrix != 0)
                {
                  assemble((*matrix)[level1], info1.matrix(k,false).matrix, info1.indices_by_block[row], info1.indices_by_block[column], level1);
                  assemble((*matrix)[level1], info1.matrix(k,true).matrix, info1.indices_by_block[row], info2.indices_by_block[column], level1);
                  assemble((*matrix)[level1], info2.matrix(k,false).matrix, info2.indices_by_block[row], info2.indices_by_block[column], level1);
                  assemble((*matrix)[level1], info2.matrix(k,true).matrix, info2.indices_by_block[row], info1.indices_by_block[column], level1);
                }
            }
          else
            {
              Assert(level1 > level2, dealii::ExcInternalError());
              // Do not add info2.M1,
              // which is done by
              // the coarser cell
              if (matrix != 0)
                assemble((*matrix)[level1], info1.matrix(k,false).matrix, info1.indices_by_block[row], info1.indices_by_block[column], level1);
              if (level1>0)
                {
                  assemble_up((*flux_up)[level1],info1.matrix(k,true).matrix, info2.indices_by_block[column], info1.indices_by_block[row], level1);
                  assemble_down((*flux_down)[level1], info2.matrix(k,true).matrix, info2.indices_by_block[row], info1.indices_by_block[column], level1);
                }
            }
        }
  }

}




#endif // MGMatrixSimpleMappedMAPPED_H
