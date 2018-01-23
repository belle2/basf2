/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <gtest/gtest.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBFieldUtil.h>
#include <tracking/trackFindingCDC/testFixtures/TrackFindingCDCTestWithTopology.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

class SecondaryWireNeighborhoodTest : public TrackFindingCDCTestWithTopology,
  public ::testing::WithParamInterface<int> {
};

TEST_P(SecondaryWireNeighborhoodTest, IsSymmetric)
{
  WireNeighborKind testNeighborKind(2, GetParam());
  short oClockDirection = testNeighborKind.getOClockDirection();
  short reverseOClockDirection = modulo(testNeighborKind.getOClockDirection() + 6, 12);

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology.getWires()) {
    MayBePtr<const CDCWire> neighbor = wire.getSecondaryNeighbor(oClockDirection);
    if (neighbor != nullptr) {
      MayBePtr<const CDCWire> neighbor_of_neighbor =
        neighbor->getSecondaryNeighbor(reverseOClockDirection);

      EXPECT_EQ(*neighbor_of_neighbor, wire);

      WireNeighborKind neighborKind = wire.getNeighborKind(*neighbor);
      EXPECT_TRUE(neighborKind.isValid());
      EXPECT_EQ(testNeighborKind.getCellDistance(), neighborKind.getCellDistance());
      EXPECT_EQ(oClockDirection, neighborKind.getOClockDirection());
      EXPECT_EQ(neighbor->getILayer() - wire.getILayer(), neighborKind.getILayerDifference());
    }
  }
}

INSTANTIATE_TEST_CASE_P(SecondaryWireNeighborhoodTest_IsSymmetric,
                        SecondaryWireNeighborhoodTest,
                        ::testing::Range(0, 12));


TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CWInwards)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology.getWires()) {
    MayBePtr<const CDCWire> neighbor = wire.getNeighborCWInwards();
    if (neighbor != nullptr) {
      MayBePtr<const CDCWire> neighbor_of_neighbor = neighbor->getNeighborCCWOutwards();
      EXPECT_EQ(*neighbor_of_neighbor, wire);

      WireNeighborKind neighborKind = wire.getNeighborKind(*neighbor);
      EXPECT_EQ(1, neighborKind.getCellDistance());
      EXPECT_EQ(5, neighborKind.getOClockDirection());
      EXPECT_EQ(-1, neighborKind.getILayerDifference());
    }
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CCWInwards)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology.getWires()) {
    MayBePtr<const CDCWire> neighbor = wire.getNeighborCCWInwards();
    if (neighbor != nullptr) {
      MayBePtr<const CDCWire> neighbor_of_neighbor = neighbor->getNeighborCWOutwards();
      EXPECT_EQ(*neighbor_of_neighbor, wire);

      WireNeighborKind neighborKind = wire.getNeighborKind(*neighbor);
      EXPECT_EQ(1, neighborKind.getCellDistance());
      EXPECT_EQ(7, neighborKind.getOClockDirection());
      EXPECT_EQ(-1, neighborKind.getILayerDifference());
    }
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CWOutwards)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology.getWires()) {
    MayBePtr<const CDCWire> neighbor = wire.getNeighborCWOutwards();
    if (neighbor != nullptr) {
      MayBePtr<const CDCWire> neighbor_of_neighbor = neighbor->getNeighborCCWInwards();
      EXPECT_EQ(*neighbor_of_neighbor, wire);

      WireNeighborKind neighborKind = wire.getNeighborKind(*neighbor);
      EXPECT_EQ(1, neighborKind.getCellDistance());
      EXPECT_EQ(1, neighborKind.getOClockDirection());
      EXPECT_EQ(1, neighborKind.getILayerDifference());
    }
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CCWOutwards)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology.getWires()) {
    MayBePtr<const CDCWire> neighbor = wire.getNeighborCCWOutwards();
    if (neighbor != nullptr) {
      MayBePtr<const CDCWire> neighbor_of_neighbor = neighbor->getNeighborCWInwards();
      EXPECT_EQ(*neighbor_of_neighbor, wire);

      WireNeighborKind neighborKind = wire.getNeighborKind(*neighbor);
      EXPECT_EQ(1, neighborKind.getCellDistance());
      EXPECT_EQ(11, neighborKind.getOClockDirection());
      EXPECT_EQ(1, neighborKind.getILayerDifference());
    }
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CCW)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology.getWires()) {
    MayBePtr<const CDCWire> neighbor = wire.getNeighborCCW();
    if (neighbor != nullptr) {
      MayBePtr<const CDCWire> neighbor_of_neighbor = neighbor->getNeighborCW();
      EXPECT_EQ(*neighbor_of_neighbor, wire);

      WireNeighborKind neighborKind = wire.getNeighborKind(*neighbor);
      EXPECT_EQ(1, neighborKind.getCellDistance());
      EXPECT_EQ(9, neighborKind.getOClockDirection());
      EXPECT_EQ(0, neighborKind.getILayerDifference());
    }
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CW)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology.getWires()) {
    MayBePtr<const CDCWire> neighbor = wire.getNeighborCW();
    if (neighbor != nullptr) {
      MayBePtr<const CDCWire> neighbor_of_neighbor = neighbor->getNeighborCCW();
      EXPECT_EQ(*neighbor_of_neighbor, wire);

      WireNeighborKind neighborKind = wire.getNeighborKind(*neighbor);
      EXPECT_EQ(1, neighborKind.getCellDistance());
      EXPECT_EQ(3, neighborKind.getOClockDirection());
      EXPECT_EQ(0, neighborKind.getILayerDifference());
    }
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_CDCWire_stereoAngle)
{
  // Test if the all wires in the same superlayer have similar skew parameters.

  double tanThetaByICLayer[CDCWireTopology::c_NLayers];
  double stereoAngleByICLayer[CDCWireTopology::c_NLayers];
  double refCylindricalRByICLayer[CDCWireTopology::c_NLayers];

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();
  for (const CDCWireLayer& wireLayer : theWireTopology.getWireLayers()) {
    const ILayer iCLayer = wireLayer.getICLayer();

    const CDCWire& firstWire = wireLayer.front();
    tanThetaByICLayer[iCLayer] = firstWire.getTanStereoAngle();
    stereoAngleByICLayer[iCLayer] = firstWire.getStereoAngle();
    refCylindricalRByICLayer[iCLayer] = firstWire.getRefCylindricalR();

    for (const CDCWire& wire : wireLayer) {
      EXPECT_NEAR(tanThetaByICLayer[iCLayer], wire.getTanStereoAngle(), 10e-6);
      EXPECT_NEAR(stereoAngleByICLayer[iCLayer], wire.getStereoAngle(), 10e-6);
      EXPECT_NEAR(refCylindricalRByICLayer[iCLayer], wire.getRefCylindricalR(), 10e-6);
    }

    B2INFO("ICLayer : " << iCLayer);
    B2INFO("   Tan Theta : " << tanThetaByICLayer[iCLayer]);
    B2INFO("   Stereo angle : " << stereoAngleByICLayer[iCLayer]);
    B2INFO("   Z range : " << wireLayer.getBackwardZ() << " to " << wireLayer.getForwardZ());
    B2INFO("   Ref. cylindricalR : " << refCylindricalRByICLayer[iCLayer]);
    B2INFO("   Max abs displacement : " << wireLayer.getWire(0).getWireVector().xy().norm());
  }

}



TEST_F(TrackFindingCDCTestWithTopology, topology_RefCylindricalRVersusZInSuperLayers)
{
  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();
  for (const CDCWireSuperLayer& wireSuperLayer : theWireTopology.getWireSuperLayers()) {
    if (wireSuperLayer.getStereoKind() == EStereoKind::c_Axial) {
      EXPECT_EQ(0.0, wireSuperLayer.getRefTanLambda());
    }

    B2INFO("ISuperLayer : " << wireSuperLayer.getISuperLayer() <<
           " Inner ref. z : " << wireSuperLayer.getInnerRefZ() <<
           " Outer ref. z : " << wireSuperLayer.getOuterRefZ() <<
           " CylindricalR Z slope : " << wireSuperLayer.getRefTanLambda()
          );

  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_ShowCurlCurv)
{
  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  double outerR = theWireTopology.getOuterCylindricalR();
  double innerR = theWireTopology.getInnerCylindricalR();

  double innerOriginCurv = 1 / (innerR / 2);
  double outerOriginCurv = 1 / (outerR / 2);

  double innerCurlCurv = 1 / innerR;
  double outerCurlCurv = 1 / outerR;

  B2INFO("Maximal curvature still reaching the CDC from IP : " << innerOriginCurv);
  B2INFO("Minimal momentum still reaching the CDC from IP : " << CDCBFieldUtil::curvatureToAbsMom2D(innerOriginCurv, 1.5) << " GeV");

  B2INFO("Maximal curvature still leaving the CDC from IP : " << outerOriginCurv);
  B2INFO("Minimal momentum still leaving the CDC from IP : " << CDCBFieldUtil::curvatureToAbsMom2D(outerOriginCurv, 1.5) << " GeV");


  B2INFO("Minimal curvature not reaching the CDC from VXD : " << innerCurlCurv);
  B2INFO("Maximal momentum not reaching the CDC from VXD : " << CDCBFieldUtil::curvatureToAbsMom2D(innerCurlCurv, 1.5) << " GeV");

  B2INFO("Minimal curvature not leaving the CDC from inside the CDC : " << outerCurlCurv);
  B2INFO("Maximal momentum not leaving the CDC from inside the CDC : " << CDCBFieldUtil::curvatureToAbsMom2D(outerCurlCurv,
         1.5) << " GeV");

  for (const CDCWireSuperLayer& wireSuperLayer : theWireTopology.getWireSuperLayers()) {
    double innerSLCurv = 1 / wireSuperLayer.getInnerCylindricalR();
    B2INFO("Maximal curvature still reaching SL " << wireSuperLayer.getISuperLayer() << " from IP : " << innerSLCurv);
    B2INFO("Minimal momentum still reaching SL " << wireSuperLayer.getISuperLayer() << " from IP : " <<
           CDCBFieldUtil::curvatureToAbsMom2D(innerSLCurv, 1.5) << " GeV");
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_ShowLayerLimits)
{
  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();
  ISuperLayer iL = -1;
  for (const CDCWireLayer& wireLayer : theWireTopology.getWireLayers()) {
    ++iL;
    B2INFO("Layer " << iL << ":");
    B2INFO("z in " << wireLayer.getForwardZ() << ", " << wireLayer.getBackwardZ());
    B2INFO("r in " << wireLayer.getInnerCylindricalR() << ", " << wireLayer.getOuterCylindricalR());
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_CDCGeometryPar_cellId)
{
  // Testing for a discrepancy in the cellId method of CDCGeometryPar
  // Example in layer four
  unsigned iCLayer = 4;

  // Middle of the layer four
  TVector3 posInMiddle(0, 0, 20.8);

  unsigned iWireInFourthLayer = CDC::CDCGeometryPar::Instance().cellId(iCLayer, posInMiddle);
  EXPECT_TRUE(iWireInFourthLayer < 160);
  EXPECT_LT(iWireInFourthLayer, 160);
}

TEST_F(TrackFindingCDCTestWithTopology, topology_sag)
{
  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();
  for (const CDCWire& wire : theWireTopology.getWires()) {
    const WireLine& wireLine = wire.getWireLine();
    const double forwardZ = wireLine.forwardZ();
    const double backwardZ = wireLine.backwardZ();
    const double centerZ = (forwardZ + backwardZ) / 2;

    EXPECT_LE(0, wireLine.sagCoeff());

    EXPECT_EQ(wireLine.nominalPos2DAtZ(forwardZ).y(), wireLine.sagPos2DAtZ(forwardZ).y());
    EXPECT_EQ(wireLine.nominalPos2DAtZ(backwardZ).y(), wireLine.sagPos2DAtZ(backwardZ).y());
    EXPECT_GE(wireLine.nominalPos2DAtZ(centerZ).y(), wireLine.sagPos2DAtZ(centerZ).y());

    EXPECT_LE(wireLine.nominalMovePerZ().y(), wireLine.sagMovePerZ(forwardZ).y());
    EXPECT_GE(wireLine.nominalMovePerZ().y(), wireLine.sagMovePerZ(backwardZ).y());
    EXPECT_EQ(wireLine.nominalMovePerZ().y(), wireLine.sagMovePerZ(centerZ).y());
  }
}
