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

#include <cdc/translators/SimpleTDCCountTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>

#include <cmath>

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

CDCWireHit::CDCWireHit()
{
}

CDCWireHit::CDCWireHit(const CDCHit* const ptrHit,
                       const double driftLength,
                       const double driftLengthVariance,
                       const double chargeDeposit)
  : m_wireID(ptrHit->getID())
  , m_wire(CDCWire::getInstance(*ptrHit))
  , m_automatonCell(1)
  , m_refDriftLength(driftLength)
  , m_refDriftLengthVariance(driftLengthVariance)
  , m_refChargeDeposit(chargeDeposit)
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

Vector3D CDCWireHit::reconstruct3D(const CDCTrajectory2D& trajectory2D, const ERightLeft rlInfo) const
{
  const EStereoKind stereoType = getStereoKind();

  if (stereoType == EStereoKind::c_StereoV or stereoType == EStereoKind::c_StereoU) {
    const WireLine& wireLine = getWire().getWireLine();
    const double signedDriftLength = isValid(rlInfo) ? rlInfo * getRefDriftLength() : 0.0;
    return trajectory2D.reconstruct3D(wireLine, signedDriftLength);

  } else { /*if (stereoType == EStereoKind::c_Axial)*/
    const Vector2D recoPos2D = reconstruct2D(trajectory2D);
    // for axial wire we can not determine the z coordinate by looking at the xy projection only
    // we set it the basic assumption.
    const double z = 0;
    return Vector3D(recoPos2D, z);
  }
}

Circle2D CDCWireHit::conformalTransformed(const Vector2D& relativeTo) const
{
  Circle2D driftCircle(getRefPos2D() - relativeTo, getRefDriftLength());
  driftCircle.conformalTransform();
  return driftCircle;
}
