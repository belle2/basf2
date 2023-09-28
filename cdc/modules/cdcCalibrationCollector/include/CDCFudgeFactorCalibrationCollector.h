/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternCDC.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <framework/dataobjects/EventT0.h>
#include <framework/datastore/StoreArray.h>
#include <string>
#include <TH1F.h>
#include <TTree.h>

namespace Belle2 {
  namespace CDC {

    /**
     * Collect hit information for cdc calibration with CAF
     */
    class CDCFudgeFactorCalibrationCollectorModule : public CalibrationCollectorModule {

    public:

      /**
       * Constructor.
       */
      CDCFudgeFactorCalibrationCollectorModule();

      /**
       * Destructor.
       */
      virtual ~CDCFudgeFactorCalibrationCollectorModule();

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
      StoreObjPtr<EventT0> m_eventTimeStoreObject;  /**< Event t0 object */
      StoreArray<Track> m_Tracks;                   /**< Tracks. */
      StoreArray<TrackFitResult> m_TrackFitResults; /**< Track fit results. */

      std::string m_trackArrayName;           /**< Belle2::Track StoreArray name. */
      std::string m_trackFitResultArrayName;  /**< Belle2::TrackFitResult StoreArray name. */
      std::string m_treeName = "tree";                 /**< Name of tree for the output file. */

      Double_t m_minCollinearityTheta = 10; /**<Minimum requirement for accolinear theta in c.m frame */
      Double_t m_minCollinearityPhi0  = 10; /**<Minimum requirement for accolinear phi0 in c.m frame */
      bool m_StoreNtuple;            /**< Option to store ntuple, =true: tree with these variables will be stored. */
      Int_t exp_run;                 /**< Exp and run numbers, encoded by exp*10^6+run. */
      Float_t ndf_pos;               /**< NDF of the positive track. */
      Float_t ndf_neg;               /**< NDF of the negative track. */
      Float_t Pval_pos;              /**< P-value of the positive track. */
      Float_t Pval_neg;              /**< P-value of the negative track. */

      Float_t D0_pos;                /**< d0 of the positive track  */
      Float_t D0_neg;                /**< d0 of the negative track  */
      Float_t Z0_pos;                /**< z0 of the positive track  */
      Float_t Z0_neg;                /**< z0 of the negative track  */
      Float_t D0ip_pos;              /**< d0 w.r.t IP of the positive track  */
      Float_t D0ip_neg;              /**< d0 w.r.t IP of the negative track  */
      Float_t Z0ip_pos;              /**< z0 w.r.t IP of the positive track  */
      Float_t Z0ip_neg;              /**< z0 w.r.t IP of the negative track  */
      Float_t Pt_pos;                /**< Transeverse momentum of the positive track  */
      Float_t Pt_neg;                /**< Transeverse momentum of the negative track  */

      Float_t Pt_pos_cm;             /**< Transeverse momentum of the positive track  in c.m frame*/
      Float_t Pt_neg_cm;             /**< Transeverse momentum of the negative track in c.m frame */
      Float_t Theta_pos_cm;          /**< theta of the positive track in c.m frame. */
      Float_t Theta_neg_cm;          /**< theta of the negative track in c.m frame. */
      Float_t Phi0_pos_cm;           /**< phi0 of the positive track in c.m frame. */
      Float_t Phi0_neg_cm;           /**< phi0 of the negative track in c.m frame. */

    };
  }
}
