/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MICROTPCDAILYREPORTMODULE_H
#define MICROTPCDAILYREPORTMODULE_H

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
  namespace microtpc {

    /**
     * Study module for Microtpcs (BEAST)
     *
     * Produces histograms from BEAST data for the Microtpcs.   *
     */
    class MicrotpcDailyReportModule : public HistoModule {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      MicrotpcDailyReportModule();

      /**  */
      virtual ~MicrotpcDailyReportModule();

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

      /** Input date of the report */
      int m_inputReportDate;
      /** Input TPC number */
      int m_inputTPCNumber;
      /** number of detectors. Read from MICROTPC.xml*/
      //int nTPC = 0;
      /** TPC coordinate */
      std::vector<TVector3> TPCCenter;

      /** Up time */
      TH1F* h_tpc_uptime[10];
      /** Rates of different particle ID */
      TH1F* h_tpc_rate[10];
      /** Gain stability */
      TH2F* h_tpc_gain[10];
      /** Trigger length */
      TH1F* h_tpc_triglength[20];
      /** Scatter plot theta v phi */
      TH2F* h_tpc_phivtheta[20];
      /** Scatter plot theta v phi weighted by the energy */
      TH2F* h_tpc_phivtheta_w[20];
      /** Tracl length v energy deposited */
      TH2F* h_tpc_edepvtrl[20];
      /** HER current */
      TH1F* h_iher[10];
      /** HER average pressure */
      TH1F* h_pher[10];
      /** HER current */
      TH1F* h_iler[10];
      /** HER average pressure */
      TH1F* h_pler[10];
      /** Impact parameter v. phi */
      TH2F* h_tpc_yvphi[10];
      /** Rates of different particle ID correlated to LER beams */
      TH1F* h_tpc_rate_ler[4][10];
      /** Rates of different particle ID correlated to HER beams */
      TH1F* h_tpc_rate_her[4][10];
      /** TPC flow rate */
      TH1F* h_tpc_flow[2];
      /** TPC pressure */
      TH1F* h_tpc_pressure[2];
      /** TPC temperature */
      TH1F* h_tpc_temperature[4];
      /** Counter */
      int Ctr[8];
      /** tpc rate */
      TH1F* h_tpc_rates;
      /** tpc dose */
      TH1F* h_tpc_doses[10];
      /** tpc dose */
      float tpc_dose[8][5];
      /** scale factor */
      double scFac;
      /** tpc dose */
      TH1F* h_tpc_Doses;
    };

  }
}

#endif /* MICROTPCDAILYREPORTMODULE_H */
