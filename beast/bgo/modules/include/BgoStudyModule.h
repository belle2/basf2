/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef BGOSTUDYMODULE_H
#define BGOSTUDYMODULE_H

#include <framework/core/HistoModule.h>
#include <vector>

#include <TH1.h>
#include <TH2.h>

namespace Belle2 {
  namespace bgo {

    /**
     * Study module for Bgos (BEAST)
     *
     * Produces histograms from BEAST data for the Bgos.   *
     */
    class BgoStudyModule : public HistoModule {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      BgoStudyModule();

      /**
       * Destructor
       */
      virtual ~BgoStudyModule();

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

      /** reads data from BGO.xml: tube location, drift data filename, sigma of impulse response function */
      //virtual void getXMLData();
      /** Energy threshold */
      std::vector<Double_t> m_Ethres;


      /** Rate */
      TH1F* h_bgo_rate[2];
      /** Rate */
      TH2F* h_bgo_rs_rate[2];
      /** Energy deposited vs TOF */
      TH2F* h_bgo_Evtof1[18];
      /** Energy deposited vs TOF */
      TH2F* h_bgo_Evtof2[18];
      /** Energy deposited vs TOF */
      TH2F* h_bgo_Evtof3[18];
      /** Energy deposited vs TOF */
      TH2F* h_bgo_Evtof[18];
      /** Energy deposited */
      TH1F* h_bgo_edep[18];
      /** Energy deposited */
      TH1F* h_bgo_edep1[18];
      /** Energy deposited */
      TH1F* h_bgo_edep2[18];
      /** Energy deposited */
      TH1F* h_bgo_edep1Weight[18];
      /** Energy deposited */
      TH1F* h_bgo_edep2Weight[18];
      /** Energy deposited */
      TH2F* h_bgo_rs_edep1[18];
      /** Energy deposited */
      TH2F* h_bgo_rs_edep2[18];
      /** Energy deposited */
      TH2F* h_bgo_rs_edep1Weight[18];
      /** Energy deposited */
      TH2F* h_bgo_rs_edep2Weight[18];
    };

  }
}

#endif /* BGOSTUDYMODULE_H */
