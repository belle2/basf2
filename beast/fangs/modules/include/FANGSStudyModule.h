/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FANGSSTUDYMODULE_H
#define FANGSSTUDYMODULE_H

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

      /**  */
      virtual ~FANGSStudyModule();

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
