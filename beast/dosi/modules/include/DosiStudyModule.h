/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DOSISTUDYMODULE_H
#define DOSISTUDYMODULE_H

#include <framework/core/HistoModule.h>
#include <string>
#include <vector>


#include <TVector3.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>


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

      /**  */
      virtual ~DosiStudyModule();

      /**  */
      virtual void initialize();

      /**  */
      virtual void beginRun();

      /**  */
      virtual void event();

      /**  */
      virtual void endRun();

      /**  */
      virtual void terminate();

      /** Defines the histograms*/
      virtual void defineHisto();


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
