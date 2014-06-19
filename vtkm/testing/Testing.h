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
#ifndef vtk_m_testing_Testing_h
#define vtk_m_testing_Testing_h

#include <vtkm/Types.h>
#include <vtkm/TypeTraits.h>
#include <vtkm/VectorTraits.h>

#include <iostream>
#include <sstream>
#include <string>

#include <math.h>

// Try to enforce using the correct testing version. (Those that include the
// control environment have more possible exceptions.) This is not guaranteed
// to work. To make it more likely, place the Testing.h include last.
#ifdef vtk_m_cont_Error_h
#ifndef vtk_m_cont_testing_Testing_h
#error Use vtkm::cont::testing::Testing instead of vtkm::testing::Testing.
#else
#define VTKM_TESTING_IN_CONT
#endif
#endif

/// \def VTKM_TEST_ASSERT(condition, message)
///
/// Asserts a condition for a test to pass. A passing condition is when \a
/// condition resolves to true. If \a condition is false, then the test is
/// aborted and failure is returned.

#define VTKM_TEST_ASSERT(condition, message) \
  ::vtkm::testing::Testing::Assert( \
      condition, __FILE__, __LINE__, message, #condition)

/// \def VTKM_TEST_FAIL(message)
///
/// Causes a test to fail with the given \a message.

#define VTKM_TEST_FAIL(message) \
  throw ::vtkm::testing::Testing::TestFailure(__FILE__, __LINE__, message)

namespace vtkm {
namespace testing {

struct Testing
{
public:
  class TestFailure
  {
  public:
    VTKM_CONT_EXPORT TestFailure(const std::string &file,
                                 vtkm::Id line,
                                 const std::string &message)
      : File(file), Line(line), Message(message) { }

    VTKM_CONT_EXPORT TestFailure(const std::string &file,
                                 vtkm::Id line,
                                 const std::string &message,
                                 const std::string &condition)
      : File(file), Line(line)
    {
      this->Message.append(message);
      this->Message.append(" (");
      this->Message.append(condition);
      this->Message.append(")");
    }

    VTKM_CONT_EXPORT const std::string &GetFile() const { return this->File; }
    VTKM_CONT_EXPORT vtkm::Id GetLine() const { return this->Line; }
    VTKM_CONT_EXPORT const std::string &GetMessage() const
    {
      return this->Message;
    }
  private:
    std::string File;
    vtkm::Id Line;
    std::string Message;
  };

  static VTKM_CONT_EXPORT void Assert(bool condition,
                                      const std::string &file,
                                      vtkm::Id line,
                                      const std::string &message,
                                      const std::string &conditionString)
  {
    if (condition)
    {
      // Do nothing.
    }
    else
    {
      throw TestFailure(file, line, message, conditionString);
    }
  }

#ifndef VTKM_TESTING_IN_CONT
  /// Calls the test function \a function with no arguments. Catches any errors
  /// generated by VTKM_TEST_ASSERT or VTKM_TEST_FAIL, reports the error, and
  /// returns "1" (a failure status for a program's main). Returns "0" (a
  /// success status for a program's main).
  ///
  /// The intention is to implement a test's main function with this. For
  /// example, the implementation of UnitTestFoo might look something like
  /// this.
  ///
  /// \code
  /// #include <vtkm/testing/Testing.h>
  ///
  /// namespace {
  ///
  /// void TestFoo()
  /// {
  ///    // Do actual test, which checks in VTKM_TEST_ASSERT or VTKM_TEST_FAIL.
  /// }
  ///
  /// } // anonymous namespace
  ///
  /// int UnitTestFoo(int, char *[])
  /// {
  ///   return vtkm::testing::Testing::Run(TestFoo);
  /// }
  /// \endcode
  ///
  template<class Func>
  static VTKM_CONT_EXPORT int Run(Func function)
  {
    try
    {
      function();
    }
    catch (TestFailure error)
    {
      std::cout << "***** Test failed @ "
                << error.GetFile() << ":" << error.GetLine() << std::endl
                << error.GetMessage() << std::endl;
      return 1;
    }
    catch (...)
    {
      std::cout << "***** Unidentified exception thrown." << std::endl;
      return 1;
    }
    return 0;
  }
#endif

  /// Check functors to be used with the TryAllTypes method.
  ///
  struct TypeCheckAlwaysTrue
  {
    template <typename T, class Functor>
    void operator()(T t, Functor function) const { function(t); }
  };
  struct TypeCheckInteger
  {
    template <typename T, class Functor>
    void operator()(T t, Functor function) const
    {
      this->DoInteger(typename vtkm::TypeTraits<T>::NumericTag(), t, function);
    }
  private:
    template <class Tag, typename T, class Functor>
    void DoInteger(Tag, T, const Functor&) const {  }
    template <typename T, class Functor>
    void DoInteger(vtkm::TypeTraitsIntegerTag, T t, Functor function) const
    {
      function(t);
    }
  };
  struct TypeCheckReal
  {
    template <typename T, class Functor>
    void operator()(T t, Functor function) const
    {
      this->DoReal(typename vtkm::TypeTraits<T>::NumericTag(), t, function);
    }
  private:
    template <class Tag, typename T, class Functor>
    void DoReal(Tag, T, const Functor&) const {  }
    template <typename T, class Functor>
    void DoReal(vtkm::TypeTraitsRealTag, T t, Functor function) const
    {
      function(t);
    }
  };
  struct TypeCheckScalar
  {
    template <typename T, class Functor>
    void operator()(T t, Functor func) const
    {
      this->DoScalar(typename vtkm::TypeTraits<T>::DimensionalityTag(), t, func);
    }
  private:
    template <class Tag, typename T, class Functor>
    void DoScalar(Tag, const T &, const Functor &) const {  }
    template <typename T, class Functor>
    void DoScalar(vtkm::TypeTraitsScalarTag, T t, Functor function) const
    {
      function(t);
    }
  };
  struct TypeCheckVector
  {
    template <typename T, class Functor>
    void operator()(T t, Functor func) const
    {
      this->DoVector(typename vtkm::TypeTraits<T>::DimensionalityTag(), t, func);
    }
  private:
    template <class Tag, typename T, class Functor>
    void DoVector(Tag, const T &, const Functor &) const {  }
    template <typename T, class Functor>
    void DoVector(vtkm::TypeTraitsVectorTag, T t, Functor function) const
    {
      function(t);
    }
  };

  template<class FunctionType>
  struct InternalPrintOnInvoke
  {
    InternalPrintOnInvoke(FunctionType function, std::string toprint)
      : Function(function), ToPrint(toprint) { }
    template <typename T> void operator()(T t)
    {
      std::cout << this->ToPrint << std::endl;
      this->Function(t);
    }
  private:
    FunctionType Function;
    std::string ToPrint;
  };

  /// Runs templated \p function on all the basic types defined in VTKm. This is
  /// helpful to test templated functions that should work on all types. If the
  /// function is supposed to work on some subset of types, then \p check can
  /// be set to restrict the types used. This Testing class contains several
  /// helpful check functors.
  ///
  template<class FunctionType, class CheckType>
  static void TryAllTypes(FunctionType function, CheckType check)
  {
    vtkm::Id id = 0;
    check(id, InternalPrintOnInvoke<FunctionType>(
            function, "*** vtkm::Id ***************"));

    vtkm::Id3 id3 = vtkm::make_Id3(0, 0, 0);
    check(id3, InternalPrintOnInvoke<FunctionType>(
            function, "*** vtkm::Id3 **************"));

    vtkm::Scalar scalar = 0.0;
    check(scalar, InternalPrintOnInvoke<FunctionType>(
            function, "*** vtkm::Scalar ***********"));

    vtkm::Vector2 vector2 = vtkm::make_Vector2(0.0, 0.0);
    check(vector2, InternalPrintOnInvoke<FunctionType>(
            function, "*** vtkm::Vector2 **********"));

    vtkm::Vector3 vector3 = vtkm::make_Vector3(0.0, 0.0, 0.0);
    check(vector3, InternalPrintOnInvoke<FunctionType>(
            function, "*** vtkm::Vector3 **********"));

    vtkm::Vector4 vector4 = vtkm::make_Vector4(0.0, 0.0, 0.0, 0.0);
    check(vector4, InternalPrintOnInvoke<FunctionType>(
            function, "*** vtkm::Vector4 **********"));
  }
  template<class FunctionType>
  static void TryAllTypes(FunctionType function)
  {
    TryAllTypes(function, TypeCheckAlwaysTrue());
  }

};

}
} // namespace vtkm::internal

/// Helper function to test two quanitites for equality accounting for slight
/// variance due to floating point numerical inaccuracies.
///
template<typename VectorType>
VTKM_EXEC_CONT_EXPORT bool test_equal(VectorType vector1,
                                      VectorType vector2,
                                      vtkm::Scalar tolerance = 0.0001)
{
  typedef typename vtkm::VectorTraits<VectorType> Traits;
  for (int component = 0; component < Traits::NUM_COMPONENTS; component++)
  {
    vtkm::Scalar value1 = vtkm::Scalar(Traits::GetComponent(vector1, component));
    vtkm::Scalar value2 = vtkm::Scalar(Traits::GetComponent(vector2, component));
    if ((fabs(value1) < 2*tolerance) && (fabs(value2) < 2*tolerance))
    {
      continue;
    }
    vtkm::Scalar ratio = value1/value2;
    if ((ratio > vtkm::Scalar(1.0) - tolerance)
        && (ratio < vtkm::Scalar(1.0) + tolerance))
    {
      // This component is OK. The condition is checked in this way to
      // correctly handle non-finites that fail all comparisons. Thus, if a
      // non-finite is encountered, this condition will fail and false will be
      // returned.
    }
    else
    {
      return false;
    }
  }
  return true;
}

/// Helper function for printing out vectors during testing.
///
template<typename T, int Size>
VTKM_EXEC_CONT_EXPORT
std::ostream &operator<<(std::ostream &stream, const vtkm::Tuple<T,Size> &tuple)
{
  stream << "[";
  for (int component = 0; component < Size-1; component++)
  {
    stream << tuple[component] << ",";
  }
  return stream << tuple[Size-1] << "]";
}

#endif //vtk_m_testing_Testing_h