/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/cdc/modules/ndFinder/CDCTriggerNDFinderModule.h"

#include <string>
#include <vector>
#include <array>
#include <cmath>

#include "trg/cdc/NDFinder.h"
#include "trg/cdc/NDFinderPeakFinder.h"

using namespace Belle2;

REG_MODULE(CDCTriggerNDFinder);

CDCTriggerNDFinderModule::CDCTriggerNDFinderModule() : Module()
{
  setDescription("CDC Trigger NDFinder Module.\n"
                 "Implements a 3D Hough transformation for \n"
                 "3D track finding in omega, phi, cot. \n"
                 "Uses numeric hit patterns for axial and \n"
                 "stereo TS.\n");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("TrackSegmentHitsName", m_trackSegmentHitsName,
           "The name of the StoreArray of the CDCTriggerSegmentHits.",
           std::string("CDCTriggerSegmentHits"));
  addParam("NDFinderTracksName", m_ndFinderTracksName,
           "The name of the StoreArray where the tracks found by this NDFinder Module are stored.",
           std::string("TRGCDCNDFinderTracks"));
  addParam("minSuperAxial", m_minSuperAxial,
           "Peak selection: Minimum number of axial super layer hits related to a peak "
           "for the peak to be considered as a track.",
           static_cast<unsigned short>(3));
  addParam("minSuperStereo", m_minSuperStereo,
           "Peak selection: Minimum number of stereo super layer hits related to a peak "
           "for the peak to be considered as a track.",
           static_cast<unsigned short>(2));
  addParam("iterations", m_iterations,
           "Peak finding: Number of iterations for the peak finding in one Hough space section.",
           static_cast<unsigned short>(1));
  addParam("omegaTrim", m_omegaTrim,
           "Peak finding: Number of deleted cells in each omega direction of the maximum.",
           static_cast<unsigned short>(5));
  addParam("phiTrim", m_phiTrim,
           "Peak finding: Number of deleted cells in each phi direction of the maximum.",
           static_cast<unsigned short>(4));
  addParam("storeHoughSpace", m_storeHoughSpace,
           "Switch for saving the full Hough space.",
           false);
  addParam("axialFile", m_axialFile,
           "File name of the axial hit representations.",
           std::string(""));
  addParam("stereoFile", m_stereoFile,
           "File name of the stereo hit representations.",
           std::string(""));
}

CDCTriggerNDFinderModule::~CDCTriggerNDFinderModule() {}

void CDCTriggerNDFinderModule::initialize()
{
  m_trackSegmentHits.isRequired(m_trackSegmentHitsName);
  m_ndFinderTracks.registerInDataStore(m_ndFinderTracksName);
  m_ndFinderTracks.registerRelationTo(m_trackSegmentHits);
  NDFinderParameters ndFinderParameters = {
    m_minSuperAxial, m_minSuperStereo,
    m_iterations, m_omegaTrim, m_phiTrim,
    m_storeHoughSpace,
    m_axialFile, m_stereoFile
  };
  m_NDFinder.init(ndFinderParameters);
}

void CDCTriggerNDFinderModule::beginRun() {}

void CDCTriggerNDFinderModule::event()
{
  m_NDFinder.reset();

  for (CDCTriggerSegmentHit& hit : m_trackSegmentHits) {
    if (hit.getPriorityPosition() == 0) continue; // no hit
    HitInfo hitInfo = {
      hit.getSegmentID(),
      hit.getISuperLayer(),
      hit.priorityTime()
    };
    m_NDFinder.addHit(hitInfo);
  }

  m_NDFinder.findTracks();

  std::vector<RawFinderTrack>* rawFinderTracks = m_NDFinder.getFinderTracks();
  for (RawFinderTrack& rawFinderTrack : *rawFinderTracks) {
    // Set the Helix parameters of the 3DFinder rawFinderTrack
    constexpr double z = 0.0;
    CDCTrigger3DHTrack* ndFinderTrack = m_ndFinderTracks.appendNew(
                                          rawFinderTrack.phi, rawFinderTrack.omega, z, rawFinderTrack.cot);

    // Set the other parameters
    ndFinderTrack->setQuadrant(getNDFinderQuadrant(*ndFinderTrack));
    ndFinderTrack->setTotalMomentum(getNDFinderTotalMomentum(*ndFinderTrack));
    ndFinderTrack->setValidTrackBit(true);

    // Get the raw maximum and set it for HW comparison
    const HoughPeak& peak = rawFinderTrack.peak;
    unsigned int peakWeight = peak.weight;
    cell_index peakCell = peak.cell;
    std::array<int, 4> raw3DHMaximum = {
      static_cast<int>(peakCell[0]),
      static_cast<int>(peakCell[1]),
      static_cast<int>(peakCell[2]),
      static_cast<int>(peakWeight)
    };
    ndFinderTrack->setRaw3DHMaximum(raw3DHMaximum);

    // Add the hough space
    ndFinderTrack->setHoughSpace(std::move(rawFinderTrack.houghSpace));

    // Add the relations to the rawFinderTrack segments
    const std::vector<unsigned short>& relatedHits = peak.hits;
    std::array<unsigned short, 9> tsVector{0};
    for (unsigned short hitIdx = 0; hitIdx < relatedHits.size(); ++hitIdx) {
      ndFinderTrack->addRelationTo(m_trackSegmentHits[relatedHits[hitIdx]]);
      unsigned short superLayer = m_trackSegmentHits[relatedHits[hitIdx]]->getISuperLayer();
      tsVector[superLayer] = m_trackSegmentHits[relatedHits[hitIdx]]->getLeftRight();
    }
    ndFinderTrack->setTSVector(tsVector);
  }
}

void CDCTriggerNDFinderModule::endRun() {}

void CDCTriggerNDFinderModule::terminate() {}

short CDCTriggerNDFinderModule::getNDFinderQuadrant(const CDCTrigger3DHTrack& ndFinderTrack)
{
  short quadrant = -1;
  const double phi = ndFinderTrack.getPhi0();
  if (phi >= -1 * M_PI_4 && phi <  1 * M_PI_4) { quadrant = 3; }
  else if (phi >=  1 * M_PI_4 && phi <  3 * M_PI_4) { quadrant = 0; }
  else if (phi >=  3 * M_PI_4 || phi < -3 * M_PI_4) { quadrant = 1; }
  else if (phi >= -3 * M_PI_4 && phi < -1 * M_PI_4) { quadrant = 2; }
  return quadrant;
}

double CDCTriggerNDFinderModule::getNDFinderTotalMomentum(const CDCTrigger3DHTrack& ndFinderTrack)
{
  double theta = std::atan2(1.0, ndFinderTrack.getCotTheta());
  if (theta < 0) theta += M_PI;
  double totalMomentum = ndFinderTrack.getPt() / std::sin(theta);
  return totalMomentum;
}
