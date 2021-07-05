/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFindingCDC/topology/CDCWireLayer.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <cdc/geometry/CDCGeometryPar.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCWireLayer::CDCWireLayer(const ConstVectorRange<CDCWire>& wireRange)
  : Super(wireRange)
{
  initialize();
}

const CDCWireLayer* CDCWireLayer::getInstance(ILayer iCLayer)
{
  return &(CDCWireTopology::getInstance().getWireLayer(iCLayer));
}

const CDCWireLayer* CDCWireLayer::getInstance(ISuperLayer iSuperLayer, ILayer iLayer)
{
  return &(CDCWireTopology::getInstance().getWireLayer(iSuperLayer, iLayer));
}

void CDCWireLayer::initialize()
{
  size_t nWiresInLayer = size();
  ILayer iCLayer = getICLayer();

  // values from CDCGeometryPar
  CDC::CDCGeometryPar& cdcgp = CDC::CDCGeometryPar::Instance();

  const double* innerRadiusWireLayer = cdcgp.innerRadiusWireLayer();
  const double* outerRadiusWireLayer = cdcgp.outerRadiusWireLayer();

  m_innerCylindricalR = innerRadiusWireLayer[iCLayer];
  m_outerCylindricalR = outerRadiusWireLayer[iCLayer];

  /// Set the numbering shift
  m_shift = ERotation(cdcgp.getShiftInSuperLayer(getISuperLayer(), getILayer()));

  // average values from wires
  m_tanStereoAngle = 0.0;
  m_minCylindricalR = 1000000.0;
  m_refZ = 0.0;
  m_refCylindricalR = 0.0;
  m_forwardCylindricalR = 0.0;
  m_backwardCylindricalR = 0.0;
  m_forwardZ = 0.0;
  m_backwardZ = 0.0;

  for (const CDCWire& wire : *this) {
    m_tanStereoAngle += wire.getTanStereoAngle();

    double minROfWire = wire.getMinCylindricalR();
    m_minCylindricalR = std::min(minROfWire, m_minCylindricalR);

    m_refZ += wire.getRefZ();
    m_refCylindricalR += wire.getRefCylindricalR();

    // calculate the forward nad backward r in the xy projection. take the average
    m_forwardCylindricalR  += wire.getForwardCylindricalR();
    m_backwardCylindricalR += wire.getBackwardCylindricalR();

    // calculate the forward and backward z position. take the average of all wires
    m_forwardZ += wire.getForwardZ();
    m_backwardZ += wire.getBackwardZ();
  }

  m_tanStereoAngle /= nWiresInLayer;

  m_refZ /= nWiresInLayer;
  m_refCylindricalR /= nWiresInLayer;

  m_forwardCylindricalR /= nWiresInLayer;
  m_backwardCylindricalR /= nWiresInLayer;

  m_forwardZ /= nWiresInLayer;
  m_backwardZ /= nWiresInLayer;
}

const CDCWire& CDCWireLayer::getClosestWire(const Vector3D& pos3D) const
{
  IWire iWire = CDC::CDCGeometryPar::Instance().cellId(getICLayer(), pos3D);
  // Safety measure against error in the cellId function
  iWire %= size();
  return getWire(iWire);
}
