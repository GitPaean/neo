#ifndef INTEGRATORS_H
#define INTEGRATORS_H

#include <deal.II/base/function.h>
#include <deal.II/base/exceptions.h>
#include <deal.II/meshworker/dof_info.h>
#include <deal.II/meshworker/integration_info.h>
#include <deal.II/integrators/laplace.h>
#include <deal.II/integrators/l2.h>
#include <deal.II/lac/vector.h>
#include <deal.II/meshworker/local_integrator.h>

#include <Diffusion.h>
#include <EquationData.h>

template <int dim,bool same_diagonal=true>
class MatrixIntegrator final : public dealii::MeshWorker::LocalIntegrator<dim>
{
public:
  MatrixIntegrator();
  void cell(dealii::MeshWorker::DoFInfo<dim> &dinfo, typename dealii::MeshWorker::IntegrationInfo<dim> &info) const;
  void boundary(dealii::MeshWorker::DoFInfo<dim> &dinfo, typename dealii::MeshWorker::IntegrationInfo<dim> &info) const;
  void face(dealii::MeshWorker::DoFInfo<dim> &dinfo1,
	    dealii::MeshWorker::DoFInfo<dim> &dinfo2,
	    typename dealii::MeshWorker::IntegrationInfo<dim> &info1,
	    typename dealii::MeshWorker::IntegrationInfo<dim> &info2) const;
};

template <int dim>
class ResidualIntegrator final : public dealii::MeshWorker::LocalIntegrator<dim>
{
public:
  ResidualIntegrator();
  void cell(dealii::MeshWorker::DoFInfo<dim> &dinfo, typename dealii::MeshWorker::IntegrationInfo<dim> &info) const;
  void boundary(dealii::MeshWorker::DoFInfo<dim> &dinfo, typename dealii::MeshWorker::IntegrationInfo<dim> &info) const;
  void face(dealii::MeshWorker::DoFInfo<dim> &dinfo1,
	  dealii::MeshWorker::DoFInfo<dim> &dinfo2,
	  typename dealii::MeshWorker::IntegrationInfo<dim> &info1,
	  typename dealii::MeshWorker::IntegrationInfo<dim> &info2) const;
};

template <int dim>
class RHSIntegrator final : public dealii::MeshWorker::LocalIntegrator<dim>
{
 public:
  RHSIntegrator() ;
  void cell(dealii::MeshWorker::DoFInfo<dim> &dinfo, typename dealii::MeshWorker::IntegrationInfo<dim> &info) const override;
  void boundary(dealii::MeshWorker::DoFInfo<dim> &dinfo, typename dealii::MeshWorker::IntegrationInfo<dim> &info) const override;
  void face(dealii::MeshWorker::DoFInfo<dim> &dinfo1,
            dealii::MeshWorker::DoFInfo<dim> &dinfo2,
            typename dealii::MeshWorker::IntegrationInfo<dim> &info1,
            typename dealii::MeshWorker::IntegrationInfo<dim> &info2) const override;
};

#endif // INTEGRATORS_H
