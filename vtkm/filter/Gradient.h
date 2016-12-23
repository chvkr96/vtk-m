//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2014 Sandia Corporation.
//  Copyright 2014 UT-Battelle, LLC.
//  Copyright 2014 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================

#ifndef vtk_m_filter_Gradient_h
#define vtk_m_filter_Gradient_h

#include <vtkm/filter/FilterCell.h>

namespace vtkm {
namespace filter {

/// Estimates the gradient of a point field in a data set. The created gradient array
/// can be determined at either each point location or at the center of each cell.
///
/// The default for the filter is output as cell centered gradients.
/// To enable point based gradient computation enable \c SetComputePointGradient
///
/// Note: If no explicit name for the output field is provided the filter will
/// default to "Gradients"
class Gradient : public vtkm::filter::FilterCell<Gradient>
{
public:
  Gradient();

  /// When this flag is on (default is off), the gradient filter will provide a
  /// point based gradients, which are significantly more costly since for each
  /// point we need to compute the gradient of each cell that uses it.
  void SetComputePointGradient(bool enable) { ComputePointGradient=enable; }
  bool GetComputePointGradient() const { return ComputePointGradient; }

  /// Add divergence field to the output data.  The name of the array
  /// will be Divergence and will be a cell field unless \c ComputePointGradient
  /// is enabled.  The input array must have 3 components in order to
  /// compute this. The default is off.
  void SetComputeDivergence(bool enable) { ComputeDivergence=enable; }
  bool GetComputeDivergence() const { return ComputeDivergence; }

  /// Add voriticity/curl field to the output data.  The name of the array
  /// will be Vorticity and will be a cell field unless \c ComputePointGradient
  /// is enabled.  The input array must have 3 components in order to
  /// compute this. The default is off.
  void SetComputeVorticity(bool enable) { ComputeVorticity=enable; }
  bool GetComputeVorticity() const { return ComputeVorticity; }

  /// Add Q-criterion field to the output data.  The name of the array
  /// will be QCriterion and will be a cell field unless \c ComputePointGradient
  /// is enabled.  The input array must have 3 components in order to
  /// compute this. The default is off.
  void SetComputeQCriterion(bool enable) { ComputeQCriterion=enable; }
  bool GetComputeQCriterion() const { return ComputeQCriterion; }


  void SetDivergenceName( const std::string& name )
    { this->DivergenceName = name; }
  const std::string& GetDivergenceName() const
    { return this->DivergenceName; }

  void SetVorticityName( const std::string& name )
    { this->VorticityName = name; }
  const std::string& GetVorticityName() const
    { return this->VorticityName; }

  void SetQCriterionName( const std::string& name )
    { this->QCriterionName = name; }
  const std::string& GetQCriterionName() const
    { return this->QCriterionName; }


  template<typename T, typename StorageType, typename DerivedPolicy, typename DeviceAdapter>
  vtkm::filter::ResultField DoExecute(const vtkm::cont::DataSet &input,
                                      const vtkm::cont::ArrayHandle<T, StorageType>& field,
                                      const vtkm::filter::FieldMetadata& fieldMeta,
                                      const vtkm::filter::PolicyBase<DerivedPolicy>& policy,
                                      const DeviceAdapter& tag);

private:
  bool ComputePointGradient;
  bool ComputeDivergence;
  bool ComputeVorticity;
  bool ComputeQCriterion;

  std::string DivergenceName;
  std::string VorticityName;
  std::string QCriterionName;
};

template<>
class FilterTraits<Gradient>
{
public:
  struct TypeListTagGradientInputs
    : vtkm::ListTagBase<
        vtkm::Float32, vtkm::Float64,
        vtkm::Vec<vtkm::Float32, 3>, vtkm::Vec<vtkm::Float64, 3> > {};

  typedef TypeListTagGradientInputs InputFieldTypeList;
};

}
} // namespace vtkm::filter


#include <vtkm/filter/Gradient.hxx>

#endif // vtk_m_filter_Gradient_h