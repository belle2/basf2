/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcLocalTracking/CDCLocalTrackingTestModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/dataobjects/EventMetaData.h>

#include <cdc/geometry/CDCGeometryPar.h>

//in type
#include <mdst/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>

//typedefs
#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

//out type
#include "genfit/TrackCand.h"

#include <time.h>
#ifdef HAS_CALLGRIND
#include <valgrind/callgrind.h>
#endif

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;
using namespace CDC;


REG_MODULE(CDCLocalTrackingTest)

CDCLocalTrackingTestModule::CDCLocalTrackingTestModule() : Module()
{
  setDescription("Newmodule with yet a description to be assigned");

  //addParam("Input collection name",  m_inColName, "Name of collection for input", string(""));

}

CDCLocalTrackingTestModule::~CDCLocalTrackingTestModule()
{
}

void CDCLocalTrackingTestModule::initialize()
{


  //StoreObjPtr<CDCWireHit>::registerPersistent();
  //register a single cdchit under the name "AnotherHit" and do not write
  //it to the output file by default
  //StoreObjPtr<CDCWireHit>::registerTransient("AnotherHit");

#ifdef CDCLOCALTRACKING_USE_ROOT
  StoreObjPtr< CDCWireHitCollection >::registerTransient("CDCAllWireHitCollection");
#endif


  //output collection
  StoreArray < genfit::TrackCand >::registerTransient("GFTrackCands");

  //m_segmentWorker.init();
  //m_trackingWorker.init();

  //StoreArray with digitized CDCHits
  //StoreArray <CDCHit> cdcHits(m_inColName);
  //cdcHits.isRequired();

  //StoreArray <CDCSimHit> storedSimhits;
  //storedSimhits.isRequired();

  //StoreArray <MCParticle> storedMCParticles;
  //storedMCParticles.isRequired();

  CDC::CDCGeometryPar& cdcGeo __attribute__((unused)) = CDC::CDCGeometryPar::Instance();
  CDCWireTopology& topo __attribute__((unused)) = CDCWireTopology::getInstance();

  CDCWireHit::initializeLowest();
  const CDCWireHit& lowest = CDCWireHit::getLowest();
  B2DEBUG(100, lowest);

#ifdef HAS_CALLGRIND
  CALLGRIND_START_INSTRUMENTATION;
#endif
  test_generalized_circle();

  batch_line2d();

  test_wire_topology();
  test_wire_neighbor_symmetry();
  test_recotangent();
  B2INFO(lowest);

  B2INFO("INVALID_INFO  " << INVALID_INFO);
  B2INFO("reversed(INVALID_INFO)  " << reversed(INVALID_INFO));

  B2INFO("-INVALID_INFO  " << OrientationInfo(-INVALID_INFO));

  B2INFO("sizeof(INVALID_INFO)  " << sizeof(INVALID_INFO));
  B2INFO("std::numeric_limits<signed short>::min()  " << std::numeric_limits<signed short>::min());
  B2INFO("-std::numeric_limits<signed short>::min()  " << -std::numeric_limits<signed short>::min());
  B2INFO("-std::numeric_limits<signed short>::min()  " << (signed short)(-std::numeric_limits<signed short>::min()));

  B2INFO("std::numeric_limits<signed short>::max()  " << std::numeric_limits<signed short>::max());
  B2INFO("-std::numeric_limits<signed short>::max()  " << -std::numeric_limits<signed short>::max());

  BoundSkewLine skewLine(Vector3D(2, -2, -2), Vector3D(2, 1, 1));
  B2INFO("Reference postion of (2,-2,-2) -> (2,1,1) " << skewLine.refPos3D());
  B2INFO("Skew of (2,-2,-2) -> (2,1,1) " << skewLine.skew());

  FloatType i = 100000000000.0;
  int k = 1;
  B2INFO("Maximal value " << i);
  while (i != i + 1.0 and k < 1000) { i = i + 1.0;  ++k; }
  B2INFO("Maximal value " << i);

  test_mock_root();
  test_sizeof();
#ifdef HAS_CALLGRIND
  CALLGRIND_STOP_INSTRUMENTATION;
#endif
}

void CDCLocalTrackingTestModule::beginRun()
{

}

void CDCLocalTrackingTestModule::event()
{

}

void CDCLocalTrackingTestModule::endRun()
{
}

void CDCLocalTrackingTestModule::terminate()
{
#ifdef HAS_CALLGRIND
  CALLGRIND_DUMP_STATS;
#endif
}

void CDCLocalTrackingTestModule::test_modulo() const
{

  B2INFO("##### Test modulo");
  signed short n = -1;
  signed short m1 = 45;
  unsigned short m2 = 45;

  B2INFO("-1 % 45 signed short "  << n % m1);
  B2INFO("-1 % 45 unsigned short "  << n % m2);

}

void CDCLocalTrackingTestModule::test_sizeof() const
{

  int i_int;
  size_t i_size_t;
  intptr_t i_intptr_t;
  uintptr_t i_uintptr_t;

  B2INFO("sizeof(int) " << sizeof(i_int));
  B2INFO("sizeof(size_t) " << sizeof(i_size_t));
  B2INFO("sizeof(intptr_t) " << sizeof(i_intptr_t));
  B2INFO("sizeof(uintptr_t) " << sizeof(i_uintptr_t));

}

void CDCLocalTrackingTestModule::test_mock_root() const
{

  B2INFO("##### Test mock root");

  UsedTObject t;
  B2INFO("sizeof(UsedTObject) " << sizeof(t)) ;

  Vector2D v2;
  Vector3D v3;
  Line2D l;
  ParameterLine2D pl;
  GeneralizedCircle gc;
  BoundSkewLine bl;

  B2INFO("sizeof(Vector2D) " << sizeof(v2)) ;
  B2INFO("sizeof(Vector3D) " << sizeof(v3)) ;
  B2INFO("sizeof(Line2D) " << sizeof(l)) ;
  B2INFO("sizeof(ParameterLine2D) " << sizeof(pl)) ;
  B2INFO("sizeof(GeneralizedCircle) " << sizeof(gc)) ;
  B2INFO("sizeof(BoundSkewLine) " << sizeof(bl)) ;

  CDCTrajectory2D t2;
  CDCTrajectorySZ tsz;

  B2INFO("sizeof(CDCTrajectory2D) " << sizeof(t2)) ;
  B2INFO("sizeof(CDCTrajectorySZ) " << sizeof(tsz)) ;

  CDCWire w;
  CDCWireHit wh;
  CDCRecoHit2D rh2;
  CDCRecoHit3D rh3;
  CDCRecoTangent rt;
  CDCRecoFacet rf;

  B2INFO("sizeof(CDCWire) " << sizeof(w)) ;
  B2INFO("sizeof(CDCWireHit) " << sizeof(wh)) ;
  B2INFO("sizeof(CDCRecoHit2D) " << sizeof(rh2)) ;
  B2INFO("sizeof(CDCRecoHit3D) " << sizeof(rh3)) ;
  B2INFO("sizeof(CDCRecoTangent) " << sizeof(rt)) ;
  B2INFO("sizeof(CDCRecoFacet) " << sizeof(rf)) ;


}


void CDCLocalTrackingTestModule::test_nullptr() const
{

  B2INFO("##### Test nullptr");

  CDCRecoHit2D* ptr = nullptr;
  CDCRecoHit2D* ptr2 = nullptr;
  ++ptr2;
  if (ptr) { B2INFO("ptr is true"); }
  else  { B2INFO("ptr is false"); }
  if (ptr2) { B2INFO("ptr2 is true"); }
  else  { B2INFO("ptr2 is false"); }


  Vector2D* v = nullptr;
  Vector2D* v2 = new Vector2D;

  cout << (v ==  nullptr) << endl;
  cout << (v2 ==  nullptr) << endl;
  cout << (v2 ==  v) << endl;
  cout << (nullptr ==  v2) << endl;
  cout << (nullptr ==  v) << endl;

  //cout << (v <  nullptr) << endl;
  //cout << (nullptr < nullptr) << endl;
  //cout << (nullptr < v2) << endl;
  //cout << (nullptr < v) << endl;
  //cout << (v2 < nullptr) << endl;
  //cout << (v2 < v) << endl;
}

void CDCLocalTrackingTestModule::test_function_templates() const
{

  // B2INFO("##### Test function templates");

  // std::set<int> container;
  // fillInto<int>(12,container);
  // fillInto(34,container);
  // B2INFO( "sizeOf(container) " << sizeOf<int>(container)) ;

  // std::vector<int> container2;
  // fillInto<int>(12,container2);
  // fillInto(34,container2);
  // B2INFO( "sizeOf(container2) " << sizeOf(container2)) ;


  // std::map<int,int> mp;
  // B2INFO( "mp[1] = " << mp[1]) ;

}


void CDCLocalTrackingTestModule::test_size_of_dataholders() const
{

  B2INFO("##### Test size of dataholders");

  CDCRecoHit2D oneplainrecohit;
  B2INFO("sizeof(oneplainrecohit) " << sizeof(oneplainrecohit)) ;
  CDCRecoTangent oneplaintangent;
  B2INFO("sizeof(oneplaintangent) " << sizeof(oneplaintangent)) ;
  CDCRecoFacet oneplainfacet;
  B2INFO("sizeof(oneplainfacet) " << sizeof(oneplainfacet)) ;

  std::vector<CDCRecoFacet> onefacet(1);
  B2INFO("sizeof(onefacet) " << sizeof(onefacet));

  std::vector<CDCRecoFacet> twofacets(2);
  B2INFO("sizeof(twofacets) " << sizeof(twofacets));

  std::pair<CDCRecoFacet, CDCRecoFacet> othertwofacets;
  B2INFO("sizeof(othertwofacets) " << sizeof(othertwofacets)) ;
}


void CDCLocalTrackingTestModule::test_generalized_circle() const
{

  B2INFO("##### Test generalized circle");
  GeneralizedCircle circle(Vector2D(0.5, 0.0), 1.5);
  B2INFO(circle);

  B2INFO("Is line " << circle.isLine());
  B2INFO("Is circle " << circle.isCircle());

  B2INFO("n0 " << circle.n0());
  B2INFO("n1 " << circle.n1());
  B2INFO("n2 " << circle.n2());
  B2INFO("n3 " << circle.n3());

  B2INFO("distance to 3,0 " << circle.distance(Vector2D(3, 0)));
  B2INFO("fast distance to 3,0 " << circle.fastDistance(Vector2D(3, 0)));
  B2INFO("absolute distance to 3,0 " << circle.absoluteDistance(Vector2D(3, 0)));

  B2INFO("closest approach to 0,0 " << circle.closestToOrigin());
  B2INFO("closest approach to 3,0 " << circle.closest(Vector2D(3, 0)));
  B2INFO("same r approach to 3,0 " << circle.samePolarR(Vector2D(3, 0)));
  B2INFO("tangential to 3,0 " << circle.tangential(Vector2D(3, 0)));

  B2INFO("opening angle from 0.5,2.5 to  3,0 " << circle.openingAngle(Vector2D(0.5, 2.5) , Vector2D(3, 0)));
  B2INFO("arc length from 0.5,2.5 to  3,0 " << circle.lengthOnCurve(Vector2D(0.5, 2.5) , Vector2D(3, 0)));


  circle.reverse();
  B2INFO("Reversed");
  B2INFO(circle);

  B2INFO("Is line " << circle.isLine());
  B2INFO("Is circle " << circle.isCircle());

  B2INFO("n0 " << circle.n0());
  B2INFO("n1 " << circle.n1());
  B2INFO("n2 " << circle.n2());
  B2INFO("n3 " << circle.n3());

  B2INFO("distance to 3,0 " << circle.distance(Vector2D(3, 0)));
  B2INFO("fast distance to 3,0 " << circle.fastDistance(Vector2D(3, 0)));
  B2INFO("absolute distance to 3,0 " << circle.absoluteDistance(Vector2D(3, 0)));

  B2INFO("closest approach to 0,0 " << circle.closestToOrigin());
  B2INFO("closest approach to 3,0 " << circle.closest(Vector2D(3, 0)));
  B2INFO("same r approach to 3,0 " << circle.samePolarR(Vector2D(3, 0)));
  B2INFO("tangential to 3,0 " << circle.tangential(Vector2D(3, 0)));

  B2INFO("opening angle from 0.5,2,5 to  3,0 " << circle.openingAngle(Vector2D(0.5, 2.5) , Vector2D(3, 0)));
  B2INFO("arc length from 0.5,2.5 to  3,0 " << circle.lengthOnCurve(Vector2D(0.5, 2.5) , Vector2D(3, 0)));


  circle.setN(1, 1, -1, 0);
  circle.normalize();
  B2INFO("Line");
  B2INFO(circle);

  B2INFO("Is line " << circle.isLine());
  B2INFO("Is circle " << circle.isCircle());

  B2INFO("n0 " << circle.n0());
  B2INFO("n1 " << circle.n1());
  B2INFO("n2 " << circle.n2());
  B2INFO("n3 " << circle.n3());

  B2INFO("distance to 3,0 " << circle.distance(Vector2D(3, 0)));
  B2INFO("fast distance to 3,0 " << circle.fastDistance(Vector2D(3, 0)));
  B2INFO("absolute distance to 3,0 " << circle.absoluteDistance(Vector2D(3, 0)));

  B2INFO("closest approach to 0,0 " << circle.closestToOrigin());
  B2INFO("closest approach to 3,0 " << circle.closest(Vector2D(3, 0)));
  B2INFO("same r approach to 3,0 " << circle.samePolarR(Vector2D(3, 0)));
  B2INFO("tangential to 3,0 " << circle.tangential(Vector2D(3, 0)));

  B2INFO("opening angle from 0,0 to  3,0 " << circle.openingAngle(Vector2D(0, 0) , Vector2D(3, 0)));
  B2INFO("arc length from 0,0 to  3,0 " << circle.lengthOnCurve(Vector2D(0, 0) , Vector2D(3, 0)));

  B2INFO("Reversed line");
  circle.reverse();
  B2INFO(circle);

  B2INFO("Is line " << circle.isLine());
  B2INFO("Is circle " << circle.isCircle());

  B2INFO("n0 " << circle.n0());
  B2INFO("n1 " << circle.n1());
  B2INFO("n2 " << circle.n2());
  B2INFO("n3 " << circle.n3());

  B2INFO("distance to 3,0 " << circle.distance(Vector2D(3, 0)));
  B2INFO("fast distance to 3,0 " << circle.fastDistance(Vector2D(3, 0)));
  B2INFO("absolute distance to 3,0 " << circle.absoluteDistance(Vector2D(3, 0)));

  B2INFO("closest approach to 0,0 " << circle.closestToOrigin());
  B2INFO("closest approach to 3,0 " << circle.closest(Vector2D(3, 0)));
  B2INFO("same r approach to 3,0 " << circle.samePolarR(Vector2D(3, 0)));
  B2INFO("tangential to 3,0 " << circle.tangential(Vector2D(3, 0)));

  B2INFO("opening angle from 0,0 to  3,0 " << circle.openingAngle(Vector2D(0, 0) , Vector2D(3, 0)));
  B2INFO("arc length from 0,0 to  3,0 " << circle.lengthOnCurve(Vector2D(0, 0) , Vector2D(3, 0)));


}


void CDCLocalTrackingTestModule::batch_line2d() const
{


  B2INFO("#### Test line");
  Line2D line(1.0, 1.0, -1.0);
  test_line2d(line);


  B2INFO("#### Test reverse line");
  line.reverse();
  test_line2d(line);
  line.reverse();

  B2INFO("#### Test reversed line");
  Line2D line2 = line.reversed();
  test_line2d(line2);

  B2INFO("#### Test invert line");
  line.invert();
  test_line2d(line);
  line.invert();

  B2INFO("#### Test inverted line");
  line2 = line.inverted();
  test_line2d(line2);


  B2INFO("#### Test line from slope intercept");
  Line2D line3 = Line2D::fromSlopeIntercept(2, 1);
  test_line2d(line3);


  B2INFO("#### Test line from slope intercept backward");
  Line2D line4 = Line2D::fromSlopeIntercept(2, 1, BACKWARD);
  test_line2d(line4);

  B2INFO("#### Test line from slope intercept reversed forward");
  Line2D line5 = Line2D::fromSlopeIntercept(2, 1);
  line5.reverse();
  test_line2d(line5);

  B2INFO("#### Test line through points 1,0 to 1,2");
  Line2D line6 = Line2D::throughPoints(Vector2D(1.0, 0.0), Vector2D(2, 1));
  test_line2d(line6);

  B2INFO("#### Test line through points 1,0 to 1,2 moved by (1,1)");
  line6.moveBy(Vector2D(1, 1));
  test_line2d(line6);

  B2INFO("#### Test line through points 1,0 to 1,2 moved by (3,1)");
  line6.moveAlongFirst(2);
  test_line2d(line6);

  B2INFO("#### Test line through points 1,0 to 1,2 moved by (3,-1)");
  line6.passiveMoveAlongSecond(2);
  test_line2d(line6);

  B2INFO("#### Test parameter line with support 0,-1 and tangent 1,1 ");
  ParameterLine2D line7(Vector2D(0, -2), Vector2D(1, 1));
  test_parameter_line2d(line7);
  B2INFO("#### Test parameter line through points 0,2 to 2,0 ");
  ParameterLine2D line8 = ParameterLine2D::throughPoints(Vector2D(0, 2), Vector2D(2, 0));
  test_parameter_line2d(line8);

  B2INFO("### Intersections");
  B2INFO("IntersectionAt " << line7.intersectionAt(line8));
  B2INFO("Intersection " << line7.intersection(line8));

  B2INFO("IntersectionAt " << line7.intersectionAt(Line2D(line8)));
  B2INFO("Intersection " << line7.intersection(Line2D(line8)));

  B2INFO("Intersection " << Line2D(line7).intersection(Line2D(line8)));




}

void CDCLocalTrackingTestModule::test_line2d(const Line2D& line) const
{

  B2INFO("n0 " << line.n0());
  B2INFO("n1 " << line.n1());
  B2INFO("n2 " << line.n2());

  B2INFO("slope " << line.slope());
  B2INFO("inverseSlope " << line.inverseSlope());
  B2INFO("intercept " << line.intercept());
  B2INFO("zero " << line.zero());

  B2INFO("tangential " << line.tangential());
  B2INFO("support " << line.support());
  B2INFO("normal " << line.normal());

  B2INFO("distance to 0,0 " << line.distanceToOrigin());
  B2INFO("distance to 3,0 " << line.distance(Vector2D(3, 0)));
  B2INFO("absolute distance to 3,0 " << line.absoluteDistance(Vector2D(3, 0)));

  B2INFO("closest approach to 0,0 " << line.closestToOrigin());
  B2INFO("closest approach to 3,0 " << line.closest(Vector2D(3, 0)));

  B2INFO("map 1 -> " << line.map(1));
  B2INFO("inverseMap 1 -> " << line.inverseMap(1));

  B2INFO("lengthOnCurve 0,0 to 3.0 : " << line.lengthOnCurve(Vector2D(0, 0), Vector2D(3, 0)));

}

void CDCLocalTrackingTestModule::test_parameter_line2d(const ParameterLine2D& line) const
{


  B2INFO("tangential " << line.tangential());
  B2INFO("support " << line.support());
  B2INFO("normal " << line.normal());

  B2INFO("at 2 " << line.at(2));

  B2INFO("slope " << line.slope());
  B2INFO("inverseSlope " << line.inverseSlope());
  B2INFO("intercept " << line.intercept());
  B2INFO("zero " << line.zero());

  B2INFO("distance to 0,0 " << line.distanceToOrigin());
  B2INFO("distance to 3,0 " << line.distance(Vector2D(3, 0)));
  B2INFO("absolute distance to 3,0 " << line.absoluteDistance(Vector2D(3, 0)));

  B2INFO("closest approach to 0,0 " << line.closestToOrigin());
  B2INFO("closest approach to 0,0 at " << line.closestToOriginAt());
  B2INFO("closest approach to 3,0 " << line.closest(Vector2D(3, 0)));
  B2INFO("closest approach to 3,0 at " << line.closestAt(Vector2D(3, 0)));

  B2INFO("map 1 -> " << line.map(1));
  B2INFO("inverseMap 1 -> " << line.inverseMap(1));

  B2INFO("lengthOnCurve 0,0 to 3.0 : " << line.lengthOnCurve(Vector2D(0, 0), Vector2D(3, 0)));

}


void CDCLocalTrackingTestModule::test_wire_topology() const
{

  CDCWireTopology topo;
  topo.initialize();

  B2INFO("#### Layers");
  const vector<CDCWireLayer>& layers = topo.getWireLayers();

  for (vector<CDCWireLayer>::const_iterator itLayer = layers.begin();
       itLayer != layers.end(); ++itLayer) {

    B2INFO("Layer cILayer " << itLayer->getICLayer());
    B2INFO("Layer iLayer " << itLayer->getILayer());
    B2INFO("Layer iSuperLayer " << itLayer->getISuperLayer());
    B2INFO("nWires " << itLayer->size());
    B2INFO("AxialType : " <<  itLayer->getAxialType());
    B2INFO("StereoAngle : " << itLayer ->getStereoAngle());
    B2INFO("Skew : " << itLayer->getSkew());
    B2INFO("Shift : " << itLayer->getShift());
    B2INFO("################################");
  }

  B2INFO("#### Superlayers");
  const vector<CDCWireSuperLayer>& superlayers = topo.getWireSuperLayers();

  for (vector<CDCWireSuperLayer>::const_iterator itSuperLayer = superlayers.begin();
       itSuperLayer != superlayers.end(); ++itSuperLayer) {

    B2INFO("Layer iSuperLayer " << itSuperLayer->getISuperLayer());
    B2INFO("nWire " << itSuperLayer->size());
    B2INFO("AxialType  " << itSuperLayer->getAxialType());

  }

  B2INFO("#### Compare with old neighbor");
  const CDCWire* zeroWire = CDCWire::getInstance(1, 4, 159);

  const CDCWire* neighborCW_IN = zeroWire->getNeighborCWInwards();
  const CDCWire* neighborCW_OUT = zeroWire->getNeighborCWOutwards();

  const CDCWire* neighborCCW_IN = zeroWire->getNeighborCCWInwards();
  const CDCWire* neighborCCW_OUT = zeroWire->getNeighborCCWOutwards();

  const CDCWire* neighborCW = zeroWire->getNeighborCW();
  const CDCWire* neighborCCW = zeroWire->getNeighborCCW();

  const CDCWire& zeroWire2 = topo.getWire(1, 4, 159);

  const CDCWire* neighbor2CW_IN = topo.getNeighborCWInwards(zeroWire2);
  const CDCWire* neighbor2CW_OUT = topo.getNeighborCWOutwards(zeroWire2);

  const CDCWire* neighbor2CCW_IN = topo.getNeighborCCWInwards(zeroWire2);
  const CDCWire* neighbor2CCW_OUT = topo.getNeighborCCWOutwards(zeroWire2);

  const CDCWire* neighbor2CW = topo.getNeighborCW(zeroWire2);
  const CDCWire* neighbor2CCW = topo.getNeighborCCW(zeroWire2);

  B2INFO("CCW_OUT "  << neighborCCW_OUT << " <-> " << neighbor2CCW_OUT);
  B2INFO("CCW_IN "  << neighborCCW_IN << " <-> " << neighbor2CCW_IN);
  B2INFO("CW_OUT "  << neighborCW_OUT << " <-> " << neighbor2CW_OUT);
  B2INFO("CW_IN "  << neighborCW_IN << " <-> " << neighbor2CW_IN);
  B2INFO("CCW "  << neighborCCW << " <-> " << neighbor2CCW);
  B2INFO("CW "  << neighborCW << " <-> " << neighbor2CW);


  B2INFO("#### Secondary neighborhood");
  B2INFO("One "  << topo.getSecondNeighorOneOClock(zeroWire2));
  B2INFO("Two "  << topo.getSecondNeighorTwoOClock(zeroWire2));
  B2INFO("Three "  << topo.getSecondNeighorThreeOClock(zeroWire2));
  B2INFO("Four "  << topo.getSecondNeighorFourOClock(zeroWire2));
  B2INFO("Five "  << topo.getSecondNeighorFiveOClock(zeroWire2));
  B2INFO("Six "  << topo.getSecondNeighorSixOClock(zeroWire2));
  B2INFO("Seven "  << topo.getSecondNeighorSevenOClock(zeroWire2));
  B2INFO("Eight "  << topo.getSecondNeighorEightOClock(zeroWire2));
  B2INFO("Nine "  << topo.getSecondNeighorNineOClock(zeroWire2));
  B2INFO("Ten "  << topo.getSecondNeighorTenOClock(zeroWire2));
  B2INFO("Eleven "  << topo.getSecondNeighorElevenOClock(zeroWire2));
  B2INFO("Twelve "  << topo.getSecondNeighorTwelveOClock(zeroWire2));

  B2INFO("#### AreNeighborsTest");
  B2INFO("CW to CW "  << topo.areNeighbors(*neighbor2CW, *neighbor2CW));
  B2INFO("CCW to CW "  << topo.areNeighbors(*neighbor2CCW, *neighbor2CW));
  B2INFO("CCW_IN to CW "  << topo.areNeighbors(*neighbor2CCW_IN, *neighbor2CW));
  B2INFO("CCW_IN to CCW "  << topo.areNeighbors(*neighbor2CCW_IN, *neighbor2CCW));
  B2INFO("CCW_OUT to CCW "  << topo.areNeighbors(*neighbor2CCW_OUT, *neighbor2CCW));
  B2INFO("CCW to CCW_IN "  << topo.areNeighbors(*neighbor2CCW, *neighbor2CCW_IN));
  B2INFO("CCW to CCW_OUT "  << topo.areNeighbors(*neighbor2CCW, *neighbor2CCW_OUT));
  B2INFO("CW_OUT to CCW_OUT "  << topo.areNeighbors(*neighbor2CW_OUT, *neighbor2CCW_OUT));
  B2INFO("CCW_IN to CW_IN "  << topo.areNeighbors(*neighbor2CCW_IN, *neighbor2CW_IN));
}


void CDCLocalTrackingTestModule::test_enums() const
{

  enum Type { zero = 0, one = 1, two = 2, three = 3 };

  if (zero) { B2INFO("zero is true"); }
  else  { B2INFO("zero is false"); }
  if (one) { B2INFO("one is true"); }
  else { B2INFO("one is false"); }
  if (two) { B2INFO("two is true"); }
  else { B2INFO("two is false"); }
  if (three) { B2INFO("three is true"); }
  else { B2INFO("three is false"); }
  if (three and zero)  { B2INFO("three and zero is true"); }
  else { B2INFO("three and zero is false"); }
  if (one and two)  { B2INFO("one and two is true"); }
  else { B2INFO("one and two is false"); }

}


void CDCLocalTrackingTestModule::test_wire_neighbor_symmetry() const
{

  CDCWireTopology& topo = CDCWireTopology::getInstance();

  B2INFO("Number of wires " << topo.size());

  B2INFO("Checking CCW_IN to CW_OUT wire neighborhood symmetry") ;

  for (CDCWireTopology::const_iterator itWire = topo.begin(); itWire != topo.end(); ++itWire) {

    const CDCWire& wire = *itWire;

    const CDCWire* neighbor = wire.getNeighborCCWInwards();
    if (neighbor != nullptr) {

      const CDCWire* neighborOfNeighbor = neighbor->getNeighborCWOutwards();

      if (&wire != neighborOfNeighbor) {
        B2WARNING("Wire neighborhood not symmetric from " << wire << " to " << neighbor);
      }
    }
  }
  //cin >> h;

  B2INFO("Checking CCW_OUT to CW_IN wire neighborhood symmetry") ;

  for (CDCWireTopology::const_iterator itWire = topo.begin(); itWire != topo.end(); ++itWire) {

    const CDCWire& wire = *itWire;

    const CDCWire* neighbor = wire.getNeighborCCWOutwards();
    if (neighbor != nullptr) {

      const CDCWire* neighborOfNeighbor = neighbor->getNeighborCWInwards();

      if (&wire != neighborOfNeighbor) {
        B2WARNING("Wire neighborhood not symmetric from " << wire << " to " << neighbor);
      }
    }
  }
  //cin >> h;

  B2INFO("Checking CW_IN to CCW_OUT wire neighborhood symmetry") ;

  for (CDCWireTopology::const_iterator itWire = topo.begin(); itWire != topo.end(); ++itWire) {

    const CDCWire& wire = *itWire;

    const CDCWire* neighbor = wire.getNeighborCWInwards();
    if (neighbor != nullptr) {

      const CDCWire* neighborOfNeighbor = neighbor->getNeighborCCWOutwards();

      if (&wire != neighborOfNeighbor) {
        B2WARNING("Wire neighborhood not symmetric from " << wire << " to " << neighbor);
      }
    }
  }
  //cin >> h;

  B2INFO("Checking CW_OUT to CCW_IN wire neighborhood symmetry") ;

  for (CDCWireTopology::const_iterator itWire = topo.begin(); itWire != topo.end(); ++itWire) {

    const CDCWire& wire = *itWire;

    const CDCWire* neighbor = wire.getNeighborCWOutwards();
    if (neighbor != nullptr) {

      const CDCWire* neighborOfNeighbor = neighbor->getNeighborCCWInwards();

      if (&wire != neighborOfNeighbor) {
        B2WARNING("Wire neighborhood not symmetric from " << wire << " to " << neighbor);
      }
    }
  }
  //cin >> h;

  B2INFO("Checking CW to CCW wire neighborhood symmetry") ;

  for (CDCWireTopology::const_iterator itWire = topo.begin(); itWire != topo.end(); ++itWire) {

    const CDCWire& wire = *itWire;

    const CDCWire* neighbor = wire.getNeighborCW();
    if (neighbor != nullptr) {

      const CDCWire* neighborOfNeighbor = neighbor->getNeighborCCW();

      if (&wire != neighborOfNeighbor) {
        B2WARNING("Wire neighborhood not symmetric from " << wire << " to " << neighbor);
      }
    }
  }

  B2INFO("Checking CCW to CW wire neighborhood symmetry") ;

  for (CDCWireTopology::const_iterator itWire = topo.begin(); itWire != topo.end(); ++itWire) {

    const CDCWire& wire = *itWire;

    const CDCWire* neighbor = wire.getNeighborCCW();
    if (neighbor != nullptr) {

      const CDCWire* neighborOfNeighbor = neighbor->getNeighborCW();

      if (&wire != neighborOfNeighbor) {
        B2WARNING("Wire neighborhood not symmetric from " << wire << " to " << neighbor);
      }
    }
  }
}


void CDCLocalTrackingTestModule::test_recotangent() const
{

  //two wires

  const CDCWire* wire0 = &(CDCWire::getLowest());
  const CDCWire* wire1 = wire0->getNeighborCCW();

  Vector2D connecting = wire1->getRefPos2D() - wire0->getRefPos2D();

  const CDCWireHit wirehit0(wire0->getWireID(), connecting.norm() / 4.0);
  const CDCWireHit wirehit1(wire1->getWireID(), connecting.norm() / 4.0);

  CDCRecoTangent t0(&wirehit0, RIGHT, &wirehit1, RIGHT);
  CDCRecoTangent t1(&wirehit0, RIGHT, &wirehit1, LEFT);
  CDCRecoTangent t2(&wirehit0, LEFT,  &wirehit1, RIGHT);
  CDCRecoTangent t3(&wirehit0, LEFT,  &wirehit1, LEFT);


  B2INFO("RR new " << t0);
  B2INFO("RL new " << t1);
  B2INFO("LR new " << t2);
  B2INFO("LL new " << t3);

  std::vector<CDCRecoTangent> oldTangents;

  //CDCRecoTangent::collectGroupOfTangents(oldTangents, &wirehit0, &wirehit1 );
  //for( std::vector<CDCRecoTangent>::iterator itOldTangents = oldTangents.begin();
  //     itOldTangents != oldTangents.end(); ++itOldTangents ){

  //  B2INFO( "Old t " << *itOldTangents );

  //}


}



