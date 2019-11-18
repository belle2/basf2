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

#include <calibration/CalibrationCollectorModule.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <framework/dataobjects/EventT0.h>
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
      void prepare() override;


      /**
       * Event action, collect information for calibration.
       */

      void collect() override;

      /**
       * Termination action.
       */
      void finish() override;

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

      float weight;            /**<  Weight of hit.*/
      float alpha;             /**< Entrance Azimuthal angle of hit (degree). */
      float theta;             /**< Entrance Polar angle of hit (degree). */
      unsigned short adc;       /**< adc value. */
      float t;                 /**< Measurement Drift time. */
      float t_fit;             /**< Drift time calculated from x_fit. */
      float evtT0;             /**< event T0*/
      float x_mea;             /**< measure drift length (signed by left right).*/
      float x_u;               /**< X_fit for unbiased track fit.*/
      float x_b;               /**< X_fit for biased track fit.*/
      int lay;                  /**< Layer ID. */
      int IWire;                /**< Wire ID. */
      float Pval;              /**< P-value of fitted track.  */
      float ndf;               /**< degree of freedom. */
      float d0;                /**< Track Parameter, d0. */
      float z0;                /**< Track Parameter, z0. */
      float phi0;              /**< Track Parameter, phi0. */
      float tanL;              /**< Track Parameter, tanL. */
      float omega;             /**< Track Parameter, omega. */
      double m_minimumPt = 0;   /**< minimum pt required for track*/

      bool m_calExpectedDriftTime = true;      /**< Calculate expected drift time from x_fit or not. */
      bool m_bField = true;                    /**< fit incase no magnetic Field of not, if false, NDF=4 in cal P-value */
      bool m_storeTrackParams = true;          /**< Store Track parameter or not. */
      bool m_eventT0Extraction = true;         /**< use Event T0 extract t0 or not. */
      bool m_isCosmic = false;                 /**< true when we process cosmic events, else false (collision). */
    };
  }
}
