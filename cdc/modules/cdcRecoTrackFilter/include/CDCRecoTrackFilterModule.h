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
#include <tracking/dataobjects/RecoTrack.h>
#include <string>

namespace Belle2 {
  namespace CDC {

    /**
     * The module excluding hits of specified Slayers in the RecoTracks.
     */

    class CDCRecoTrackFilterModule : public Module {

    public:
      /**
       * Constructor.
       */
      CDCRecoTrackFilterModule();

      /**
       * Destructor.
       */
      virtual ~CDCRecoTrackFilterModule();


      /**
       * Initializes the Module.
       */
      void initialize() override;

      /**
       * Begin run action.
       */
      void beginRun() override;
      /**
       * Event action (main routine).
       */
      void event() override;
      /**
       * End run action.
       */
      void endRun() override;
      /**
       * Termination action.
       */
      void terminate() override;


    private:
      std::string m_recoTrackArrayName ;                  /**< Belle2::RecoTrack StoreArray name */
      std::vector<unsigned short> m_excludeSLayer;        /**< Super layers (0-8) not used in the track fitting. */
      std::vector<unsigned short> m_excludeICLayer;       /**< IClayers (0-55) not used in the track fitting. */
      StoreArray<RecoTrack> m_RecoTracks;                 /**< Tracks. */
    };
  }
}

