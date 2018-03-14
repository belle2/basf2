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
#include <framework/datastore/StoreArray.h>
#include <beast/csi/dataobjects/CsiSimHit.h>
#include <beast/csi/dataobjects/CsiHit_v2.h>
#include <generators/SAD/dataobjects/SADMetaHit.h>

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
