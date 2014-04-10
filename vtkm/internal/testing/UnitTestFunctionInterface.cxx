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

#include <vtkm/internal/FunctionInterface.h>

#include <vtkm/testing/Testing.h>

#include <sstream>
#include <string>

#ifndef _WIN32
#include <limits.h>
#include <sys/time.h>
#include <unistd.h>
#endif

namespace {

// TODO: Once device adapters are implemented and contain timers, this class
// should be removed and replaced with that. Also remove the inclusion of
// limits.h, sys/time.h, and unistd.h.
class Timer
{
public:
  VTKM_CONT_EXPORT Timer()
  {
    this->Reset();
  }

  VTKM_CONT_EXPORT void Reset()
  {
    this->StartTime = this->GetCurrentTime();
  }

  VTKM_CONT_EXPORT vtkm::Scalar GetElapsedTime()
  {
    TimeStamp currentTime = this->GetCurrentTime();

    vtkm::Scalar elapsedTime;
    elapsedTime = currentTime.Seconds - this->StartTime.Seconds;
    elapsedTime += ((currentTime.Microseconds - this->StartTime.Microseconds)
                    /vtkm::Scalar(1000000));

    return elapsedTime;
  }

private:
  struct TimeStamp {
    vtkm::internal::Int64Type Seconds;
    vtkm::internal::Int64Type Microseconds;
  };
  TimeStamp StartTime;

  VTKM_CONT_EXPORT
  TimeStamp GetCurrentTime()
  {
    TimeStamp retval;
#ifdef _WIN32
    timeb currentTime;
    ::ftime(&currentTime);
    retval.Seconds = currentTime.time;
    retval.Microseconds = 1000*currentTime.millitm;
#else
    timeval currentTime;
    gettimeofday(&currentTime, NULL);
    retval.Seconds = currentTime.tv_sec;
    retval.Microseconds = currentTime.tv_usec;
#endif
    return retval;
  }
};

typedef vtkm::Id Type1;
const Type1 Arg1 = 1234;

typedef vtkm::Scalar Type2;
const Type2 Arg2 = 5678.125;

typedef std::string Type3;
const Type3 Arg3("Third argument");

typedef vtkm::Vector3 Type4;
const Type4 Arg4(1.2, 3.4, 5.6);

typedef vtkm::Id3 Type5;
const Type5 Arg5(4, 5, 6);

struct ThreeArgFunctor {
  void operator()(const Type1 &a1, const Type2 &a2, const Type3 &a3) const
  {
    std::cout << "In 3 arg functor." << std::endl;

    VTKM_TEST_ASSERT(a1 == Arg1, "Arg 1 incorrect.");
    VTKM_TEST_ASSERT(a2 == Arg2, "Arg 2 incorrect.");
    VTKM_TEST_ASSERT(a3 == Arg3, "Arg 3 incorrect.");
  }
};

struct ThreeArgModifyFunctor {
  void operator()(Type1 &a1, Type2 &a2, Type3 &a3) const
  {
    std::cout << "In 3 arg modify functor." << std::endl;

    a1 = Arg1;
    a2 = Arg2;
    a3 = Arg3;
  }
};

struct GetReferenceFunctor
{
  template<typename T>
  struct ReturnType {
    typedef const typename boost::remove_reference<T>::type *type;
  };

  template<typename T>
  const T *operator()(const T &x) const { return &x; }
};

struct ThreePointerArgFunctor {
  void operator()(const Type1 *a1, const Type2 *a2, const Type3 *a3) const
  {
    std::cout << "In 3 arg functor." << std::endl;

    VTKM_TEST_ASSERT(*a1 == Arg1, "Arg 1 incorrect.");
    VTKM_TEST_ASSERT(*a2 == Arg2, "Arg 2 incorrect.");
    VTKM_TEST_ASSERT(*a3 == Arg3, "Arg 3 incorrect.");
  }
};

struct ThreeArgFunctorWithReturn {
  std::string operator()(const Type1 &a1,
                         const Type2 &a2,
                         const Type3 &a3) const
  {
    std::cout << "In 3 arg functor with return." << std::endl;

    std::stringstream buffer;
    buffer.precision(10);
    buffer << a1 << " " << a2 << " " << a3;
    return buffer.str();
  }
};

struct FiveArgFunctor {
  void operator()(const Type1 &a1,
                  const Type2 &a2,
                  const Type3 &a3,
                  const Type4 &a4,
                  const Type5 &a5) const
  {
    std::cout << "In 5 arg functor." << std::endl;

    VTKM_TEST_ASSERT(a1 == Arg1, "Arg 1 incorrect.");
    VTKM_TEST_ASSERT(a2 == Arg2, "Arg 2 incorrect.");
    VTKM_TEST_ASSERT(a3 == Arg3, "Arg 3 incorrect.");
    VTKM_TEST_ASSERT(a4 == Arg4, "Arg 4 incorrect.");
    VTKM_TEST_ASSERT(a5 == Arg5, "Arg 5 incorrect.");
  }
};

struct FiveArgSwizzledFunctor {
  void operator()(const Type5 &a5,
                  const Type1 &a1,
                  const Type3 &a3,
                  const Type4 &a4,
                  const Type2 &a2) const
  {
    std::cout << "In 5 arg functor." << std::endl;

    VTKM_TEST_ASSERT(a1 == Arg1, "Arg 1 incorrect.");
    VTKM_TEST_ASSERT(a2 == Arg2, "Arg 2 incorrect.");
    VTKM_TEST_ASSERT(a3 == Arg3, "Arg 3 incorrect.");
    VTKM_TEST_ASSERT(a4 == Arg4, "Arg 4 incorrect.");
    VTKM_TEST_ASSERT(a5 == Arg5, "Arg 5 incorrect.");
  }
};

struct LotsOfArgsFunctor {
  LotsOfArgsFunctor() : Field(0) {  }

  void operator()(vtkm::Scalar arg1,
                  vtkm::Scalar arg2,
                  vtkm::Scalar arg3,
                  vtkm::Scalar arg4,
                  vtkm::Scalar arg5,
                  vtkm::Scalar arg6,
                  vtkm::Scalar arg7,
                  vtkm::Scalar arg8,
                  vtkm::Scalar arg9,
                  vtkm::Scalar arg10) {
    VTKM_TEST_ASSERT(arg1 == 1.0, "Got bad argument");
    VTKM_TEST_ASSERT(arg2 == 2.0, "Got bad argument");
    VTKM_TEST_ASSERT(arg3 == 3.0, "Got bad argument");
    VTKM_TEST_ASSERT(arg4 == 4.0, "Got bad argument");
    VTKM_TEST_ASSERT(arg5 == 5.0, "Got bad argument");
    VTKM_TEST_ASSERT(arg6 == 6.0, "Got bad argument");
    VTKM_TEST_ASSERT(arg7 == 7.0, "Got bad argument");
    VTKM_TEST_ASSERT(arg8 == 8.0, "Got bad argument");
    VTKM_TEST_ASSERT(arg9 == 9.0, "Got bad argument");
    VTKM_TEST_ASSERT(arg10 == 10.0, "Got bad argument");

    this->Field +=
      arg1 + arg2 + arg3 + arg4 + arg5 + arg6 + arg7 + arg8 + arg9 + arg10;
  }
  vtkm::Scalar Field;
};

template<typename T>
std::string ToString(const T &value)
{
  std::stringstream stream;
  stream.precision(10);
  stream << value;
  return stream.str();
}
std::string ToString(const std::string &value)
{
  return value;
}

struct StringTransform
{
  template<typename T>
  std::string operator()(const T &input) const { return ToString(input); }
};

struct ThreeArgStringFunctorWithReturn
{
  std::string operator()(std::string arg1,
                         std::string arg2,
                         std::string arg3) const
  {
    return arg1 + " " + arg2 + " " + arg3;
  }
};

struct DynamicTransformFunctor
{
  template<typename T, typename ContinueFunctor>
  void operator()(const T &input, const ContinueFunctor continueFunc) const
  {
    continueFunc(input);
    continueFunc(ToString(input));
  }

  template<typename ContinueFunctor>
  void operator()(const std::string &input, const ContinueFunctor continueFunc) const
  {
    continueFunc(input);
  }
};

vtkm::Id g_DynamicTransformFinishCalls;

struct DynamicTransformFinish
{
  template<typename Signature>
  void operator()(vtkm::internal::FunctionInterface<Signature> &funcInterface) const
  {
    g_DynamicTransformFinishCalls++;
    VTKM_TEST_ASSERT(ToString(funcInterface.template GetParameter<1>()) == ToString(Arg1),
                     "Arg 1 incorrect");
    VTKM_TEST_ASSERT(ToString(funcInterface.template GetParameter<2>()) == ToString(Arg2),
                     "Arg 2 incorrect");
    VTKM_TEST_ASSERT(ToString(funcInterface.template GetParameter<3>()) == ToString(Arg3),
                     "Arg 3 incorrect");
  }
};

void TryFunctionInterface5(
    vtkm::internal::FunctionInterface<void(Type1,Type2,Type3,Type4,Type5)> funcInterface)
{
  std::cout << "Checking 5 parameter function interface." << std::endl;
  VTKM_TEST_ASSERT(funcInterface.GetArity() == 5,
                  "Got wrong number of parameters.");
  VTKM_TEST_ASSERT(funcInterface.GetParameter<1>() == Arg1, "Arg 1 incorrect.");
  VTKM_TEST_ASSERT(funcInterface.GetParameter<2>() == Arg2, "Arg 2 incorrect.");
  VTKM_TEST_ASSERT(funcInterface.GetParameter<3>() == Arg3, "Arg 3 incorrect.");
  VTKM_TEST_ASSERT(funcInterface.GetParameter<4>() == Arg4, "Arg 4 incorrect.");
  VTKM_TEST_ASSERT(funcInterface.GetParameter<5>() == Arg5, "Arg 5 incorrect.");

  std::cout << "Checking invocation." << std::endl;
  funcInterface.InvokeCont(FiveArgFunctor());
  funcInterface.InvokeExec(FiveArgFunctor());

  std::cout << "Swizzling parameters with replace." << std::endl;
  funcInterface.Replace<1>(Arg5)
      .Replace<2>(Arg1)
      .Replace<5>(Arg2)
      .InvokeCont(FiveArgSwizzledFunctor());
}

void TestBasicFunctionInterface()
{
  std::cout << "Creating basic function interface." << std::endl;
  vtkm::internal::FunctionInterface<void(Type1,Type2,Type3)> funcInterface =
      vtkm::internal::make_FunctionInterface<void>(Arg1, Arg2, Arg3);

  std::cout << "Checking parameters." << std::endl;
  VTKM_TEST_ASSERT(funcInterface.GetArity() == 3,
                   "Got wrong number of parameters.");
  VTKM_TEST_ASSERT(funcInterface.GetParameter<1>() == Arg1, "Arg 1 incorrect.");
  VTKM_TEST_ASSERT(funcInterface.GetParameter<2>() == Arg2, "Arg 2 incorrect.");
  VTKM_TEST_ASSERT(funcInterface.GetParameter<3>() == Arg3, "Arg 3 incorrect.");

  std::cout << "Checking invocation." << std::endl;
  funcInterface.InvokeCont(ThreeArgFunctor());
  funcInterface.InvokeExec(ThreeArgFunctor());

  std::cout << "Checking invocation with argument modification." << std::endl;
  funcInterface.SetParameter<1>(Type1());
  funcInterface.SetParameter<2>(Type2());
  funcInterface.SetParameter<3>(Type3());
  VTKM_TEST_ASSERT(funcInterface.GetParameter<1>() != Arg1, "Arg 1 not cleared.");
  VTKM_TEST_ASSERT(funcInterface.GetParameter<2>() != Arg2, "Arg 2 not cleared.");
  VTKM_TEST_ASSERT(funcInterface.GetParameter<3>() != Arg3, "Arg 3 not cleared.");

  funcInterface.InvokeCont(ThreeArgModifyFunctor());
  VTKM_TEST_ASSERT(funcInterface.GetParameter<1>() == Arg1, "Arg 1 incorrect.");
  VTKM_TEST_ASSERT(funcInterface.GetParameter<2>() == Arg2, "Arg 2 incorrect.");
  VTKM_TEST_ASSERT(funcInterface.GetParameter<3>() == Arg3, "Arg 3 incorrect.");

  TryFunctionInterface5(
        vtkm::internal::make_FunctionInterface<void>(Arg1,Arg2,Arg3,Arg4,Arg5));
}

void TestInvokeResult()
{
  std::cout << "Checking invocation with return." << std::endl;
  vtkm::internal::FunctionInterface<std::string(Type1,Type2,Type3)> funcInterface
      = vtkm::internal::make_FunctionInterface<std::string>(Arg1, Arg2, Arg3);

  funcInterface.InvokeCont(ThreeArgFunctorWithReturn());
  std::string result = funcInterface.GetReturnValue();

  std::cout << "Got result: " << result << std::endl;
  VTKM_TEST_ASSERT(result == "1234 5678.125 Third argument",
                   "Got bad result from invoke.");
}

void TestAppend()
{
  std::cout << "Appending interface with return value." << std::endl;
  vtkm::internal::FunctionInterface<std::string(Type1,Type2)>
      funcInterface2ArgWRet =
        vtkm::internal::make_FunctionInterface<std::string>(Arg1,Arg2);

  vtkm::internal::FunctionInterface<std::string(Type1,Type2,Type3)>
      funcInterface3ArgWRet = funcInterface2ArgWRet.Append(Arg3);
  VTKM_TEST_ASSERT(funcInterface3ArgWRet.GetParameter<1>() == Arg1, "Arg 1 incorrect.");
  VTKM_TEST_ASSERT(funcInterface3ArgWRet.GetParameter<2>() == Arg2, "Arg 2 incorrect.");
  VTKM_TEST_ASSERT(funcInterface3ArgWRet.GetParameter<3>() == Arg3, "Arg 3 incorrect.");

  std::cout << "Invoking appended function interface." << std::endl;
  funcInterface3ArgWRet.InvokeExec(ThreeArgFunctorWithReturn());
  std::string result = funcInterface3ArgWRet.GetReturnValue();

  std::cout << "Got result: " << result << std::endl;
  VTKM_TEST_ASSERT(result == "1234 5678.125 Third argument",
                   "Got bad result from invoke.");

  std::cout << "Appending another value." << std::endl;
  vtkm::internal::FunctionInterface<std::string(Type1,Type2,Type3,Type4)>
      funcInterface4ArgWRet = funcInterface3ArgWRet.Append(Arg4);
  VTKM_TEST_ASSERT(funcInterface4ArgWRet.GetParameter<4>() == Arg4, "Arg 4 incorrect.");
  VTKM_TEST_ASSERT(funcInterface4ArgWRet.GetReturnValue() == "1234 5678.125 Third argument",
                   "Got bad result from copy.");

  std::cout << "Checking double append." << std::endl;
  vtkm::internal::FunctionInterface<void(Type1,Type2,Type3)> funcInterface3 =
      vtkm::internal::make_FunctionInterface<void>(Arg1,Arg2,Arg3);
  TryFunctionInterface5(funcInterface3
                        .Append(Arg4)
                        .Append(Arg5));
}

void TestTransformInvoke()
{
  std::cout << "Trying transform invoke." << std::endl;
  vtkm::internal::FunctionInterface<std::string(Type1,Type2,Type3)>
      funcInterface =
        vtkm::internal::make_FunctionInterface<std::string>(Arg1, Arg2, Arg3);

  funcInterface.InvokeCont(ThreeArgStringFunctorWithReturn(),
                           StringTransform());
  std::string result = funcInterface.GetReturnValue();

  std::cout << "Got result: " << result << std::endl;
  VTKM_TEST_ASSERT(result == "1234 5678.125 Third argument",
                   "Got bad result from invoke.");
}

void TestDynamicTransform()
{
  std::cout << "Trying dynamic transform." << std::endl;
  vtkm::internal::FunctionInterface<void(Type1,Type2,Type3)> funcInterface =
      vtkm::internal::make_FunctionInterface<void>(Arg1, Arg2, Arg3);

  g_DynamicTransformFinishCalls = 0;

  funcInterface.DynamicTransformCont(DynamicTransformFunctor(),
                                     DynamicTransformFinish());

  // We use an idiosyncrasy of DynamicTransform to call finish with two
  // permutations for every non string argument and one permutation for every
  // string argument. Thus, we expect it to be called 4 times.
  std::cout << "Number of finish calls: " << g_DynamicTransformFinishCalls
            << std::endl;
  VTKM_TEST_ASSERT(g_DynamicTransformFinishCalls == 4,
                   "DynamicTransform did not call finish the right number of times.");
}

void TestInvokeTime()
{
  std::cout << "Checking time to call lots of args lots of times." << std::endl;
  static vtkm::Id NUM_TRIALS = 50000;
  LotsOfArgsFunctor f;

  Timer timer;
  for (vtkm::Id trial = 0; trial < NUM_TRIALS; trial++)
  {
    f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f);
  }
  vtkm::Scalar directCallTime = timer.GetElapsedTime();
  std::cout << "Time for direct call: " << directCallTime << " seconds"
            << std::endl;

  vtkm::internal::FunctionInterface<void(vtkm::Scalar,
                                         vtkm::Scalar,
                                         vtkm::Scalar,
                                         vtkm::Scalar,
                                         vtkm::Scalar,
                                         vtkm::Scalar,
                                         vtkm::Scalar,
                                         vtkm::Scalar,
                                         vtkm::Scalar,
                                         vtkm::Scalar)> funcInterface =
      vtkm::internal::make_FunctionInterface<void>(vtkm::Scalar(1),
                                                   vtkm::Scalar(2),
                                                   vtkm::Scalar(3),
                                                   vtkm::Scalar(4),
                                                   vtkm::Scalar(5),
                                                   vtkm::Scalar(6),
                                                   vtkm::Scalar(7),
                                                   vtkm::Scalar(8),
                                                   vtkm::Scalar(9),
                                                   vtkm::Scalar(10));

  timer.Reset();
  for (vtkm::Id trial = 0; trial < NUM_TRIALS; trial++)
  {
    funcInterface.InvokeCont(f);
  }
  vtkm::Scalar invokeCallTime = timer.GetElapsedTime();
  std::cout << "Time for invoking function interface: " << invokeCallTime
            << " seconds" << std::endl;
  std::cout << "Pointless result (makeing sure compiler computes it) "
            << f.Field << std::endl;

  // Might need to disable this for non-release builds.
  VTKM_TEST_ASSERT(invokeCallTime < 1.05*directCallTime,
                   "Function interface invoke took longer than expected.");
}

void TestFunctionInterface()
{
  TestBasicFunctionInterface();
  TestInvokeResult();
  TestAppend();
  TestTransformInvoke();
  TestDynamicTransform();
  TestInvokeTime();
}

} // anonymous namespace

int UnitTestFunctionInterface(int, char *[])
{
  return vtkm::testing::Testing::Run(TestFunctionInterface);
}
