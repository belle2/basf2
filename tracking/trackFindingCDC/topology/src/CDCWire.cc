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
#include <tracking/trackFindingCDC/topology/CDCWireSuperLayer.h>
#include <tracking/trackFindingCDC/topology/CDCWireLayer.h>

#include <tracking/trackFindingCDC/topology/EWirePositionToCDC.h>
#include <tracking/trackFindingCDC/topology/EWirePosition.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/CDCHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

const CDCWire* CDCWire::getInstance(const WireID& wireID)
{
  return &(CDCWireTopology::getInstance().getWire(wireID));
}

const CDCWire* CDCWire::getInstance(ISuperLayer iSuperLayer, ILayer iLayer, IWire iWire)
{
  return &(CDCWireTopology::getInstance().getWire(iSuperLayer, iLayer, iWire));
}

const CDCWire* CDCWire::getInstance(const CDCHit& hit)
{
  if (not CDCWireTopology::getInstance().isValidWireID(WireID(hit.getID()))) {
    WireID wireID(hit.getID());
    B2ERROR("Invalid encoded wire id of cdc hit " << wireID);
    B2ERROR("Superlayer id: " << wireID.getISuperLayer());
    B2ERROR("Layer cid: " << wireID.getICLayer());
    B2ERROR("Layer id: " << wireID.getILayer());
    B2ERROR("Wire id: " << wireID.getIWire());
    B2FATAL("Do not continue with wrong wire id");
    return nullptr;
  }

  const CDCWire& wire = CDCWireTopology::getInstance().getWire(WireID(hit.getID()));

  if (wire.getEWire() != hit.getID()) {
    B2ERROR("WireID.getEWire() differs from CDCHit.getID()");
    B2ERROR("WireID.getEWire() : " << wire.getEWire());
    B2ERROR("CDCHit.getEWire() : " << hit.getID());
  }
  return &wire;
}

CDCWire::CDCWire(const WireID& wireID)
  : m_wireID(wireID)
{
  EWirePosition wirePosition = EWirePosition::c_Base;
  initialize(wirePosition, false);
}

CDCWire::CDCWire(ISuperLayer iSuperLayer, ILayer iLayer, IWire iWire)
  : CDCWire(WireID(iSuperLayer, iLayer, iWire))
{
}

void CDCWire::initialize(EWirePosition wirePosition, bool ignoreWireSag)
{
  CDC::CDCGeometryPar& cdcgp = CDC::CDCGeometryPar::Instance();
  CDC::CDCGeometryPar::EWirePosition wirePosSet = toCDC(wirePosition);

  IWire iWire = getIWire();
  ILayer iCLayer = getICLayer();

  Vector3D forwardPos{cdcgp.wireForwardPosition(iCLayer, iWire, wirePosSet)};
  Vector3D backwardPos{cdcgp.wireBackwardPosition(iCLayer, iWire, wirePosSet)};
  double sagCoeff = ignoreWireSag ? 0 : cdcgp.getWireSagCoef(wirePosSet, iCLayer, iWire);

  m_wireLine = WireLine(forwardPos, backwardPos, sagCoeff);
  m_refCylindricalR = getRefPos2D().norm();

  /// used to check for odd stereo wires -- did not trigger in ages
  if (not isAxial() and (m_wireLine.tanTheta() == 0)) {
    B2WARNING("Odd wire " << *this);
    B2WARNING("wireForwardPosition  " << forwardPos);
    B2WARNING("wireBackwardPosition " << backwardPos);
    B2WARNING("forward              " << m_wireLine.forward3D());
    B2WARNING("backward             " << m_wireLine.backward3D());
    B2WARNING("ref                  " << m_wireLine.refPos3D());
    B2WARNING("tan theta            " << m_wireLine.tanTheta());
  }
}

bool CDCWire::isInCell(const Vector3D& pos3D) const
{
  // Necessary for cppcheck not understanding that the variable is read on the next line
  // cppcheck-suppress unreadVariable
  bool inZ = getBackwardZ() < pos3D.z() and pos3D.z() < getForwardZ();
  if (not inZ) return false;

  ILayer iCLayer = getICLayer();
  const CDCWireLayer& wireLayer = getWireLayer();
  double innerCylindricalR = wireLayer.getInnerCylindricalR();
  double outerCylindricalR = wireLayer.getOuterCylindricalR();
  double cylindricalR = pos3D.cylindricalR();

  // Necessary for cppcheck not understanding that the variable is read on the next line
  // cppcheck-suppress unreadVariable
  bool inCylindricalR = innerCylindricalR < cylindricalR and cylindricalR < outerCylindricalR;
  if (not inCylindricalR) return false;

  IWire iWire = CDC::CDCGeometryPar::Instance().cellId(iCLayer, pos3D);
  // Safety measure against error in the cellId function
  iWire %= wireLayer.size();
  bool inPhi = iWire == getIWire();
  return inPhi;
}

double CDCWire::getRadialCellWidth() const
{
  return getWireLayer().getRadialCellWidth();
}

double CDCWire::getLateralCellWidth() const
{
  return getWireLayer().getLateralCellWidth();
}

const CDCWireLayer& CDCWire::getWireLayer() const
{
  ILayer iCLayer = getICLayer();
  return CDCWireTopology::getInstance().getWireLayer(iCLayer);
}

const CDCWireSuperLayer& CDCWire::getWireSuperLayer() const
{
  ISuperLayer iSuperLayer = getISuperLayer();
  return CDCWireTopology::getInstance().getWireSuperLayer(iSuperLayer);
}

WireNeighborKind CDCWire::getNeighborKind(const CDCWire& wire) const
{
  return CDCWireTopology::getInstance().getNeighborKind(getWireID(), wire.getWireID());
}

bool CDCWire::isPrimaryNeighborWith(const CDCWire& wire) const
{
  return CDCWireTopology::getInstance().arePrimaryNeighbors(getWireID(), wire.getWireID());
}

WireNeighborPair CDCWire::getNeighborsInwards() const
{
  return CDCWireTopology::getInstance().getNeighborsInwards(getWireID());
}

WireNeighborPair CDCWire::getNeighborsOutwards() const
{
  return CDCWireTopology::getInstance().getNeighborsOutwards(getWireID());
}

MayBePtr<const CDCWire> CDCWire::getNeighborCCW() const
{
  return CDCWireTopology::getInstance().getNeighborCCW(getWireID());
}

MayBePtr<const CDCWire> CDCWire::getNeighborCW() const
{
  return CDCWireTopology::getInstance().getNeighborCW(getWireID());
}

MayBePtr<const CDCWire> CDCWire::getNeighborCCWInwards() const
{
  return CDCWireTopology::getInstance().getNeighborCCWInwards(getWireID());
}

MayBePtr<const CDCWire> CDCWire::getNeighborCWInwards() const
{
  return CDCWireTopology::getInstance().getNeighborCWInwards(getWireID());
}

MayBePtr<const CDCWire> CDCWire::getNeighborCCWOutwards() const
{
  return CDCWireTopology::getInstance().getNeighborCCWOutwards(getWireID());
}

MayBePtr<const CDCWire> CDCWire::getNeighborCWOutwards() const
{
  return CDCWireTopology::getInstance().getNeighborCWOutwards(getWireID());
}

MayBePtr<const CDCWire> CDCWire::getSecondaryNeighbor(short oClockDirection) const
{
  return CDCWireTopology::getInstance().getSecondaryNeighbor(oClockDirection, getWireID());
}
