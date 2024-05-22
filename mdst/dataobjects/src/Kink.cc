/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <mdst/dataobjects/Kink.h>

using namespace Belle2;

Kink::Kink():
  m_trackIndexMother(-1),
  m_trackIndexDaughter(-1),
  m_trackFitResultIndexMotherStart(-1),
  m_trackFitResultIndexMotherEnd(-1),
  m_trackFitResultIndexDaughter(-1),
  m_fittedVertexX(0.0),
  m_fittedVertexY(0.0),
  m_fittedVertexZ(0.0),
  m_filterFlag(-1)
{}

Kink::Kink(const std::pair<const Belle2::Track*, std::pair<const Belle2::TrackFitResult*, const Belle2::TrackFitResult*> >&
           trackPairMother,
           const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairDaughter,
           Double32_t vertexX, Double32_t vertexY, Double32_t vertexZ, short filterFlag) :
  m_trackIndexMother(trackPairMother.first->getArrayIndex()),
  m_trackIndexDaughter(trackPairDaughter.first->getArrayIndex()),
  m_trackFitResultIndexMotherStart(trackPairMother.second.first->getArrayIndex()),
  m_trackFitResultIndexMotherEnd(trackPairMother.second.second->getArrayIndex()),
  m_trackFitResultIndexDaughter(trackPairDaughter.second->getArrayIndex()),
  m_fittedVertexX(vertexX),
  m_fittedVertexY(vertexY),
  m_fittedVertexZ(vertexZ),
  m_filterFlag(filterFlag)
{}

Track* Kink::getMotherTrack() const
{
  StoreArray<Track> tracks;
  return tracks[m_trackIndexMother];
}

Track* Kink::getDaughterTrack() const
{
  StoreArray<Track> tracks;
  return tracks[m_trackIndexDaughter];
}

TrackFitResult* Kink::getMotherTrackFitResultStart() const
{
  StoreArray <TrackFitResult> trackFitResults;
  return trackFitResults[m_trackFitResultIndexMotherStart];
}

TrackFitResult* Kink::getMotherTrackFitResultEnd() const
{
  StoreArray <TrackFitResult> trackFitResults;
  return trackFitResults[m_trackFitResultIndexMotherEnd];
}

TrackFitResult* Kink::getDaughterTrackFitResult() const
{
  StoreArray <TrackFitResult> trackFitResults;
  return trackFitResults[m_trackFitResultIndexDaughter];
}