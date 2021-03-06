
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
//  Copyright (c) 2016, Los Alamos National Security, LLC
//  All rights reserved.
//
//  Copyright 2016. Los Alamos National Security, LLC.
//  This software was produced under U.S. Government contract DE-AC52-06NA25396
//  for Los Alamos National Laboratory (LANL), which is operated by
//  Los Alamos National Security, LLC for the U.S. Department of Energy.
//  The U.S. Government has rights to use, reproduce, and distribute this
//  software.  NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY, LLC
//  MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY FOR THE
//  USE OF THIS SOFTWARE.  If software is modified to produce derivative works,
//  such modified software should be clearly marked, so as not to confuse it
//  with the version available from LANL.
//
//  Additionally, redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the following conditions
//  are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Los Alamos National Security, LLC, Los Alamos
//     National Laboratory, LANL, the U.S. Government, nor the names of its
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND
//  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
//  BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LOS ALAMOS
//  NATIONAL SECURITY, LLC OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
//  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
//  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//============================================================================

#ifndef vtkm_worklet_cosmotools_graft_particle_h
#define vtkm_worklet_cosmotools_graft_particle_h

#include <vtkm/worklet/WorkletMapField.h>
#include <vtkm/worklet/cosmotools/TagTypes.h>

namespace vtkm
{
namespace worklet
{
namespace cosmotools
{

// Worklet to graft particles together to form halos
template <typename T>
class GraftParticles : public vtkm::worklet::WorkletMapField
{
public:
  typedef void ControlSignature(
    FieldIn<IdType> index,             // (input) index into particles
    FieldIn<IdType> partId,            // (input) particle id sorted by bin
    FieldIn<IdType> binId,             // (input) bin id sorted by bin
    FieldIn<UInt32TagType> activeFlag, // (input) flag indicates which of neighbor ranges are used
    WholeArrayIn<IdType> partIdArray,  // (input) particle id sorted by bin entire array
    WholeArrayIn<Vec3TagType<T>> location, // (input) location of particles
    WholeArrayIn<IdType> firstParticleId,  // (input) first particle index vector
    WholeArrayIn<IdType> lastParticleId,   // (input) last particle index vector
    WholeArrayOut<IdType> haloId);
  typedef void ExecutionSignature(_1, _2, _3, _4, _5, _6, _7, _8, _9);
  typedef _1 InputDomain;

  vtkm::Id xNum, yNum, zNum;
  vtkm::Id NUM_NEIGHBORS;
  T linkLenSq;

  // Constructor
  VTKM_EXEC_CONT
  GraftParticles(const vtkm::Id XNum,
                 const vtkm::Id YNum,
                 const vtkm::Id ZNum,
                 const vtkm::Id NumNeighbors,
                 const T LinkLen)
    : xNum(XNum)
    , yNum(YNum)
    , zNum(ZNum)
    , NUM_NEIGHBORS(NumNeighbors)
    , linkLenSq(LinkLen * LinkLen)
  {
  }

  template <typename InIdPortalType,
            typename InFieldPortalType,
            typename InVectorPortalType,
            typename OutPortalType>
  VTKM_EXEC void operator()(const vtkm::Id& i,
                            const vtkm::Id& iPartId,
                            const vtkm::Id& iBinId,
                            const vtkm::UInt32& activeFlag,
                            const InIdPortalType& partIdArray,
                            const InFieldPortalType& location,
                            const InVectorPortalType& firstParticleId,
                            const InVectorPortalType& lastParticleId,
                            OutPortalType& haloId) const
  {
    const vtkm::Id yVal = (iBinId / xNum) % yNum;
    const vtkm::Id zVal = iBinId / (xNum * yNum);
    vtkm::UInt32 flag = activeFlag;
    vtkm::Id cnt = 0;

    // Iterate on both sides of the bin this particle is in
    for (vtkm::Id z = zVal - 1; z <= zVal + 1; z++)
    {
      for (vtkm::Id y = yVal - 1; y <= yVal + 1; y++)
      {
        if (flag & 0x1)
        {
          vtkm::Id firstBinId = NUM_NEIGHBORS * i + cnt;
          vtkm::Id startParticle = firstParticleId.Get(firstBinId);
          vtkm::Id endParticle = lastParticleId.Get(firstBinId);

          for (vtkm::Id j = startParticle; j < endParticle; j++)
          {
            vtkm::Id jPartId = partIdArray.Get(j);
            vtkm::Vec<T, 3> iloc = location.Get(iPartId);
            vtkm::Vec<T, 3> jloc = location.Get(jPartId);
            T xDist = iloc[0] - jloc[0];
            T yDist = iloc[1] - jloc[1];
            T zDist = iloc[2] - jloc[2];
            if ((xDist * xDist + yDist * yDist + zDist * zDist) <= linkLenSq)
            {
              if ((haloId.Get(iPartId) == haloId.Get(haloId.Get(iPartId))) &&
                  (haloId.Get(jPartId) < haloId.Get(iPartId)))
              {
                haloId.Set(haloId.Get(iPartId), haloId.Get(jPartId));
              }
            }
          }
        }
        flag = flag >> 1;
        cnt++;
      }
    }
  }
}; // GraftParticles
}
}
}

#endif
