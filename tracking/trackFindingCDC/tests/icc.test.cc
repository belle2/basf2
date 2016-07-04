/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/*
This file tests whether the C++ compiler of Intel implements C++.
*/

#include <gtest/gtest.h>

namespace {

  /// Simple class containing a single member.
  template<class T>
  class Wrapper {

  public:
    /// Simple constructor wrapping a copy of the object.
    explicit Wrapper(T t) : m_t(t)
    {}

  public:
    /**
     *  Allow unpacking by conversion operator.
     *  Uncommenting explicit leads to a compilation error in icpc.
     */
    /*explicit*/ operator T& ()
    { return m_t; }

  private:
    /// Memory for the contained instance.
    T m_t;
  };

  TEST(IntelCompiler, static_cast_calls_conversion_operator_template)
  {
    Wrapper<int> wrappedInt(6);

    int& i1(wrappedInt.operator int& ());
    int& i2(static_cast<int&>(wrappedInt));
    int& i3(wrappedInt);

    EXPECT_EQ(6, i1);
    EXPECT_EQ(6, i2);
    EXPECT_EQ(6, i3);
  }


  /// Simple class containing a single member.
  class IntWrapper {

  public:
    /// Simple constructor wrapping a copy of the object.
    explicit IntWrapper(int t) : m_t(t)
    {}

  public:
    /**
     *  Allow unpacking by conversion operator.
     *  Uncommenting explicit leads to a compilation error in icpc
     */
    /*explicit*/ operator int& ()
    { return m_t; }

  private:
    /// Memory for the contained instance.
    int m_t;
  };

  TEST(IntelCompiler, static_cast_calls_conversion_operator_notemplate)
  {
    IntWrapper wrappedInt(6);

    int& i1(wrappedInt.operator int& ());
    int& i2(static_cast<int&>(wrappedInt));
    int& i3(wrappedInt);

    EXPECT_EQ(6, i1);
    EXPECT_EQ(6, i2);
    EXPECT_EQ(6, i3);
  }

}
