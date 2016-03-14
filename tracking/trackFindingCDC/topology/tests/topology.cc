/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBField.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <cdc/geometry/CDCGeometryPar.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/test_fixtures/TrackFindingCDCTestWithTopology.h>

#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CWInwards)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCWInwards();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCCWOutwards();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CCWInwards)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCCWInwards();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCWOutwards();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CWOutwards)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCWOutwards();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCCWInwards();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CCWOutwards)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCCWOutwards();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCWInwards();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CCW)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCCW();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCW();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}

TEST_F(TrackFindingCDCTestWithTopology, topology_WireNeighborSymmetry_CW)
{

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();

  for (const CDCWire& wire : theWireTopology) {
    const CDCWire* neighbor = wire.getNeighborCW();
    if (neighbor != nullptr) {
      const CDCWire* neighbor_of_neighbor = neighbor->getNeighborCCW();
      EXPECT_EQ(*neighbor_of_neighbor, wire);
    }
  }
}


TEST_F(TrackFindingCDCTestWithTopology, topology_CDCWire_stereoAngle)
{
  // Test if the all wires in the same superlayer have similar skew parameters.

  FloatType tanThetaByICLayer[CDCWireTopology::N_LAYERS];
  FloatType stereoAngleByICLayer[CDCWireTopology::N_LAYERS];
  FloatType refCylindricalRByICLayer[CDCWireTopology::N_LAYERS];

  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();
  for (const CDCWireLayer& wireLayer : theWireTopology.getWireLayers()) {
    const ILayerType iCLayer = wireLayer.getICLayer();

    const CDCWire& firstWire = wireLayer.first();
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
    B2INFO("   Phi spread : " << wireLayer.getForwardPhiToRef() - wireLayer.getBackwardPhiToRef());
    B2INFO("   Ref. cylindricalR : " << refCylindricalRByICLayer[iCLayer]);
    B2INFO("   Max abs displacement : " << wireLayer.getWire(0).getWireVector().xy().norm());
  }

}



TEST_F(TrackFindingCDCTestWithTopology, topology_RefCylindricalRVersusZInSuperLayers)
{
  const CDCWireTopology& theWireTopology  = CDCWireTopology::getInstance();
  for (const CDCWireSuperLayer& wireSuperLayer : theWireTopology.getWireSuperLayers()) {
    if (wireSuperLayer.getStereoType() == AXIAL) {
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

  FloatType outerR = theWireTopology.getOuterCylindricalR();
  FloatType innerR = theWireTopology.getInnerCylindricalR();

  FloatType innerOriginCurv = 1 / (innerR / 2);
  FloatType outerOriginCurv = 1 / (outerR / 2);

  FloatType innerCurlCurv = 1 / innerR;
  FloatType outerCurlCurv = 1 / outerR;

  B2INFO("Maximal curvature reaching the CDC from IP : " << innerOriginCurv);
  B2INFO("Minimal momentum reaching the CDC from IP : " << curvatureToAbsMom2D(innerOriginCurv, 1.5));

  B2INFO("Maximal curvature leaving the CDC from IP : " << outerOriginCurv);
  B2INFO("Minimal momentum leaving the CDC from IP : " << curvatureToAbsMom2D(outerOriginCurv, 1.5));


  B2INFO("Minimal curvature not reaching the CDC from VXD : " << innerCurlCurv);
  B2INFO("Maximal momentum not reaching the CDC from VXD : " << curvatureToAbsMom2D(innerCurlCurv, 1.5));

  B2INFO("Minimal curvature not leaving the CDC from inside the CDC : " << outerCurlCurv);
  B2INFO("Maximal momentum not leaving the CDC from inside the CDC : " << curvatureToAbsMom2D(outerCurlCurv, 1.5));
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
