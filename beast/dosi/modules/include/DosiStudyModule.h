/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DOSISTUDYMODULE_H
#define DOSISTUDYMODULE_H

#include <framework/core/HistoModule.h>

#include <TH1.h>
#include <TH2.h>

namespace Belle2 {
  namespace dosi {

    /**
     * Study module for Dosis (BEAST)
     *
     * Produces histograms from BEAST data for the Dosis.   *
     */
    class DosiStudyModule : public HistoModule {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      DosiStudyModule();

      /**
       * Destructor
       */
      virtual ~DosiStudyModule();

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

      /** reads data from DOSI.xml: tube location, drift data filename, sigma of impulse response function */
      //virtual void getXMLData();

      /** Energy deposited */
      TH1F* h_dosi_edep0[18];
      /** Energy deposited */
      TH1F* h_dosi_edep1[18];
      /** Energy deposited */
      TH1F* h_dosi_edep2[18];
      /** Energy deposited */
      TH1F* h_dosi_edep3[18];
      /** Energy deposited */
      TH1F* h_dosi_edep4[18];
      /** Energy deposited */
      TH1F* h_dosi_edep5[18];
      /** Energy deposited */
      TH1F* h_dosi_edep6[18];
      /** Energy deposited */
      TH1F* h_dosi_edep7[18];
      /** Energy deposited */
      TH1F* h_dosi_edep8[18];

      /** Energy deposited */
      TH2F* h_dosi_rs_edep0[18];
      /** Energy deposited */
      TH2F* h_dosi_rs_edep1[18];
      /** Energy deposited */
      TH2F* h_dosi_rs_edep2[18];
      /** Energy deposited */
      TH2F* h_dosi_rs_edep3[18];
      /** Energy deposited */
      TH2F* h_dosi_rs_edep4[18];
      /** Energy deposited */
      TH2F* h_dosi_rs_edep5[18];
      /** Energy deposited */
      TH2F* h_dosi_rs_edep6[18];
      /** Energy deposited */
      TH2F* h_dosi_rs_edep7[18];
      /** Energy deposited */
      TH2F* h_dosi_rs_edep8[18];


    };

  }
}

#endif /* DOSISTUDYMODULE_H */
