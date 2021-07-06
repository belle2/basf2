/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef QCSMONITORSTUDYMODULE_H
#define QCSMONITORSTUDYMODULE_H

#include <framework/core/HistoModule.h>

#include <TH1.h>
#include <TH2.h>

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

      /**
       * Destructor
       */
      virtual ~QcsmonitorStudyModule();

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
      TH1F* h_qcss_hitrate0;
      /** Energy deposited */
      TH1F* h_qcss_hitrate1;
      /** Energy deposited */
      TH1F* h_qcss_hitrate2;
      /** Energy deposited */
      TH1F* h_qcss_hitrate1W;
      /** Energy deposited */
      TH1F* h_qcss_hitrate2W;

      /** Energy deposited */
      TH2F* h_qcss_rs_hitrate1;
      /** Energy deposited */
      TH2F* h_qcss_rs_hitrate2;
      /** Energy deposited */
      TH2F* h_qcss_rs_hitrate1W;
      /** Energy deposited */
      TH2F* h_qcss_rs_hitrate2W;

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
