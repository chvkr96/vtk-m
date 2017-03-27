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

#include <vtkm/worklet/ScatterCounting.h>
#include <vtkm/worklet/DispatcherMapField.h>

namespace vtkm {
namespace filter {

//-----------------------------------------------------------------------------
inline VTKM_CONT
Triangulate::Triangulate():
  vtkm::filter::FilterDataSet<Triangulate>(),
  Worklet()
{
}

//-----------------------------------------------------------------------------
template<typename DerivedPolicy,
         typename DeviceAdapter>
inline VTKM_CONT
vtkm::filter::ResultDataSet Triangulate::DoExecute(
                                                 const vtkm::cont::DataSet& input,
                                                 const vtkm::filter::PolicyBase<DerivedPolicy>&,
                                                 const DeviceAdapter& device)
{
  typedef vtkm::cont::CellSetStructured<2> CellSetStructuredType;
  typedef vtkm::cont::CellSetExplicit<> CellSetExplicitType;

  const vtkm::cont::DynamicCellSet& cells =
                  input.GetCellSet(this->GetActiveCellSetIndex());

  vtkm::cont::CellSetSingleType<> outCellSet;

  if (cells.IsType<CellSetStructuredType>())
  {
    outCellSet = this->Worklet.Run(cells.Cast<CellSetStructuredType>(),
                                   device);
  }
  else
  {
    outCellSet = this->Worklet.Run(cells.Cast<CellSetExplicitType>(), 
                                   device);
  }

  // create the output dataset
  vtkm::cont::DataSet output;
  output.AddCellSet(outCellSet);
  output.AddCoordinateSystem(input.GetCoordinateSystem(this->GetActiveCoordinateSystemIndex()) );

  return vtkm::filter::ResultDataSet(output);
}

//-----------------------------------------------------------------------------
template<typename T,
         typename StorageType,
         typename DerivedPolicy,
         typename DeviceAdapter>
inline VTKM_CONT
bool Triangulate::DoMapField(
                           vtkm::filter::ResultDataSet& result,
                           const vtkm::cont::ArrayHandle<T, StorageType>& input,
                           const vtkm::filter::FieldMetadata& fieldMeta,
                           const vtkm::filter::PolicyBase<DerivedPolicy>&,
                           const DeviceAdapter& device)
{
  // point data is copied as is because it was not collapsed
  if(fieldMeta.IsPointField())
  {
    result.GetDataSet().AddField(fieldMeta.AsField(input));
    return true;
  }
  
  // cell data must be scattered to the cells created per input cell
  if(fieldMeta.IsCellField())
  {
    vtkm::cont::ArrayHandle<T, StorageType> output = 
        this->Worklet.ProcessField(input, device);

    result.GetDataSet().AddField(fieldMeta.AsField(output));
    return true;

  }

  return false;
}

}
}