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
#ifndef vtk_m_rendering_ColorBarAnnotation_h
#define vtk_m_rendering_ColorBarAnnotation_h

#include <vtkm/rendering/vtkm_rendering_export.h>

#include <vtkm/cont/DataSet.h>
#include <vtkm/rendering/AxisAnnotation2D.h>
#include <vtkm/rendering/Camera.h>
#include <vtkm/rendering/Canvas.h>
#include <vtkm/rendering/ColorTable.h>

namespace vtkm
{
namespace rendering
{

class VTKM_RENDERING_EXPORT ColorBarAnnotation
{
protected:
  vtkm::rendering::ColorTable ColorTable;
  vtkm::rendering::AxisAnnotation2D Axis;
  vtkm::Bounds Position;
  bool Horizontal;
  std::string FieldName;

public:
  ColorBarAnnotation();

  virtual ~ColorBarAnnotation();

  VTKM_CONT
  void SetColorTable(const vtkm::rendering::ColorTable& colorTable)
  {
    this->ColorTable = colorTable;
  }

  VTKM_CONT
  void SetRange(const vtkm::Range& range, vtkm::IdComponent numTicks);

  VTKM_CONT
  void SetFieldName(const std::string& fieldName);

  VTKM_CONT
  void SetRange(vtkm::Float64 l, vtkm::Float64 h, vtkm::IdComponent numTicks)
  {
    this->SetRange(vtkm::Range(l, h), numTicks);
  }


  VTKM_CONT
  void SetPosition(const vtkm::Bounds& position);

  virtual void Render(const vtkm::rendering::Camera& camera,
                      const vtkm::rendering::WorldAnnotator& worldAnnotator,
                      vtkm::rendering::Canvas& canvas);
};
}
} //namespace vtkm::rendering

#endif // vtk_m_rendering_ColorBarAnnotation_h
