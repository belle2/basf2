/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dong Van Thanh                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCCRTestModule_H_
#define CDCCRTestModule_H_

#include <framework/core/HistoModule.h>

#include <cdc/dataobjects/WireID.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <string>

#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <genfit/DetPlane.h>
#include <genfit/StateOnPlane.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/dataobjects/EventT0.h>
#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TProfile.h"
#include "TTree.h"

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
      virtual ~CDCCRTestModule();

      /**
       * Initializes the Module.
       */
      virtual void initialize();

      /**
       * Begin run action.
       */
      virtual void beginRun();

      /**
       * Event action (main routine).
       */

      virtual void event();
      /**
       * End run action.
       */

      virtual void endRun();

      /**
       * Termination action.
       */
      virtual void terminate();

      /**
       *  Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
       *  to be placed in this function.
       */
      virtual void defineHisto();

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
      TVector3 getTriggerHitPosition(Belle2::RecoTrack* track);

      /**
       * Cal Hit eff.
       */
      void HitEfficiency(const Belle2::RecoTrack* track);

      /**
       * Convert slayer and ilayer to iclayer.
       */
      int getICLayer(int slayer, int ilayer)
      {
        if (slayer == 0) {return ilayer;}
        else {return 8 + (slayer - 1) * 6 + ilayer;}
      }

      const Belle2::TrackFitResult* fitresult;/**< Track fit result. */

      /**
       * Event timing. The event time is fetched from the data store using this pointer.
       */
      StoreObjPtr<EventT0> m_eventTimeStoreObject;
      std::string m_trackArrayName;           /**< Belle2::Track StoreArray name. */
      std::string m_cdcHitArrayName ;         /**< Belle2::CDCHit StoreArray name. */
      std::string m_recoTrackArrayName ;      /**< Belle2::RecoTrack StoreArray nam.e */
      std::string m_trackFitResultArrayName;  /**< Belle2::TrackFitResult StoreArray name. */
      std::string m_relRecoTrackTrackName;    /**< Relation between RecoTrack and Belle2:Track. */
      std::string m_histogramDirectoryName;   /**< subdir where to place the histograms. */
      std::vector<TH1*> m_allHistos;          /**< A list of 1d histograms. */
      std::string m_treeName;                 /**< Name of tree for the output file. */

      TTree* m_tree;                          /**< Output tree recording the information of each hit. */
      TH1* m_hNTracks;                        /**< Number of track fitted, Convergence, not conv, not fit. */
      TH1* m_hNTracksPerEvent;                /**< Number of TrackCand per Event. */
      TH1* m_hNTracksPerEventFitted;          /**< Number of TrackCand per Event**/
      TH1* m_hNDF;                            /**< Number of Degree Freedom. */
      TH1* m_hNHits;                          /**< Number of Hits per track. */
      TH1* m_hNHits_trackcand;                /**< Number of Hits per trackCand. */
      TH1* m_hChi2;                           /**< Chi2 histo. */
      TH1* m_hPval;                           /**< Fit Probability histo. */
      TH2* m_hNDFChi2;                        /**< Chi2 vs degree-of-freedom histo. */
      TH2* m_hNDFPval;                        /**< Degree-of-freedom vs Probability histo. */
      TH1* m_hAlpha;                          /**< Alpha of each Hit. */
      TH1* m_hPhi0;                           /**< Phi0 of ttrack, see Helix. */
      TH1* m_hTheta;                          /**< Theta of each Hit. */
      TH1* m_hHitDistInCDCHit[56];            /**< Hit Dist. from CDCHit. */
      TH1* m_hHitDistInTrCand[56];            /**< Hit Dist. Before Fit. */
      TH1* m_hHitDistInTrack[56];             /**< Hit Dist. after fit (Weight of Hit >0.5). */
      TH1* m_hResidualU[56];                  /**< Residual distribution (in cm)  */
      TH1* m_hEvtT0;                               /**< Event T0. */
      TH2* m_hNDFResidualU[56];               /**< Residual vs. ndf. */
      TH1* m_hNormalizedResidualU[56];        /**< Residual distribution normalized with tracking error.  */
      TH2* m_hNDFNormalizedResidualU[56];     /**< Normalized residual vs. ndf. */
      TH2* m_hDxDt[56];                       /**< Unbiased x_fit vs. drift time. */
      TProfile* m_hHitEff_soft[56];           /**< Hit efficience of each layer, software */
      TH2* m_h2DHitDistInCDCHit;              /**< 2D Hit Dist.(ICLay vs IWire) from CDCHit. */
      TH2* m_h2DHitDistInTrCand;              /**< 2D Hit Dist.(ICLay vs IWire) of Track candidates. */
      TH2* m_h2DHitDistInTrack;               /**< 2D Hit Dist..(ICLay vs IWire) have weight>0.5 after fit with DAF */
      TH2* m_hTriggerHitZX;                   /**< Trigger hit image. */


      double res_b;             /**< Biased residual. */
      double res_u;             /**< Unbiased residual. */
      double res_b_err;         /**< Biased residual error. */
      double res_u_err;         /**< Unbiased residual error.*/
      double weight;            /**<  Weight of hit.*/
      double absRes_b;          /**< absolute value of biased residual.*/
      double absRes_u;          /**< absolute value of unbiased residual.*/
      double alpha;             /**< Entrance Azimuthal angle of hit (degree). */
      double theta;             /**< Entrance Polar angle of hit (degree). */
      unsigned short adc;       /**< adc value. */
      short tdc;                /**< tdc value. */
      double t;                 /**< Measurement Drift time. */
      double t_fit;             /**< Drift time calculated from x_fit. */
      double dt_flight;         /**< Time of flight. */
      double dt_flight_sim;     /**< Time of flight (Simulation). */
      double dt_prop;           /**< Time of propagation. */
      double evtT0;             /**< Event time*/
      double Pt;                /**< Transverse momentum*/

      double x_mea;             /**< measure drift length (signed by left right).*/
      double x_u;               /**< X_fit for unbiased track fit.*/
      double x_b;               /**< X_fit for biased track fit.*/
      double x_sim;             /**< Simulation DriftLength .*/
      double z;                 /**< Z of hit on wire. */
      double z_sim;             /**< Z of hit on wire (simulation). */
      double z_prop;            /**< Propagation Length along the sense wire. */
      int lay;                  /**< Layer ID. */
      int IWire;                /**< Wire ID. */
      int lr;                   /**< Left or right. */
      int numhits;              /**< Number of hits. */
      int boardID;              /**< Electrical Board ID. */
      double Pval;              /**< P-value of fitted track.  */
      double TrPval;            /**< P-value of fitted track.  */
      double ndf;               /**< degree of freedom. */
      double d0;                /**< Track Parameter, d0. */
      double z0;                /**< Track Parameter, z0. */
      double phi0;              /**< Track Parameter, phi0. */
      double tanL;              /**< Track Parameter, tanL. */
      double omega;             /**< Track Parameter, omega. */
      double m_MinimumPt;       /**< Minimum Transverse momentum of tracks*/

      TVector3 m_trigHitPos;                            /**< Trigger position. */
      std::vector<double> m_TriggerPos;                 /**< Nominal center position of trigger counter. */
      std::vector<double> m_TriggerPlaneDirection;      /**< Nominal center position of trigger counter. */
      std::vector<double> m_TriggerSize;                /**< Size of trigger counter (Width x length). */
      std::vector<int> m_up;                            /**< upper channel list for each board. */
      std::vector<int> m_low;                           /**< lower channel list for each board. */
      double trigHitPos_x;                              /**< X-position of track at trigger counter */
      double trigHitPos_z;                              /**< Z-position of track at trigger counter */
      int trighit;                                      /**< Trigger hit information. 1 if track hits trigger counter, otherwise 0. */

      bool m_fillExpertHistos;                 /**< Fill some histogram for monitoring fit quality. */
      bool m_plotResidual;                     /**< Process track to get the hit information of fitted track. */
      bool m_hitEfficiency;                    /**< calculate hit eff or not, Haven't finished. */
      bool m_calExpectedDriftTime;             /**< Calculate expected drift time from x_fit or not. */
      bool m_noBFit;                           /**< fit incase no magnetic Field of not, if true, NDF=4 in cal P-value */
      bool m_ToP;                              /**< Enable to correct ToP if true. */
      bool m_ToF;                              /**< Enable to correct ToF if true. */
      bool m_StoreCDCSimHitInfo;               /**< Store CDCSimHit Information. */
      bool m_EstimateResultForUnFittedLayer;   /**< Calculate residual for layer that we do not use in track fitting. */
      bool m_SmallerOutput;                    /**< make output smaller by ignore some variable. */
      bool m_StoreTrackParams;                 /**< Store Track parameter or not. */
      bool m_MakeHitDist;                      /**< Switch to make histograms of hit distribution. */
      bool m_EventT0Extraction;                /**< use Event T0 extract t0 or not. */
    };
  }
}
#endif
