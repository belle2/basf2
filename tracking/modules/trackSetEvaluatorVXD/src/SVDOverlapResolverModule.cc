/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Jonas Wagner                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackSetEvaluatorVXD/SVDOverlapResolverModule.h>

#include <framework/logging/Logger.h>


#include <tracking/spacePointCreation/SpacePoint.h>

#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapMatrixCreator.h>
#include <tracking/trackFindingVXD/trackSetEvaluator/HopfieldNetwork.h>
#include <tracking/trackFindingVXD/trackSetEvaluator/Scrooge.h>
#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapResolverNodeInfo.h>

#include <vxd/geometry/SensorInfoBase.h>

#include <vector>

using namespace std;
using namespace Belle2;

REG_MODULE(SVDOverlapResolver)

SVDOverlapResolverModule::SVDOverlapResolverModule() : Module()
{
  setDescription("Module checks for overlaps of SpacePointTrackCands");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("NameSpacePointTrackCands", m_nameSpacePointTrackCands, "Name of expected StoreArray.", string(""));

  addParam("NameSVDClusters", m_nameSVDClusters, "Name of expected StoreArray.", string(""));

  addParam("ResolveMethod", m_resolveMethod, "Strategy used to resolve overlaps. Currently implemented are \"greedy\" and "
           " \"hopfield\" ", string("greedy"));

  addParam("minActivityState", m_minActivityState, "Sets the minimal value of activity for acceptance. (0,1)",
           float(0.7));
}


void SVDOverlapResolverModule::initialize()
{
  m_spacePointTrackCands.isRequired(m_nameSpacePointTrackCands);
  m_svdClusters.isRequired(m_nameSVDClusters);

  B2ASSERT("ResolveMethod has to be either 'greedy' or 'hopfield'. Selected ResolveMethod: " << m_resolveMethod,
           m_resolveMethod == "greedy" || m_resolveMethod == "hopfield");
}

void SVDOverlapResolverModule::event()
{

  //Create matrix[svdCluster][track]
  unsigned short nHits = m_svdClusters.getEntries();

  //Create subset of active Candidates
  vector<SpacePointTrackCand*> activeCandidates;
  auto requiredSpace = m_spacePointTrackCands.getEntries();
  if (m_estimatedActiveCandidates < m_spacePointTrackCands.getEntries()) {
    requiredSpace = m_estimatedActiveCandidates;
  }
  activeCandidates.reserve(requiredSpace);
  for (SpacePointTrackCand& sptc : m_spacePointTrackCands) {
    if (sptc.hasRefereeStatus(SpacePointTrackCand::c_isActive)) activeCandidates.push_back(&sptc);
  }
  unsigned short const nActiveCandidates = activeCandidates.size();
  if (nActiveCandidates < 2) {
    B2DEBUG(29, "Less than 2 active SPTC. No reason to do SVDOverlapResolver!");
    return;
  }

  //now fill the cluster/track matrix:
  vector<vector<unsigned short> > svdHitRelatedTracks(nHits);
  //TODO: Check if one saves time by reserving some space for each single of those vectors;
  for (unsigned short iCand = 0; iCand < nActiveCandidates; ++iCand) {

    for (const SpacePoint* spacePointPointer : activeCandidates.at(iCand)->getHits()) {
      //only SVD is handled with this algorithm
      if (spacePointPointer->getType() != VXD::SensorInfoBase::SensorType::SVD) continue;

      //at the position of the svdCluster Index, the track index is pushed back;
      RelationVector<SVDCluster> svdClusterRelations = spacePointPointer->getRelationsTo<SVDCluster>(m_nameSVDClusters);
      for (SVDCluster const& svdClusterPointer : svdClusterRelations) {
        svdHitRelatedTracks[svdClusterPointer.getArrayIndex()].push_back(iCand);
      }
    }
  }

  //Create the overlap matrix and store it into the OverlapNetwork
  OverlapMatrixCreator overlapMatrixCreator(svdHitRelatedTracks, nActiveCandidates);
  auto overlapMatrix = overlapMatrixCreator.getOverlapMatrix();

  //Resolve overlap
  //Create an empty object of the type,
  //that needs to be given to Scrooge.
  std::vector<OverlapResolverNodeInfo> qiTrackOverlap;
  qiTrackOverlap.reserve(nActiveCandidates);

  //fill this object with the necessary information:
  for (unsigned short iCand = 0; iCand < nActiveCandidates; ++iCand) {
    if (overlapMatrix.at(iCand).size()) {
      qiTrackOverlap.emplace_back(
        activeCandidates[iCand]->getQualityIndicator(),
        iCand,
        overlapMatrix.at(iCand),
        1);
    }
  }
  if (qiTrackOverlap.size() < 2) {
    B2DEBUG(29, "Less than 2 tracks overlap. Do not resolve overlaps.");
    return;
  }


  if (m_resolveMethod == "greedy") {
    //make a Scrooge and udpate the activity
    Scrooge scrooge;
    scrooge.performSelection(qiTrackOverlap);

  } else if (m_resolveMethod == "hopfield") {
    //Performs the actual HNN.
    //As the parameter is taken as reference, the values are changed and can be reused below.
    HopfieldNetwork hopfieldNetwork;
    unsigned maxIterations = 20;
    if (hopfieldNetwork.doHopfield(qiTrackOverlap, maxIterations) == maxIterations) {
      B2WARNING("Hopfield Network failed converge.");
    }
  }

  for (auto && track : qiTrackOverlap) {
    if (track.activityState < m_minActivityState) {
      activeCandidates[track.trackIndex]->removeRefereeStatus(SpacePointTrackCand::c_isActive);
    }
  }
}
