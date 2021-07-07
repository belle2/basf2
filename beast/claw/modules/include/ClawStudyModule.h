/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef CLAWSTUDYMODULE_H
#define CLAWSTUDYMODULE_H

#include <framework/core/HistoModule.h>

#include <TH1.h>
#include <TH2.h>

namespace Belle2 {
  namespace claw {

    /**
     * Study module for Claws (BEAST)
     *
     * Produces histograms from BEAST data for the Claws.   *
     */
    class ClawStudyModule : public HistoModule {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      ClawStudyModule();

      /**
       * Destructor
       */
      virtual ~ClawStudyModule();

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

      /** reads data from CLAW.xml */
      virtual void getXMLData();
      /** Energy threshold */
      double m_Ethres;

      /** Energy deposited vs TOF */
      TH2F* h_claws_Evtof1[8];
      /** Energy deposited vs TOF */
      TH2F* h_claws_Evtof2[8];
      /** Energy deposited vs TOF */
      TH2F* h_claws_Evtof3[8];
      /** Energy deposited vs TOF */
      TH2F* h_claws_Evtof4[8];
      /** Energy deposited */
      TH1F* h_claws_edep[8];
      /** Energy deposited */
      TH1F* h_Wclaws_edep[8];

      /** Energy deposited */
      TH1F* h_claws_rate1[8];
      /** Energy deposited */
      TH1F* h_claws_rate2[8];
      /** Energy deposited */
      TH1F* h_claws_rate1W[8];
      /** Energy deposited */
      TH1F* h_claws_rate2W[8];

      /** Energy deposited */
      TH1F* h_claws_hitrate1;
      /** Energy deposited */
      TH1F* h_claws_hitrate2;
      /** Energy deposited */
      TH1F* h_claws_hitrate1W;
      /** Energy deposited */
      TH1F* h_claws_hitrate2W;

      /** Energy deposited */
      TH2F* h_claws_rs_hitrate1;
      /** Energy deposited */
      TH2F* h_claws_rs_hitrate2;
      /** Energy deposited */
      TH2F* h_claws_rs_hitrate1W;
      /** Energy deposited */
      TH2F* h_claws_rs_hitrate2W;

      /** Energy deposited */
      TH2F* h_claws_rs_rate1[8];
      /** Energy deposited */
      TH2F* h_claws_rs_rate2[8];
      /** Energy deposited */
      TH2F* h_claws_rs_rate1W[8];
      /** Energy deposited */
      TH2F* h_claws_rs_rate2W[8];

      /** Energy deposited */
      TH2F* h_claws_pe1[8];
      /** Energy deposited */
      TH2F* h_claws_pe2[8];
      /** Energy deposited */
      TH2F* h_claws_pe1W[8];
      /** Energy deposited */
      TH2F* h_claws_pe2W[8];

    };

  }
}

#endif /* CLAWSTUDYMODULE_H */
