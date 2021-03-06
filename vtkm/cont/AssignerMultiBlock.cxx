//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2015 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
//  Copyright 2015 UT-Battelle, LLC.
//  Copyright 2015 Los Alamos National Security.
//
//  Under the terms of Contract DE-NA0003525 with NTESS,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================
#include <vtkm/cont/AssignerMultiBlock.h>

#if defined(VTKM_ENABLE_MPI)

// clang-format off
#include <vtkm/cont/EnvironmentTracker.h>
#include VTKM_DIY(diy/mpi.hpp)
// clang-format on

#include <algorithm> // std::lower_bound
#include <numeric>   // std::iota

namespace vtkm
{
namespace cont
{

VTKM_CONT
AssignerMultiBlock::AssignerMultiBlock(const vtkm::cont::MultiBlock& mb)
  : diy::Assigner(vtkm::cont::EnvironmentTracker::GetCommunicator().size(), 1)
  , IScanBlockCounts()
{
  auto comm = vtkm::cont::EnvironmentTracker::GetCommunicator();
  const auto nblocks = mb.GetNumberOfBlocks();

  vtkm::Id iscan;
  diy::mpi::scan(comm, nblocks, iscan, std::plus<vtkm::Id>());
  diy::mpi::all_gather(comm, iscan, this->IScanBlockCounts);

  this->set_nblocks(static_cast<int>(this->IScanBlockCounts.back()));
}

VTKM_CONT
void AssignerMultiBlock::local_gids(int rank, std::vector<int>& gids) const
{
  if (rank == 0)
  {
    assert(this->IScanBlockCounts.size() > 0);
    gids.resize(this->IScanBlockCounts[rank]);
    std::iota(gids.begin(), gids.end(), 0);
  }
  else if (rank > 0 && rank < static_cast<int>(this->IScanBlockCounts.size()))
  {
    gids.resize(this->IScanBlockCounts[rank] - this->IScanBlockCounts[rank - 1]);
    std::iota(gids.begin(), gids.end(), this->IScanBlockCounts[rank - 1]);
  }
}

VTKM_CONT
int AssignerMultiBlock::rank(int gid) const
{
  return static_cast<int>(
    std::lower_bound(this->IScanBlockCounts.begin(), this->IScanBlockCounts.end(), gid + 1) -
    this->IScanBlockCounts.begin());
}


} // vtkm::cont
} // vtkm

#endif // defined(VTKM_ENABLE_MPI)
