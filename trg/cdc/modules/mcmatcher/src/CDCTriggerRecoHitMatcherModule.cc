/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "trg/cdc/modules/mcmatcher/CDCTriggerRecoHitMatcherModule.h"

#include <cdc/dataobjects/CDCHit.h>

#include <framework/gearbox/Const.h>

namespace {
  //small anonymous helper construct making converting a pair of iterators usable
  //with range based for
  template<class Iter>
  struct iter_pair_range : std::pair<Iter, Iter> {
    explicit iter_pair_range(std::pair<Iter, Iter> const& x) : std::pair<Iter, Iter>(x) {}
    Iter begin() const {return this->first;}
    Iter end()   const {return this->second;}
  };

  template<class Iter>
  inline iter_pair_range<Iter> as_range(std::pair<Iter, Iter> const& x)
  {
    return iter_pair_range<Iter>(x);
  }

  typedef int HitId;
  typedef int TrackId;
  typedef float Purity;
  typedef float Efficiency;
}



using namespace Belle2;
using namespace std;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(CDCTriggerRecoHitMatcher)

CDCTriggerRecoHitMatcherModule::CDCTriggerRecoHitMatcherModule() : Module()
{
  setDescription("A module to match CDCTriggerTracks to RecoTracks.\n"
                 "First makes relations from RecoTracks to CDCTriggerSegmentHits, "
                 "then makes relations from RecoTracks to CDCTriggerTracks "
                 "and vice-versa.");

  addParam("RecoTrackCollectionName", m_RecoTrackCollectionName,
           "Name of the RecoTrack StoreArray to be matched.",
           string("RecoTracks"));
  addParam("MCParticleCollectionName", m_MCParticleCollectionName,
           "Name of the MCParticle StoreArray to be matched.",
           string("MCParticles"));
  addParam("MCTrackableCollectionName", m_MCTrackableCollectionName,
           "Name of a new StoreArray holding MCParticles considered as trackable.",
           string("MCTracks"));
  addParam("RecoTrackableCollectionName", m_RecoTrackableCollectionName,

           "Name of the StoreArray holding primary MCParticle matched to a RecoTrack, but the vertex and momentum is overridden with the RecoTrack values.",
           string("RecoTrackable"));
  addParam("hitCollectionName", m_hitCollectionName,
           "Name of the StoreArray of CDCTriggerSegmentHits used for the matching.",
           string(""));
  addParam("TrackCollectionName", m_TrackCollectionName,
           "Name of the Track StoreArray.",
           string("Tracks"));
}


void
CDCTriggerRecoHitMatcherModule::initialize()
{
  m_segmentHits.isRequired(m_hitCollectionName);
  m_recoTracks.isRequired(m_RecoTrackCollectionName);
  m_recoTrackable.registerInDataStore(m_RecoTrackableCollectionName);
  m_mdstTracks.isRequired(m_TrackCollectionName);

  m_recoTracks.registerRelationTo(m_segmentHits);
  m_recoTrackable.registerRelationTo(m_segmentHits);
  m_recoTrackable.registerRelationTo(m_recoTracks);
}


void
CDCTriggerRecoHitMatcherModule::event()
{
  /*
  // create relations from RecoTracks to SegmentHits via CDCHits
  for (int ireco = 0; ireco < m_recoTracks.getEntries(); ++ireco) {
  RecoTrack* recoTrack = m_recoTracks[ireco];
  // if relations exist already, skip this step
  // (matching may be done several times with different trigger tracks)
  bool relateReco = true;
  if (recoTrack->getRelationsTo<CDCTriggerSegmentHit>(m_hitCollectionName).size() > 0)
    relateReco = false;
  if (recoTrack->getRelationsTo<Track>(m_TrackCollectionName).size() == 0)
    continue;
  Track* ttrack = recoTrack->getRelatedFrom<Track>(m_TrackCollectionName);
  const TrackFitResult* fitres = ttrack->getTrackFitResultWithClosestMass(Belle2::Const::ChargedStable(
                                   211));
  continue;
  if (fitres) {
    // double omega = fitres->getOmega();
    TVector3 mom = fitres->getMomentum();
    //   double pt = mom.Pt();
    //   double phi = mom.Phi();
    //   double theta = mom.theta();
    TVector3 vtx = fitres->getPosition();


    MCParticle* recoTrackable = m_recoTrackable.appendNew();
    recoTrackable->setProductionVertex(vtx);
    recoTrackable->setMomentum(mom);
    recoTrackable->addRelationTo(recoTrack);
  }

  vector<CDCHit*> cdcHits = recoTrack->getCDCHitList();
  for (unsigned iHit = 0; iHit < cdcHits.size(); ++iHit) {
    RelationVector<CDCTriggerSegmentHit> relHits =
      cdcHits[iHit]->getRelationsFrom<CDCTriggerSegmentHit>(m_hitCollectionName);
    for (unsigned iTS = 0; iTS < relHits.size(); ++iTS) {
      // create relations only for priority hits (relation weight 2)
      if (relHits.weight(iTS) > 1) {
        if (relateReco)
          recoTrack->addRelationTo(relHits[iTS]);
      }
    }
  }
  }
  */
}
