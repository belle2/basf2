/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/cdc/modules/ndFinder/CDCTriggerNDFinderModule.h"

using namespace std;
using namespace Belle2;

REG_MODULE(CDCTriggerNDFinder)

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
           string("CDCTriggerSegmentHits"));
  addParam("NDFinderTracksName", m_NDFinderTracksName,
           "The name of the StoreArray where the tracks found by this NDFinder Module are stored.",
           string("CDCTrigger3DFinderTracks"));
  addParam("minsuper_axial", m_minsuper_axial,
           "Cluster pruning: Minimum number of axial super layer hits related to a cluster "
           "for the cluster to be considered as a track.",
           4);
  addParam("minsuper_stereo", m_minsuper_stereo,
           "Cluster pruning: Minimum number of stereo super layer hits related to a cluster "
           "for the cluster to be considered as a track.",
           3);
  addParam("clustercut", m_clustercut,
           "Hit to cluster relation small cluster deletion: "
           "If a cluster has fewer hits than the number of required track segments minus clustercut hits the small cluster is deleted",
           2);
  addParam("minweight", m_minweight,
           "Clustering: Minimum weight of a cell in Hough space "
           "for the cell to be considered as a cluster member.",
           24);
  addParam("minpts", m_minpts,
           "Clustering: Minimum number of neighbor cells with minweight "
           "for a cell to be considered a core cell.",
           1);
  addParam("thresh", m_thresh,
           "Track estimation: Minimum weight of a cluster member cell "
           "relative to the peak weight of the cluster "
           "for the cell to enter in the weighted mean "
           "track parameter value estimation.",
           0.85);
  addParam("minassign", m_minassign,
           "Hit to cluster assignment limit: "
           "Minimum relatively larger weight contribution to the largest cluster.",
           0.2);
  addParam("diagonal", m_diagonal,
           "Clustering: consider diagonal neighbors.",
           true);
  addParam("mincells", m_mincells,
           "Clustering: minimum number of cells for a cluster.",
           1);
  addParam("dbscanning", m_dbscanning,
           ".Clustering method: When true: dbscan, when false: fixed 3d volume.",
           false);
  addParam("mintotalweight", m_mintotalweight,
           "Clustering: minimum total weight of all cells in the 3d volume.",
           500);
  addParam("minpeakweight", m_minpeakweight,
           "Clustering: minimum peak cell weight of a cluster.",
           0);
  addParam("iterations", m_iterations,
           "Clustering: Number of iterations for the cluster finding in one Hough space.",
           5);
  addParam("omegatrim", m_omegatrim,
           "Clustering: Number of deleted cells in each omega direction of the maximum.",
           5);
  addParam("phitrim", m_phitrim,
           "Clustering: Number of deleted cells in each phi direction of the maximum.",
           5);
  addParam("thetatrim", m_thetatrim,
           "Clustering: Number of deleted cells in each theta direction of the maximum.",
           5);
  addParam("verbose", m_verbose,
           "Print Hough planes and verbose output. ",
           false);
  addParam("axialFile", m_axialFile,
           "File name of the axial hit patterns. ",
           string("data/trg/cdc/ndFinderAxialShallow.txt.gz"));
  addParam("stereoFile", m_stereoFile,
           "File name of the stereo hit patterns. ",
           string("data/trg/cdc/ndFinderStereoShallow.txt.gz"));
  addParam("NDFinderInfosName", m_NDFinderInfosName,
           "The name of the StoreArray where the tracks clusters found by this NDFinder Module are stored.",
           string("CDCTriggerClusterInfos"));
}

CDCTriggerNDFinderModule::~CDCTriggerNDFinderModule()
{
}

void CDCTriggerNDFinderModule::initialize()
{
  B2DEBUG(11, "CDCTriggerNDFinderModule initialize, m_minweight=" << m_minweight <<
          ", m_minpts=" << m_minpts << ", m_diagonal=" << m_diagonal <<
          ", m_minsuper_axial=" << m_minsuper_axial << ", m_minsuper_stereo=" << m_minsuper_stereo <<
          ", m_thresh= " << m_thresh <<
          ", m_minassign=" << m_minassign <<
          ", m_clustercut=" << m_clustercut <<
          ", m_mincells=" << m_mincells <<
          ", m_dbscanning=" << m_dbscanning <<
          ", m_mintotalweight=" << m_mintotalweight <<
          ", m_minpeakweight=" << m_minpeakweight <<
          ", m_iterations=" << m_iterations <<
          ", m_omegatrim=" << m_omegatrim <<
          ", m_phitrim=" << m_phitrim <<
          ", m_thetatrim=" << m_thetatrim <<
          ", m_verbose= " << m_verbose);
  m_TrackSegmentHits.isRequired(m_TrackSegmentHitsName);
  m_NDFinderTracks.registerInDataStore(m_NDFinderTracksName);
  m_NDFinderTracks.registerRelationTo(m_TrackSegmentHits);
  m_NDFinderInfos.registerInDataStore(m_NDFinderInfosName);
  m_NDFinderTracks.registerRelationTo(m_NDFinderInfos);
  m_NDFinder.init(m_minweight, m_minpts, m_diagonal, m_minsuper_axial, m_minsuper_stereo,
                  m_thresh, m_minassign, m_clustercut, m_mincells, m_dbscanning, m_mintotalweight, m_minpeakweight, m_iterations,
                  m_omegatrim, m_phitrim, m_thetatrim, m_verbose,
                  m_axialFile, m_stereoFile);
  m_NDFinder.printParams();
}

void CDCTriggerNDFinderModule::beginRun()
{
}

void CDCTriggerNDFinderModule::event()
{
  m_NDFinder.reset();
  for (CDCTriggerSegmentHit& hit : m_TrackSegmentHits) {
    m_NDFinder.addHit(hit.getSegmentID(), hit.getISuperLayer(), hit.getPriorityPosition(), hit.priorityTime());
  }
  m_NDFinder.findTracks();

  vector<NDFinderTrack>* resultTracks = m_NDFinder.getFinderTracks();
  for (NDFinderTrack trackND : *resultTracks) {
    const CDCTriggerTrack* NDFinderTrack =
      m_NDFinderTracks.appendNew(trackND.getPhi0(), trackND.getOmega(),
                                 0., 0., trackND.getCot(), 0.);
    SimpleCluster Cluster = trackND.getCluster();
    std::vector<TVector3> houghspace = trackND.getHoughSpace();
    std::vector<TVector3> ndreadout = trackND.getNDReadout();
    const CDCTrigger3DFinderInfo* NDFinderInfo =
      m_NDFinderInfos.appendNew(houghspace, ndreadout);
    NDFinderTrack->addRelationTo(NDFinderInfo);
    vector<unsigned short> relHits = trackND.get_relHits();
    for (ulong i = 0; i < relHits.size(); i++) {
      NDFinderTrack->addRelationTo(m_TrackSegmentHits[relHits[i]]);
    }
  }
}

void CDCTriggerNDFinderModule::endRun()
{
}

void CDCTriggerNDFinderModule::terminate()
{
  m_NDFinder.printParams();
}
