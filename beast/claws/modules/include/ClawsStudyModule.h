/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef CLAWSSTUDYMODULE_H
#define CLAWSSTUDYMODULE_H

#include <framework/core/HistoModule.h>

#include <TH1.h>
#include <TH2.h>

namespace Belle2 {
  namespace claws {

    /**
     * Study module for Clawss (BEAST)
     *
     * Produces histograms from BEAST data for the Clawss.   *
     */
    class ClawsStudyModule : public HistoModule {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      ClawsStudyModule();

      /**
       * Destructor
       */
      virtual ~ClawsStudyModule();

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

      /**
       * Defines the histograms
       */
      virtual void defineHisto() override;


    private:

      /** reads data from CLAWS.xml */
      virtual void getXMLData();
      /** Energy threshold */
      double m_Ethres;

      // These histrograms are currently
      // not used in the code, but just declared.
      // Commented out by the SW shifter, March 14th 2018
      /** Energy deposited vs TOF */
      //TH2F* h_clawss_Evtof1[16];
      /** Energy deposited vs TOF */
      //TH2F* h_clawss_Evtof2[16];
      /** Energy deposited vs TOF */
      //TH2F* h_clawss_Evtof3[16];
      /** Energy deposited vs TOF */
      //TH2F* h_clawss_Evtof4[16];
      /** Energy deposited */
      //TH1F* h_clawss_edep[16];
      /** Energy deposited */
      //TH1F* h_Wclawss_edep[16];

      /** Energy deposited */
      TH1F* h_clawss_rate1[16];
      /** Energy deposited */
      TH1F* h_clawss_rate2[16];
      /** Energy deposited */
      TH1F* h_clawss_rate1W[16];
      /** Energy deposited */
      TH1F* h_clawss_rate2W[16];

      /** Energy deposited */
      TH1F* h_clawss_hitrate1;
      /** Energy deposited */
      TH1F* h_clawss_hitrate2;
      /** Energy deposited */
      TH1F* h_clawss_hitrate1W;
      /** Energy deposited */
      TH1F* h_clawss_hitrate2W;

      /** Energy deposited */
      TH2F* h_clawss_rs_hitrate1;
      /** Energy deposited */
      TH2F* h_clawss_rs_hitrate2;
      /** Energy deposited */
      TH2F* h_clawss_rs_hitrate1W;
      /** Energy deposited */
      TH2F* h_clawss_rs_hitrate2W;

      /** Energy deposited */
      TH2F* h_clawss_rs_rate1[16];
      /** Energy deposited */
      TH2F* h_clawss_rs_rate2[16];
      /** Energy deposited */
      TH2F* h_clawss_rs_rate1W[16];
      /** Energy deposited */
      TH2F* h_clawss_rs_rate2W[16];

      /** Energy deposited */
      TH2F* h_clawss_pe1[16];
      /** Energy deposited */
      TH2F* h_clawss_pe2[16];
      /** Energy deposited */
      TH2F* h_clawss_pe1W[16];
      /** Energy deposited */
      TH2F* h_clawss_pe2W[16];

    };

  }
}

#endif /* CLAWSSTUDYMODULE_H */
