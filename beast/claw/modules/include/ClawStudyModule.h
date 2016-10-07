/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CLAWSTUDYMODULE_H
#define CLAWSTUDYMODULE_H

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

      /**  */
      virtual ~ClawStudyModule();

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
