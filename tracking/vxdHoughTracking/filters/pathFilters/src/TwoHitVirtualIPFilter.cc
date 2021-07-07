/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/vxdHoughTracking/filters/pathFilters/TwoHitVirtualIPFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/geometry/BFieldManager.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/BeamSpot.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace vxdHoughTracking;

void TwoHitVirtualIPFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "cosRZCut"), m_param_cosRZCut,
                                "Cut on the absolute value of cosine between the vectors (oHit - cHit) and (cHit - iHit).",
                                m_param_cosRZCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "circleIPDistanceCut"), m_param_circleIPDistanceCut,
                                "Cut on the difference between circle radius and circle center to check whether the circle is compatible with passing through the IP.",
                                m_param_circleIPDistanceCut);
}

void TwoHitVirtualIPFilter::beginRun()
{
  const double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_threeHitVariables.setBFieldZ(bFieldZ);

  /// BeamSpot from DB
  DBObjPtr<BeamSpot> beamSpotDB;
  if (beamSpotDB.isValid()) {
    const B2Vector3D& BeamSpotPosition = (*beamSpotDB).getIPPosition();
    m_virtualIPPosition.SetXYZ(BeamSpotPosition.X(), BeamSpotPosition.Y(), BeamSpotPosition.Z());
  } else {
    m_virtualIPPosition.SetXYZ(0., 0., 0.);
  }
}


TrackFindingCDC::Weight
TwoHitVirtualIPFilter::operator()(const BasePathFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const VXDHoughState*>>& previousHits = pair.first;

  // Do nothing if path is too short or too long
  if (previousHits.size() != 1) {
    return NAN;
  }

  const B2Vector3D& lastHitPos    = previousHits.at(0)->getHit()->getPosition();
  const B2Vector3D& currentHitPos = pair.second->getHit()->getPosition();

  // filter expects hits from outer to inner
  m_threeHitVariables.setHits(lastHitPos, currentHitPos, m_virtualIPPosition);

  if (m_threeHitVariables.getCosAngleRZSimple() < m_param_cosRZCut) {
    return NAN;
  }

  const double circleDistanceIP = m_threeHitVariables.getCircleDistanceIP();

  if (circleDistanceIP > m_param_circleIPDistanceCut) {
    return NAN;
  }

  return fabs(1.0 / circleDistanceIP);
}
