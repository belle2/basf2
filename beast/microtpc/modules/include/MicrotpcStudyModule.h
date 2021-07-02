/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef MICROTPCSTUDYMODULE_H
#define MICROTPCSTUDYMODULE_H

#include <framework/core/HistoModule.h>
#include <vector>

#include <TVector3.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TGraph.h>

namespace Belle2 {
  namespace microtpc {

    /**
     * Study module for Microtpcs (BEAST)
     *
     * Produces histograms from BEAST data for the Microtpcs.   *
     */
    class MicrotpcStudyModule : public HistoModule {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      MicrotpcStudyModule();

      /**
       * Destructor
       */
      virtual ~MicrotpcStudyModule();

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

      /** reads data from MICROTPC.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

      /** Chip column number */
      int m_ChipColumnNb;
      /** Chip row number */
      int m_ChipRowNb;
      /** Chip column x dimension */
      double m_ChipColumnX;
      /** Chip row y dimension */
      double m_ChipRowY;
      /** z drift gap */
      double m_z_DG;
      /** number of detectors. Read from MICROTPC.xml*/
      int nTPC = 0;
      /** TPC coordinate */
      std::vector<TVector3> TPCCenter;
      /** Event counter */
      //int Event = 0;
      /** rate */
      TH1F* h_tpc_rate[20];
      /** Neutron kin energy dis */
      TH2F* h_mctpc_kinetic[20];
      /** Neutron kin energy dis */
      TH2F* h_mctpc_kinetic_zoom[20];
      /** theta v phi dis */
      TH2F* h_mctpc_tvp[20];
      /** theta v phi dis */
      TH2F* h_mctpc_tvpW[20];
      /** r v z */
      TH2F* h_mctpc_zr[20];
      /** Charged density vs z vs section */
      TH1F* h_z[8];
      /** Charged density vs x vs y */
      TH2F* h_xy[8];
      /** Charged density vs z vs r */
      TH2F* h_zr[8];
      /** Charged density vs x vs r */
      TH2F* h_zx[8];
      /** Charged density vs y vs r */
      TH2F* h_zy[8];
      /** Track length v. energy deposited per TPC */
      TH2F* h_evtrl[8];
      /** Track length v. energy deposited per TPC */
      TH2F* h_evtrlb[8];
      /** Track length v. energy deposited per TPC */
      TH2F* h_evtrlc[8];
      /** Track length v. energy deposited per TPC */
      TH2F* h_evtrld[8];
      /** Track length v. energy deposited per TPC for He */
      TH2F* h_evtrl_He[8];
      /** Track length v. energy deposited per TPC for He and x */
      TH2F* h_evtrl_Hex[8];
      /** Track length v. energy deposited per TPC for He w/ edge cuts */
      TH2F* h_evtrl_He_pure[8];
      /** Track length v. energy deposited per TPC for p */
      TH2F* h_evtrl_p[8];
      /** Track length v. energy deposited per TPC for O */
      TH2F* h_evtrl_O[8];
      /** Track length v. energy deposited per TPC for C */
      TH2F* h_evtrl_C[8];
      /** Track length v. energy deposited per TPC for X-ray */
      TH2F* h_evtrl_x[8];

      /** Track length v. energy deposited per TPC */
      TH2F* h_tevtrl[8];
      /** Track length v. energy deposited per TPC for He */
      TH2F* h_tevtrl_He[8];
      /** Track length v. energy deposited per TPC for He and x */
      TH2F* h_tevtrl_Hex[8];
      /** Track length v. energy deposited per TPC for He w/ edge cuts */
      TH2F* h_tevtrl_He_pure[8];
      /** Track length v. energy deposited per TPC for p */
      TH2F* h_tevtrl_p[8];
      /** Track length v. energy deposited per TPC for O */
      TH2F* h_tevtrl_O[8];
      /** Track length v. energy deposited per TPC for C */
      TH2F* h_tevtrl_C[8];
      /** Track length v. energy deposited per TPC for X-ray */
      TH2F* h_tevtrl_x[8];

      /** Phi v. theta per TPC */
      TH2F* h_tvp[8];
      /** Phi v. theta per TPC */
      TH2F* h_tvpb[8];
      /** Phi v. theta per TPC */
      TH2F* h_tvpc[8];
      /** Phi v. theta per TPC */
      TH2F* h_tvpd[8];
      /** Phi v. theta per TPC */
      TH2F* h_wtvpb[8];
      /** Phi v. theta per TPC */
      TH2F* h_wtvpc[8];
      /** Phi v. theta per TPC */
      TH2F* h_wtvpd[8];
      /** Phi v. theta per TPC for He */
      TH2F* h_tvp_He[8];
      /** Phi v. theta per TPC for He and x */
      TH2F* h_tvp_Hex[8];
      /** Phi v. theta per TPC for He w/ edge cuts */
      TH2F* h_tvp_He_pure[8];
      /** Phi v. theta per TPC for p */
      TH2F* h_tvp_p[8];
      /** Phi v. theta per TPC for O */
      TH2F* h_tvp_O[8];
      /** Phi v. theta per TPC for C */
      TH2F* h_tvp_C[8];
      /** Phi v. theta per TPC for X-ray */
      TH2F* h_tvp_x[8];

      /** Phi v. theta per TPC */
      TH2F* h_ttvp[8];
      /** Phi v. theta per TPC for He */
      TH2F* h_ttvp_He[8];
      /** Phi v. theta per TPC for He and x */
      TH2F* h_ttvp_Hex[8];
      /** Phi v. theta per TPC for He w/ edge cuts */
      TH2F* h_ttvp_He_pure[8];
      /** Phi v. theta per TPC for p */
      TH2F* h_ttvp_p[8];
      /** Phi v. theta per TPC for O */
      TH2F* h_ttvp_O[8];
      /** Phi v. theta per TPC for C */
      TH2F* h_ttvp_C[8];
      /** Phi v. theta per TPC for X-ray */
      TH2F* h_ttvp_x[8];
      /** Phi v. theta */
      TH2F* h_Wtvp1[8][12];
      /** Phi v. theta */
      TH2F* h_Wtvp2[8][12];
      /** e v l */
      TH2F* h_Wevtrl1[8][12];
      /** e v l */
      TH2F* h_Wevtrl2[8][12];
      /** Phi v. theta per TPC for He w/ edge cuts weighted */
      TH2F* h_twtvp_He_pure[8];
      /** recoil energy */
      TH3F* h_mctpc_recoil[3];
      /** weighted recoil energy */
      TH3F* h_mctpc_recoilW[3];
      /** vector of interaction probability vs E graphs for all recoils*/
      std::vector<TGraph*> m_intProb;
      /** vector of maximal energy fraction transfered to recoil*/
      std::vector<double> m_maxEnFrac;
      /** Phi v. theta, weighted per TPC */
      TH2F* h_wtvp[8];
      /** Phi v. theta, weighted per TPC for He */
      TH2F* h_wtvp_He[8];
      /** Phi v. theta, weighted per TPC for He and x */
      TH2F* h_wtvp_Hex[8];
      /** Phi v. theta, weighted per TPC for He w/ edge cuts */
      TH2F* h_wtvp_He_pure[8];
      /** Phi v. theta, weighted per TPC for p */
      TH2F* h_wtvp_p[8];
      /** Phi v. theta, weighted per TPC for O */
      TH2F* h_wtvp_O[8];
      /** Phi v. theta, weighted per TPC for C */
      TH2F* h_wtvp_C[8];
      /** Phi v. theta, weighted per TPC for X-ray */
      TH2F* h_wtvp_x[8];
      /** X-ray boolean per TPC */
      Bool_t xRec[8];
      /** p boolean per TPC */
      Bool_t pRec[8];
      /** He boolean per TPC */
      Bool_t HeRec[8];
      /** O boolean per TPC */
      Bool_t ORec[8];
      /** C boolean per TPC */
      Bool_t CRec[8];
      /** A boolean per TPC */
      //Bool_t ARec[8];
      /** First hit of ... */
      int pid_old[8];

      /** Number of proton recoils */
      //int npRecoils = 0;
      /** Number of alpha recoils */
      //int nHeRecoils = 0;
      /** Number of carbon recoils */
      //int nCRecoils = 0;
      /** Number of oxygen recoils */
      //int nORecoils = 0;

    };

  }
}

#endif /* MICROTPCSTUDYMODULE_H */
