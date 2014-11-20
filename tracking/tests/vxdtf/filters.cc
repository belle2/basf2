/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni (eugenio.paoloni@pi.infn.it              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>


#include "tracking/trackFindingVXD/FilterTools/Shortcuts.h"

#include <tuple>
#include <iostream>
#include <math.h>

using namespace std;

using namespace Belle2;

namespace VXDTFfilterTest {
  typedef tuple<float, float, float> spacePoint;

  class SquaredDistance3D : public SelectionVariable< spacePoint , float > {
  public:
    static float value(const spacePoint& p1, const spacePoint& p2) {
      return
        pow(get<0>(p1) - get<0>(p2) , 2) +
        pow(get<1>(p1) - get<1>(p2) , 2) +
        pow(get<2>(p1) - get<2>(p2) , 2) ;
    }
  };

  class SquaredDistance2Dxy : public SelectionVariable< spacePoint , float > {
  public:
    static float value(const spacePoint& p1, const spacePoint& p2) {
      return
        pow(get<0>(p1) - get<0>(p2) , 2) +
        pow(get<1>(p1) - get<1>(p2) , 2) ;
    }
  };

  class SquaredDistance1Dx : public SelectionVariable< spacePoint , float > {
  public:
    static float value(const spacePoint& p1, const spacePoint& p2) {
      return
        pow(get<0>(p1) - get<0>(p2) , 2);
    }
  };

  class BooleanVariable : public SelectionVariable< spacePoint , bool > {
  public:
    static float value(const spacePoint& p1, const spacePoint& p2) {
      return
        get<0>(p1) - get<0>(p2) == 0.;
    }
  };


  template < class T>
  class counter {
  public:
    static int N;
    counter() {};
    ~counter() {};
  };

  template<>
  int counter< SquaredDistance3D   >::N(0);

  template<>
  int counter< SquaredDistance2Dxy >::N(0);

  template<>
  int counter< SquaredDistance1Dx  >::N(0);


  class Observer : public VoidObserver {
  public:
    template<class Var>
    static void notify(const typename Var::argumentType& ,
                       const typename Var::argumentType& ,
                       const Var& ,
                       typename Var::variableType) {
      counter<Var>::N ++ ;
    }

  };



  /** Test class for the RecoTrack object. */
  class FilterTest : public ::testing::Test {
  protected:
  };


  TEST_F(FilterTest, Range)
  {

    Range<double, double> range(0. , 1.);
    EXPECT_TRUE(range.contains(0.5));
    EXPECT_FALSE(range.contains(-1.));
    EXPECT_FALSE(range.contains(0.));
    EXPECT_FALSE(range.contains(1.));
    EXPECT_FALSE(range.contains(2.));
    EXPECT_EQ(0. , range.getInf());
    EXPECT_EQ(1. , range.getSup());
  }

  TEST_F(FilterTest, UpperBoundedSet)
  {

    UpperBoundedSet<double> upperBoundedSet(0.);
    EXPECT_TRUE(upperBoundedSet.contains(-1.));
    EXPECT_FALSE(upperBoundedSet.contains(0.));
    EXPECT_FALSE(upperBoundedSet.contains(1.));
    EXPECT_EQ(0. , upperBoundedSet.getSup());
  }


  TEST_F(FilterTest, LowerBoundedSet)
  {

    LowerBoundedSet<double> lowerBoundedSet(0.);
    EXPECT_TRUE(lowerBoundedSet.contains(1.));
    EXPECT_FALSE(lowerBoundedSet.contains(0.));
    EXPECT_FALSE(lowerBoundedSet.contains(-1.));
    EXPECT_EQ(0. , lowerBoundedSet.getInf());
  }

  TEST_F(FilterTest, SelectionVariableName)
  {

    EXPECT_EQ("VXDTFfilterTest::SquaredDistance3D" , SquaredDistance3D().name());

  }

  TEST_F(FilterTest, BasicFilter)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< SquaredDistance3D, Range<double, double>, VoidObserver > filter(Range<double, double>(0., 1.));

    spacePoint x1(0.0f , 0.0f, 0.0f);
    spacePoint x2(0.5f , 0.0f, 0.0f);
    spacePoint x3(2.0f , 0.0f, 0.0f);

    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_FALSE(filter.accept(x1, x3));

  }

  TEST_F(FilterTest, ObservedFilter)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< SquaredDistance3D, Range<double, double>, VoidObserver > unobservedFilter(Range<double, double>(0., 1.));

    Filter< SquaredDistance3D, Range<double, double>, Observer > filter(unobservedFilter);
    spacePoint x1(0.0f , 0.0f, 0.0f);
    spacePoint x2(0.5f , 0.0f, 0.0f);
    spacePoint x3(2.0f , 0.0f, 0.0f);
    counter< SquaredDistance3D >::N = 0;

    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_FALSE(filter.accept(x1, x3));
    EXPECT_EQ(2 , counter< SquaredDistance3D >::N);
  }


  TEST_F(FilterTest, BypassableFilter)
  {
    bool bypassControl(false);
    // Very verbose declaration, see below for convenient shortcuts
    Filter< SquaredDistance3D, Range<double, double>, Observer > nonBypassableFilter(Range<double, double>(0., 1.));
    auto filter = nonBypassableFilter.bypass(bypassControl);
    spacePoint x1(0.0f , 0.0f, 0.0f);
    spacePoint x2(2.0f , 0.0f, 0.0f);
    counter< SquaredDistance3D >::N = 0;

    EXPECT_FALSE(filter.accept(x1, x2));
    EXPECT_EQ(1 , counter< SquaredDistance3D >::N);

    bypassControl = true;
    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_EQ(2 , counter< SquaredDistance3D >::N);

  }

  TEST_F(FilterTest, Shortcuts)
  {

    spacePoint x1(0.0f , 0.0f, 0.0f);
    spacePoint x2(0.5f , 0.0f, 0.0f);
    spacePoint x3(2.0f , 0.0f, 0.0f);

    auto filterSup = (SquaredDistance3D() < 1.) ;
    EXPECT_TRUE(filterSup.accept(x1, x2));
    EXPECT_FALSE(filterSup.accept(x1, x3));

    auto filterSup2 = (1 > SquaredDistance3D()) ;
    EXPECT_TRUE(filterSup2.accept(x1, x2));
    EXPECT_FALSE(filterSup2.accept(x1, x3));

    auto filterInf = (SquaredDistance3D() > 1.) ;
    EXPECT_TRUE(filterInf.accept(x1, x3));
    EXPECT_FALSE(filterInf.accept(x1, x2));

    auto filterInf2 = (1 < SquaredDistance3D()) ;
    EXPECT_TRUE(filterInf2.accept(x1, x3));
    EXPECT_FALSE(filterInf2.accept(x1, x2));

    auto filterRange = (0.1 < SquaredDistance3D() < 1);
    EXPECT_FALSE(filterRange.accept(x1, x1));
    EXPECT_TRUE(filterRange.accept(x1, x2));
    EXPECT_FALSE(filterRange.accept(x1, x3));

  }

  TEST_F(FilterTest, BooleanOperations)
  {


    spacePoint x1(0.0f , 0.0f, 0.0f);
    spacePoint x2(1.0f , 0.0f, 0.0f);
    spacePoint x3(2.0f , 0.0f, 0.0f);

    auto filter = !(SquaredDistance3D() > 1.);
    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_TRUE(filter.accept(x1, x1));
    EXPECT_FALSE(filter.accept(x1, x3));

    auto filter2 =
      !(SquaredDistance3D() > 1.) &&
      !(SquaredDistance3D() < 1);
    // i.e. SquaredDistance3D == 1
    EXPECT_TRUE(filter2.accept(x1, x2));
    EXPECT_FALSE(filter2.accept(x1, x1));
    EXPECT_FALSE(filter2.accept(x1, x3));


    auto filter3 =
      (SquaredDistance3D() > 1.) ||
      (SquaredDistance3D() < 1);
    // i.e. SquaredDistance3D != 1
    EXPECT_FALSE(filter3.accept(x1, x2));
    EXPECT_TRUE(filter3.accept(x1, x1));
    EXPECT_TRUE(filter3.accept(x1, x3));


  }

  TEST_F(FilterTest, ShortCircuitsEvaluation)
  {
    auto filter(
      (SquaredDistance2Dxy() < 1).observe(Observer()) &&
      (SquaredDistance3D()   < 1).observe(Observer())
    );

    spacePoint x1(0.0f , 0.0f, 0.0f);
    spacePoint x2(1.0f , 0.0f, 0.0f);
    spacePoint x3(2.0f , 0.0f, 0.0f);

    counter< SquaredDistance3D   >::N = 0;
    counter< SquaredDistance2Dxy >::N = 0;

    EXPECT_FALSE(filter.accept(x1, x3));
    EXPECT_EQ(1 , counter< SquaredDistance2Dxy >::N);
    EXPECT_EQ(0 , counter< SquaredDistance3D >::N);

    EXPECT_TRUE(filter.accept(x1, x1));
    EXPECT_EQ(2 , counter< SquaredDistance2Dxy >::N);
    EXPECT_EQ(1 , counter< SquaredDistance3D >::N);

  }

  TEST_F(FilterTest, BooleanVariableShortcuts)
  {
    auto filter1(BooleanVariable() == true);
    auto filter2(false == BooleanVariable());
    spacePoint x1(0.0f , 0.0f, 0.0f);
    spacePoint x2(1.0f , 0.0f, 0.0f);

    EXPECT_TRUE(filter1.accept(x1, x1));
    EXPECT_FALSE(filter1.accept(x1, x2));


    EXPECT_FALSE(filter2.accept(x1, x1));
    EXPECT_TRUE(filter2.accept(x1, x2));



  }
}
