#include <EquationData.h>

extern double eps ;
extern double max_energy ;

template <int dim>
Coefficient<dim>::Coefficient()
  : dealii::Function<dim>()
{}

template <int dim>
double
Coefficient<dim>::value (const dealii::Point<dim> &,
                         const unsigned int d) const
{
  return (d == 0) ? 1. : 1.;//1./(0.05 + 2.*p.square()) : 1.;
}

template <int dim>
void Coefficient<dim>::value_list (const std::vector<dealii::Point<dim> > &points,
                                   std::vector<double>                    &values,
                                   const unsigned int                     component) const
{
  Assert (values.size() == points.size(),
          dealii::ExcDimensionMismatch (values.size(), points.size()));

  const unsigned int n_points = points.size();
  for (unsigned int i=0; i<n_points; ++i)
    values[i] = value(points[i],component);
}

template <int dim>
dealii::Tensor<1,dim>
Coefficient<dim>::gradient (const dealii::Point<dim> &/*p*/,
                            const unsigned int  d) const
{
  dealii::Tensor<1,dim> return_grad;
  if (d == 0)
    for (unsigned int i=0; i<dim; ++i)
      return_grad[i]=0.;/*-p(i)/((p.square()+0.025)*(p.square()+0.025));*/

  return return_grad;
}

template <int dim>
Boundaries<dim>::Boundaries(unsigned int n_comp_)
{
#ifdef CG
  dealii::ComponentMask component_mask(n_comp_, true);
  for (unsigned int i = 0; i < 2*dim; ++i)
    set_boundary(i, component_mask);
#else
#endif  // CG
}

template <int dim>
void
Boundaries<dim>::set_boundary(dealii::types::boundary_id index,
			      dealii::ComponentMask mask)
{
  boundary_masks.insert(std::pair<dealii::types::boundary_id, dealii::ComponentMask>(index, mask));
}

template <int dim>
ReferenceFunction<dim>::ReferenceFunction(unsigned int n_comp_)
  : dealii::Function<dim> (n_comp_)
{}

template <int dim>
double
ReferenceFunction<dim>::value(const dealii::Point<dim> &p,
                              const unsigned int /*component = 0*/) const
{
  const double pi2 = dealii::numbers::PI;
  return std::sin(pi2*p(0))*std::sin(pi2*p(1)) + 1.;
}

template <int dim>
dealii::Tensor<1,dim>
ReferenceFunction<dim>::gradient (const dealii::Point<dim> &p,
                                  const unsigned int /*component = 0*/) const
{
  const double pi2 = dealii::numbers::PI;
  dealii::Tensor<1,dim> return_grad;
  return_grad[0]=pi2*std::cos(pi2*p(0))*std::sin(pi2*p(1));
  return_grad[1]=pi2*std::sin(pi2*p(0))*std::cos(pi2*p(1));

  return return_grad;
}

template <int dim>
double
ReferenceFunction<dim>::laplacian(const dealii::Point<dim> &p,
                                  const unsigned int /*component = 0*/) const
{
  const double pi2 = dealii::numbers::PI;
  const double return_value = -2*pi2*pi2*std::sin(pi2*p(0))*std::sin(pi2*p(1));
  return return_value;
}

template <int dim>
XS<dim>::XS():grid {0.9,0.8,0.7,0.6,0.5,0.4,0.3,0.2,0.1,1.E-5}
{
  for (auto & e : grid) e *= max_energy ;
}

template <int dim>
std::vector<std::vector<double> > XS<dim>::total(const std::vector<dealii::Point<dim> > &points,
                                                 unsigned int n_angles,
                                                 unsigned int ,
                                                 double factor)
{
  std::vector<std::vector<double> > xs ;
  xs.resize(n_angles) ;
  for (auto &angle : xs)
    {
      angle.resize(points.size());
      for (auto &point : angle)
        point = 1. / eps * factor  ;
    }
  return xs ;
}

template <int dim>
std::vector<std::vector<std::vector<double> > > XS<dim>::scattering(const std::vector<dealii::Point<dim> > &points,
    unsigned int n_angles,
    unsigned int,
    unsigned int,
    double factor)
{
  std::vector<std::vector<std::vector<double> > > xs ;
  xs.resize(n_angles);
  for (auto &angle_out : xs)
    {
      angle_out.resize(n_angles);
      for (auto &quads : angle_out)
        {
          quads.resize(points.size());
          for (auto &point : quads)
            point = ( 1. / eps * factor ) ;
        }
    }
  return xs ;
}

template <int dim>
std::vector<std::vector<double> > XS<dim>::absorption(const std::vector<dealii::Point<dim> > &points,
                                                      const std::vector<double> &weights,
                                                      unsigned int n_angles,
                                                      unsigned int n_groups,
                                                      unsigned int ,
                                                      double total_factor,
                                                      double scattering_factor)
{
  auto abs = total(points,n_angles,0,total_factor);
  for (unsigned int bout = 0; bout < n_groups ; ++bout)
    for (unsigned int cout = 0; cout < n_angles ; ++cout)
      for (unsigned int q = 0; q < points.size() ; ++q)
	{
	  abs[cout][q] -= scattering(points,n_angles,0,bout,scattering_factor)[0][0][q];
	  if (abs[cout][q] < 1.E-10) abs[cout][q] = 0.;
	}
  return abs ;
}

template <int dim>
Angle<dim>::Angle(const std::string &filename)
{
  std::ifstream file(filename.c_str());
  AssertThrow (file.is_open(), dealii::ExcIO());

  unsigned int filedim, n_points, n_moments;
  file >> filedim;
  file >> n_points;
  file >> n_moments;

  AssertThrow(filedim == dim, dealii::ExcDimensionMismatch(filedim,dim));

  std::vector<dealii::Point<dim> > points(n_points);
  std::vector<double> weights(n_points, 0.);

  for (unsigned int i=0; i<points.size(); ++i)
    {
      for (unsigned int d=0; d<filedim; ++d)
        file >> points[i](d);
      file >> weights[i];
    }

  AssertThrow(weights[n_points-1] != 0., dealii::ExcIO());
  dealii::Quadrature<dim>::initialize(points, weights);
}

//---------------------------------------------------------------------------
//    $Id: solution.h 67 2015-03-03 11:34:17Z kronbichler $
//    Version: $Name$
//
//    Copyright (C) 2013 - 2014 by Katharina Kormann and Martin Kronbichler
//
//---------------------------------------------------------------------------
template <>
const dealii::Point<1>
SolutionBase<1>::source_centers[SolutionBase<1>::n_source_centers]
  = { dealii::Point<1>(-1.0 / 3.0),
      dealii::Point<1>(0.0),
      dealii::Point<1>(+1.0 / 3.0)
    };

template <>
const dealii::Point<2>
SolutionBase<2>::source_centers[SolutionBase<2>::n_source_centers]
  = { dealii::Point<2>(-0.5, +0.5),
      dealii::Point<2>(-0.5, -0.5),
      dealii::Point<2>(+0.5, -0.5)
    };

template <>
const dealii::Point<3>
SolutionBase<3>::source_centers[SolutionBase<3>::n_source_centers]
  = { dealii::Point<3>(-0.5, +0.5, 0.25),
      dealii::Point<3>(-0.6, -0.5, -0.125),
      dealii::Point<3>(+0.5, -0.5, 0.5)
    };

template <int dim>
const double SolutionBase<dim>::width = 1./3.;


//MFSolution
template <int dim>
double MFSolution<dim>::value (const dealii::Point<dim>   &p,
                               const unsigned int) const
{
  const double pi = dealii::numbers::PI;
  double return_value = 0;
  for (unsigned int i=0; i<this->n_source_centers; ++i)
    {
      const dealii::Tensor<1,dim> x_minus_xi = p - this->source_centers[i];
      return_value += std::exp(-x_minus_xi.norm_square() /
                               (this->width * this->width));
    }

  return return_value /
         dealii::Utilities::fixed_power<dim>(std::sqrt(2 * pi) * this->width);
  //  return 0.;
}

template <int dim>
dealii::Tensor<1,dim> MFSolution<dim>::gradient (const dealii::Point<dim>   &p,
                                                 const unsigned int) const
{
  const double pi = dealii::numbers::PI;
  dealii::Tensor<1,dim> return_value;

  for (unsigned int i=0; i<this->n_source_centers; ++i)
    {
      const dealii::Tensor<1,dim> x_minus_xi = p - this->source_centers[i];

      return_value += (-2 / (this->width * this->width) *
                       std::exp(-x_minus_xi.norm_square() /
                                (this->width * this->width)) *
                       x_minus_xi);
    }

  return return_value / dealii::Utilities::fixed_power<dim>(std::sqrt(2 * pi) *
                                                            this->width);
  //  return dealii::Tensor<1,dim>{} ;
}

//MFRightHandSide
template <int dim>
double MFRightHandSide<dim>::value (const dealii::Point<dim>   &p,
                                    const unsigned int) const
{
  const double pi = dealii::numbers::PI;
  double return_value = 0;
  for (unsigned int i=0; i<this->n_source_centers; ++i)
    {
      const dealii::Tensor<1,dim> x_minus_xi = p - this->source_centers[i];

      return_value +=
        ( (2*dim - 4*x_minus_xi.norm_square()/(this->width * this->width) )/
          (this->width * this->width) *
          std::exp(-x_minus_xi.norm_square() /
                   (this->width * this->width)));
    }

  return return_value / dealii::Utilities::fixed_power<dim>(std::sqrt(2 * pi) *
                                                            this->width);
  //  return 1.;
}

template class Coefficient<2>;
template class Coefficient<3>;
template class Boundaries<2>;
template class Boundaries<3>;
template class ReferenceFunction<2>;
template class ReferenceFunction<3>;
template class Angle<2>;
template class Angle<3>;
template class XS<2>;
template class XS<3>;

template class SolutionBase<1>;
template class SolutionBase<2>;
template class SolutionBase<3>;
template class MFSolution<1>;
template class MFSolution<2>;
template class MFSolution<3>;
template class MFRightHandSide<1>;
template class MFRightHandSide<2>;
template class MFRightHandSide<3>;
