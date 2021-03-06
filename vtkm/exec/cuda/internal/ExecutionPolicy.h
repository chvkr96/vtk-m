
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
#ifndef vtk_m_exec_cuda_internal_ExecutionPolicy_h
#define vtk_m_exec_cuda_internal_ExecutionPolicy_h

#include <vtkm/BinaryPredicates.h>
#include <vtkm/cont/cuda/ErrorCuda.h>
#include <vtkm/exec/cuda/internal/WrappedOperators.h>

VTKM_THIRDPARTY_PRE_INCLUDE
#include <thrust/execution_policy.h>
#include <thrust/system/cuda/execution_policy.h>
#include <thrust/system/cuda/memory.h>
VTKM_THIRDPARTY_POST_INCLUDE

#define ThrustCudaPolicyPerThread ::thrust::cuda::par.on(cudaStreamPerThread)

struct vtkm_cuda_policy : thrust::device_execution_policy<vtkm_cuda_policy>
{
};

//Specialize the sort call for cuda pointers using less/greater operators.
//The purpose of this is that for 32bit types (UInt32,Int32,Float32) thrust
//will call a super fast radix sort only if the operator is thrust::less
//or thrust::greater.
template <typename T>
__host__ __device__ void sort(
  const vtkm_cuda_policy& exec,
  T* first,
  T* last,
  vtkm::exec::cuda::internal::WrappedBinaryPredicate<T, vtkm::SortLess> comp)
{ //sort for concrete pointers and less than op
  //this makes sure that we invoke the thrust radix sort and not merge sort
  return thrust::sort(ThrustCudaPolicyPerThread, first, last, thrust::less<T>());
}

template <typename T, typename RandomAccessIterator>
__host__ __device__ void sort_by_key(
  const vtkm_cuda_policy& exec,
  T* first,
  T* last,
  RandomAccessIterator values_first,
  vtkm::exec::cuda::internal::WrappedBinaryPredicate<T, vtkm::SortLess> comp)
{ //sort for concrete pointers and less than op
  //this makes sure that we invoke the thrust radix sort and not merge sort
  return thrust::sort_by_key(
    ThrustCudaPolicyPerThread, first, last, values_first, thrust::less<T>());
}

template <typename T>
__host__ __device__ void sort(
  const vtkm_cuda_policy& exec,
  T* first,
  T* last,
  vtkm::exec::cuda::internal::WrappedBinaryPredicate<T, ::thrust::less<T>> comp)
{ //sort for concrete pointers and less than op
  //this makes sure that we invoke the thrust radix sort and not merge sort
  return thrust::sort(ThrustCudaPolicyPerThread, first, last, thrust::less<T>());
}

template <typename T, typename RandomAccessIterator>
__host__ __device__ void sort_by_key(
  const vtkm_cuda_policy& exec,
  T* first,
  T* last,
  RandomAccessIterator values_first,
  vtkm::exec::cuda::internal::WrappedBinaryPredicate<T, ::thrust::less<T>> comp)
{ //sort for concrete pointers and less than op
  //this makes sure that we invoke the thrust radix sort and not merge sort
  return thrust::sort_by_key(
    ThrustCudaPolicyPerThread, first, last, values_first, thrust::less<T>());
}

template <typename T>
__host__ __device__ void sort(
  const vtkm_cuda_policy& exec,
  T* first,
  T* last,
  vtkm::exec::cuda::internal::WrappedBinaryPredicate<T, vtkm::SortGreater> comp)
{ //sort for concrete pointers and greater than op
  //this makes sure that we invoke the thrust radix sort and not merge sort
  return thrust::sort(ThrustCudaPolicyPerThread, first, last, thrust::greater<T>());
}

template <typename T, typename RandomAccessIterator>
__host__ __device__ void sort_by_key(
  const vtkm_cuda_policy& exec,
  T* first,
  T* last,
  RandomAccessIterator values_first,
  vtkm::exec::cuda::internal::WrappedBinaryPredicate<T, vtkm::SortGreater> comp)
{ //sort for concrete pointers and greater than op
  //this makes sure that we invoke the thrust radix sort and not merge sort
  return thrust::sort_by_key(
    ThrustCudaPolicyPerThread, first, last, values_first, thrust::greater<T>());
}

template <typename T>
__host__ __device__ void sort(
  const vtkm_cuda_policy& exec,
  T* first,
  T* last,
  vtkm::exec::cuda::internal::WrappedBinaryPredicate<T, ::thrust::greater<T>> comp)
{ //sort for concrete pointers and greater than op
  //this makes sure that we invoke the thrust radix sort and not merge sort
  return thrust::sort(ThrustCudaPolicyPerThread, first, last, thrust::greater<T>());
}

template <typename T, typename RandomAccessIterator>
__host__ __device__ void sort_by_key(
  const vtkm_cuda_policy& exec,
  T* first,
  T* last,
  RandomAccessIterator values_first,
  vtkm::exec::cuda::internal::WrappedBinaryPredicate<T, ::thrust::greater<T>> comp)
{ //sort for concrete pointers and greater than op
  //this makes sure that we invoke the thrust radix sort and not merge sort
  return thrust::sort_by_key(
    ThrustCudaPolicyPerThread, first, last, values_first, thrust::greater<T>());
}

template <typename RandomAccessIterator, typename StrictWeakOrdering>
__host__ __device__ void sort(const vtkm_cuda_policy& exec,
                              RandomAccessIterator first,
                              RandomAccessIterator last,
                              StrictWeakOrdering comp)
{
  //At this point the pointer type is not a cuda pointers and/or
  //the operator is not an approved less/greater operator.
  //This most likely will cause thrust to internally determine that
  //the best sort implementation is merge sort.
  return thrust::sort(ThrustCudaPolicyPerThread, first, last, comp);
}

template <typename RandomAccessIteratorKeys,
          typename RandomAccessIteratorValues,
          typename StrictWeakOrdering>
__host__ __device__ void sort_by_key(const vtkm_cuda_policy& exec,
                                     RandomAccessIteratorKeys first,
                                     RandomAccessIteratorKeys last,
                                     RandomAccessIteratorValues values_first,
                                     StrictWeakOrdering comp)
{
  //At this point the pointer type is not a cuda pointers and/or
  //the operator is not an approved less/greater operator.
  //This most likely will cause thrust to internally determine that
  //the best sort implementation is merge sort.
  return thrust::sort_by_key(ThrustCudaPolicyPerThread, first, last, values_first, comp);
}

template <typename T,
          typename InputIterator2,
          typename OutputIterator1,
          typename OutputIterator2,
          typename BinaryPredicate,
          typename BinaryFunction>
__host__ __device__::thrust::pair<OutputIterator1, OutputIterator2> reduce_by_key(
  const vtkm_cuda_policy& exec,
  T* keys_first,
  T* keys_last,
  InputIterator2 values_first,
  OutputIterator1 keys_output,
  OutputIterator2 values_output,
  BinaryPredicate binary_pred,
  BinaryFunction binary_op)

{
#if defined(__CUDACC_VER_MAJOR__) && (__CUDACC_VER_MAJOR__ == 7) && (__CUDACC_VER_MINOR__ >= 5)
  ::thrust::pair<OutputIterator1, OutputIterator2> result =
    thrust::reduce_by_key(ThrustCudaPolicyPerThread,
                          keys_first.get(),
                          keys_last.get(),
                          values_first,
                          keys_output,
                          values_output,
                          binary_pred,
                          binary_op);

//only sync if we are being invoked from the host
#ifndef __CUDA_ARCH__
  VTKM_CUDA_CALL(cudaStreamSynchronize(cudaStreamPerThread));
#endif

  return result;
#else
  return thrust::reduce_by_key(ThrustCudaPolicyPerThread,
                               keys_first,
                               keys_last,
                               values_first,
                               keys_output,
                               values_output,
                               binary_pred,
                               binary_op);
#endif
}

#endif
