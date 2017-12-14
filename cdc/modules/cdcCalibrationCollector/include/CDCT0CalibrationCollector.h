/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC Group                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <calibration/CalibrationCollectorModule.h>

#include <framework/datastore/RelationIndex.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/dataobjects/EventT0.h>
#include <string>

namespace Belle2 {
  namespace CDC {

    /**
     * Collect hit information for cdc calibration with CAF
     */
    class CDCT0CalibrationCollectorModule : public CalibrationCollectorModule {

    public:

      /**
       * Constructor.
       */
      CDCT0CalibrationCollectorModule();

      /**
       * Destructor.
       */
      virtual ~CDCT0CalibrationCollectorModule();

      /**
       * Initializes the Module.
       */
      virtual void prepare();


      /**
       * Event action, collect information for calibration.
       */

      virtual void collect();

      /**
       * Termination action.
       */
      virtual void finish();

    private:


      StoreObjPtr<EventT0> m_eventTimeStoreObject; /**<Event t0 object */
      std::string m_trackArrayName;           /**< Belle2::Track StoreArray name. */
      std::string m_cdcHitArrayName ;         /**< Belle2::CDCHit StoreArray name. */
      std::string m_recoTrackArrayName ;      /**< Belle2::RecoTrack StoreArray nam.e */
      std::string m_trackFitResultArrayName;  /**< Belle2::TrackFitResult StoreArray name. */
      std::string m_relRecoTrackTrackName;    /**< Relation between RecoTrack and Belle2:Track. */
      double m_MinimumPt = 0;   /**< minimum pt required for track*/
      double m_PvalCut = 0;  /**< minimum pt required for track*/
      double m_ndfCut = 15;  /**< minimum pt required for track*/
      double m_xmin = 0.1; /**< Min drift length*/
      bool m_BField;       /**< fit in case no magnetic field or not, if false, NDF=4 in cal P-value */
      bool m_EventT0Extraction;                /**< use Event T0 extraction or not. */
      double halfCSize[56]; /**< Half cell size*/
    };
  }
}
