/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CSISTUDY_V2MODULE_H
#define CSISTUDY_V2MODULE_H

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
  namespace csi {

    /**
     * Study module for Csis (BEAST)
     *
     * Produces histograms from BEAST data for the Csis.   *
     */
    class CsiStudy_v2Module : public HistoModule {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      CsiStudy_v2Module();

      /**  */
      virtual ~CsiStudy_v2Module();

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

      /** reads data from CSI.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();
      /** Energy threshold */
      double m_Ethres;


      /** Rate */
      TH1F* h_csi_rate[2];
      /** Rate */
      TH2F* h_csi_rs_rate[2];
      /** Energy deposited vs TOF */
      TH2F* h_csi_Evtof1[18];
      /** Energy deposited vs TOF */
      TH2F* h_csi_Evtof2[18];
      /** Energy deposited vs TOF */
      TH2F* h_csi_Evtof3[18];
      /** Energy deposited vs TOF */
      TH2F* h_csi_Evtof[18];
      /** Energy deposited */
      TH1F* h_csi_edep[18];
      /** Energy deposited */
      TH1F* h_csi_edep1[18];
      /** Energy deposited */
      TH1F* h_csi_edep2[18];
      /** Energy deposited */
      TH1F* h_csi_edep1Weight[18];
      /** Energy deposited */
      TH1F* h_csi_edep2Weight[18];
      /** Energy deposited */
      TH2F* h_csi_rs_edep1[18];
      /** Energy deposited */
      TH2F* h_csi_rs_edep2[18];
      /** Energy deposited */
      TH2F* h_csi_rs_edep1Weight[18];
      /** Energy deposited */
      TH2F* h_csi_rs_edep2Weight[18];

    };

  }
}

#endif /* CSISTUDY_V2MODULE_H */
