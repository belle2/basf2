/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackSetEvaluatorVXD/SVDOverlapCheckerModule.h>

#include <framework/logging/Logger.h>

#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapMatrixCreator.h>
#include <tracking/spacePointCreation/SpacePoint.h>

#include <vxd/geometry/SensorInfoBase.h>

#include <vector>

using namespace std;
using namespace Belle2;

REG_MODULE(SVDOverlapChecker)

SVDOverlapCheckerModule::SVDOverlapCheckerModule() : Module()
{
  setDescription("Module checks for overlaps of SpacePointTrackCands\
 and stores them in an OverlapNetwork, which is basically a matrix of overlaps.");

  addParam("NameSpacePointTrackCands", m_nameSpacePointTrackCands, "Name of expected StoreArray.", string(""));
}

void SVDOverlapCheckerModule::event()
{
  //Create matrix[svdCluster][track]
  unsigned short nHits = m_svdClusters.getEntries();
  vector<vector<unsigned short> > svdHitRelatedTracks(nHits);
  //TODO: Check if one saves time by reserving some space for each single of those vectors;

  //now fill the cluster/track matrix:
  unsigned short const nSpacePointTrackCandidates = m_spacePointTrackCands.getEntries();
  for (int ii = 0; ii < nSpacePointTrackCandidates; ii++) {

    for (auto && spacePointPointer : m_spacePointTrackCands[ii]->getHits()) {
      //only SVD is handled with this algorithm
      if (spacePointPointer->getType() != VXD::SensorInfoBase::SensorType::SVD) continue;

      //at the position of the svdCluster Index, the track index is pushed back;
      RelationVector<SVDCluster> svdClusterRelations = spacePointPointer->getRelationsTo<SVDCluster>();
      for (SVDCluster const& svdClusterPointer : svdClusterRelations) {
        svdHitRelatedTracks[svdClusterPointer.getArrayIndex()].push_back(ii);
      }
    }
  }

  //Create the overlap matrix and store it into the OverlapNetwork
  OverlapMatrixCreator overlapMatrixCreator(svdHitRelatedTracks, nSpacePointTrackCandidates);
  m_overlapNetwork.appendNew(OverlapNetwork(overlapMatrixCreator.getOverlapMatrix()));
}
