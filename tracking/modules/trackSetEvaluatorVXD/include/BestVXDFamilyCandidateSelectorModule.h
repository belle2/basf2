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
  /** Module that selects the best candidate for each SPTC family.*/
  class BestVXDFamilyCandidateSelectorModule : public Module {
  public:
    /** Constructor of the module. */
    BestVXDFamilyCandidateSelectorModule();

    /** Requires SpacePointTrackCands. */
    void initialize() override final;

    /** Application of the cut. */
    void event() override final;

  protected:

    // parameters

    /** Name of input StoreArray containing SpacePointTrackCands */
    std::string m_nameSpacePointTrackCands;


    // member variables

    /** StoreArray for input SpacePointTrackCands*/
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;
  };
}
