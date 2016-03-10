#include<EquationData.h>


template <int dim>
double Coefficient<dim>::value (const dealii::Point<dim> &p,
                                const unsigned int) const
{
  return 0.27 + 0.0*p.square();
  // step-37 coefficient:  return 1. / (0.05 + 2.*p.square());
}

template <int dim>
void Coefficient<dim>::value_list (const std::vector<dealii::Point<dim> > &points,
                                   std::vector<double>                    &values,
                                   const unsigned int                     component) const
{
  Assert (values.size() == points.size(),
		  dealii::ExcDimensionMismatch (values.size(), points.size()));
  Assert (component == 0,
		  dealii::ExcIndexRange (component, 0, 1));
  const unsigned int n_points = points.size();
  for (unsigned int i=0; i<n_points; ++i)
    values[i] = value(points[i],component);
}

template class Coefficient<2>;
template class Coefficient<3>;