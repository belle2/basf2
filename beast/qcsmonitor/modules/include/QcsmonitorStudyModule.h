/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef QCSMONITORSTUDYMODULE_H
#define QCSMONITORSTUDYMODULE_H

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
  namespace qcsmonitor {

    /**
     * Study module for Qcsmonitor (BEAST)
     *
     * Produces histograms from BEAST data for the Qcsmonitor.   *
     */
    class QcsmonitorStudyModule : public HistoModule {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      QcsmonitorStudyModule();

      /**  */
      virtual ~QcsmonitorStudyModule();

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

      /** reads data from QCSMONITOR.xml */
      virtual void getXMLData();
      /** Energy threshold */
      double m_Ethres;

      /** Energy deposited vs TOF */
      TH2F* h_qcss_Evtof1[48];
      /** Energy deposited vs TOF */
      TH2F* h_qcss_Evtof2[48];
      /** Energy deposited vs TOF */
      TH2F* h_qcss_Evtof3[48];
      /** Energy deposited vs TOF */
      TH2F* h_qcss_Evtof4[48];
      /** Energy deposited */
      TH1F* h_qcss_edep[48];
      /** Energy deposited */
      TH1F* h_Wqcss_edep[48];

      /** Energy deposited */
      TH1F* h_qcss_rate1[48];
      /** Energy deposited */
      TH1F* h_qcss_rate2[48];
      /** Energy deposited */
      TH1F* h_qcss_rate1W[48];
      /** Energy deposited */
      TH1F* h_qcss_rate2W[48];

      /** Energy deposited */
      TH1F* h_qcss_hitrate1;
      /** Energy deposited */
      TH1F* h_qcss_hitrate2;
      /** Energy deposited */
      TH1F* h_qcss_hitrate1W;
      /** Energy deposited */
      TH1F* h_qcss_hitrate2W;

      /** Energy deposited */
      TH2F* h_qcss_rs_rate1[48];
      /** Energy deposited */
      TH2F* h_qcss_rs_rate2[48];
      /** Energy deposited */
      TH2F* h_qcss_rs_rate1W[48];
      /** Energy deposited */
      TH2F* h_qcss_rs_rate2W[48];

      /** Energy deposited */
      TH2F* h_qcss_pe1[48];
      /** Energy deposited */
      TH2F* h_qcss_pe2[48];
      /** Energy deposited */
      TH2F* h_qcss_pe1W[48];
      /** Energy deposited */
      TH2F* h_qcss_pe2W[48];

    };

  }
}

#endif /* QCSMONITORSTUDYMODULE_H */
