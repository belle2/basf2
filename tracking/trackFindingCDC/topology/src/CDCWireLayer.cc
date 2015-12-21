/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/topology/CDCWireLayer.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <cdc/geometry/CDCGeometryPar.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

using namespace TrackFindingCDC;


CDCWireLayer::CDCWireLayer()
//averages of wire values
  : m_shift(ERotation::c_Invalid),
    m_tanStereoAngle(0.0),
    m_minCylindricalR(1000000.0),
    m_refCylindricalR(0.0),
    m_refZ(0.0),
    m_forwardCylindricalR(0.0), m_backwardCylindricalR(0.0),
    m_forwardZ(0.0), m_backwardZ(0.0),
    m_forwardPhiToRef(0.0), m_backwardPhiToRef(0.0),
    //form CDCGeometryPar
    m_innerCylindricalR(0.0),
    m_outerCylindricalR(0.0)
{
}

CDCWireLayer::CDCWireLayer(const const_iterator& begin, const const_iterator& end)
  : m_begin(begin),
    m_end(end),
    //averages of wire values
    m_shift(ERotation::c_Invalid),
    m_tanStereoAngle(0.0),
    m_minCylindricalR(1000000.0),
    m_refCylindricalR(0.0),
    m_refZ(0.0),
    m_forwardCylindricalR(0.0) , m_backwardCylindricalR(0.0),
    m_forwardZ(0.0) , m_backwardZ(0.0),
    m_forwardPhiToRef(0.0), m_backwardPhiToRef(0.0),
    //form CDCGeometryPar
    m_innerCylindricalR(0.0),
    m_outerCylindricalR(0.0)
{
  initialize();
}

const CDCWireLayer* CDCWireLayer::getInstance(const ILayerType& iCLayer)
{
  return &(CDCWireTopology::getInstance().getWireLayer(iCLayer));
}

const CDCWireLayer* CDCWireLayer::getInstance(ISuperLayer iSuperLayer, const ILayerType& iLayer)
{
  return &(CDCWireTopology::getInstance().getWireLayer(iSuperLayer, iLayer));
}

void CDCWireLayer::initialize()
{
  size_t nWiresInLayer = size();
  ILayerType iCLayer = getICLayer();

  // values from CDCGeometryPar
  CDCGeometryPar& cdcgp = CDCGeometryPar::Instance();

  const double* innerRadiusWireLayer = cdcgp.innerRadiusWireLayer();
  const double* outerRadiusWireLayer = cdcgp.outerRadiusWireLayer();

  m_innerCylindricalR = innerRadiusWireLayer[iCLayer];
  m_outerCylindricalR = outerRadiusWireLayer[iCLayer];
  //B2INFO("iCLayer : " << iCLayer);
  //B2INFO("inner cylindrical r : " << getInnerCylindricalR());
  //B2INFO("outer cylindrical r : " << getOuterCylindricalR());


  // average values from wires
  m_tanStereoAngle = 0.0;
  m_minCylindricalR = 1000000.0;
  m_refZ = 0.0;
  m_refCylindricalR = 0.0;
  m_forwardCylindricalR = 0.0;
  m_backwardCylindricalR = 0.0;
  m_forwardZ = 0.0;
  m_backwardZ = 0.0;
  m_forwardPhiToRef = 0.0;
  m_backwardPhiToRef = 0.0;

  for (const_iterator itWire = begin(); itWire != end(); ++itWire) {

    const CDCWire& wire = *itWire;

    m_tanStereoAngle += wire.getTanStereoAngle();

    double minROfWire = wire.getMinCylindricalR();
    m_minCylindricalR = std::min(minROfWire, m_minCylindricalR);

    m_refZ += wire.getRefZ();
    m_refCylindricalR += wire.getRefCylindricalR();

    //calculate the forward nad backward r in the xy projection. take the average
    m_forwardCylindricalR  += wire.getForwardCylindricalR();
    m_backwardCylindricalR += wire.getBackwardCylindricalR();

    //calculate the forward and backward z position. take the average of all wires
    m_forwardZ += wire.getForwardZ();
    m_backwardZ += wire.getBackwardZ();

    //calculate the forward and backward phi. take the average of all wires
    m_forwardPhiToRef += wire.getForwardPhiToRef();
    m_backwardPhiToRef += wire.getBackwardPhiToRef();

  }

  m_tanStereoAngle /= nWiresInLayer;

  m_refZ /= nWiresInLayer;
  m_refCylindricalR /= nWiresInLayer;

  m_forwardCylindricalR /= nWiresInLayer;
  m_backwardCylindricalR /= nWiresInLayer;

  m_forwardZ /= nWiresInLayer;
  m_backwardZ /= nWiresInLayer;

  m_forwardPhiToRef /= nWiresInLayer;
  m_backwardPhiToRef /= nWiresInLayer;

  m_phiRangeToRef = m_forwardPhiToRef < m_backwardPhiToRef ?
                    std::make_pair(m_forwardPhiToRef , m_backwardPhiToRef) :
                    std::make_pair(m_backwardPhiToRef , m_forwardPhiToRef) ;

}

const CDCWire& CDCWireLayer::getClosestWire(const Vector3D& pos3D) const
{
  IWireType iWire = CDCGeometryPar::Instance().cellId(getICLayer(), pos3D);
  // Safety measure against error in the cellId function
  iWire %= size();
  return getWire(iWire);
}
