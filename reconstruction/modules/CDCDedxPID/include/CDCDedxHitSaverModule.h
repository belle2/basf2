/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <reconstruction/dataobjects/CDCDedxHit.h>


namespace Belle2 {

  /**
   * Module that stores CDC hit information needed for dedx
   */
  class CDCDedxHitSaverModule : public Module {

  public:

    /** Default constructor */
    CDCDedxHitSaverModule();

    /** Destructor */
    virtual ~CDCDedxHitSaverModule();

    /** Initialize the module */
    virtual void initialize() override;

    /** This method is called for each event. All processing of the event takes place in this method. */
    virtual void event() override;

  private:

    StoreArray<Track> m_tracks; /**< required collection of tracks */
    StoreArray<RecoTrack> m_recoTracks; /**< required collection of reco tracks */
    StoreArray<CDCDedxHit> m_hits; /**< output collection of hits */

  };

} // Belle2 namespace
