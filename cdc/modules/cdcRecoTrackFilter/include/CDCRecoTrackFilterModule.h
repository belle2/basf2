/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dong Van Thanh,...                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
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
      std::vector<unsigned short> m_excludeICLayer;        /**< IClayers (0-55) not used in the track fitting. */
    };
  }
}

