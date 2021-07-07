/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef CDCTRIGGERRECOMATCHERMODULE_H
#define CDCTRIGGERRECOMATCHERMODULE_H

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {

  /** A module to match CDCTriggerTracks to RecoTracks.
   */
  class CDCTriggerRecoMatcherModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    CDCTriggerRecoMatcherModule();

    /** Destructor. */
    virtual ~CDCTriggerRecoMatcherModule() {}

    /** Initialize the module. */
    virtual void initialize() override;

    /** Called once for each event. */
    virtual void event() override;

  protected:
    /** Name of the RecoTrack StoreArray to be matched */
    std::string m_RecoTrackCollectionName;
    /** Name of the CDCTriggerTrack Store Array to be matched */
    std::string m_TrgTrackCollectionName;
    /** Name of the StoreArray containing the hits that are used for the matching. */
    std::string m_hitCollectionName;
    /** switch for 2D matching */
    bool m_axialOnly;
    /** minimum purity */
    double m_minPurity;
    /** minimum efficiency */
    double m_minEfficiency;
    /** switch for creating relations for clones and merged tracks */
    bool m_relateClonesAndMerged;
    /** switch for creating hit relations based on wire ID */
    bool m_relateHitsByID;

    /** list of hits that are used for the matching */
    StoreArray<CDCTriggerSegmentHit> m_segmentHits;
    /** list of CDCTriggerTracks to be matched */
    StoreArray<CDCTriggerTrack> m_trgTracks;
    /** list of RecoTracks to be matched */
    StoreArray<RecoTrack> m_recoTracks;
  };
}
#endif
