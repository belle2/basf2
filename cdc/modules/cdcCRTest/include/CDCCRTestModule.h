/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>

#include <cdc/dataobjects/WireID.h>

#include <string>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/dataobjects/EventT0.h>
#include <framework/datastore/StoreArray.h>
#include <framework/geometry/B2Vector3.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TTree.h>

namespace Belle2 {

  namespace CDC {

    /**
     * CDC Cosmic test calibration module.
     */
    class CDCCRTestModule : public HistoModule {

    public:

      /**
       * Constructor.
       */
      CDCCRTestModule();

      /**
       * Destructor.
       */
      virtual ~CDCCRTestModule() override;

      /**
       * Initializes the Module.
       */
      void initialize() override;

      /**
       * Event action (main routine).
       */

      void event() override;

      /**
       *  Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
       *  to be placed in this function.
       */
      void defineHisto() override;

    private:

      /**
       * Create 1D histogram.
       */
      TH1* getHist(const char* name, const char* title,
                   int nBins, double x0, double x1)
      {
        TH1* h = new TH1D(name, title, nBins, x0, x1);
        m_allHistos.push_back(h);
        return h;
      }

      /**
       * Create profile plot.
       */
      TProfile* getHistProfile(const char* name, const char* title,
                               int nBins, double x0, double x1)
      {
        TProfile* h = new TProfile(name, title, nBins, x0, x1);
        m_allHistos.push_back(h);
        return h;
      }

      /**
       * Create 2d-histogram.
       */
      TH2* getHist(const char* name, const char* title,
                   int nBinsX, double x0, double x1,
                   int nBinsY, double y0, double y1)
      {
        TH2* h = new TH2D(name, title, nBinsX, x0, x1, nBinsY, y0, y1);
        m_allHistos.push_back(h);
        return h;
      }

      /**
       * Create 1d-histogram.
       */
      TH1* getHist(const std::string& name, const std::string& title,
                   int nBins, double x0, double x1)
      {
        return getHist(name.c_str(), title.c_str(), nBins, x0, x1);
      }

      /**
       * Create profile plot.
       */
      TProfile* getHistProfile(const std::string& name, const std::string& title,
                               int nBins, double x0, double x1)
      {
        return getHistProfile(name.c_str(), title.c_str(), nBins, x0, x1);
      }

      /**
       * Create 2d-histogram.
       */
      TH2* getHist(const std::string& name, const std::string& title,
                   int nBinsX, double x0, double x1,
                   int nBinsY, double y0, double y1)
      {
        return getHist(name.c_str(), title.c_str(), nBinsX, x0, x1, nBinsY, y0, y1);
      }


      /**
       * Construct a plane for the hit.
       */
      const genfit::SharedPlanePtr constructPlane(const genfit::MeasuredStateOnPlane& state, WireID m_wireID);

      /**
       * Calculate residual for Layers which didn't use int fitting.
       */
      void getResidualOfUnFittedLayer(Belle2::RecoTrack* track);

      /**
       * Plot track parameters and related variables.
       */
      void plotResults(Belle2::RecoTrack* track);

      /**
       * Make hit distribution from track candidate.
       */
      void getHitDistInTrackCand(const RecoTrack* track);//Draw hit distribution from track candidate

      /**
       * extrapolation track to trigger counter plane (y position).
       */
      B2Vector3D getTriggerHitPosition(Belle2::RecoTrack* track);

      /**
       * Cal Hit eff.
       */
      void HitEfficiency(const Belle2::RecoTrack* track);

      /**
       * Convert slayer and ilayer to iclayer.
       */
      static int getICLayer(int slayer, int ilayer)
      {
        if (slayer == 0) {return ilayer;}
        else {return 8 + (slayer - 1) * 6 + ilayer;}
      }

      const Belle2::TrackFitResult* fitresult = nullptr;/**< Track fit result. */

      /**
       * Event timing. The event time is fetched from the data store using this pointer.
       */
      StoreObjPtr<EventT0> m_eventTimeStoreObject;

      /** Tracks. */
      StoreArray<Track> m_Tracks;

      /** Tracks. */
      StoreArray<RecoTrack> m_RecoTracks;

      /** Track fit results. */
      StoreArray<TrackFitResult> m_TrackFitResults;

      /** CDC hits. */
      StoreArray<CDCHit> m_CDCHits;

      std::string m_trackArrayName;           /**< Belle2::Track StoreArray name. */
      std::string m_cdcHitArrayName ;         /**< Belle2::CDCHit StoreArray name. */
      std::string m_recoTrackArrayName ;      /**< Belle2::RecoTrack StoreArray name.e */
      std::string m_trackFitResultArrayName;  /**< Belle2::TrackFitResult StoreArray name. */
      std::string m_relRecoTrackTrackName;    /**< Relation between RecoTrack and Belle2:Track. */
      std::string m_histogramDirectoryName;   /**< subdir where to place the histograms. */
      std::vector<TH1*> m_allHistos;          /**< A list of 1d histograms. */
      std::string m_treeName;                 /**< Name of tree for the output file. */

      TTree* m_tree = nullptr;                          /**< Output tree recording the information of each hit. */
      TH1* m_hNTracks = nullptr;                        /**< Number of track fitted, Convergence, not conv, not fit. */
      TH1* m_hNTracksPerEvent = nullptr;                /**< Number of TrackCand per Event. */
      TH1* m_hNTracksPerEventFitted = nullptr;          /**< Number of TrackCand per Event**/
      TH1* m_hNDF = nullptr;                            /**< Number of Degree Freedom. */
      TH1* m_hNHits = nullptr;                          /**< Number of Hits per track. */
      TH1* m_hNHits_trackcand = nullptr;                /**< Number of Hits per trackCand. */
      TH1* m_hChi2 = nullptr;                           /**< Chi2 histo. */
      TH1* m_hPval = nullptr;                           /**< Fit Probability histo. */
      TH2* m_hNDFChi2 = nullptr;                        /**< Chi2 vs degree-of-freedom histo. */
      TH2* m_hNDFPval = nullptr;                        /**< Degree-of-freedom vs Probability histo. */
      TH1* m_hAlpha = nullptr;                          /**< Alpha of each Hit. */
      TH1* m_hPhi0 = nullptr;                           /**< Phi0 of ttrack, see Helix. */
      TH1* m_hTheta = nullptr;                          /**< Theta of each Hit. */
      TH1* m_hHitDistInCDCHit[56] = {};            /**< Hit Dist. from CDCHit. */
      TH1* m_hHitDistInTrCand[56] = {};            /**< Hit Dist. Before Fit. */
      TH1* m_hHitDistInTrack[56] = {};             /**< Hit Dist. after fit (Weight of Hit >0.5). */
      TH1* m_hResidualU[56] = {};                  /**< Residual distribution (in cm)  */
      TH1* m_hEvtT0 = nullptr;                               /**< Event T0. */
      TH2* m_hNDFResidualU[56] = {};               /**< Residual vs. ndf. */
      TH1* m_hNormalizedResidualU[56] = {};        /**< Residual distribution normalized with tracking error.  */
      TH2* m_hNDFNormalizedResidualU[56] = {};     /**< Normalized residual vs. ndf. */
      TH2* m_hDxDt[56] = {};                       /**< Unbiased x_fit vs. drift time. */
      TProfile* m_hHitEff_soft[56] = {};           /**< Hit efficiency of each layer, software */
      TH2* m_h2DHitDistInCDCHit = nullptr;              /**< 2D Hit Dist.(ICLay vs IWire) from CDCHit. */
      TH2* m_h2DHitDistInTrCand = nullptr;              /**< 2D Hit Dist.(ICLay vs IWire) of Track candidates. */
      TH2* m_h2DHitDistInTrack = nullptr;               /**< 2D Hit Dist..(ICLay vs IWire) have weight>0.5 after fit with DAF */
      TH2* m_hTriggerHitZX = nullptr;                   /**< Trigger hit image. */


      double res_b = 0.0;             /**< Biased residual. */
      double res_u = 0.0;             /**< Unbiased residual. */
      double res_b_err = 0.0;         /**< Biased residual error. */
      double res_u_err = 0.0;         /**< Unbiased residual error.*/
      double weight = 0.0;            /**<  Weight of hit.*/
      double absRes_b = 0.0;          /**< absolute value of biased residual.*/
      double absRes_u = 0.0;          /**< absolute value of unbiased residual.*/
      double alpha = 0.0;             /**< Entrance Azimuthal angle of hit (degree). */
      double theta = 0.0;             /**< Entrance Polar angle of hit (degree). */
      unsigned short adc = 0;       /**< adc value. */
      short tdc = 0;                /**< tdc value. */
      double t = 0.0;                 /**< Measurement Drift time. */
      double t_fit = 0.0;             /**< Drift time calculated from x_fit. */
      double dt_flight = 0.0;         /**< Time of flight. */
      double dt_flight_sim = 0.0;     /**< Time of flight (Simulation). */
      double dt_prop = 0.0;           /**< Time of propagation. */
      double evtT0 = 0.0;             /**< Event time*/
      double Pt = 0.0;                /**< Transverse momentum*/

      double x_mea = 0.0;             /**< measure drift length (signed by left right).*/
      double x_u = 0.0;               /**< X_fit for unbiased track fit.*/
      double x_b = 0.0;               /**< X_fit for biased track fit.*/
      double x_sim = 0.0;             /**< Simulation DriftLength .*/
      double z = 0.0;                 /**< Z of hit on wire. */
      double z_sim = 0.0;             /**< Z of hit on wire (simulation). */
      double z_prop = 0.0;            /**< Propagation Length along the sense wire. */
      int lay = 0;                  /**< Layer ID. */
      int IWire = 0;                /**< Wire ID. */
      int lr = 0;                   /**< Left or right. */
      int numhits = 0;              /**< Number of hits. */
      int boardID = 0;              /**< Electrical Board ID. */
      double Pval = 0.0;              /**< P-value of fitted track.  */
      double TrPval = 0.0;            /**< P-value of fitted track.  */
      double ndf = 0.0;               /**< degree of freedom. */
      double d0 = 0.0;                /**< Track Parameter, d0. */
      double z0 = 0.0;                /**< Track Parameter, z0. */
      double phi0 = 0.0;              /**< Track Parameter, phi0. */
      double tanL = 0.0;              /**< Track Parameter, tanL. */
      double omega = 0.0;             /**< Track Parameter, omega. */
      double m_MinimumPt = 0.0;       /**< Minimum Transverse momentum of tracks*/

      B2Vector3D m_trigHitPos;                          /**< Trigger position. */
      std::vector<double> m_TriggerPos;                 /**< Nominal center position of trigger counter. */
      std::vector<double> m_TriggerPlaneDirection;      /**< Nominal center position of trigger counter. */
      std::vector<double> m_TriggerSize;                /**< Size of trigger counter (Width x length). */
      std::vector<int> m_up;                            /**< upper channel list for each board. */
      std::vector<int> m_low;                           /**< lower channel list for each board. */
      double trigHitPos_x = 0.0;                              /**< X-position of track at trigger counter */
      double trigHitPos_z = 0.0;                              /**< Z-position of track at trigger counter */
      int trighit = 0;                                      /**< Trigger hit information. 1 if track hits trigger counter, otherwise 0. */

      bool m_fillExpertHistos = false;                 /**< Fill some histogram for monitoring fit quality. */
      bool m_plotResidual = false;                     /**< Process track to get the hit information of fitted track. */
      bool m_hitEfficiency = false;                    /**< calculate hit eff or not, Haven't finished. */
      bool m_calExpectedDriftTime = false;             /**< Calculate expected drift time from x_fit or not. */
      bool m_noBFit = false;                           /**< fit incase no magnetic Field of not, if true, NDF=4 in cal P-value */
      bool m_ToP = false;                              /**< Enable to correct ToP if true. */
      bool m_ToF = false;                              /**< Enable to correct ToF if true. */
      bool m_StoreCDCSimHitInfo = false;               /**< Store CDCSimHit Information. */
      bool m_EstimateResultForUnFittedLayer = false;   /**< Calculate residual for layer that we do not use in track fitting. */
      bool m_SmallerOutput = false;                    /**< make output smaller by ignore some variable. */
      bool m_StoreTrackParams = false;                 /**< Store Track parameter or not. */
      bool m_MakeHitDist = false;                      /**< Switch to make histograms of hit distribution. */
      bool m_EventT0Extraction = false;                /**< use Event T0 extract t0 or not. */
    };
  }
}
