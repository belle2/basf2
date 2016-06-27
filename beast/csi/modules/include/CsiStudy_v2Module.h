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

      /** Energy deposited vs TOF */
      TH2F* h_csi_Evtof1[8];
      /** Energy deposited vs TOF */
      TH2F* h_csi_Evtof2[8];
      /** Energy deposited vs TOF */
      TH2F* h_csi_Evtof3[8];
      /** Energy deposited vs TOF */
      TH2F* h_csi_Evtof4[8];
      /** Energy deposited */
      TH1F* h_csi_edep[8];
      /** Energy deposited */
      TH1F* h_csi_edep_nocut[8];
      /** Energy deposited */
      TH1F* h_csi_edep_test[8];
      /** Scattering position in the rings */
      TH1F* h_csi_s;
      /** Scattering position in the rings */
      TH1F* h_csi_s_cut;
    };

  }
}

#endif /* CSISTUDY_V2MODULE_H */
