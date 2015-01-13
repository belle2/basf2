/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCWire.h"

#include <cdc/geometry/CDCGeometryPar.h>
#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>


using namespace std;
using namespace Belle2;
using namespace CDC;

using namespace CDCLocalTracking;


CDCLOCALTRACKING_SwitchableClassImp(CDCWire)


CDCWire::CDCWire(const WireID& wireID) :
  m_wireID(wireID),
  m_forwardPhiToRef(0.0),
  m_backwardPhiToRef(0.0),
  m_phiRangeToRef(0.0, 0.0)
{ initialize(); }

CDCWire::CDCWire(
  const ILayerType& iSuperLayer,
  const ILayerType& iLayer,
  const IWireType&   iWire
) :
  m_wireID(iSuperLayer, iLayer, iWire),
  m_forwardPhiToRef(0.0),
  m_backwardPhiToRef(0.0),
  m_phiRangeToRef(0.0, 0.0)
{ initialize(); }

const CDCWire* CDCWire::getInstance(const CDCWire& wire)
{ return &(CDCWireTopology::getInstance().getWire(wire.getWireID())); }

const CDCWire* CDCWire::getInstance(const ILayerType& iSuperLayer,
                                    const ILayerType& iLayer,
                                    const IWireType& iWire)
{ return &(CDCWireTopology::getInstance().getWire(iSuperLayer, iLayer, iWire)); }

const CDCWire* CDCWire::getInstance(const CDCHit& hit)
{

  if (not CDCWireTopology::getInstance().isValidIWire(WireID(hit.getID()))) {
    B2FATAL("Invalid wire id of cdc hit " <<  WireID(hit.getID()));
  }

  const CDCWire& wire = CDCWireTopology::getInstance().getWire(WireID(hit.getID()));

  if (wire.getEWire() != hit.getID()) {
    B2ERROR("WireID.getEWire() differs from CDCHit.getID()");
    B2ERROR("WireID.getEWire() : " << wire.getEWire());
    B2ERROR("CDCHit.getEWire() : " << hit.getID());
  }
  return &wire;

}


void CDCWire::initialize()
{

  CDCGeometryPar& cdcgp = CDCGeometryPar::Instance();

  IWireType iWire = getIWire();
  ILayerType iCLayer = getICLayer();

  TVector3 forwardPos = cdcgp.wireForwardPosition(iCLayer, iWire);
  TVector3 backwardPos = cdcgp.wireBackwardPosition(iCLayer, iWire);

  m_skewLine = BoundSkewLine(forwardPos, backwardPos);

  m_forwardPhiToRef = m_skewLine.forwardPhiToRef();
  m_backwardPhiToRef = m_skewLine.backwardPhiToRef();

  /// used to check for odd stereo wires
  if ((m_forwardPhiToRef == 0 or
       m_backwardPhiToRef == 0 or
       isNAN(m_forwardPhiToRef) or
       isNAN(m_backwardPhiToRef)) and
      not isAxial()) {

    B2WARNING("Odd wire " << this);
    B2WARNING("wireForwardPosition  " << Vector3D(forwardPos));
    B2WARNING("wireBackwardPosition " << Vector3D(backwardPos));
    B2WARNING("forward              " << m_skewLine.forward3D());
    B2WARNING("backward             " << m_skewLine.backward3D());
    B2WARNING("ref                  " << m_skewLine.refPos3D());
    B2WARNING("skew                 " << m_skewLine.skew());
    B2WARNING("m_forwardToRefAngle  " << m_forwardPhiToRef);
    B2WARNING("m_backwardToRefAngle " << m_backwardPhiToRef);
    //double d;
    //std::cin >> d;
  }

  m_phiRangeToRef = m_forwardPhiToRef < m_backwardPhiToRef ?
                    std::make_pair(m_forwardPhiToRef , m_backwardPhiToRef) :
                    std::make_pair(m_backwardPhiToRef , m_forwardPhiToRef) ;

}

WireNeighborType CDCWire::isNeighborWith(const CDCWire& wire) const
{ return CDCWireTopology::getInstance().areNeighbors(getWireID(), wire.getWireID()); }



CDCWire::NeighborPair CDCWire::getNeighborsInwards() const
{ return CDCWireTopology::getInstance().getNeighborsInwards(getWireID()); }


CDCWire::NeighborPair CDCWire::getNeighborsOutwards() const
{ return CDCWireTopology::getInstance().getNeighborsOutwards(getWireID()); }



const CDCWire* CDCWire::getNeighborCCW() const
{ return CDCWireTopology::getInstance().getNeighborCCW(getWireID()); }


const CDCWire* CDCWire::getNeighborCW() const
{ return CDCWireTopology::getInstance().getNeighborCW(getWireID()); }




const CDCWire* CDCWire::getNeighborCCWInwards() const
{ return CDCWireTopology::getInstance().getNeighborCCWInwards(getWireID()); }



const CDCWire* CDCWire::getNeighborCWInwards() const
{ return CDCWireTopology::getInstance().getNeighborCWInwards(getWireID()); }


const CDCWire* CDCWire::getNeighborCCWOutwards() const
{ return CDCWireTopology::getInstance().getNeighborCCWOutwards(getWireID()); }


const CDCWire* CDCWire::getNeighborCWOutwards() const
{ return CDCWireTopology::getInstance().getNeighborCWOutwards(getWireID()); }




