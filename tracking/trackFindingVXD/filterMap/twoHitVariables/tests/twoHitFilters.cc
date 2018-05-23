/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni (eugenio.paoloni@pi.infn.it)             *
 *               Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/filterMap/twoHitVariables/Distance3DSquared.h>
#include <tracking/trackFindingVXD/filterMap/twoHitVariables/Distance2DXYSquared.h>
#include <tracking/trackFindingVXD/filterMap/twoHitVariables/Distance1DZ.h>
#include <tracking/trackFindingVXD/filterMap/twoHitVariables/SlopeRZ.h>
#include <tracking/trackFindingVXD/filterMap/twoHitVariables/CosDirectionXY.h>
#include <tracking/trackFindingVXD/filterMap/twoHitVariables/Distance3DNormed.h>

#include <tracking/trackFindingVXD/filterMap/filterFramework/Shortcuts.h>

#include <vxd/geometry/SensorInfoBase.h>
#include <tuple>
#include <iostream>
#include <math.h>

#include <functional>

using namespace std;

using namespace Belle2;

namespace VXDTFtwoHitFilterTest {

  /** Test class for these new and shiny two-hit-filters. */
  class TwoHitFilterTest : public ::testing::Test {
  protected:
  };


  /** this is a small helper function to create a sensorInfo to be used */
  VXD::SensorInfoBase createSensorInfo(VxdID aVxdID, double globalX = 0., double globalY = 0., double globalZ = -0.)
  {
    // (SensorType type, VxdID id, double width, double length, double thickness, int uCells, int vCells, double width2=-1, double splitLength=-1, int vCells2=0)
    VXD::SensorInfoBase sensorInfoBase(VXD::SensorInfoBase::PXD, aVxdID, 2.3, 4.2, 0.3, 2, 4, -1);

    TGeoRotation r1;
    r1.SetAngles(45, 20, 30);      // rotation defined by Euler angles
    TGeoTranslation t1(globalX, globalY, globalZ);
    TGeoCombiTrans c1(t1, r1);
    TGeoHMatrix transform = c1;
    sensorInfoBase.setTransformation(transform);
    // also need the reco transform
    sensorInfoBase.setTransformation(transform, true);

    return sensorInfoBase;
  }


  /** when given global coordinates, a SpacePoint lying there will be returned */
  SpacePoint provideSpacePointDummy(double X, double Y, double Z)
  {
    VxdID aVxdID = VxdID(1, 1, 1);
    VXD::SensorInfoBase sensorInfoBase = createSensorInfo(aVxdID, X, Y, Z);

    PXDCluster aCluster = PXDCluster(aVxdID, 0., 0., 0.1, 0.1, 0, 0, 1, 1, 1, 1, 1, 1);
    SpacePoint testPoint = SpacePoint(&aCluster, &sensorInfoBase);

    return SpacePoint(&aCluster, &sensorInfoBase);
  }


  /** a tiny counter class for counting stuff */
  template < class T>
  class counter {
  public:
    static unsigned int used; /**< count number of times used */
    static unsigned int accepted; /**< count number of times result was accepted */
    static unsigned int rejected; /**< count number of times result was rejected */
    static unsigned int wasInf; /**< count number of times result was inf */
    static unsigned int wasNan; /**< count number of times result was nan */
    counter() {}; /**< constructor. */
    ~counter() {}; /**< destructor. */
    static void resetCounter()
    {
      counter<T>::used = 0;
      counter<T>::accepted = 0;
      counter<T>::rejected = 0;
      counter<T>::wasInf = 0;
      counter<T>::wasNan = 0;
    } /**< resets all counters. */
  };



  /** Initialise all static variables */
  template<class T>
  unsigned int counter<T>::used(0);
  template<class T>
  unsigned int counter<T>::accepted(0);
  template<class T>
  unsigned int counter<T>::rejected(0);
  template<class T>
  unsigned int counter<T>::wasInf(0);
  template<class T>
  unsigned int counter<T>::wasNan(0);



  /** this observer does simply count the number of times, the attached Filter was used */
  class CountingObserver : public VoidObserver {
  public:
    /** notifier counting how often a SelectionVariable was used */
    template<class Var, typename ... otherTypes>
    static void notify(const Var&,
                       otherTypes ...)
    {
      counter<Var>::used ++ ;
    }

  };





  /** this observer does simply print the name of the SelectionVariable and the result of its value-function as an Error */
  class ErrorObserver : public VoidObserver {
  public:
    template<class Var, class Range, typename ... otherTypes>
    /** notifier: print the name of the SelectionVariable and the result of its value-function as an Error. */
    static void notify(const Var& filterType,
                       typename Var::variableType fResult,
                       const Range& range,
                       const typename Var::argumentType& outerHit,
                       const typename Var::argumentType& innerHit,
                       otherTypes ...)
    {
      B2ERROR(" Filter " << filterType.name() << " got result of " << fResult);
    }

  };





  /** this observer does simply print the name of the SelectionVariable and the result of its value-function as a Warning */
  class InfoObserver : public VoidObserver {
  public:
    /** notifier: print the name of the SelectionVariable and the result of its value-function as a Warning. */
    template<class Var, class Range, typename ... otherTypes>
    static void notify(const Var& filterType,
                       typename Var::variableType fResult,
                       const Range&,
                       const typename Var::argumentType& outerHit,
                       const typename Var::argumentType& innerHit,
                       otherTypes ...)
    {
      B2WARNING(" Filter " << filterType.name() << " with outerhit/innerhit: " << outerHit.getPosition().PrintStringXYZ() << "/" <<
                innerHit.getPosition().PrintStringXYZ() << " got result of " << fResult);
    }

  };




  /** this observer does simply collect other observers which are to be executed during ::notify */
  template<class FilterType> class VectorOfObservers : public VoidObserver {
  public:

    /** a typedef to make the stuff more readable */
    typedef std::function< void (const typename FilterType::argumentType&, const typename FilterType::argumentType&, const FilterType&, typename FilterType::variableType)>
    observerFunction;

    /** a typedef to make the c-style pointer more readable (can not be done with classic typedef) */
    using CStyleFunctionPointer = void(*)(const typename FilterType::argumentType&, const typename FilterType::argumentType&,
                                          const FilterType&, typename FilterType::variableType) ;

    /** iterate over all stored Observers and execute their notify-function */
    template<typename ... otherTypes>
    static void notify(const FilterType& filterType,
                       typename FilterType::variableType fResult,
                       const typename FilterType::argumentType& outerHit,
                       const typename FilterType::argumentType& innerHit,
                       otherTypes ...)
    {
      B2INFO(" Filter " << filterType.name() << " with Mag of outer-/innerHit " << outerHit.getPosition().Mag() << "/" <<
             innerHit.getPosition().Mag() << " got result of " << fResult << " and Observer-Vector sm_collectedObservers got " <<
             VectorOfObservers<FilterType>::sm_collectedObservers.size() << " observers collected");
      B2INFO(" Filter " << filterType.name() << " with Mag of outer-/innerHit " << outerHit.getPosition().Mag() << "/" <<
             innerHit.getPosition().Mag() << " got result of " << fResult << " and Observer-Vector sm_collectedObserversCSTYLE got " <<
             VectorOfObservers<FilterType>::sm_collectedObserversCSTYLE.size() << " observers collected");

      /// the idea of the following three lines have to work in the end (I basically want to loop over all attached observers and execute their notify function):
      //    for(auto& anObserver : CollectedObservers<FilterType>::collectedObservers) {
      //    anObserver(outerHit, innerHit, fResult);
      //    }
      /// or
    }


    /** collects observers to be executed during notify (can not be used so far, but is long-term goal) */
    template <typename ObserverType>
    static void addObserver(observerFunction newObserver)
    {
      VectorOfObservers<FilterType>::sm_collectedObservers.push_back(std::bind(&newObserver, std::placeholders::_1, std::placeholders::_2,
          FilterType(), std::placeholders::_3));
    }


    /** collects observers with std::function to be executed during notify (variant A)*/
    static std::vector< observerFunction > sm_collectedObservers;
    /** collects observers with c-style function pointers to be executed during notify (variant B)*/
    static std::vector< CStyleFunctionPointer > sm_collectedObserversCSTYLE;

    /** demangled typeID used as type for the Vector */
//  static std::vector< std::_Bind<void (*(std::_Placeholder<1>, std::_Placeholder<2>, Belle2::Distance3DSquared, std::_Placeholder<3>))(Belle2::SpacePoint const&, Belle2::SpacePoint const&, Belle2::Distance3DSquared const&, float)> > sm_collectedObserversTry2;
  };

  /** initialize static member of variant A*/
  template<typename FilterType> std::vector< typename VectorOfObservers<FilterType>::observerFunction >
  VectorOfObservers<FilterType>::sm_collectedObservers  = {};
  /** initialize static member of variant B*/
  template<typename FilterType> std::vector< typename VectorOfObservers<FilterType>::CStyleFunctionPointer >
  VectorOfObservers<FilterType>::sm_collectedObserversCSTYLE  = {};
  /** initialize static member of variant Try2*/
//   template<typename FilterType> std::vector< typename VectorOfObservers<FilterType>::CStyleFunctionPointer > VectorOfObservers<FilterType>::sm_collectedObserversTry2  = {};



  /** presents the functionality of the SpacePoint-creating function written above */
  TEST_F(TwoHitFilterTest, TestObserverFlexibility)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< Distance3DSquared<SpacePoint>, Range<double, double>, VoidObserver > unobservedFilter(Range<double, double>(0.,
        1.));

    //     Filter< Distance3DSquared<SpacePoint>, Range<double, double>, VectorOfObservers<Distance3DSquared> > filter(unobservedFilter);
    Filter< Distance3DSquared<SpacePoint>, Range<double, double>, InfoObserver > filter(unobservedFilter);
    SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
    SpacePoint x2 = provideSpacePointDummy(0.5f , 0.0f, 0.0f);
    SpacePoint x3 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);
    auto myCounter = counter<Distance3DSquared<SpacePoint>>();
    myCounter.resetCounter();

    /// variant A (doesn't work, because of static function(?)):
//  auto storeFuncVariantA = std::bind( ((VectorOfObservers<Distance3DSquared>::observerFunction) &CountingObserver::notify), std::placeholders::_1, std::placeholders::_2, Distance3DSquared(), std::placeholders::_3);
    //  VectorOfObservers<Distance3DSquared>::sm_collectedObservers.push_back(storeFuncVariantA);

    /// variant B:
    auto storeFuncVariantB = std::bind(((VectorOfObservers<Distance3DSquared<SpacePoint>>::CStyleFunctionPointer)
                                        &CountingObserver::notify), std::placeholders::_1, std::placeholders::_2, Distance3DSquared<SpacePoint>(),
                                       std::placeholders::_3);

    char* realname(NULL);
    int status(0);
    realname = abi::__cxa_demangle(typeid(storeFuncVariantB).name(), 0, 0, &status);
    std::string name(realname);
    free(realname);
    B2INFO("storeFuncVariantB is of type: " << name);

//  VectorOfObservers<Distance3DSquared>::sm_collectedObserversTry2.push_back(storeFuncVariantB);


    /// doesn't work, different type, additionally static_cast doesn't work too (reinterpret_casat too dangerous, didn't try):
//  VectorOfObservers<Distance3DSquared>::sm_collectedObserversCSTYLE.push_back(storeFuncVariantB);

    /// long-term goal, something comparable to this:
    // VectorOfObservers<Distance3DSquared>::addObserver(CountingObserver);
    // VectorOfObservers<Distance3DSquared>::addObserver(WarningObserver);


    filter.accept(x2, x1);
    filter.accept(x3, x1);
    EXPECT_EQ(0 , myCounter.used);


    /** this following lines are just used for testing and do nothing! */
    double b = 12.;
    if (b == 3) b = 123.;
  }



  /** ignore what is written in this test, is simply used as a temporary storage for currently not used code-snippets */
  TEST_F(TwoHitFilterTest, ignoreMe)
  {
    //garbage:
    //  VectorOfObservers<Distance3DSquared>::addObserver(CountingObserver::notify);
    //  auto storeFunc = std::bind((void(*)(const SpacePoint&, const SpacePoint&, const Belle2::Distance3DSquared&, float))&CountingObserver::notify, std::placeholders::_1, std::placeholders::_2, Distance3DSquared(), std::placeholders::_3);
//  auto storeFunc1 = std::bind(void(*)(const typename Distance3DSquared::argumentType&, const typename Distance3DSquared::argumentType&, const Distance3DSquared&, typename Distance3DSquared::variableType) /*&CountingObserver::notify*/), std::placeholders::_1, std::placeholders::_2, Distance3DSquared(), std::placeholders::_3);

//  ObserverVector<Distance3DSquared>::sm_collectedObservers.push_back(storeFunc);
    //  ObserverVector<Distance3DSquared>::sm_collectedObservers.push_back(std::bind((void(*)(const SpacePoint&, const SpacePoint&, const Belle2::Distance3DSquared&, float))&CountingObserver::notify, std::placeholders::_1, std::placeholders::_2, Distance3DSquared(), std::placeholders::_3));
    // std::vector< typename ObserverVector<FilterType>::observerFunction >();
    //     ObserverVector<VectorOfObservers>::sm_collectedObservers = std::vector< typename VectorOfObservers<Distance3DSquared>::observerFunction >();
//  ErrorObserver anErrorBla();
    //  WarningObserver aWarningBla();
    //  ObserverVector<Distance3DSquared>::addObserver(aWarningBla/*, filterBla*/);
    //  ObserverVector<Distance3DSquared>::addObserver(aBla/*, filterBla*/);
    // // // // // //   Distance3DSquared filterBla();

    /** a tiny CollectedObservers class for CollectedObservers stuff */
//   template<class FilterType> class CollectedObservers {
//   public:
//  typedef std::function< void (const typename FilterType::argumentType&, const typename FilterType::argumentType&, const FilterType&, typename FilterType::variableType)> observerFunction;
//
//  /** collects observers to be executed during notify */
//  static std::vector< observerFunction > collectedObservers;
//  CollectedObservers() {};
//  ~CollectedObservers() {};
//   };

    //  static void addObserver( CountingObserver& newObserver) {
//    //  static void addObserver(observerFunction newObserver) {
//    ObserverVector<FilterType>::sm_collectedObservers.push_back(std::bind(&newObserver::notify, std::placeholders::_1, std::placeholders::_2, FilterType(), std::placeholders::_3));
//  }
  }


  /** presents the functionality of the SpacePoint-creating function written above */
  TEST_F(TwoHitFilterTest, SpacePointCreation)
  {
    SpacePoint testSP = provideSpacePointDummy(1.2, 2.3, 4.2);
    EXPECT_FLOAT_EQ(1.2, testSP.getPosition().X()) ;
    EXPECT_FLOAT_EQ(2.3, testSP.getPosition().Y()) ;
    EXPECT_FLOAT_EQ(4.2, testSP.getPosition().Z()) ;

  }


  /** shows the functionality of the auto naming capability of the Filter */
  TEST_F(TwoHitFilterTest, SelectionVariableName)
  {
    auto dist3D = Distance3DSquared<SpacePoint>();
    EXPECT_EQ("Distance3DSquared" , dist3D.name());
    auto dist2DXY = Distance2DXYSquared<SpacePoint>();
    EXPECT_EQ("Distance2DXYSquared" , dist2DXY.name());
    auto dist1DZ = Distance1DZ<SpacePoint>();
    EXPECT_EQ("Distance1DZ" , dist1DZ.name());
    auto slopeRZ = SlopeRZ<SpacePoint>();
    EXPECT_EQ("SlopeRZ" , slopeRZ.name());

  }


  /** shows how to use the filter Distance3DSquared in a simple case */
  TEST_F(TwoHitFilterTest, BasicFilterTestDistance3DSquared)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< Distance3DSquared<SpacePoint>, Range<double, double>, VoidObserver > filter(Range<double, double>(0., 1.));

    SpacePoint x1 = provideSpacePointDummy(0. , 0., 0.);
    SpacePoint x2 = provideSpacePointDummy(.5 , 0., 0.);
    SpacePoint x3 = provideSpacePointDummy(2. , 0., 0.);

    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_FALSE(filter.accept(x1, x3));

  }


  /** shows how to use the filter Distance3DSquared in a simple case */
  TEST_F(TwoHitFilterTest, BasicFilterTestDistance2DXYSquared)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< Distance2DXYSquared<SpacePoint>, Range<double, double>, VoidObserver > filter(Range<double, double>(0., 1.));

    SpacePoint x1 = provideSpacePointDummy(0. , 0., 0.);
    SpacePoint x2 = provideSpacePointDummy(.5 , 0., 0.);
    SpacePoint x3 = provideSpacePointDummy(2. , 0., 0.);
    SpacePoint x4 = provideSpacePointDummy(0. , 0., 2.);

    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_FALSE(filter.accept(x1, x3));
    EXPECT_TRUE(filter.accept(x2, x4));

  }


  /** shows how to use the filter Distance1DZ in a simple case */
  TEST_F(TwoHitFilterTest, BasicFilterTestDistance1DZ)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< Distance1DZ<SpacePoint>, Range<double, double>, VoidObserver > filter(Range<double, double>(0., 1.));

    SpacePoint x1 = provideSpacePointDummy(0. , 0., 0.);
    SpacePoint x2 = provideSpacePointDummy(0. , 0., .5);
    SpacePoint x3 = provideSpacePointDummy(.25 , .25, 0.);
    SpacePoint x4 = provideSpacePointDummy(0. , 0., 1.);

    EXPECT_TRUE(filter.accept(x2, x1));
    EXPECT_FALSE(filter.accept(x1, x2)); // the input order is relevant
    EXPECT_FALSE(filter.accept(x1, x3));
    EXPECT_FALSE(filter.accept(x1, x4));

  }


  /** shows how to use the filter Distance1DZ in a simple case */
  TEST_F(TwoHitFilterTest, TemplateFilterTestDistance1DZ)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< Distance1DZ<SpacePoint>, Range<double, double>, VoidObserver > filter(Range<double, double>(0., 1.));

    SpacePoint x1 = provideSpacePointDummy(0. , 0., 0.);
    SpacePoint x2 = provideSpacePointDummy(0. , 0., .5);
    SpacePoint x3 = provideSpacePointDummy(.25 , .25, 0.);
    SpacePoint x4 = provideSpacePointDummy(0. , 0., 1.);

    EXPECT_TRUE(filter.accept(x2, x1));
    EXPECT_FALSE(filter.accept(x1, x2)); // the input order is relevant
    EXPECT_FALSE(filter.accept(x1, x3));
    EXPECT_FALSE(filter.accept(x1, x4));

  }


  /** shows how to use the filter SlopeRZ and its expected behavior for some tricky cases */
  TEST_F(TwoHitFilterTest, BasicFilterTestSlopeRZ)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< SlopeRZ<SpacePoint>, Range<double, double>, VoidObserver > filter(Range<double, double>(atan(2.), atan(3.)));

    SpacePoint innerSP = provideSpacePointDummy(1 , 2, 3);
    SpacePoint outerSP1 = provideSpacePointDummy(1 , 4, 4);
    SpacePoint outerSP2 = provideSpacePointDummy(1 , 4, 3.95);
    SpacePoint outerSP3 = provideSpacePointDummy(1 , 4, 4.05);
    SpacePoint outerSP4 = provideSpacePointDummy(1 , 3, 3.45);
    SpacePoint outerSP5 = provideSpacePointDummy(1 , 3, 3.55);
    SpacePoint outerSP6 = provideSpacePointDummy(1 , 4, 3);
    SpacePoint outerSP7 = provideSpacePointDummy(1 , 0, 4);

    EXPECT_FALSE(filter.accept(outerSP3, innerSP));
    EXPECT_FALSE(filter.accept(outerSP1, innerSP));
    EXPECT_TRUE(filter.accept(outerSP2, innerSP));
    EXPECT_FALSE(filter.accept(innerSP, outerSP2)); // reverse order not same result (because of z)
    EXPECT_TRUE(filter.accept(outerSP4, innerSP));
    EXPECT_FALSE(filter.accept(outerSP5, innerSP));
    EXPECT_EQ(filter.accept(outerSP1, innerSP), filter.accept(outerSP7,
                                                              innerSP)); // (direction of r-vector not relevant, only its length)


    auto sRZ = SlopeRZ<SpacePoint>();
    EXPECT_FLOAT_EQ(0., sRZ.value(innerSP, innerSP));
    EXPECT_FLOAT_EQ(atan(2.), sRZ.value(outerSP1, innerSP));
    EXPECT_FLOAT_EQ(atan(2. / 0.95), sRZ.value(outerSP2, innerSP));
    EXPECT_FLOAT_EQ(M_PI - sRZ.value(outerSP2, innerSP),
                    sRZ.value(innerSP, outerSP2)); // with latest bugfix reverse order will result in (Pi - SlopeRZ)
    EXPECT_FLOAT_EQ(atan(2. / 1.05), sRZ.value(outerSP3, innerSP));
    EXPECT_FLOAT_EQ(atan(1. / 0.45), sRZ.value(outerSP4, innerSP));
    EXPECT_FLOAT_EQ(atan(1. / 0.55), sRZ.value(outerSP5, innerSP));
    EXPECT_FLOAT_EQ(M_PI * 0.5, sRZ.value(outerSP6, innerSP)); // no problem with division by 0 in Z
    EXPECT_FLOAT_EQ(atan(2. / 1.05), sRZ.value(outerSP3, innerSP));
    EXPECT_FLOAT_EQ(sRZ.value(outerSP1, innerSP), sRZ.value(outerSP7,
                                                            innerSP)); // (direction of r-vector not relevant, only its length)

  }


  /** shows how to use the filter Distance3DNormed in a simple case */
  TEST_F(TwoHitFilterTest, BasicFilterTestDistance3DNormed)
  {
    // Very verbose declaration, the old normed distance 3D has only an upper cut, no lower one:
    Filter< Distance3DNormed<SpacePoint>, UpperBoundedSet<double>, VoidObserver > filter(UpperBoundedSet<double>(1.));

    // prepare spacePoints for new stuff
    SpacePoint innerSP = provideSpacePointDummy(1 , 2, 3);
    SpacePoint outerSP1 = provideSpacePointDummy(2 , 3, 4);
    SpacePoint outerSP2 = provideSpacePointDummy(1 , 2, 4);
    SpacePoint outerSP3 = provideSpacePointDummy(2 , 3, 3);

    auto d3Dn = Distance3DNormed<SpacePoint>();
    EXPECT_FLOAT_EQ(2. / 3., d3Dn.value(outerSP1, innerSP));
    EXPECT_FLOAT_EQ(0., d3Dn.value(outerSP2, innerSP));
    EXPECT_FLOAT_EQ(1., d3Dn.value(outerSP3, innerSP));
    EXPECT_FLOAT_EQ(0., d3Dn.value(innerSP, innerSP));

  }


  /** shows how to attach an observer to a filter of interest */
  TEST_F(TwoHitFilterTest, ObservedFilter)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< Distance3DSquared<SpacePoint>, Range<double, double>, VoidObserver > unobservedFilter(Range<double, double>(0.,
        1.));

    Filter< Distance3DSquared<SpacePoint>, Range<double, double>, CountingObserver > filter(unobservedFilter);
    SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
    SpacePoint x2 = provideSpacePointDummy(0.5f , 0.0f, 0.0f);
    SpacePoint x3 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);
    auto myCounter = counter<Distance3DSquared<SpacePoint>>();
    myCounter.resetCounter();

    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_FALSE(filter.accept(x1, x3));
    EXPECT_EQ(2 , myCounter.used);
  }


  /** shows how to bypass a filter which itself was not initially planned to be bypassed */
  TEST_F(TwoHitFilterTest, BypassableFilter)
  {
    bool bypassControl(false);
    // Very verbose declaration, see below for convenient shortcuts
    Filter< Distance3DSquared<SpacePoint>, Range<double, double>, CountingObserver > nonBypassableFilter(Range<double, double>
        (0., 1.));
    auto filter = nonBypassableFilter.bypass(bypassControl);
    SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
    SpacePoint x2 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);
    auto myCounter = counter<Distance3DSquared<SpacePoint>>();
    myCounter.resetCounter();

    EXPECT_FALSE(filter.accept(x1, x2));
    EXPECT_EQ(1 , myCounter.used);

    bypassControl = true;
    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_EQ(2 , myCounter.used);

  }


  /** shows how to write compact code using the new filter design */
  TEST_F(TwoHitFilterTest, Shortcuts)
  {

    SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
    SpacePoint x2 = provideSpacePointDummy(0.5f , 0.0f, 0.0f);
    SpacePoint x3 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);

    auto filterSup = (Distance3DSquared<SpacePoint>() < 1.) ;
    EXPECT_TRUE(filterSup.accept(x1, x2));
    EXPECT_FALSE(filterSup.accept(x1, x3));

    auto filterSup2 = (1 > Distance3DSquared<SpacePoint>()) ;
    EXPECT_TRUE(filterSup2.accept(x1, x2));
    EXPECT_FALSE(filterSup2.accept(x1, x3));

    auto filterInf = (Distance3DSquared<SpacePoint>() > 1.) ;
    EXPECT_TRUE(filterInf.accept(x1, x3));
    EXPECT_FALSE(filterInf.accept(x1, x2));

    auto filterInf2 = (1 < Distance3DSquared<SpacePoint>()) ;
    EXPECT_TRUE(filterInf2.accept(x1, x3));
    EXPECT_FALSE(filterInf2.accept(x1, x2));

    auto filterRange = (0.1 < Distance3DSquared<SpacePoint>() < 1);
    EXPECT_FALSE(filterRange.accept(x1, x1));
    EXPECT_TRUE(filterRange.accept(x1, x2));
    EXPECT_FALSE(filterRange.accept(x1, x3));

  }


  /** tests compatibility of filters with boolean operations for easy coupling of filters */
  TEST_F(TwoHitFilterTest, BooleanOperations)
  {

    SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
    SpacePoint x2 = provideSpacePointDummy(1.0f , 0.0f, 0.0f);
    SpacePoint x3 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);

    auto filter = !(Distance3DSquared<SpacePoint>() > 1.);
    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_TRUE(filter.accept(x1, x1));
    EXPECT_FALSE(filter.accept(x1, x3));

    auto filter2 =
      !(Distance3DSquared<SpacePoint>() > 1.) &&
      !(Distance3DSquared<SpacePoint>() < 1);
    // i.e. Distance3DSquared == 1
    EXPECT_TRUE(filter2.accept(x1, x2));
    EXPECT_FALSE(filter2.accept(x1, x1));
    EXPECT_FALSE(filter2.accept(x1, x3));


    auto filter3 =
      (Distance3DSquared<SpacePoint>() > 1.) ||
      (Distance3DSquared<SpacePoint>() < 1);
    // i.e. Distance3DSquared != 1
    EXPECT_FALSE(filter3.accept(x1, x2));
    EXPECT_TRUE(filter3.accept(x1, x1));
    EXPECT_TRUE(filter3.accept(x1, x3));

  }


  /** evaluating compatibility of filters with lazy evaluation */
  TEST_F(TwoHitFilterTest, ShortCircuitsEvaluation)
  {
    auto filter(
      ((Distance2DXYSquared<SpacePoint>() < 1) &&
       (Distance3DSquared<SpacePoint>()   < 1)).observe(CountingObserver())
    );

    SpacePoint x1 = provideSpacePointDummy(0.0f , 0.0f, 0.0f);
    SpacePoint x2 = provideSpacePointDummy(1.0f , 0.0f, 0.0f);
    SpacePoint x3 = provideSpacePointDummy(2.0f , 0.0f, 0.0f);

    auto counter3D = counter<Distance3DSquared<SpacePoint>>();
    auto counter2D = counter<Distance2DXYSquared<SpacePoint>>();
    counter3D.used = 0;
    counter2D.used = 0;

    EXPECT_FALSE(filter.accept(x1, x3));
    EXPECT_EQ(1 , counter2D.used);
    EXPECT_EQ(0 , counter3D.used);

    EXPECT_TRUE(filter.accept(x1, x1));
    EXPECT_EQ(2 , counter2D.used);
    EXPECT_EQ(1 , counter3D.used);

  }

  /** simple test to test the direction evaluation */
  TEST_F(TwoHitFilterTest, TestCosDirectionXY)
  {
    SpacePoint x1 = provideSpacePointDummy(2.0f , 1.0f, 0.0f);
    SpacePoint x2 = provideSpacePointDummy(1.0f , -2.0f, 0.0f);
    SpacePoint x3 = provideSpacePointDummy(1.0f , 0.0f, 0.0f);
    SpacePoint x4 = provideSpacePointDummy(0.5f , 0.86602540378443f, 0.0f);

    EXPECT_FLOAT_EQ(0., CosDirectionXY<SpacePoint>::value(x1, x2));
    EXPECT_NEAR(0.5, CosDirectionXY<SpacePoint>::value(x3, x4), 0.0000001);
  }
}
