/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCWireLayer.h"
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>
#include <cdc/geometry/CDCGeometryPar.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

using namespace CDCLocalTracking;

CDCLOCALTRACKING_SwitchableClassImp(CDCWireLayer)

CDCWireLayer::CDCWireLayer() :

//averages of wire values
  m_shift(INVALID_INFO),
  m_skew(0.0),
  m_minPolarR(1000000.0),
  m_refPolarR(0.0),
  m_refZ(0.0),
  m_forwardPolarR(0.0), m_backwardPolarR(0.0),
  m_forwardZ(0.0), m_backwardZ(0.0),
  m_forwardPhiToRef(0.0), m_backwardPhiToRef(0.0),

//form CDCGeometryPar
  m_innerPolarR(0.0),
  m_outerPolarR(0.0)

{;}

CDCWireLayer::CDCWireLayer(const const_iterator& begin, const const_iterator& end) :
  m_begin(begin), m_end(end),

//averages of wire values
  m_shift(INVALID_INFO),
  m_skew(0.0),
  m_minPolarR(1000000.0),
  m_refPolarR(0.0),
  m_refZ(0.0),
  m_forwardPolarR(0.0) , m_backwardPolarR(0.0),
  m_forwardZ(0.0) , m_backwardZ(0.0),
  m_forwardPhiToRef(0.0), m_backwardPhiToRef(0.0),

//form CDCGeometryPar
  m_innerPolarR(0.0),
  m_outerPolarR(0.0)
{ initialize();}

const CDCWireLayer*
CDCWireLayer::getInstance(const ILayerType& iCLayer)
{ return &(CDCWireTopology::getInstance().getWireLayer(iCLayer)); }

const CDCWireLayer*
CDCWireLayer::getInstance(const ILayerType& iSuperLayer, const ILayerType& iLayer)
{ return &(CDCWireTopology::getInstance().getWireLayer(iSuperLayer, iLayer)); }

void CDCWireLayer::initialize()
{

  size_t nWiresInLayer = size();
  ILayerType iCLayer = getICLayer();

  // values from CDCGeometryPar
  CDCGeometryPar& cdcgp = CDCGeometryPar::Instance();

  const double* innerRadiusWireLayer = cdcgp.innerRadiusWireLayer();
  const double* outerRadiusWireLayer = cdcgp.outerRadiusWireLayer();

  m_innerPolarR = innerRadiusWireLayer[iCLayer];
  m_outerPolarR = outerRadiusWireLayer[iCLayer];
  //B2INFO("iCLayer : " << iCLayer);
  //B2INFO("inner polar r : " << getInnerPolarR());
  //B2INFO("outer polar r : " << getOuterPolarR());


  // average values from wires
  m_skew = 0.0;
  m_minPolarR = 1000000.0;
  m_refZ = 0.0;
  m_refPolarR = 0.0;
  m_forwardPolarR = 0.0;
  m_backwardPolarR = 0.0;
  m_forwardZ = 0.0;
  m_backwardZ = 0.0;
  m_forwardPhiToRef = 0.0;
  m_backwardPhiToRef = 0.0;



  for (const_iterator itWire = begin(); itWire != end(); ++itWire) {

    const CDCWire& wire = *itWire;

    m_skew += wire.getSkew();

    FloatType minROfWire = wire.getMinPolarR();
    m_minPolarR = std::min(minROfWire, m_minPolarR);

    m_refZ += wire.getRefZ();
    m_refPolarR += wire.getRefPolarR();

    //calculate the forward nad backward r in the xy projection. take the average
    m_forwardPolarR  += wire.getForwardPolarR();
    m_backwardPolarR += wire.getBackwardPolarR();

    //calculate the forward and backward z position. take the average of all wires
    m_forwardZ += wire.getForwardZ();
    m_backwardZ += wire.getBackwardZ();

    //calculate the forward and backward phi. take the average of all wires
    m_forwardPhiToRef += wire.getForwardPhiToRef();
    m_backwardPhiToRef += wire.getBackwardPhiToRef();

  }

  m_skew /= nWiresInLayer;

  m_refZ /= nWiresInLayer;
  m_refPolarR /= nWiresInLayer;

  m_forwardPolarR /= nWiresInLayer;
  m_backwardPolarR /= nWiresInLayer;

  m_forwardZ /= nWiresInLayer;
  m_backwardZ /= nWiresInLayer;

  m_forwardPhiToRef /= nWiresInLayer;
  m_backwardPhiToRef /= nWiresInLayer;

  m_phiRangeToRef = m_forwardPhiToRef < m_backwardPhiToRef ?
                    std::make_pair(m_forwardPhiToRef , m_backwardPhiToRef) :
                    std::make_pair(m_backwardPhiToRef , m_forwardPhiToRef) ;

}

CDCWireLayer::~CDCWireLayer() {;}


const CDCWire* CDCWireLayer::nextWire(const CDCWire* wire) const
{
  if (wire == nullptr) {
    if (begin() == end()) {
      return nullptr;
    } else {
      const CDCWire& nextwire = *begin();
      return &nextwire;
    }
  }
  IWireType iWire = wire->getIWire();
  if (isValidIWire(iWire + 1)) {
    const CDCWire& nextwire = getWire(iWire + 1);
    return &nextwire;
  } else {
    return nullptr;
  }
  return nullptr;
}


