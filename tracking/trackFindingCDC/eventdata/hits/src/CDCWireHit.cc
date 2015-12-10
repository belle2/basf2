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

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <cdc/translators/SimpleTDCCountTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>

#include <cmath>

using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;

TDCCountTranslatorBase& CDCWireHit::getTDCCountTranslator()
{
  return CDCWireHitTopology::getInstance().getTDCCountTranslator();
}

CDCWireHit::CDCWireHit() :
  m_automatonCell(1)
{}

CDCWireHit::CDCWireHit(const CDCHit* const ptrHit, TDCCountTranslatorBase* ptrTranslator):
  m_automatonCell(1),
  m_wire(ptrHit ? CDCWire::getInstance(*ptrHit) : nullptr),
  m_hit(ptrHit)
{
  if (not ptrHit) {
    B2ERROR("CDCWireHit constructor invoked with nullptr CDCHit");
    return;
  }
  const CDCHit& hit = *ptrHit;

  TDCCountTranslatorBase& translator = ptrTranslator ? *ptrTranslator : getTDCCountTranslator();

  float initialTOFEstimate = 0;

  float refDriftLengthRight = translator.getDriftLength(hit.getTDCCount(),
                                                        getWireID(),
                                                        initialTOFEstimate,
                                                        false, //bool leftRight
                                                        getWire().getRefZ());

  float refDriftLengthLeft = translator.getDriftLength(hit.getTDCCount(),
                                                       getWireID(),
                                                       initialTOFEstimate,
                                                       true, //bool leftRight
                                                       getWire().getRefZ());


  m_refDriftLength = (refDriftLengthLeft + refDriftLengthRight) / 2.0;

  m_refDriftLengthVariance = translator.getDriftLengthResolution(m_refDriftLength,
                             getWireID(),
                             false, //bool leftRight ?
                             getWire().getRefZ());
}

CDCWireHit::CDCWireHit(const WireID& wireID,
                       const double driftLength,
                       const double driftLengthVariance):
  m_automatonCell(1),
  m_refDriftLength(driftLength),
  m_refDriftLengthVariance(driftLengthVariance),
  m_wire(CDCWire::getInstance(wireID)),
  m_hit(nullptr)
{}

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
  const EStereoType stereoType = getStereoType();

  if (stereoType == EStereoType::c_StereoV or stereoType == EStereoType::c_StereoU) {
    const WireLine& wireLine = getWire().getSkewLine();
    const double signedDriftLength = isValid(rlInfo) ? rlInfo * getRefDriftLength() : 0.0;
    return trajectory2D.reconstruct3D(wireLine, signedDriftLength);

  } else if (stereoType == EStereoType::c_Axial) {
    const Vector2D recoPos2D = reconstruct2D(trajectory2D);
    // for axial wire we can not determine the z coordinate by looking at the xy projection only
    // we set it the basic assumption.
    const double z = 0;
    return Vector3D(recoPos2D, z);
  }

  B2FATAL("Reconstructing on invalid stereo type " << static_cast<int>(stereoType));
}
