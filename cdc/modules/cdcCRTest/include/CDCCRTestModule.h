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
#include <vxd/geometry/GeoCache.h>

#include <cdc/dataobjects/WireID.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <string>

#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationVector.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <genfit/TrackCand.h>
#include <genfit/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
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
      enum {
        firstLayer = 0,
        lastLayer = 55,
        NLayer = lastLayer - firstLayer + 1,
      };

      /**
       * Get layer index fromm layer ID.
       */
      inline int getIndex(const int& layer)
      {
        return layer - firstLayer;
      }

      /**
       * Get layer ID from index.
       */
      inline int getLayer(const int& index)
      {
        return index + firstLayer;
      }

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

      const Belle2::TrackFitResult* fitresult; /**< Track fit result. */

      std::string m_trackArrayName;           /**< Belle2::Track StoreArray name. */
      std::string m_cdcHitArrayName ;         /**< Belle2::CDCHit StoreArray name. */
      std::string m_recoTrackArrayName ;       /**< Belle2::RecoTrack StoreArray nam.e */
      std::string m_trackFitResultArrayName;  /**< Belle2::TrackFitResult StoreArray name. */
      std::string m_relRecoTrackTrackName;     /**< Releation between RecoTrack and Belle2:Track. */
      std::string m_histogramDirectoryName;   /**< subdir where to place the histograms. */
      std::vector<TH1*> m_allHistos;          /**< All histograms for easy deletion. */

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

      TTree* tree;                          /**< output tree, save info of each hit. */
      TFile* tfile;                         /**< Output file. */
      TH1* m_hNTracks;                      /**< Number of track fitted, Convegence, not conv, not fit. */
      TH1* m_hNTracksPerEvent;              /**< Number of TrackCand per Event. */
      TH1* m_hNTracksPerEventFitted;        /**< Number of TrackCand per Event**/
      TH1* m_hE1Dist; /**< Energy distribution of 1st track. */
      TH1* m_hE2Dist; /**< Energy distribution of 2nd track if it exists. */
      TH1* m_hNDF;                          /**< Number of Degree Freedom. */
      TH1* m_hNHits;                        /**< Number of Hits per track. */
      TH1* m_hNHits_trackcand;              /**< Number of Hits per trackCand. */
      TH1* m_hChi2;                         /**< Chi2 histo. */
      TH1* m_hPval;                         /**< Fit Probability histo. */
      TH2* m_hNDFChi2;                      /**< Chi2 vs degree-of-freedom histo. */
      TH2* m_hNDFPval;                      /**< Degree-of-freedom vs Probability histo. */
      TH1* m_hAlpha;                        /**< Alpha of each Hit. */
      TH1* m_hPhi0;                         /**< Phi0 of ttrack, see Helix. */
      TH1* m_hTheta;                        /**< Theta of each Hit. */
      TH1* m_hHitDistInCDCHit[NLayer];      /**< Hit Dist. from CDCHit. */
      TH1* m_hHitDistInTrCand[NLayer];      /**< Hit Dist. Before Fit. */
      TH1* m_hHitDistInTrack[NLayer];       /**< Hit Dist. after fit (Weight of Hit >0.5). */
      TH2* m_h2DHitDistInCDCHit;            /**< 2D Hit Dist.(ICLay vs IWire) from CDCHit. */
      TH2* m_h2DHitDistInTrCand;            /**< 2D Hit Dist.(ICLay vs IWire) befofir. */
      TH2* m_h2DHitDistInTrack;             /**< 2D Hit Dist..(ICLay vs IWire) have weight>0.5 after fit with DAF */
      TH2* m_hTriggerHitZX;                 /**< Trigier hit image. */


      double res_b; /**< Biased residual. */
      double res_u; /**< Unbiased residual. */
      double res_b_err; /**< Biased residual error. */
      double res_u_err; /**< Unbiased residual error.*/
      double weight;/**<  Weight of hit.*/
      double absRes_b;/**< absolute value of biased residual.*/
      double absRes_u;/**< absolute value of unbiased residual.*/
      double alpha;   /**< Entrance Azimuthual angle of hit (degree). */
      double theta;   /**< Entrance Polar angle of hit (degree). */
      double Phi0;   /**< Phi0 angle of track, see in TrackFitResult (degree). */
      unsigned short adc; /**< adc value. */
      short tdc; /**< tdc value. */
      double t;          /**< Measurement Drift time. */
      double t_fit;          /**< Drift time calculated from x_fit. */
      double dt_flight;          /**< Time of flight. */
      double dt_prop;          /**< Time of propagation. */

      double x_u; /**< X_fit for unbiased track fit.*/
      double x_b; /**< X_fit for biased track fit.*/
      double z_u; /**< Z_fit for unbiasedtrack fit.*/
      double z_b; /**< Z_fit for biased track fit.*/
      double z_prop; /**< Propagation Length along the sense wire. */
      double r_flight; /**< Flight Length. */
      int lay; /**< Layer ID. */
      int IWire; /**< Wire ID. */
      int lr;  /**< Left or right. */
      int numhits; /**< Number of hits. */
      int boardID;    /**< Electrical Board ID. */
      double t0;                                     /**< time offset. */
      double tdcBinWidth;                            /**< width of tdc bin, =1.01777728. */
      double Chi2;      /**< Chi2 value. */
      double Pval;      /**< P-value of fitted track.  */
      double TrPval;    /**< P-value of fitted track.  */
      double ndf;       /**< degree of freedom. */

      TVector3 trigHitPos; /**< Trigger position. */
      std::vector<double> m_TriggerPos; /**< Nominal center position of trigger counter. */
      std::vector<int> m_up; /**< upper channel list for each board. */
      std::vector<int> m_low; /**< lower channel list for each board. */
      double trigHitPos_x; /**< X-position of track at trigger counter */
      double trigHitPos_z; /**< Z-position of track at trigger counter */
      int trighit; /**< Trigger hit information. 1 if track hits trigger counter, otherwise 0. */


      TH1* m_hResidualU[NLayer]; /**< Residual distribution (in cm)  */
      TH2* m_hNDFResidualU[NLayer]; /**< Residual vs. ndf. */


      TH1* m_hNormalizedResidualU[NLayer]; /**< Residual distribution normalized with tracking error.  */
      TH2* m_hNDFNormalizedResidualU[NLayer]; /**< Normalized residual vs. ndf. */
      TH2* m_hDxDt[NLayer]; /**< Unbiased x_fit vs. drift time. */


      /**
       * Plot track parameters and related variables.
       */
      void plotResults(const Belle2::RecoTrack* track);

      /**
       * calculate measurement Drift time
       */
      double getCorrectedDriftTime(WireID wireid, unsigned short tdc, unsigned short adc, double z,
                                   double z0); //get Drift time was corect ToP,...
      /**
       * make hit distribution from track candidate.
       */
      void getHitDistInTrackCand(const RecoTrack* track);//Draw hit distribution from track candidate
      /**
       * extrapolation track to trigger counter plane (y position).
       */
      TVector3 getTriggerHitPosition(const Helix h, double yofcounter);
      /**
       * Cal Hit eff.
       */
      void HitEfficiency(const Belle2::Helix h);

      /**
       * Convert slayer and ilayer to iclayer.
       */
      int getICLayer(int slayer, int ilayer)
      {
        if (slayer == 0) {return ilayer;}
        else {return 8 + (slayer - 1) * 6 + ilayer;}
      }

      bool m_fillExpertHistos; /**< Fill some histogram for monitoring fit quality. */
      bool m_plotResidual;  /**< Process track to get the hit information of fitted track. */
      bool m_hitEfficiency; /**< calculate hit eff or not, Haven't finished. */
      bool m_calExpectedDriftTime; /**< Calculate expected drift time from x_fit or not. */
      bool m_noBFit; /**< fit incase no magnetic Field of not, if true, NDF=4 in cal P-value */
      bool m_ToP; /**< Enable to correct ToP if true. */
      bool m_ToF; /**< Enable to correct ToF if true. */
      bool m_IncomingToF; /**< Invert top for incoming track or not. */

    };
  }
}
#endif
