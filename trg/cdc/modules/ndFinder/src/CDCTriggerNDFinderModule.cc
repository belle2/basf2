/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/cdc/modules/ndFinder/CDCTriggerNDFinderModule.h"

using namespace Belle2;

REG_MODULE(CDCTriggerNDFinder);

CDCTriggerNDFinderModule::CDCTriggerNDFinderModule() : Module()
{
  setDescription("CDC Trigger NDFinder Module.\n"
                 "Implements a 3D Hough transformation for \n"
                 "3D track finding in omega, phi, theta. \n"
                 "Uses trained hit patterns for axial and \n"
                 "stereo TS and a density based clustering \n"
                 "algorithm.\n");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("TrackSegmentHitsName", m_TrackSegmentHitsName,
           "The name of the StoreArray of the CDCTriggerSegmentHits.",
           std::string("CDCTriggerSegmentHits"));
  addParam("NDFinderTracksName", m_NDFinderTracksName,
           "The name of the StoreArray where the tracks found by this NDFinder Module are stored.",
           std::string("CDCTrigger3DFinderTracks"));
  addParam("minSuperAxial", m_minSuperAxial,
           "Cluster pruning: Minimum number of axial super layer hits related to a cluster "
           "for the cluster to be considered as a track.",
           4);
  addParam("minSuperStereo", m_minSuperStereo,
           "Cluster pruning: Minimum number of stereo super layer hits related to a cluster "
           "for the cluster to be considered as a track.",
           3);
  addParam("thresh", m_thresh,
           "Track estimation: Minimum weight of a cluster member cell "
           "relative to the peak weight of the cluster "
           "for the cell to enter in the weighted mean "
           "track parameter value estimation.",
           0.85);
  addParam("minTotalWeight", m_minTotalWeight,
           "Clustering: minimum total weight of all cells in the 3d volume.",
           450);
  addParam("minPeakWeight", m_minPeakWeight,
           "Clustering: minimum peak cell weight of a cluster.",
           32);
  addParam("iterations", m_iterations,
           "Clustering: Number of iterations for the cluster finding in one Hough space quadrant.",
           2);
  addParam("omegaTrim", m_omegaTrim,
           "Clustering: Number of deleted cells in each omega direction of the maximum.",
           5);
  addParam("phiTrim", m_phiTrim,
           "Clustering: Number of deleted cells in each phi direction of the maximum.",
           4);
  addParam("thetaTrim", m_thetaTrim,
           "Clustering: Number of deleted cells in each theta direction of the maximum.",
           4);
  addParam("verbose", m_verbose,
           "Print Hough planes and verbose output. ",
           false);
  addParam("axialFile", m_axialFile,
           "File name of the axial hit patterns. ",
           std::string("data/trg/cdc/ndFinderAxialShallow.txt.gz"));
  addParam("stereoFile", m_stereoFile,
           "File name of the stereo hit patterns. ",
           std::string("data/trg/cdc/ndFinderStereoShallow.txt.gz"));
  addParam("NDFinderInfosName", m_NDFinderInfosName,
           "The name of the StoreArray where the tracks clusters found by this NDFinder Module are stored.",
           std::string("CDCTriggerClusterInfos"));
}

CDCTriggerNDFinderModule::~CDCTriggerNDFinderModule() {}

void CDCTriggerNDFinderModule::initialize()
{
  B2DEBUG(25, "CDCTriggerNDFinderModule initialize, " <<
          ", m_minSuperAxial=" << m_minSuperAxial <<
          ", m_minSuperStereo=" << m_minSuperStereo <<
          ", m_thresh= " << m_thresh <<
          ", m_minTotalWeight=" << m_minTotalWeight <<
          ", m_minPeakWeight=" << m_minPeakWeight <<
          ", m_iterations=" << m_iterations <<
          ", m_omegaTrim=" << m_omegaTrim <<
          ", m_phiTrim=" << m_phiTrim <<
          ", m_thetaTrim=" << m_thetaTrim <<
          ", m_verbose= " << m_verbose);
  m_TrackSegmentHits.isRequired(m_TrackSegmentHitsName);
  m_NDFinderTracks.registerInDataStore(m_NDFinderTracksName);
  m_NDFinderTracks.registerRelationTo(m_TrackSegmentHits);
  m_NDFinderInfos.registerInDataStore(m_NDFinderInfosName);
  m_NDFinderTracks.registerRelationTo(m_NDFinderInfos);
  m_NDFinder.init(m_minSuperAxial, m_minSuperStereo, m_thresh,
                  m_minTotalWeight, m_minPeakWeight, m_iterations,
                  m_omegaTrim, m_phiTrim, m_thetaTrim, m_verbose,
                  m_axialFile, m_stereoFile);
}

void CDCTriggerNDFinderModule::beginRun() {}

void CDCTriggerNDFinderModule::event()
{
  m_NDFinder.reset();
  for (CDCTriggerSegmentHit& hit : m_TrackSegmentHits) {
    m_NDFinder.addHit(hit.getSegmentID(), hit.getISuperLayer(), hit.getPriorityPosition(), hit.priorityTime());
  }
  m_NDFinder.findTracks();

  std::vector<NDFinderTrack>* resultTracks = m_NDFinder.getFinderTracks();
  for (NDFinderTrack trackND : *resultTracks) {
    const CDCTriggerTrack* NDFinderTrack =
      m_NDFinderTracks.appendNew(trackND.getPhi0(), trackND.getOmega(),
                                 0., 0., trackND.getCot(), 0.);
    SimpleCluster Cluster = trackND.getCluster();
    std::vector<ROOT::Math::XYZVector> houghspace = trackND.getHoughSpace();
    std::vector<ROOT::Math::XYZVector> ndreadout = trackND.getNDReadout();
    const CDCTrigger3DFinderInfo* NDFinderInfo =
      m_NDFinderInfos.appendNew(houghspace, ndreadout);
    NDFinderTrack->addRelationTo(NDFinderInfo);
    std::vector<unsigned short> relHits = trackND.getRelHits();
    for (ulong i = 0; i < relHits.size(); i++) {
      NDFinderTrack->addRelationTo(m_TrackSegmentHits[relHits[i]]);
    }
  }
}

void CDCTriggerNDFinderModule::endRun() {}

void CDCTriggerNDFinderModule::terminate() {}
