/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <cdc/geometry/CDCGeometryPar.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCWire::CDCWire(const WireID& wireID)
  : m_wireID(wireID),
    m_forwardPhiToRef(0.0),
    m_backwardPhiToRef(0.0),
    m_phiRangeToRef(0.0, 0.0)
{
  initialize();
}

CDCWire::CDCWire(ISuperLayer iSuperLayer,
                 const ILayerType& iLayer,
                 const IWireType&  iWire)
  : m_wireID(iSuperLayer, iLayer, iWire),
    m_forwardPhiToRef(0.0),
    m_backwardPhiToRef(0.0),
    m_phiRangeToRef(0.0, 0.0)
{
  initialize();
}


const CDCWire* CDCWire::getInstance(const WireID& wireID)
{
  return &(CDCWireTopology::getInstance().getWire(wireID));
}

const CDCWire* CDCWire::getInstance(const CDCWire& wire)
{
  return &(CDCWireTopology::getInstance().getWire(wire.getWireID()));
}

const CDCWire* CDCWire::getInstance(ISuperLayer iSuperLayer,
                                    const ILayerType& iLayer,
                                    const IWireType& iWire)
{
  return &(CDCWireTopology::getInstance().getWire(iSuperLayer, iLayer, iWire));
}

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
  CDC::CDCGeometryPar& cdcgp = CDC::CDCGeometryPar::Instance();

  IWireType iWire = getIWire();
  ILayerType iCLayer = getICLayer();

  Vector3D forwardPos{cdcgp.wireForwardPosition(iCLayer, iWire)};
  Vector3D backwardPos{cdcgp.wireBackwardPosition(iCLayer, iWire)};

  m_wireLine = WireLine(forwardPos, backwardPos);
  m_refCylindricalR = getRefPos2D().norm();

  m_forwardPhiToRef = m_wireLine.forwardPhiToRef();
  m_backwardPhiToRef = m_wireLine.backwardPhiToRef();

  /// used to check for odd stereo wires
  if ((m_forwardPhiToRef == 0 or
       m_backwardPhiToRef == 0 or
       std::isnan(m_forwardPhiToRef) or
       std::isnan(m_backwardPhiToRef)) and
      not isAxial()) {

    B2WARNING("Odd wire " << *this);
    B2WARNING("wireForwardPosition  " << forwardPos);
    B2WARNING("wireBackwardPosition " << backwardPos);
    B2WARNING("forward              " << m_wireLine.forward3D());
    B2WARNING("backward             " << m_wireLine.backward3D());
    B2WARNING("ref                  " << m_wireLine.refPos3D());
    B2WARNING("tan theta            " << m_wireLine.tanTheta());
    B2WARNING("m_forwardToRefAngle  " << m_forwardPhiToRef);
    B2WARNING("m_backwardToRefAngle " << m_backwardPhiToRef);
    //double d;
    //std::cin >> d;
  }

  m_phiRangeToRef = std::minmax(m_forwardPhiToRef, m_backwardPhiToRef);
}

bool CDCWire::isInCell(const Vector3D& pos3D) const
{
  bool inZ = getBackwardZ() < pos3D.z() and pos3D.z() < getForwardZ();
  if (not inZ) return false;

  ILayerType iCLayer = getICLayer();
  const CDCWireLayer& wireLayer = CDCWireTopology::getInstance().getWireLayer(iCLayer);
  const double innerCylindricalR = wireLayer.getInnerCylindricalR();
  const double outerCylindricalR = wireLayer.getOuterCylindricalR();
  double cylindricalR = pos3D.cylindricalR();

  bool inCylindricalR = innerCylindricalR < cylindricalR and cylindricalR < outerCylindricalR;
  if (not inCylindricalR) return false;

  IWireType iWire = CDC::CDCGeometryPar::Instance().cellId(iCLayer, pos3D);
  // Safety measure against error in the cellId function
  iWire %= wireLayer.size();
  bool inPhi = iWire == getIWire();
  return inPhi;
}

EWireNeighborKind CDCWire::getNeighborKind(const CDCWire& wire) const
{
  return CDCWireTopology::getInstance().getNeighborKind(getWireID(), wire.getWireID());
}

bool CDCWire::isNeighborWith(const CDCWire& wire) const
{
  return CDCWireTopology::getInstance().areNeighbors(getWireID(), wire.getWireID());
}

CDCWire::NeighborPair CDCWire::getNeighborsInwards() const
{
  return CDCWireTopology::getInstance().getNeighborsInwards(getWireID());
}

CDCWire::NeighborPair CDCWire::getNeighborsOutwards() const
{
  return CDCWireTopology::getInstance().getNeighborsOutwards(getWireID());
}

const CDCWire* CDCWire::getNeighborCCW() const
{
  return CDCWireTopology::getInstance().getNeighborCCW(getWireID());
}

const CDCWire* CDCWire::getNeighborCW() const
{
  return CDCWireTopology::getInstance().getNeighborCW(getWireID());
}

const CDCWire* CDCWire::getNeighborCCWInwards() const
{
  return CDCWireTopology::getInstance().getNeighborCCWInwards(getWireID());
}

const CDCWire* CDCWire::getNeighborCWInwards() const
{
  return CDCWireTopology::getInstance().getNeighborCWInwards(getWireID());
}

const CDCWire* CDCWire::getNeighborCCWOutwards() const
{
  return CDCWireTopology::getInstance().getNeighborCCWOutwards(getWireID());
}

const CDCWire* CDCWire::getNeighborCWOutwards() const
{
  return CDCWireTopology::getInstance().getNeighborCWOutwards(getWireID());
}
