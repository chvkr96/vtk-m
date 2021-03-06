//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2014 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
//  Copyright 2014 UT-Battelle, LLC.
//  Copyright 2014 Los Alamos National Security.
//
//  Under the terms of Contract DE-NA0003525 with NTESS,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================

#include <vtkm/filter/FieldMetadata.h>
#include <vtkm/filter/FilterTraits.h>
#include <vtkm/filter/PolicyDefault.h>

#include <vtkm/filter/internal/ResolveFieldTypeAndExecute.h>

#include <vtkm/cont/Error.h>
#include <vtkm/cont/ErrorBadAllocation.h>
#include <vtkm/cont/ErrorExecution.h>
#include <vtkm/cont/MultiBlock.h>

#include <vtkm/cont/cuda/DeviceAdapterCuda.h>
#include <vtkm/cont/tbb/DeviceAdapterTBB.h>

namespace vtkm
{
namespace filter
{

//----------------------------------------------------------------------------
template <class Derived>
inline VTKM_CONT FilterField<Derived>::FilterField()
  : Tracker(vtkm::cont::GetGlobalRuntimeDeviceTracker())
{
}

//----------------------------------------------------------------------------
template <class Derived>
inline VTKM_CONT FilterField<Derived>::~FilterField()
{
}

//-----------------------------------------------------------------------------
template <typename Derived>
inline VTKM_CONT Result FilterField<Derived>::Execute(const vtkm::cont::DataSet& input,
                                                      const std::string& inFieldName)
{
  return this->Execute(input, input.GetField(inFieldName), vtkm::filter::PolicyDefault());
}

//-----------------------------------------------------------------------------
template <typename Derived>
inline VTKM_CONT std::vector<vtkm::filter::Result> FilterField<Derived>::Execute(
  const vtkm::cont::MultiBlock& input,
  const std::string& inFieldName)
{
  std::vector<vtkm::filter::Result> results;

  for (vtkm::Id j = 0; j < input.GetNumberOfBlocks(); j++)
  {
    vtkm::filter::Result result = this->Execute(
      input.GetBlock(j), input.GetBlock(j).GetField(inFieldName), vtkm::filter::PolicyDefault());
    results.push_back(result);
  }

  return results;
}
//-----------------------------------------------------------------------------
template <typename Derived>
template <typename DerivedPolicy>
inline VTKM_CONT std::vector<vtkm::filter::Result> FilterField<Derived>::Execute(
  const vtkm::cont::MultiBlock& input,
  const std::string& inFieldName,
  const vtkm::filter::PolicyBase<DerivedPolicy>& policy)
{
  std::vector<vtkm::filter::Result> results;

  for (vtkm::Id j = 0; j < input.GetNumberOfBlocks(); j++)
  {
    vtkm::filter::Result result =
      this->Execute(input.GetBlock(j), input.GetBlock(j).GetField(inFieldName), policy);
    results.push_back(result);
  }

  return results;
}

//-----------------------------------------------------------------------------
template <typename Derived>
inline VTKM_CONT Result FilterField<Derived>::Execute(const vtkm::cont::DataSet& input,
                                                      const vtkm::cont::Field& field)

{
  return this->Execute(input, field, vtkm::filter::PolicyDefault());
}

//-----------------------------------------------------------------------------
template <typename Derived>
inline VTKM_CONT Result FilterField<Derived>::Execute(const vtkm::cont::DataSet& input,
                                                      const vtkm::cont::CoordinateSystem& field)
{
  return this->Execute(input, field, vtkm::filter::PolicyDefault());
}

//-----------------------------------------------------------------------------
template <typename Derived>
template <typename DerivedPolicy>
inline VTKM_CONT Result
FilterField<Derived>::Execute(const vtkm::cont::DataSet& input,
                              const std::string& inFieldName,
                              const vtkm::filter::PolicyBase<DerivedPolicy>& policy)
{
  return this->Execute(input, input.GetField(inFieldName), policy);
}

//-----------------------------------------------------------------------------
template <typename Derived>
template <typename DerivedPolicy>
inline VTKM_CONT Result
FilterField<Derived>::Execute(const vtkm::cont::DataSet& input,
                              const vtkm::cont::Field& field,
                              const vtkm::filter::PolicyBase<DerivedPolicy>& policy)
{
  return this->PrepareForExecution(input, field, policy);
}

//-----------------------------------------------------------------------------
template <typename Derived>
template <typename DerivedPolicy>
inline VTKM_CONT Result
FilterField<Derived>::Execute(const vtkm::cont::DataSet& input,
                              const vtkm::cont::CoordinateSystem& field,
                              const vtkm::filter::PolicyBase<DerivedPolicy>& policy)
{
  //we need to state that the field is actually a coordinate system, so that
  //the filter uses the proper policy to convert the types.
  return this->PrepareForExecution(input, field, policy);
}

//-----------------------------------------------------------------------------
template <typename Derived>
template <typename DerivedPolicy>
inline VTKM_CONT Result
FilterField<Derived>::PrepareForExecution(const vtkm::cont::DataSet& input,
                                          const vtkm::cont::Field& field,
                                          const vtkm::filter::PolicyBase<DerivedPolicy>& policy)
{
  vtkm::filter::FieldMetadata metaData(field);
  Result result;

  typedef internal::ResolveFieldTypeAndExecute<Derived, DerivedPolicy, Result> FunctorType;
  FunctorType functor(static_cast<Derived*>(this), input, metaData, policy, result);

  typedef vtkm::filter::FilterTraits<Derived> Traits;
  vtkm::cont::CastAndCall(
    vtkm::filter::ApplyPolicy(field, policy, Traits()), functor, this->Tracker);
  return result;
}

//-----------------------------------------------------------------------------
template <typename Derived>
template <typename DerivedPolicy>
inline VTKM_CONT Result
FilterField<Derived>::PrepareForExecution(const vtkm::cont::DataSet& input,
                                          const vtkm::cont::CoordinateSystem& field,
                                          const vtkm::filter::PolicyBase<DerivedPolicy>& policy)
{
  //We have a special signature just for CoordinateSystem, so that we can ask
  //the policy for the storage types and value types just for coordinate systems

  vtkm::filter::FieldMetadata metaData(field);
  Result result;

  typedef internal::ResolveFieldTypeAndExecute<Derived, DerivedPolicy, Result> FunctorType;
  FunctorType functor(static_cast<Derived*>(this), input, metaData, policy, result);
  vtkm::cont::CastAndCall(field, functor, this->Tracker);

  return result;
}
}
}
