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

#include <string>

namespace Belle2 {

  /** The module to deactivate the SpacePointTrackCandidates with less than *minSVDSPs* SVD SpacePoints.
   * This module is used for combined SVD and PXD tracking, where SPTCs can be created from just PXD SPs
   * with none or few SVD SPs.
   *  */
  class PXDSVDCutModule : public Module {

  public:

    /** Constructor of the module. */
    PXDSVDCutModule();

    /** Initializes the Module. */
    void initialize() override;

    /** Applies the selected quality estimation method for a given set of TCs */
    void event() override;


  protected:

    // module parameters

    /** Minimum number of SVD SpacePointss to keep a SpacePointTrackCandidate */
    int m_minSVDSPs;

    /** sets the name of the expected StoreArray containing SpacePointTrackCands */
    std::string m_SpacePointTrackCandsStoreArrayName;

    // member variables

    /** the storeArray for SpacePointTrackCands as member, is faster than recreating link for each event */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;
  };
}
