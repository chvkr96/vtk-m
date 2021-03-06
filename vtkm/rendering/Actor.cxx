//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2016 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
//  Copyright 2016 UT-Battelle, LLC.
//  Copyright 2016 Los Alamos National Security.
//
//  Under the terms of Contract DE-NA0003525 with NTESS,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================

#include <vtkm/rendering/Actor.h>

#include <vtkm/Assert.h>
#include <vtkm/cont/TryExecute.h>

namespace vtkm
{
namespace rendering
{

struct Actor::InternalsType
{
  vtkm::cont::DynamicCellSet Cells;
  vtkm::cont::CoordinateSystem Coordinates;
  vtkm::cont::Field ScalarField;
  vtkm::rendering::ColorTable ColorTable;

  vtkm::Range ScalarRange;
  vtkm::Bounds SpatialBounds;

  VTKM_CONT
  InternalsType(const vtkm::cont::DynamicCellSet& cells,
                const vtkm::cont::CoordinateSystem& coordinates,
                const vtkm::cont::Field& scalarField,
                const vtkm::rendering::ColorTable& colorTable)
    : Cells(cells)
    , Coordinates(coordinates)
    , ScalarField(scalarField)
    , ColorTable(colorTable)
  {
  }
};

Actor::Actor(const vtkm::cont::DynamicCellSet& cells,
             const vtkm::cont::CoordinateSystem& coordinates,
             const vtkm::cont::Field& scalarField,
             const vtkm::rendering::Color& color)
  : Internals(
      new InternalsType(cells, coordinates, scalarField, vtkm::rendering::ColorTable(color)))
{
  this->Init(coordinates, scalarField);
}

Actor::Actor(const vtkm::cont::DynamicCellSet& cells,
             const vtkm::cont::CoordinateSystem& coordinates,
             const vtkm::cont::Field& scalarField,
             const vtkm::rendering::ColorTable& colorTable)
  : Internals(new InternalsType(cells, coordinates, scalarField, colorTable))
{
  this->Init(coordinates, scalarField);
}

void Actor::Init(const vtkm::cont::CoordinateSystem& coordinates,
                 const vtkm::cont::Field& scalarField)
{
  VTKM_ASSERT(scalarField.GetData().GetNumberOfComponents() == 1);

  scalarField.GetRange(&this->Internals->ScalarRange);
  this->Internals->SpatialBounds = coordinates.GetBounds();
}

void Actor::Render(vtkm::rendering::Mapper& mapper,
                   vtkm::rendering::Canvas& canvas,
                   const vtkm::rendering::Camera& camera) const
{
  mapper.SetCanvas(&canvas);
  mapper.SetActiveColorTable(this->Internals->ColorTable);
  mapper.RenderCells(this->Internals->Cells,
                     this->Internals->Coordinates,
                     this->Internals->ScalarField,
                     this->Internals->ColorTable,
                     camera,
                     this->Internals->ScalarRange);
}

const vtkm::cont::DynamicCellSet& Actor::GetCells() const
{
  return this->Internals->Cells;
}

const vtkm::cont::CoordinateSystem& Actor::GetCoordinates() const
{
  return this->Internals->Coordinates;
}

const vtkm::cont::Field& Actor::GetScalarField() const
{
  return this->Internals->ScalarField;
}

const vtkm::rendering::ColorTable& Actor::GetColorTable() const
{
  return this->Internals->ColorTable;
}

const vtkm::Range& Actor::GetScalarRange() const
{
  return this->Internals->ScalarRange;
}

const vtkm::Bounds& Actor::GetSpatialBounds() const
{
  return this->Internals->SpatialBounds;
}

void Actor::SetScalarRange(const vtkm::Range& scalarRange)
{
  this->Internals->ScalarRange = scalarRange;
}
}
} // namespace vtkm::rendering
