/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef CSISTUDY_V2MODULE_H
#define CSISTUDY_V2MODULE_H

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <beast/csi/dataobjects/CsiSimHit.h>
#include <beast/csi/dataobjects/CsiHit_v2.h>
#include <generators/SAD/dataobjects/SADMetaHit.h>

#include <TH1.h>
#include <TH2.h>

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

      /**
       * Destructor
       */
      virtual ~CsiStudy_v2Module();

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

      StoreArray<CsiSimHit>  SimHits; /**< Array of sim hits */
      StoreArray<CsiHit_v2>  Hits; /**< Array of digi hits */
      StoreArray<SADMetaHit> MetaHits; /**< Array of SAD particle */

      /** reads data from CSI.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();
      /** Energy threshold */
      double m_Ethres;

      // These histrograms are currently
      // not used in the code, but just declared.
      // Commented out by the SW shifter, March 14th 2018
      /** Rate */
      //TH1F* h_csi_rate[2];
      /** Rate */
      //TH2F* h_csi_rs_rate[2];
      /** Energy deposited vs TOF */
      //TH2F* h_csi_Evtof1[18];
      /** Energy deposited vs TOF */
      //TH2F* h_csi_Evtof2[18];
      /** Energy deposited vs TOF */
      //TH2F* h_csi_Evtof3[18];
      /** Energy deposited vs TOF */
      //TH2F* h_csi_Evtof[18];
      /** Energy deposited */
      //TH1F* h_csi_edep[18];
      /** Energy deposited */
      //TH1F* h_csi_edep1[18];
      /** Energy deposited */
      //TH1F* h_csi_edep2[18];
      /** Energy deposited */
      //TH1F* h_csi_edep1Weight[18];
      /** Energy deposited */
      //TH1F* h_csi_edep2Weight[18];
      /** Energy deposited */
      //TH2F* h_csi_rs_edep1[18];
      /** Energy deposited */
      //TH2F* h_csi_rs_edep2[18];
      /** Energy deposited */
      //TH2F* h_csi_rs_edep1Weight[18];
      /** Energy deposited */
      //TH2F* h_csi_rs_edep2Weight[18];
      /** Energy deposited per section */
      //TH2F* h_csi_energyVrs1[18];
      /** Energy deposited per section */
      //TH2F* h_csi_energyVrs2[18];
      /** Energy deposited per section */
      //TH2F* h_csi_energyVrs3[18];
      /** Energy deposited per section */
      //TH2F* h_csi_energyVrs1W[18];
      /** Energy deposited per section */
      //TH2F* h_csi_energyVrs2W[18];
      /** Energy deposited per section */
      //TH2F* h_csi_energyVrs3W[18];
      /** Energy deposited for all section */
      //TH1F* h_csi_energy1[18];
      /** Energy deposited for all section */
      //TH1F* h_csi_energy2[18];
      /** Energy deposited for all section */
      //TH1F* h_csi_energy3[18];
      /** Energy deposited for all section */
      //TH1F* h_csi_energy1W[18];
      /** Energy deposited for all section */
      //TH1F* h_csi_energy2W[18];
      /** Energy deposited for all section */
      //TH1F* h_csi_energy3W[18];

      /** Energy threshold */
      double m_Thres_hitRate[18][200];
      /** Energy threshold */
      double m_Thres_sumE[18][200];

      /** Rate */
      TH1F* h_csi_drate[153];
      /** Rate */
      TH2F* h_csi_rs_drate[153];
      /** Energy */
      TH1F* h_csi_dedep[18][153];
      /** Energy */
      TH2F* h_csi_rs_dedep[18][153];
      /** Energy */
      TH1F* h_csi_denergy[18][153];
      /** Energy */
      TH2F* h_csi_rs_denergy[18][153];

    };

  }
}

#endif /* CSISTUDY_V2MODULE_H */
