/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

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
