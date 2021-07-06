/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef FANGSSTUDYMODULE_H
#define FANGSSTUDYMODULE_H

#include <framework/core/HistoModule.h>

#include <TF1.h>
#include <TH1.h>
#include <TH2.h>

/** size of hit */
constexpr int maxSIZE = 3000;

namespace Belle2 {
  namespace fangs {

    /**
     * Study module for Fangs (BEAST)
     *
     * Produces histograms from BEAST data for the Fangs.   *
     */
    class FANGSStudyModule : public HistoModule {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      FANGSStudyModule();

      /**
       * Destructor
       */
      virtual ~FANGSStudyModule();

      /**
       * Initialize the Module.
       * This method is called at the beginning of data processing.
       */
      virtual void initialize() override;

      /**
       * Called when entering a new run.
       * Set run dependent things like run header parameters, alignment, etc.
       */
      virtual void beginRun() override;

      /**
       * Event processor.
       */
      virtual void event() override;

      /**
       * End-of-run action.
       * Save run-related stuff, such as statistics.
       */
      virtual void endRun() override;

      /**
       * Termination action.
       * Clean-up, close files, summarize statistics, etc.
       */
      virtual void terminate() override;

      /** Defines the histograms*/
      virtual void defineHisto() override;


    private:

      /** reads data from FANGS.xml */
      virtual void getXMLData();
      /** Define Q calib 1 */
      TF1* fctQ_Calib1;
      /** Define Q calib 2 */
      TF1* fctQ_Calib2;
      /** Pixel threshold */
      int m_PixelThreshold;
      /** Pixel threshold RMS */
      int m_PixelThresholdRMS;
      /** Chip column number */
      int m_ChipColumnNb;
      /** Chip row number */
      int m_ChipRowNb;
      /** Chip column x dimension */
      double m_ChipColumnX;
      /** Chip row y dimension */
      double m_ChipRowY;
      /** Pixel time number of bin */
      int m_PixelTimeBinNb;
      /** Pixel time bin */
      double m_PixelTimeBin;
      /** TOT factor A1 */
      double m_TOTA1;
      /** TOT factor B1 */
      double m_TOTB1;
      /** TOT factor C1 */
      double m_TOTC1;
      /** TOT factor Q1 */
      double m_TOTQ1;
      /** TOT factor A2 */
      double m_TOTA2;
      /** TOT factor B2 */
      double m_TOTB2;
      /** TOT factor C2 */
      double m_TOTC2;
      /** TOT factor Q2 */
      double m_TOTQ2;

      /** Work function */
      double m_Workfct;
      /** sensor width */
      double m_sensor_width;
      /** Drift velocity in sensor */
      double m_v_sensor;

      /** x point of the track */
      float x[maxSIZE];
      /** y point of the track */
      float y[maxSIZE];
      /** z point of the track */
      float z[maxSIZE];
      /** e point of the track */
      float e[maxSIZE];

      /** col v raw*/
      TH2F* h_cvr[20];
      /** Digitized energy deposited per detector */
      TH2F* h_Edep;
      /** Pixel number per detector */
      TH2F* h_pxNb;
      /** Time distribution */
      TH2F* h_time;
      /** Time distribution weighted per the energy deposited */
      TH2F* h_timeWeighted;
      /** Time distribution with energy threshold applied */
      TH2F* h_timeThres;
      /** Time distribution weighted per the energy deposited with energy threshold applied */
      TH2F* h_timeWeightedThres;
      /** Energy deposited per time bin */
      TH2F* h_edep;
      /** Energy deposited per time bin */
      TH2F* h_edep1;
      /** Energy deposited per time bin */
      TH2F* h_edep2;
      /** Energy deposited per time bin */
      TH2F* h_edep3;

      /** Energy deposited above threshold per time bin */
      TH2F* h_edepThres;
      /** Energy deposited above threshold per time bin */
      TH2F* h_edepThres1;
      /** Energy deposited above threshold per time bin */
      TH2F* h_edepThres2;
      /** Energy deposited above threshold per time bin */
      TH2F* h_edepThres3;


      /** Energy deposited vs z */
      TH1F* h_zvedep[3];
      /** Energy vs y vs z */
      TH2F* h_yvzvedep[3];
      /** Energy vs x vs z */
      TH2F* h_xvzvedep[3];
      /** Energy vs x vs y */
      TH2F* h_xvyvedep[3];

      /** Energy deposited vs z */
      TH1F* h_zvedepW[3];
      /** Energy vs y vs z */
      TH2F* h_yvzvedepW[3];
      /** Energy vs x vs z */
      TH2F* h_xvzvedepW[3];
      /** Energy vs x vs y */
      TH2F* h_xvyvedepW[3];

      /** Energy deposited vs z */
      TH1F* h_zvedepT[3];
      /** Energy vs y vs z */
      TH2F* h_yvzvedepT[3];
      /** Energy vs x vs z */
      TH2F* h_xvzvedepT[3];
      /** Energy vs x vs y */
      TH2F* h_xvyvedepT[3];

      /** Energy deposited vs z */
      TH1F* h_zvedepWT[3];
      /** Energy vs y vs z */
      TH2F* h_yvzvedepWT[3];
      /** Energy vs x vs z */
      TH2F* h_xvzvedepWT[3];
      /** Energy vs x vs y */
      TH2F* h_xvyvedepWT[3];

      /** Energy vs x vs y */
      TH2F* h_rvzvedep[3];
      /** Energy vs x vs y */
      TH2F* h_rvzvedepT[3];

      /** Energy vs x vs y */
      TH2F* h_rvzvedepW[3];
      /** Energy vs x vs y */
      TH2F* h_rvzvedepWT[3];

    };

  }
}

#endif /* FANGSSTUDYMODULE_H */
