##============================================================================
##  Copyright (c) Kitware, Inc.
##  All rights reserved.
##  See LICENSE.txt for details.
##  This software is distributed WITHOUT ANY WARRANTY; without even
##  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
##  PURPOSE.  See the above copyright notice for more information.
##
##  Copyright 2014 Sandia Corporation.
##  Copyright 2014 UT-Battelle, LLC.
##  Copyright 2014 Los Alamos National Security.
##
##  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
##  the U.S. Government retains certain rights in this software.
##
##  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
##  Laboratory (LANL), the U.S. Government retains certain rights in
##  this software.
##============================================================================

set(headers
  CellAverage.h
  CellFilter.h
  DataSetFilter.h
  DataSetWithFieldFilter.h
  DefaultPolicy.h
  ExternalFaces.h
  FieldFilter.h
  FieldMetadata.h
  FilterTraits.h
  MarchingCubes.h
  PointElevation.h
  PolicyBase.h
  Threshold.h
  VertexClustering.h
)

set(header_template_sources
  CellAverage.hxx
  CellFilter.hxx
  DataSetFilter.hxx
  DataSetWithFieldFilter.hxx
  ExternalFaces.hxx
  FieldFilter.hxx
  MarchingCubes.hxx
  PointElevation.hxx
  Threshold.hxx
  VertexClustering.hxx
)

vtkm_declare_headers(${headers})

vtkm_install_headers(${header_template_sources})

add_subdirectory(internal)

#-----------------------------------------------------------------------------
add_subdirectory(testing)
