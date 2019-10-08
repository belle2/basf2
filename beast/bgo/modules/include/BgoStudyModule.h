/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

      /**  */
      virtual ~BgoStudyModule();

      /**  */
      virtual void initialize() override;

      /**  */
      virtual void beginRun() override;

      /**  */
      virtual void event() override;

      /**  */
      virtual void endRun() override;

      /**  */
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
