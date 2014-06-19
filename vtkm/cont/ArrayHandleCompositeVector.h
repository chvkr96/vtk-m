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
//  Copyright 2014. Los Alamos National Security
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================
#ifndef vtk_m_ArrayHandleCompositeVector_h
#define vtk_m_ArrayHandleCompositeVector_h

#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ErrorControlBadValue.h>
#include <vtkm/cont/ErrorControlInternal.h>

#include <vtkm/VectorTraits.h>

#include <vtkm/internal/FunctionInterface.h>

#include <boost/static_assert.hpp>

namespace vtkm {
namespace cont {

namespace internal {

namespace detail {

template<typename ValueType>
struct CompositeVectorSwizzleFunctor
{
  static const int NUM_COMPONENTS =
      vtkm::VectorTraits<ValueType>::NUM_COMPONENTS;
  typedef vtkm::Tuple<int, NUM_COMPONENTS> ComponentMapType;

  // Caution! This is a reference.
  const ComponentMapType &SourceComponents;

  VTKM_EXEC_CONT_EXPORT
  CompositeVectorSwizzleFunctor(const ComponentMapType &sourceComponents)
    : SourceComponents(sourceComponents) {  }

  // Currently only supporting 1-4 components.
  template<typename T1>
  VTKM_EXEC_CONT_EXPORT
  ValueType operator()(const T1 &p1) const {
    return ValueType(
          vtkm::VectorTraits<T1>::GetComponent(p1, this->SourceComponents[0]));
  }

  template<typename T1, typename T2>
  VTKM_EXEC_CONT_EXPORT
  ValueType operator()(const T1 &p1, const T2 &p2) const {
    return ValueType(
          vtkm::VectorTraits<T1>::GetComponent(p1, this->SourceComponents[0]),
          vtkm::VectorTraits<T2>::GetComponent(p2, this->SourceComponents[1]));
  }

  template<typename T1, typename T2, typename T3>
  VTKM_EXEC_CONT_EXPORT
  ValueType operator()(const T1 &p1, const T2 &p2, const T3 &p3) const {
    return ValueType(
          vtkm::VectorTraits<T1>::GetComponent(p1, this->SourceComponents[0]),
          vtkm::VectorTraits<T2>::GetComponent(p2, this->SourceComponents[1]),
          vtkm::VectorTraits<T3>::GetComponent(p3, this->SourceComponents[2]));
  }

  template<typename T1, typename T2, typename T3, typename T4>
  VTKM_EXEC_CONT_EXPORT
  ValueType operator()(const T1 &p1,
                       const T2 &p2,
                       const T3 &p3,
                       const T4 &p4) const {
    return ValueType(
          vtkm::VectorTraits<T1>::GetComponent(p1, this->SourceComponents[0]),
          vtkm::VectorTraits<T2>::GetComponent(p2, this->SourceComponents[1]),
          vtkm::VectorTraits<T3>::GetComponent(p3, this->SourceComponents[2]),
          vtkm::VectorTraits<T4>::GetComponent(p4, this->SourceComponents[3]));
  }
};

template<typename ReturnValueType>
struct CompositeVectorPullValueFunctor
{
  vtkm::Id Index;

  VTKM_EXEC_EXPORT
  CompositeVectorPullValueFunctor(vtkm::Id index) : Index(index) {  }

  // This form is to pull values out of array arguments.
  template<typename PortalType>
  VTKM_EXEC_EXPORT
  typename PortalType::ValueType operator()(const PortalType &portal) const {
    return portal.Get(this->Index);
  }

  // This form is an identity to pass the return value back.
  VTKM_EXEC_EXPORT
  const ReturnValueType &operator()(const ReturnValueType &value) const {
    return value;
  }
};

struct CompositeVectorArrayToPortalCont {
  template<typename ArrayHandleType>
  struct ReturnType {
    typedef typename ArrayHandleType::PortalConstControl type;
  };

  template<typename ArrayHandleType>
  VTKM_CONT_EXPORT
  typename ReturnType<ArrayHandleType>::type
  operator()(const ArrayHandleType &array) const {
    return array.GetPortalConstControl();
  }
};

template<typename DeviceAdapterTag>
struct CompositeVectorArrayToPortalExec {
  template<typename ArrayHandleType>
  struct ReturnType {
    typedef typename ArrayHandleType::template ExecutionTypes<
          DeviceAdapterTag>::PortalConst type;
  };

  template<typename ArrayHandleType>
  VTKM_CONT_EXPORT
  typename ReturnType<ArrayHandleType>::type
  operator()(const ArrayHandleType &array) const {
    return array.PrepareForInput(DeviceAdapterTag());
  }
};

struct CheckArraySizeFunctor {
  vtkm::Id ExpectedSize;
  CheckArraySizeFunctor(vtkm::Id expectedSize) : ExpectedSize(expectedSize) {  }

  template<typename T>
  void operator()(const T &a) const {
    if (a.GetNumberOfValues() != this->ExpectedSize)
    {
      throw vtkm::cont::ErrorControlBadValue(
            "All input arrays to ArrayHandleCompositeVector must be the same size.");
    }
  }
};

} // namespace detail

/// \brief A portal that gets values from components of other portals.
///
/// This is the portal used within ArrayHandleCompositeVector.
///
template<typename SignatureWithPortals>
class ArrayPortalCompositeVector
{
  typedef vtkm::internal::FunctionInterface<SignatureWithPortals> PortalTypes;
  typedef vtkm::Tuple<int, PortalTypes::ARITY> ComponentMapType;

public:
  typedef typename PortalTypes::ResultType ValueType;
  static const int NUM_COMPONENTS =
      vtkm::VectorTraits<ValueType>::NUM_COMPONENTS;

  BOOST_STATIC_ASSERT(NUM_COMPONENTS == PortalTypes::ARITY);

  VTKM_EXEC_CONT_EXPORT
  ArrayPortalCompositeVector() {  }

  VTKM_CONT_EXPORT
  ArrayPortalCompositeVector(
      const PortalTypes portals,
      vtkm::Tuple<int, NUM_COMPONENTS> sourceComponents)
    : Portals(portals), SourceComponents(sourceComponents) {  }

  VTKM_EXEC_EXPORT
  vtkm::Id GetNumberOfValues() const {
    return this->Portals.template GetParameter<1>().GetNumberOfValues();
  }

  VTKM_EXEC_EXPORT
  ValueType Get(vtkm::Id index) const {
    // This might be inefficient because we are copying all the portals only
    // because they are coupled with the return value.
    PortalTypes localPortals = this->Portals;
    localPortals.InvokeExec(
          detail::CompositeVectorSwizzleFunctor<ValueType>(this->SourceComponents),
          detail::CompositeVectorPullValueFunctor<ValueType>(index));
    return localPortals.GetReturnValue();
  }

private:
  PortalTypes Portals;
  ComponentMapType SourceComponents;
};

/// \brief A "portal" that holds arrays to get components from.
///
/// This class takes place as the control-side portal within an
/// ArrayHandleCompositeVector. This is an incomplete implementation, so you
/// really can't use it to get values. However, between this and the
/// specialization ArrayTransfer, it's enough to get values to the execution
/// environment.
///
template<typename SignatureWithArrays>
class ArrayPortalCompositeVectorCont
{
  typedef vtkm::internal::FunctionInterface<SignatureWithArrays>
      FunctionInterfaceArrays;

public:
  typedef typename FunctionInterfaceArrays::ResultType ValueType;
  static const int NUM_COMPONENTS =
      vtkm::VectorTraits<ValueType>::NUM_COMPONENTS;
  typedef vtkm::Tuple<int, NUM_COMPONENTS> ComponentMapType;

  // If you get a compile error here, it means you probably tried to create
  // an ArrayHandleCompositeVector with a return type of a vector with a
  // different number of components than the number of arrays given.
  BOOST_STATIC_ASSERT(NUM_COMPONENTS == FunctionInterfaceArrays::ARITY);

  VTKM_CONT_EXPORT
  ArrayPortalCompositeVectorCont() : NumberOfValues(0) {  }

  VTKM_CONT_EXPORT
  ArrayPortalCompositeVectorCont(
      const FunctionInterfaceArrays &arrays,
      const ComponentMapType &vtkmNotUsed(sourceComponents))
    : NumberOfValues(arrays.template GetParameter<1>().GetNumberOfValues()) {  }

  VTKM_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const {
    return this->NumberOfValues;
  }

  VTKM_CONT_EXPORT
  ValueType Get(vtkm::Id vtkmNotUsed(index)) const {
    throw vtkm::cont::ErrorControlInternal("Not implemented.");
  }

  VTKM_CONT_EXPORT
  void Set(vtkm::Id vtkmNotUsed(index), ValueType vtkmNotUsed(value)) {
    throw vtkm::cont::ErrorControlInternal("Not implemented.");
  }

  // Not a viable type, but there is no implementation.
  typedef ValueType *IteratorType;

  VTKM_CONT_EXPORT
  IteratorType GetIteratorBegin() const {
    throw vtkm::cont::ErrorControlInternal("Not implemented.");
  }

  VTKM_CONT_EXPORT
  IteratorType GetIteratorEnd() const {
    throw vtkm::cont::ErrorControlInternal("Not implemented.");
  }

private:
  vtkm::Id NumberOfValues;
};

template<typename SignatureWithArrays>
struct ArrayContainerControlTagCompositeVector {  };

/// A convenience class that provides a typedef to the appropriate tag for
/// a counting array container.
template<typename SignatureWithArrays>
struct ArrayHandleCompositeVectorTraits
{
  typedef vtkm::cont::internal::ArrayContainerControlTagCompositeVector<
                                                SignatureWithArrays> Tag;
  typedef typename vtkm::internal::FunctionInterface<SignatureWithArrays>::ResultType
          ValueType;
  typedef vtkm::cont::internal::ArrayContainerControl<
          ValueType, Tag> ContainerType;
};

// It may seem weird that this specialization throws an exception for
// everything, but that is because all the functionality is handled in the
// ArrayTransfer class.
template<typename SignatureWithArrays>
class ArrayContainerControl<
    typename ArrayHandleCompositeVectorTraits<SignatureWithArrays>::ValueType,
    vtkm::cont::internal::ArrayContainerControlTagCompositeVector<SignatureWithArrays> >
{
  typedef vtkm::internal::FunctionInterface<SignatureWithArrays>
      FunctionInterfaceWithArrays;
  static const int NUM_COMPONENTS = FunctionInterfaceWithArrays::ARITY;
  typedef vtkm::Tuple<int, NUM_COMPONENTS> ComponentMapType;

public:
  typedef ArrayPortalCompositeVectorCont<SignatureWithArrays> PortalType;
  typedef PortalType PortalConstType;
  typedef typename PortalType::ValueType ValueType;

  VTKM_CONT_EXPORT
  ArrayContainerControl() : Valid(false) {  }

  VTKM_CONT_EXPORT
  ArrayContainerControl(const FunctionInterfaceWithArrays &arrays,
                        const ComponentMapType &sourceComponents)
    : Arrays(arrays), SourceComponents(sourceComponents), Valid(true)
  {
    arrays.ForEachCont(
          detail::CheckArraySizeFunctor(this->GetNumberOfValues()));
  }

  VTKM_CONT_EXPORT
  PortalType GetPortal() {
    throw vtkm::cont::ErrorControlBadValue(
          "Composite vector arrays are read only.");
  }

  VTKM_CONT_EXPORT
  PortalConstType GetPortalConst() const {
    if (!this->Valid)
    {
      throw vtkm::cont::ErrorControlBadValue(
            "Tried to use an ArrayHandleCompositeHandle without dependent arrays.");
    }
    return PortalConstType(this->Arrays, this->SourceComponents);
  }

  VTKM_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const {
    if (!this->Valid)
    {
      throw vtkm::cont::ErrorControlBadValue(
            "Tried to use an ArrayHandleCompositeHandle without dependent arrays.");
    }
    return this->Arrays.template GetParameter<1>().GetNumberOfValues();
  }

  VTKM_CONT_EXPORT
  void Allocate(vtkm::Id vtkmNotUsed(numberOfValues)) {
    throw vtkm::cont::ErrorControlInternal(

          "The allocate method for the composite vector control array "
          "container should never have been called. The allocate is generally "
          "only called by the execution array manager, and the array transfer "
          "for the transform container should prevent the execution array "
          "manager from being directly used.");
  }

  VTKM_CONT_EXPORT
  void Shrink(vtkm::Id vtkmNotUsed(numberOfValues)) {
    throw vtkm::cont::ErrorControlBadValue(
          "Composite vector arrays are read-only.");
  }

  VTKM_CONT_EXPORT
  void ReleaseResources() {
    if (this->Valid)
    {
      // TODO: Implement this.
    }
  }

  VTKM_CONT_EXPORT
  const FunctionInterfaceWithArrays &GetArrays() const {
    VTKM_ASSERT_CONT(this->Valid);
    return this->Arrays;
  }

  VTKM_CONT_EXPORT
  const ComponentMapType &GetSourceComponents() const {
    VTKM_ASSERT_CONT(this->Valid);
    return this->SourceComponents;
  }

private:
  FunctionInterfaceWithArrays Arrays;
  ComponentMapType SourceComponents;
  bool Valid;
};

template<typename SignatureWithArrays, typename DeviceAdapterTag>
class ArrayTransfer<
    typename ArrayHandleCompositeVectorTraits<SignatureWithArrays>::ValueType,
    vtkm::cont::internal::ArrayContainerControlTagCompositeVector<SignatureWithArrays>,
    DeviceAdapterTag>
{
  VTKM_IS_DEVICE_ADAPTER_TAG(DeviceAdapterTag);

  typedef typename ArrayHandleCompositeVectorTraits<SignatureWithArrays>::ContainerType
      ContainerType;

  typedef vtkm::internal::FunctionInterface<SignatureWithArrays>
      FunctionWithArrays;
  typedef typename FunctionWithArrays::template StaticTransformType<
        detail::CompositeVectorArrayToPortalExec<DeviceAdapterTag> >::type
      FunctionWithPortals;
  typedef typename FunctionWithPortals::Signature SignatureWithPortals;

public:
  typedef typename ArrayHandleCompositeVectorTraits<SignatureWithArrays>::ValueType
      ValueType;

  // These are not currently fully implemented.
  typedef typename ContainerType::PortalType PortalControl;
  typedef typename ContainerType::PortalConstType PortalConstControl;

  typedef ArrayPortalCompositeVector<SignatureWithPortals> PortalExecution;
  typedef ArrayPortalCompositeVector<SignatureWithPortals> PortalConstExecution;

  VTKM_CONT_EXPORT
  ArrayTransfer() : ContainerValid(false) {  }

  VTKM_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const {
    VTKM_ASSERT_CONT(this->ContainerValid);
    return this->Container.GetNumberOfValues();
  }

  VTKM_CONT_EXPORT
  void LoadDataForInput(PortalConstControl vtkmNotUsed(contPortal))
  {
    throw vtkm::cont::ErrorControlInternal(
          "ArrayHandleCompositeVector in a bad state. "
          "There must be a UserArray set, but how did that happen?");
  }

  VTKM_CONT_EXPORT
  void LoadDataForInput(const ContainerType &controlArray)
  {
    this->Container = controlArray;
    this->ContainerValid = true;
  }

  VTKM_CONT_EXPORT
  void LoadDataForInPlace(ContainerType &vtkmNotUsed(controlArray))
  {
    throw vtkm::cont::ErrorControlBadValue(
          "Composite vector arrays cannot be used for output or in place.");
  }

  VTKM_CONT_EXPORT
  void AllocateArrayForOutput(ContainerType &vtkmNotUsed(controlArray),
                              vtkm::Id vtkmNotUsed(numberOfValues))
  {
    throw vtkm::cont::ErrorControlBadValue(
          "Composite vector arrays cannot be used for output.");
  }

  VTKM_CONT_EXPORT
  void RetrieveOutputData(ContainerType &vtkmNotUsed(controlArray)) const
  {
    throw vtkm::cont::ErrorControlBadValue(
          "Composite vector arrays cannot be used for output.");
  }

  VTKM_CONT_EXPORT
  void Shrink(vtkm::Id vtkmNotUsed(numberOfValues))
  {
    throw vtkm::cont::ErrorControlBadValue(
          "Composite vector arrays cannot be resized.");
  }

  VTKM_CONT_EXPORT
  PortalExecution GetPortalExecution()
  {
    throw vtkm::cont::ErrorControlBadValue(
          "Composite vector arrays are read-only. (Get the const portal.)");
  }

  VTKM_CONT_EXPORT
  PortalConstExecution GetPortalConstExecution() const
  {
    VTKM_ASSERT_CONT(this->ContainerValid);
    return
        PortalConstExecution(
          this->Container.GetArrays().StaticTransformCont(
            detail::CompositeVectorArrayToPortalExec<DeviceAdapterTag>()),
          this->Container.GetSourceComponents());
  }

  VTKM_CONT_EXPORT
  void ReleaseResources() {
    this->Container.ReleaseResources();
  }

private:
  bool ContainerValid;
  ContainerType Container;
};

} // namespace internal

/// \brief An \c ArrayHandle that combines components from other arrays.
///
/// \c ArrayHandleCompositeVector is a specialization of \c ArrayHandle that
/// derives its content from other arrays. It takes up to 4 other \c
/// ArrayHandle objects and mimics an array that contains vectors with
/// components that come from these delegate arrays.
///
/// The easiest way to create and type an \c ArrayHandleCompositeVector is
/// to use the \c make_ArrayHandleCompositeVector functions.
///
template<typename Signature>
class ArrayHandleCompositeVector
    : public vtkm::cont::ArrayHandle<
        typename internal::ArrayHandleCompositeVectorTraits<Signature>::ValueType,
        typename internal::ArrayHandleCompositeVectorTraits<Signature>::Tag>
{
  typedef typename internal::ArrayHandleCompositeVectorTraits<Signature>::ContainerType
      ArrayContainerControlType;
  typedef typename internal::ArrayPortalCompositeVectorCont<Signature>::ComponentMapType
      ComponentMapType;

public:
  typedef vtkm::cont::ArrayHandle<
      typename internal::ArrayHandleCompositeVectorTraits<Signature>::ValueType,
      typename internal::ArrayHandleCompositeVectorTraits<Signature>::Tag>
    Superclass;
  typedef typename Superclass::ValueType ValueType;

  VTKM_CONT_EXPORT
  ArrayHandleCompositeVector() : Superclass() {  }

  VTKM_CONT_EXPORT
  ArrayHandleCompositeVector(
      const vtkm::internal::FunctionInterface<Signature> &arrays,
      const ComponentMapType &sourceComponents)
    : Superclass(ArrayContainerControlType(arrays, sourceComponents))
  {  }

  /// Template constructors for passing in types. You'll get weird compile
  /// errors if the argument types do not actually match the types in the
  /// signature.
  ///
  template<typename ArrayHandleType1>
  VTKM_CONT_EXPORT
  ArrayHandleCompositeVector(const ArrayHandleType1 &array1,
                             int sourceComponent1)
    : Superclass(ArrayContainerControlType(
                   vtkm::internal::make_FunctionInterface<ValueType>(array1),
                   ComponentMapType(sourceComponent1)))
  {  }
  template<typename ArrayHandleType1,
           typename ArrayHandleType2>
  VTKM_CONT_EXPORT
  ArrayHandleCompositeVector(const ArrayHandleType1 &array1,
                             int sourceComponent1,
                             const ArrayHandleType2 &array2,
                             int sourceComponent2)
    : Superclass(ArrayContainerControlType(
                   vtkm::internal::make_FunctionInterface<ValueType>(
                     array1, array2),
                   ComponentMapType(sourceComponent1,
                                    sourceComponent2)))
  {  }
  template<typename ArrayHandleType1,
           typename ArrayHandleType2,
           typename ArrayHandleType3>
  VTKM_CONT_EXPORT
  ArrayHandleCompositeVector(const ArrayHandleType1 &array1,
                             int sourceComponent1,
                             const ArrayHandleType2 &array2,
                             int sourceComponent2,
                             const ArrayHandleType3 &array3,
                             int sourceComponent3)
    : Superclass(ArrayContainerControlType(
                   vtkm::internal::make_FunctionInterface<ValueType>(
                     array1, array2, array3),
                   ComponentMapType(sourceComponent1,
                                    sourceComponent2,
                                    sourceComponent3)))
  {  }
  template<typename ArrayHandleType1,
           typename ArrayHandleType2,
           typename ArrayHandleType3,
           typename ArrayHandleType4>
  VTKM_CONT_EXPORT
  ArrayHandleCompositeVector(const ArrayHandleType1 &array1,
                             int sourceComponent1,
                             const ArrayHandleType2 &array2,
                             int sourceComponent2,
                             const ArrayHandleType3 &array3,
                             int sourceComponent3,
                             const ArrayHandleType4 &array4,
                             int sourceComponent4)
    : Superclass(ArrayContainerControlType(
                   vtkm::internal::make_FunctionInterface<ValueType>(
                     array1, array2, array3, array4),
                   ComponentMapType(sourceComponent1,
                                    sourceComponent2,
                                    sourceComponent3,
                                    sourceComponent4)))
  {  }
};

/// \brief Get the type for an ArrayHandleCompositeVector
///
/// The ArrayHandleCompositeVector has a difficult template specification.
/// Use this helper template to covert a list of array handle types to a
/// composite vector of these array handles. Here is a simple example.
///
/// \code{.cpp}
/// typedef vtkm::cont::ArrayHandleCompositeVector<
///     vtkm::cont::ArrayHandle<vtkm::Scalar>,
///     vtkm::cont::ArrayHandle<vtkm::Scalar> >::type OutArrayType;
/// OutArrayType outArray = vtkm::cont::make_ArrayHandleCompositeVector(a1,a2);
/// \endcode
///
template<typename ArrayHandleType1,
         typename ArrayHandleType2 = void,
         typename ArrayHandleType3 = void,
         typename ArrayHandleType4 = void>
struct ArrayHandleCompositeVectorType
{
private:
  typedef typename vtkm::VectorTraits<typename ArrayHandleType1::ValueType>::ComponentType
      ComponentType;
  typedef vtkm::Tuple<ComponentType,4> Signature(
      ArrayHandleType1,ArrayHandleType2,ArrayHandleType3,ArrayHandleType4);
public:
  typedef vtkm::cont::ArrayHandleCompositeVector<Signature> type;
};

template<typename ArrayHandleType1,
         typename ArrayHandleType2,
         typename ArrayHandleType3>
struct ArrayHandleCompositeVectorType<
    ArrayHandleType1,ArrayHandleType2,ArrayHandleType3>
{
private:
  typedef typename vtkm::VectorTraits<typename ArrayHandleType1::ValueType>::ComponentType
      ComponentType;
  typedef vtkm::Tuple<ComponentType,3> Signature(
      ArrayHandleType1,ArrayHandleType2,ArrayHandleType3);
public:
  typedef vtkm::cont::ArrayHandleCompositeVector<Signature> type;
};

template<typename ArrayHandleType1,
         typename ArrayHandleType2>
struct ArrayHandleCompositeVectorType<ArrayHandleType1,ArrayHandleType2>
{
private:
  typedef typename vtkm::VectorTraits<typename ArrayHandleType1::ValueType>::ComponentType
      ComponentType;
  typedef vtkm::Tuple<ComponentType,2> Signature(
      ArrayHandleType1,ArrayHandleType2);
public:
  typedef vtkm::cont::ArrayHandleCompositeVector<Signature> type;
};

template<typename ArrayHandleType1>
struct ArrayHandleCompositeVectorType<ArrayHandleType1>
{
private:
  typedef typename vtkm::VectorTraits<typename ArrayHandleType1::ValueType>::ComponentType
      ComponentType;
  typedef ComponentType Signature(ArrayHandleType1);
public:
  typedef vtkm::cont::ArrayHandleCompositeVector<Signature> type;
};

/// Create a composite vector array from other arrays.
///
template<typename ValueType1, typename Container1>
VTKM_CONT_EXPORT
typename ArrayHandleCompositeVectorType<
  vtkm::cont::ArrayHandle<ValueType1,Container1> >::type
make_ArrayHandleCompositeVector(
    const vtkm::cont::ArrayHandle<ValueType1,Container1> &array1,
    int sourceComponent1)
{
  return typename ArrayHandleCompositeVectorType<
      vtkm::cont::ArrayHandle<ValueType1,Container1> >::type(array1,
                                                             sourceComponent1);
}
template<typename ValueType1, typename Container1,
         typename ValueType2, typename Container2>
VTKM_CONT_EXPORT
typename ArrayHandleCompositeVectorType<
  vtkm::cont::ArrayHandle<ValueType1,Container1>,
  vtkm::cont::ArrayHandle<ValueType2,Container2> >::type
make_ArrayHandleCompositeVector(
    const vtkm::cont::ArrayHandle<ValueType1,Container1> &array1,
    int sourceComponent1,
    const vtkm::cont::ArrayHandle<ValueType2,Container2> &array2,
    int sourceComponent2)
{
  return typename ArrayHandleCompositeVectorType<
      vtkm::cont::ArrayHandle<ValueType1,Container1>,
      vtkm::cont::ArrayHandle<ValueType2,Container2> >::type(array1,
                                                             sourceComponent1,
                                                             array2,
                                                             sourceComponent2);
}
template<typename ValueType1, typename Container1,
         typename ValueType2, typename Container2,
         typename ValueType3, typename Container3>
VTKM_CONT_EXPORT
typename ArrayHandleCompositeVectorType<
  vtkm::cont::ArrayHandle<ValueType1,Container1>,
  vtkm::cont::ArrayHandle<ValueType2,Container2>,
  vtkm::cont::ArrayHandle<ValueType3,Container3> >::type
make_ArrayHandleCompositeVector(
    const vtkm::cont::ArrayHandle<ValueType1,Container1> &array1,
    int sourceComponent1,
    const vtkm::cont::ArrayHandle<ValueType2,Container2> &array2,
    int sourceComponent2,
    const vtkm::cont::ArrayHandle<ValueType3,Container3> &array3,
    int sourceComponent3)
{
  return typename ArrayHandleCompositeVectorType<
      vtkm::cont::ArrayHandle<ValueType1,Container1>,
      vtkm::cont::ArrayHandle<ValueType2,Container2>,
      vtkm::cont::ArrayHandle<ValueType3,Container3> >::type(array1,
                                                             sourceComponent1,
                                                             array2,
                                                             sourceComponent2,
                                                             array3,
                                                             sourceComponent3);
}
template<typename ValueType1, typename Container1,
         typename ValueType2, typename Container2,
         typename ValueType3, typename Container3,
         typename ValueType4, typename Container4>
VTKM_CONT_EXPORT
typename ArrayHandleCompositeVectorType<
  vtkm::cont::ArrayHandle<ValueType1,Container1>,
  vtkm::cont::ArrayHandle<ValueType2,Container2>,
  vtkm::cont::ArrayHandle<ValueType3,Container3>,
  vtkm::cont::ArrayHandle<ValueType4,Container4> >::type
make_ArrayHandleCompositeVector(
    const vtkm::cont::ArrayHandle<ValueType1,Container1> &array1,
    int sourceComponent1,
    const vtkm::cont::ArrayHandle<ValueType2,Container2> &array2,
    int sourceComponent2,
    const vtkm::cont::ArrayHandle<ValueType3,Container3> &array3,
    int sourceComponent3,
    const vtkm::cont::ArrayHandle<ValueType4,Container4> &array4,
    int sourceComponent4)
{
  return typename ArrayHandleCompositeVectorType<
      vtkm::cont::ArrayHandle<ValueType1,Container1>,
      vtkm::cont::ArrayHandle<ValueType2,Container2>,
      vtkm::cont::ArrayHandle<ValueType3,Container3>,
      vtkm::cont::ArrayHandle<ValueType4,Container4> >::type(array1,
                                                             sourceComponent1,
                                                             array2,
                                                             sourceComponent2,
                                                             array3,
                                                             sourceComponent3,
                                                             array4,
                                                             sourceComponent4);
}

}
} // namespace vtkm::cont

#endif //vtk_m_ArrayHandleCompositeVector_h