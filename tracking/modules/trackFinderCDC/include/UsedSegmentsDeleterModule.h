/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCSegment2D;
    class CDCTrack;
  }

  namespace TrackFindingCDC {
    /**
     * Small helper module to release all fully taken segments (all containing hits do have the taken flag)
     * from the data store.
     */
    class UsedSegmentsDeleterModule : public Module {

    public:
      /**
       * Constructor.
       */
      UsedSegmentsDeleterModule();

      /**
       * Initialize the store obj pointer.
       */
      void initialize() override;

      /**
       * Delete all fully used segments.
       */
      void event() override;

    private:
      /// Parameter: Name of the output StoreObjPtr of the segments to be handled by this module.
      std::string m_param_segmentsStoreObjName;
    };
  }
}
