/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>



namespace Belle2 {

  /** Module that selects a subset with a fixed minimum qualityIndicator out of all SpacePointTrackCandidates.
   *
   *  Expects SpacePointTrackCandidates.
   *  Selects all candidates passing a qualityIndicator requirement
   *  Either deactivates the remaining candidates
   *  or fills a StoreArray with the selected candidates.
   *  If the target StoreArray is the same as the source StoreArray not matching candidates will be deleted.
   */
  class VXDTrackCandidatesQualityIndicatorCutterModule : public Module {
  public:
    /** Constructor of the module. */
    VXDTrackCandidatesQualityIndicatorCutterModule();

    /** Requires SpacePointTrackCands. */
    void initialize() override final;

    /** Application of the cut. */
    void event() override final;

  protected:

    /** Don't copy/delete candidates but rather deactivate them by setting a SpacePointTrackCandidate flag. */
    void deactivateCandidates();

    /** Copy or delete candidates to achieve a subset creation. */
    void selectSubset();

    // parameters

    /** selection criteria*/
    float m_minRequiredQuality;

    /** Name of input StoreArray containing SpacePointTrackCands */
    std::string m_nameSpacePointTrackCands;

    /** If True copy selected SpacePoints to new StoreArray,
     * If False deactivate remaining SpacePointTrackCands.
     */
    bool m_subsetCreation;

    /** Name of optional output StoreArray containing SpacePointTrackCands */
    std::string m_newNameSpacePointTrackCands;


    // member variables

    /** StoreArray for input SpacePointTrackCands*/
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    /** StoreArray for optional output SpacePointTrackCands*/
    StoreArray<SpacePointTrackCand> m_newSpacePointTrackCands;

    /** SubsetSelector operating on a custom selection criteria*/
    SelectSubset<SpacePointTrackCand> m_goodCandidates;
  };
}
