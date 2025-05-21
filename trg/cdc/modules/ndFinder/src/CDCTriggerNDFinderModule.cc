/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <string>
#include <vector>
#include <Math/Vector3D.h>
#include "trg/cdc/modules/ndFinder/CDCTriggerNDFinderModule.h"
#include "trg/cdc/NDFinder.h"

using namespace Belle2;

REG_MODULE(CDCTriggerNDFinder);

CDCTriggerNDFinderModule::CDCTriggerNDFinderModule() : Module()
{
  setDescription("CDC Trigger NDFinder Module.\n"
                 "Implements a 3D Hough transformation for \n"
                 "3D track finding in omega, phi, cot. \n"
                 "Uses trained hit patterns for axial and \n"
                 "stereo TS and a density based clustering \n"
                 "algorithm.\n");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("TrackSegmentHitsName", m_trackSegmentHitsName,
           "The name of the StoreArray of the CDCTriggerSegmentHits.",
           std::string("CDCTriggerSegmentHits"));
  addParam("NDFinderTracksName", m_ndFinderTracksName,
           "The name of the StoreArray where the tracks found by this NDFinder Module are stored.",
           std::string("CDCTrigger3DFinderTracks"));
  addParam("minSuperAxial", m_minSuperAxial,
           "Cluster pruning: Minimum number of axial super layer hits related to a cluster "
           "for the cluster to be considered as a track.",
           static_cast<unsigned short>(3));
  addParam("minSuperStereo", m_minSuperStereo,
           "Cluster pruning: Minimum number of stereo super layer hits related to a cluster "
           "for the cluster to be considered as a track.",
           static_cast<unsigned short>(2));
  addParam("minPeakWeight", m_minPeakWeight,
           "Clustering: minimum peak cell weight of a cluster.",
           static_cast<unsigned short>(20));
  addParam("iterations", m_iterations,
           "Clustering: Number of iterations for the cluster finding in one Hough space quadrant.",
           static_cast<unsigned short>(1));
  addParam("omegaTrim", m_omegaTrim,
           "Clustering: Number of deleted cells in each omega direction of the maximum.",
           static_cast<unsigned short>(5));
  addParam("phiTrim", m_phiTrim,
           "Clustering: Number of deleted cells in each phi direction of the maximum.",
           static_cast<unsigned short>(4));
  addParam("storeAdditionalReadout", m_storeAdditionalReadout,
           "Switch for writing the full Hough space and the cluster information to the 3DFinderInfo class.",
           false);
  addParam("axialFile", m_axialFile,
           "File name of the axial hit patterns (they are the same for the shallow representations).",
           std::string("data/trg/cdc/ndFinderAxial40x384x9.txt.gz"));
  addParam("stereoFile", m_stereoFile,
           "File name of the stereo hit patterns. Use ndFinderStereoSteep40x384x9.txt.gz"
           "for steep hit representations (change range in NDFinder.h).",
           std::string("data/trg/cdc/ndFinderStereoSteep40x384x9.txt.gz"));
  addParam("NDFinderInfosName", m_ndFinderInfosName,
           "The name of the StoreArray where the tracks clusters found by this NDFinder Module are stored.",
           std::string("CDCTriggerClusterInfos"));
}

CDCTriggerNDFinderModule::~CDCTriggerNDFinderModule() {}

void CDCTriggerNDFinderModule::initialize()
{
  m_trackSegmentHits.isRequired(m_trackSegmentHitsName);
  m_ndFinderTracks.registerInDataStore(m_ndFinderTracksName);
  m_ndFinderTracks.registerRelationTo(m_trackSegmentHits);
  m_ndFinderInfos.registerInDataStore(m_ndFinderInfosName);
  m_ndFinderTracks.registerRelationTo(m_ndFinderInfos);
  NDFinderParameters ndFinderParameters = {
    m_minSuperAxial, m_minSuperStereo,
    m_minPeakWeight, m_iterations,
    m_omegaTrim, m_phiTrim,
    m_storeAdditionalReadout,
    m_axialFile, m_stereoFile
  };
  m_NDFinder.init(ndFinderParameters);
}

void CDCTriggerNDFinderModule::beginRun() {}

void CDCTriggerNDFinderModule::event()
{
  m_NDFinder.reset();

  for (CDCTriggerSegmentHit& hit : m_trackSegmentHits) {
    HitInfo hitInfo = {
      hit.getSegmentID(),
      hit.getISuperLayer(),
      hit.getPriorityPosition(),
      hit.priorityTime()
    };
    m_NDFinder.addHit(hitInfo);
  }

  m_NDFinder.findTracks();

  std::vector<NDFinderTrack>* resultTracks = m_NDFinder.getFinderTracks();
  for (NDFinderTrack track : *resultTracks) {
    const CDCTriggerTrack* ndFinderTrack =
      m_ndFinderTracks.appendNew(track.getPhi0(), track.getOmega(),
                                 0., 0., track.getCot(), 0.);
    std::vector<ROOT::Math::XYZVector> readoutHoughSpace = track.getHoughSpace();
    std::vector<ROOT::Math::XYZVector> readoutCluster = track.getClusterReadout();
    const CDCTrigger3DFinderInfo* ndFinderInfo =
      m_ndFinderInfos.appendNew(readoutHoughSpace, readoutCluster);
    ndFinderTrack->addRelationTo(ndFinderInfo);
    std::vector<unsigned short> relatedHits = track.getRelatedHits();
    for (unsigned short hitIdx = 0; hitIdx < relatedHits.size(); ++hitIdx) {
      ndFinderTrack->addRelationTo(m_trackSegmentHits[relatedHits[hitIdx]]);
    }
  }
}

void CDCTriggerNDFinderModule::endRun() {}

void CDCTriggerNDFinderModule::terminate() {}
