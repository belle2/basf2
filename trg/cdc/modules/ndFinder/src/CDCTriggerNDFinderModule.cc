/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sebastian Skambraks                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
  addParam("minhits", m_minhits,
           "Cluster pruning: Minimum number of hits related to a cluster "
           "for the cluster to be considered as a track.",
           4);
  addParam("minhits_axial", m_minhits_axial,
           "Cluster pruning: Minimum number of axial hits related to a cluster "
           "for the cluster to be considered as a track.",
           0);
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
  addParam("verbose", m_verbose,
           "Print Hough planes and verbose output. ",
           false);
  addParam("axialFile", m_axialFile,
           "File name of the axial hit patterns. ",
           string("data/trg/cdc/ndFinderAxialShallow.txt.gz"));
  addParam("stereoFile", m_stereoFile,
           "File name of the stereo hit patterns. ",
           string("data/trg/cdc/ndFinderStereoShallow.txt.gz"));
}

CDCTriggerNDFinderModule::~CDCTriggerNDFinderModule()
{
}

void CDCTriggerNDFinderModule::initialize()
{
  B2DEBUG(11, "CDCTriggerNDFinderModule initialize, m_minweight=" << m_minweight <<
          ", m_minpts=" << m_minpts << ", m_diagonal=" << m_diagonal <<
          ", m_minhits=" << m_minhits << ", m_minhits_axial=" << m_minhits_axial <<
          ", m_thresh= " << m_thresh <<
          ", m_minassign=" << m_minassign <<
          ", m_mincells=" << m_mincells <<
          ", m_verbose= " << m_verbose);
  m_TrackSegmentHits.isRequired(m_TrackSegmentHitsName);
  m_NDFinderTracks.registerInDataStore(m_NDFinderTracksName);
  m_NDFinderTracks.registerRelationTo(m_TrackSegmentHits);
  m_NDFinder.init(m_minweight, m_minpts, m_diagonal, m_minhits,
                  m_minhits_axial,
                  m_thresh, m_minassign, m_mincells, m_verbose,
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
    m_NDFinder.addHit(hit.getSegmentID(), hit.getPriorityPosition());
  }
  m_NDFinder.findTracks();

  vector<NDFinderTrack>* resultTracks = m_NDFinder.getFinderTracks();
  for (NDFinderTrack trackND : *resultTracks) {
    const CDCTriggerTrack* NDFinderTrack =
      m_NDFinderTracks.appendNew(trackND.getPhi0(), trackND.getOmega(),
                                 0., 0., trackND.getCot(), 0.);
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
