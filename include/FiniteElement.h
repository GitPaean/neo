#ifndef FINITEELEMENT_H
#define FINITEELEMENT_H

#include <deal.II/fe/fe_dgp.h>
#include <deal.II/fe/fe_dgq.h>
#include <deal.II/fe/mapping_q1.h>
#include <deal.II/fe/fe_system.h>

template <int dim>
class FiniteElement final
{
public:
  FiniteElement (unsigned int degree) ;
  const dealii::MappingQ1<dim>                        mapping;
  dealii::FESystem<dim>                               fe;
};

#ifdef HEADER_IMPLEMENTATION
#include <FiniteElement.cc>
#endif

#endif // FINITEELEMENT_H
