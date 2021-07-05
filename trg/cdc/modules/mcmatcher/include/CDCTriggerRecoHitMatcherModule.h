/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef CDCTRIGGERRECOHITMATCHERMODULE_H
#define CDCTRIGGERRECOHITMATCHERMODULE_H

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <tracking/dataobjects/RecoTrack.h>
//#include <genfit/Track.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>

namespace Belle2 {

  /** A module to match CDCTriggerTracks to RecoTracks.
   */
  class CDCTriggerRecoHitMatcherModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    CDCTriggerRecoHitMatcherModule();

    /** Destructor. */
    virtual ~CDCTriggerRecoHitMatcherModule() {}

    /** Initialize the module. */
    virtual void initialize() override;

    /** Called once for each event. */
    virtual void event() override;

  protected:
    /** Name of the RecoTrack StoreArray to be matched */
    std::string m_RecoTrackCollectionName;
    /** Name of the MCParticle StoreArray to be matched */
    std::string m_MCParticleCollectionName;
    /** Name of a new StoreArray holding MCParticles considered as trackable */
    std::string m_MCTrackableCollectionName;
    /** Name of the StoreArray holding primary MCParticle matched to a RecoTrack, but the vertex and momentum is overridden with the RecoTrack values */
    std::string m_RecoTrackableCollectionName;
    /** Name of the StoreArray containing the hits that are used for the matching. */
    std::string m_hitCollectionName;
    /** Name of a new StoreArray holding Tracks */
    std::string m_TrackCollectionName;

    /** list of hits that are used for the matching */
    StoreArray<CDCTriggerSegmentHit> m_segmentHits;
    /** list of RecoTracks to be matched */
    StoreArray<RecoTrack> m_recoTracks;
    /** list of MCParticles to be matched */
    StoreArray<MCParticle> m_mcParticles;
    /** list of MCParticles considered as trackable */
    StoreArray<MCParticle> m_mcTracks;
    /** list of MCParticles where x and p is overriden with the matched RecoTrack values */
    StoreArray<MCParticle> m_recoTrackable;
    /** list of mdst Tracks */
    StoreArray<Track> m_mdstTracks;
  };
}
#endif
