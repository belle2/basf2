/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <mdst/dataobjects/Kink.h>

using namespace Belle2;

Kink::Kink(const std::pair<const Belle2::Track*,
           std::pair<const Belle2::TrackFitResult*, const Belle2::TrackFitResult*> >& trackPairMother,
           const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairDaughter,
           const Double32_t vertexX, const Double32_t vertexY, const Double32_t vertexZ, const short filterFlag) :
  m_fittedVertexX(vertexX),
  m_fittedVertexY(vertexY),
  m_fittedVertexZ(vertexZ),
  m_trackIndexMother(trackPairMother.first->getArrayIndex()),
  m_trackIndexDaughter(trackPairDaughter.first->getArrayIndex()),
  m_trackFitResultIndexMotherStart(trackPairMother.second.first->getArrayIndex()),
  m_trackFitResultIndexMotherEnd(trackPairMother.second.second->getArrayIndex()),
  m_trackFitResultIndexDaughter(trackPairDaughter.second->getArrayIndex()),
  m_filterFlag(filterFlag)
{}

Track* Kink::getMotherTrack() const
{
  // safety check in case of default constructor was used (should not)
  if (m_trackIndexMother == -1) return nullptr;
  StoreArray<Track> tracks;
  return tracks[m_trackIndexMother];
}

Track* Kink::getDaughterTrack() const
{
  // safety check in case of default constructor was used (should not)
  if (m_trackIndexDaughter == -1) return nullptr;
  StoreArray<Track> tracks;
  return tracks[m_trackIndexDaughter];
}

TrackFitResult* Kink::getMotherTrackFitResultStart() const
{
  // safety check in case of default constructor was used (should not)
  if (m_trackFitResultIndexMotherStart == -1) return nullptr;
  StoreArray <TrackFitResult> trackFitResults;
  return trackFitResults[m_trackFitResultIndexMotherStart];
}

TrackFitResult* Kink::getMotherTrackFitResultEnd() const
{
  // safety check in case of default constructor was used (should not)
  if (m_trackFitResultIndexMotherEnd == -1) return nullptr;
  StoreArray <TrackFitResult> trackFitResults;
  return trackFitResults[m_trackFitResultIndexMotherEnd];
}

TrackFitResult* Kink::getDaughterTrackFitResult() const
{
  // safety check in case of default constructor was used (should not)
  if (m_trackFitResultIndexDaughter == -1) return nullptr;
  StoreArray <TrackFitResult> trackFitResults;
  return trackFitResults[m_trackFitResultIndexDaughter];
}
