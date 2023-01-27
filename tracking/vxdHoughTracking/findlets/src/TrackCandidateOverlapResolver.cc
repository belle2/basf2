/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/vxdHoughTracking/findlets/TrackCandidateOverlapResolver.h>
#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapMatrixCreator.h>
#include <tracking/trackFindingVXD/trackSetEvaluator/HopfieldNetwork.h>
#include <tracking/trackFindingVXD/trackSetEvaluator/Scrooge.h>
#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapResolverNodeInfo.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace vxdHoughTracking;

TrackCandidateOverlapResolver::~TrackCandidateOverlapResolver() = default;


TrackCandidateOverlapResolver::TrackCandidateOverlapResolver() : Super()
{
}

void TrackCandidateOverlapResolver::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "ResolveMethod"), m_resolveMethod,
                                "Strategy used to resolve overlaps. Currently implemented are \"greedy\" and \"hopfield\".",
                                m_resolveMethod);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "NameSVDClusters"), m_nameSVDClusters,
                                "Name of expected SVDClusters StoreArray.", m_nameSVDClusters);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minActivityState"), m_minActivityState,
                                "Sets the minimal value of activity for acceptance. [0,1]", m_minActivityState);
}

void TrackCandidateOverlapResolver::initialize()
{
  Super::initialize();

  m_svdClusters.isRequired(m_nameSVDClusters);

  B2ASSERT("ResolveMethod has to be either 'greedy' or 'hopfield'. Selected ResolveMethod: " << m_resolveMethod,
           m_resolveMethod == "greedy" || m_resolveMethod == "hopfield");

}

void TrackCandidateOverlapResolver::apply(std::vector<SpacePointTrackCand>& spacePointTrackCandsToResolve)
{
  //Create matrix[svdCluster][track]
  unsigned short nHits = m_svdClusters.getEntries();

  //Create subset of active Candidates
  std::vector<SpacePointTrackCand*> activeCandidates;
  auto requiredSpace = spacePointTrackCandsToResolve.size();
  activeCandidates.reserve(requiredSpace);
  for (SpacePointTrackCand& sptc : spacePointTrackCandsToResolve) {
    if (sptc.hasRefereeStatus(SpacePointTrackCand::c_isActive)) activeCandidates.push_back(&sptc);
  }
  unsigned short const nActiveCandidates = activeCandidates.size();
  if (nActiveCandidates < 2) {
    B2DEBUG(29, "Less than 2 active SPTC. No reason to do SVDOverlapResolver!");
    return;
  }

  //now fill the cluster/track matrix:
  std::vector<std::vector<unsigned short> > svdHitRelatedTracks(nHits);
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
    qiTrackOverlap.emplace_back(
      activeCandidates[iCand]->getQualityIndicator(),
      iCand,
      overlapMatrix.at(iCand),
      1);
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

  for (auto&& track : qiTrackOverlap) {
    if (track.activityState < m_minActivityState) {
      activeCandidates[track.trackIndex]->removeRefereeStatus(SpacePointTrackCand::c_isActive);
    }
  }
}
