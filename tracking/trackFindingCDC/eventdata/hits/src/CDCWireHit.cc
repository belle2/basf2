/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/topology/CDCWireSuperLayer.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/topology/EStereoKind.h>

#include <tracking/trackFindingCDC/geometry/Circle2D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/numerics/ERightLeft.h>
#include <tracking/trackFindingCDC/numerics/Index.h>

#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>

#include <cdc/dataobjects/TDCCountTranslatorBase.h>
#include <cdc/dataobjects/ADCCountTranslatorBase.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/WireID.h>
#include <framework/logging/Logger.h>

#include <ostream>

using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;

TDCCountTranslatorBase& CDCWireHit::getTDCCountTranslator()
{
  static CDC::RealisticTDCCountTranslator s_tdcCountTranslator;
  return s_tdcCountTranslator;
}

ADCCountTranslatorBase& CDCWireHit::getADCCountTranslator()
{
  static CDC::LinearGlobalADCCountTranslator s_adcCountTranslator;
  return s_adcCountTranslator;
}

CDCWireHit::CDCWireHit(const CDCHit* const ptrHit,
                       const double driftLength,
                       const double driftLengthVariance,
                       const double chargeDeposit,
                       const double driftTime)
  : m_wireID(ptrHit->getID())
  , m_wire(CDCWire::getInstance(*ptrHit))
  , m_automatonCell(1)
  , m_refDriftLength(driftLength)
  , m_refDriftLengthVariance(driftLengthVariance)
  , m_refChargeDeposit(chargeDeposit)
  , m_refDriftTime(driftTime)
  , m_hit(ptrHit)
{
}

CDCWireHit::CDCWireHit(const CDCHit* const ptrHit,
                       TDCCountTranslatorBase* ptrTDCCountTranslator,
                       ADCCountTranslatorBase* ptrADCCountTranslator)
  : m_wireID(ptrHit->getID())
  , m_wire(ptrHit ? CDCWire::getInstance(*ptrHit) : nullptr)
  , m_automatonCell(1)
  , m_hit(ptrHit)
{
  if (not ptrHit) {
    B2ERROR("CDCWireHit constructor invoked with nullptr CDCHit");
    return;
  }
  const CDCHit& hit = *ptrHit;

  TDCCountTranslatorBase& tdcCountTranslator =
    ptrTDCCountTranslator ? *ptrTDCCountTranslator : getTDCCountTranslator();
  ADCCountTranslatorBase& adcCountTranslator =
    ptrADCCountTranslator ? *ptrADCCountTranslator : getADCCountTranslator();

  float initialTOFEstimate = 0;

  float refDriftLengthRight = tdcCountTranslator.getDriftLength(hit.getTDCCount(),
                              getWireID(),
                              initialTOFEstimate,
                              false, // bool leftRight
                              getWire().getRefZ());

  float refDriftLengthLeft = tdcCountTranslator.getDriftLength(hit.getTDCCount(),
                             getWireID(),
                             initialTOFEstimate,
                             true, // bool leftRight
                             getWire().getRefZ());

  m_refDriftTime = tdcCountTranslator.getDriftTime(hit.getTDCCount(),
                                                   getWireID(),
                                                   initialTOFEstimate,
                                                   getWire().getRefZ(),
                                                   hit.getADCCount());

  m_refDriftLength = (refDriftLengthLeft + refDriftLengthRight) / 2.0;

  m_refDriftLengthVariance = tdcCountTranslator.getDriftLengthResolution(m_refDriftLength,
                             getWireID(),
                             false, // bool leftRight ?
                             getWire().getRefZ());

  m_refChargeDeposit = adcCountTranslator.getCharge(hit.getADCCount(),
                                                    getWireID(),
                                                    false, // bool leftRight
                                                    getWire().getRefZ(),
                                                    0); // theta
}

CDCWireHit::CDCWireHit(const WireID& wireID,
                       const double driftLength,
                       const double driftLengthVariance,
                       const double chargeDeposit)
  : m_wireID(wireID)
  , m_wire(CDCWire::getInstance(wireID))
  , m_automatonCell(1)
  , m_refDriftLength(driftLength)
  , m_refDriftLengthVariance(driftLengthVariance)
  , m_refChargeDeposit(chargeDeposit)
  , m_hit(nullptr)
{
}

bool CDCWireHit::operator<(const CDCHit& hit)
{
  return this->getWireID().getEWire() < hit.getID();
}

bool TrackFindingCDC::operator<(const CDCWireHit& wireHit, const CDCWireSuperLayer& wireSuperLayer)
{
  return wireHit.getISuperLayer() < wireSuperLayer.getISuperLayer();
}

bool TrackFindingCDC::operator<(const CDCWireSuperLayer& wireSuperLayer, const CDCWireHit& wireHit)
{
  return wireSuperLayer.getISuperLayer() < wireHit.getISuperLayer();
}

bool TrackFindingCDC::operator<(const CDCWireHit& wireHit, const CDCHit& hit)
{
  return wireHit.getWireID().getEWire() < hit.getID();
}

bool TrackFindingCDC::operator<(const CDCHit& hit, const CDCWireHit& wireHit)
{
  return hit.getID() < wireHit.getWireID().getEWire();
}

const CDCWire& CDCWireHit::attachWire() const
{
  m_wire = CDCWire::getInstance(m_wireID);
  assert(m_wire);
  return *m_wire;
}

Vector2D CDCWireHit::reconstruct2D(const CDCTrajectory2D& trajectory2D) const
{
  const Vector2D& refPos2D = getRefPos2D();
  Vector2D recoPos2D = trajectory2D.getClosest(refPos2D);

  const Vector2D& wirePos2D = getWire().getRefPos2D();
  const double driftLength = getRefDriftLength();

  Vector2D disp2D = recoPos2D - wirePos2D;

  // Fix the displacement to lie on the drift circle.
  disp2D.normalizeTo(driftLength);
  return wirePos2D + disp2D;
}

Vector3D CDCWireHit::reconstruct3D(const CDCTrajectory2D& trajectory2D,
                                   const ERightLeft rlInfo,
                                   const double z) const
{
  const EStereoKind stereoType = getStereoKind();

  if (stereoType == EStereoKind::c_StereoV or stereoType == EStereoKind::c_StereoU) {
    const WireLine& wireLine = getWire().getWireLine();
    const double signedDriftLength = isValid(rlInfo) ? rlInfo * getRefDriftLength() : 0.0;
    return trajectory2D.reconstruct3D(wireLine, signedDriftLength, z);

  } else { /*if (stereoType == EStereoKind::c_Axial)*/
    const Vector2D recoPos2D = reconstruct2D(trajectory2D);
    // for axial wire we can not determine the z coordinate by looking at the xy projection only
    // we set it the basic assumption.
    return Vector3D(recoPos2D, z);
  }
}

Circle2D CDCWireHit::conformalTransformed(const Vector2D& relativeTo) const
{
  Circle2D driftCircle(getRefPos2D() - relativeTo, getRefDriftLength());
  driftCircle.conformalTransform();
  return driftCircle;
}

Index CDCWireHit::getStoreIHit() const
{
  return getHit() ? getHit()->getArrayIndex() : c_InvalidIndex;
}

const Vector2D& CDCWireHit::getRefPos2D() const
{
  return getWire().getRefPos2D();
}

const Vector3D& CDCWireHit::getRefPos3D() const
{
  return getWire().getRefPos3D();
}

double CDCWireHit::getRefCylindricalR() const
{
  return getWire().getRefCylindricalR();
}

std::ostream& TrackFindingCDC::operator<<(std::ostream& output, const CDCWireHit& wirehit)
{
  return output << "CDCWireHit(" << wirehit.getWireID()
         << ", drift length=" << wirehit.getRefDriftLength() << ")";
}
