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
#include <analysis/dataobjects/ParticleList.h>
#include <framework/dataobjects/EventT0.h>
#include <framework/datastore/StoreArray.h>
#include <string>

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
      StoreObjPtr<ParticleList> m_DiMuonList;       /**< List of the reconstructed dimion */

      std::string m_DiMuonListName;                  /**< List name for the reconstruted dimuon */
      std::string m_GammaListName;                  /**< List name for the reconstruted dimuon */
      std::string m_trackArrayName;           /**< Belle2::Track StoreArray name. */
      std::string m_trackFitResultArrayName;  /**< Belle2::TrackFitResult StoreArray name. */
      std::string m_treeName = "tree";                 /**< Name of tree for the output file. */

      Double_t m_minCollinearityTheta = 10; /**<Minimum requirement for accolinear theta in c.m frame */
      Double_t m_minCollinearityPhi0  = 10; /**<Minimum requirement for accolinear phi0 in c.m frame */
      bool m_StoreNtuple;            /**< Option to store ntuple, =true: tree with these variables will be stored. */
      Int_t expRun;                 /**< Exp and run numbers, encoded by exp*10^6+run. */
      Float_t ndfPos;               /**< NDF of the positive track. */
      Float_t ndfNeg;               /**< NDF of the negative track. */
      Float_t pvalPos;              /**< P-value of the positive track. */
      Float_t pvalNeg;              /**< P-value of the negative track. */
      Float_t ncdcPos;              /**< Number of CDC hit of the positive track. */
      Float_t ncdcNeg;              /**< Number of CDC hit of the negative track. */
      Float_t npxdPos;              /**< Number of PXD hit of the positive track. */
      Float_t npxdNeg;              /**< Number of PXD hit of the negative track. */
      Float_t nsvdPos;              /**< Number of SVD hit of the positive track. */
      Float_t nsvdNeg;              /**< Number of SVD hit of the negative track. */
      Float_t muidPos;              /**< Muon ID for positive charged track */
      Float_t muidNeg;              /**< Muon ID for negative charged track */
      Float_t eidPos;               /**< Electron ID for positive charged track */
      Float_t eidNeg;               /**< Electron ID for negative charged track */


      Float_t nExtraCDCHits;         /**< Number of CDC hits not assigned to any tracks */
      Float_t eclTrack;            /**< sum of ECL associated to track */
      Float_t eclNeutral;          /**< Sum of neutral ECL clusgter */

      Float_t d0Pos;                /**< d0 of the positive track  */
      Float_t d0Neg;                /**< d0 of the negative track  */
      Float_t z0Pos;                /**< z0 of the positive track  */
      Float_t z0Neg;                /**< z0 of the negative track  */
      Float_t d0ipPos;              /**< d0 w.r.t IP of the positive track  */
      Float_t d0ipNeg;              /**< d0 w.r.t IP of the negative track  */
      Float_t z0ipPos;              /**< z0 w.r.t IP of the positive track  */
      Float_t z0ipNeg;              /**< z0 w.r.t IP of the negative track  */
      Float_t ptPos;                /**< Transeverse momentum of the positive track  */
      Float_t ptNeg;                /**< Transeverse momentum of the negative track  */
      Float_t pzPos;                /**< Longitudinal momentum of the positive track  */
      Float_t pzNeg;                /**< Longitudinal momentum of the negative track  */

      Float_t ptPosCm;             /**< Transeverse momentum of the positive track  in c.m frame*/
      Float_t ptNegCm;             /**< Transeverse momentum of the negative track in c.m frame */
      Float_t pzPosCm;             /**< Longitudinal momentum of the positive track  in c.m frame*/
      Float_t pzNegCm;             /**< Longitudinal momentum of the negative track in c.m frame */

      Float_t thetaPosCm;          /**< theta of the positive track in c.m frame. */
      Float_t thetaNegCm;          /**< theta of the negative track in c.m frame. */
      Float_t phi0PosCm;           /**< phi0 of the positive track in c.m frame. */
      Float_t phi0NegCm;           /**< phi0 of the negative track in c.m frame. */

    };
  }
}
