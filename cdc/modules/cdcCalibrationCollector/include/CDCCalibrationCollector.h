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
#include "TH1.h"
#include "TTree.h"
#include <string>

namespace Belle2 {
  namespace CDC {

    /**
     * Collect hit information for cdc calibration with CAF
     */
    class CDCCalibrationCollectorModule : public CalibrationCollectorModule {

    public:

      /**
       * Constructor.
       */
      CDCCalibrationCollectorModule();

      /**
       * Destructor.
       */
      virtual ~CDCCalibrationCollectorModule();

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


      /**
       * collect hit information of fitted track.
       */
      void harvest(Belle2::RecoTrack* track);

      StoreObjPtr<EventT0> m_eventTimeStoreObject; /**<Event t0 object */
      std::string m_trackArrayName;           /**< Belle2::Track StoreArray name. */
      std::string m_cdcHitArrayName ;         /**< Belle2::CDCHit StoreArray name. */
      std::string m_recoTrackArrayName ;      /**< Belle2::RecoTrack StoreArray nam.e */
      std::string m_trackFitResultArrayName;  /**< Belle2::TrackFitResult StoreArray name. */
      std::string m_relRecoTrackTrackName;    /**< Relation between RecoTrack and Belle2:Track. */
      std::string m_treeName;                 /**< Name of tree for the output file. */

      double weight;            /**<  Weight of hit.*/
      double alpha;             /**< Entrance Azimuthal angle of hit (degree). */
      double theta;             /**< Entrance Polar angle of hit (degree). */
      unsigned short adc;       /**< adc value. */
      double t;                 /**< Measurement Drift time. */
      double t_fit;             /**< Drift time calculated from x_fit. */
      double evtT0;             /**< event T0*/
      double x_mea;             /**< measure drift length (signed by left right).*/
      double x_u;               /**< X_fit for unbiased track fit.*/
      double x_b;               /**< X_fit for biased track fit.*/
      int lay;                  /**< Layer ID. */
      int IWire;                /**< Wire ID. */
      double Pval;              /**< P-value of fitted track.  */
      double ndf;               /**< degree of freedom. */
      double d0;                /**< Track Parameter, d0. */
      double z0;                /**< Track Parameter, z0. */
      double phi0;              /**< Track Parameter, phi0. */
      double tanL;              /**< Track Parameter, tanL. */
      double omega;             /**< Track Parameter, omega. */
      double m_MinimumPt = 0;   /**< minimum pt required for track*/

      bool m_calExpectedDriftTime;             /**< Calculate expected drift time from x_fit or not. */
      bool m_BField;                           /**< fit incase no magnetic Field of not, if false, NDF=4 in cal P-value */
      bool m_StoreTrackParams;                 /**< Store Track parameter or not. */
      bool m_EventT0Extraction;                /**< use Event T0 extract t0 or not. */
    };
  }
}
