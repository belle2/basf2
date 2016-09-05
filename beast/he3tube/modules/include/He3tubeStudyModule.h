/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam de Jong                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HE3TUBESTUDYMODULE_H
#define HE3TUBESTUDYMODULE_H

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
  namespace he3tube {

    /**
     * Study module for He3tubes (BEAST)
     *
     * Produces histograms from BEAST data for the He3tubes.   *
     */
    class He3tubeStudyModule : public HistoModule {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      He3tubeStudyModule();

      /**  */
      virtual ~He3tubeStudyModule();

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

      int m_sampletime;  /**< The sample time in us */

      /** Neutron Hits */
      TH1F* h_NeutronHits;
      /** Neutron Hits */
      TH1F* h_NeutronHitsWeighted;
      /** Definite Neutron Hits */
      TH1F* h_DefNeutronHits;
      /** Definite Neutron Hits */
      TH1F* h_DefNeutronHitsWeighted;
      /** Neutron Hits */
      TH2F* h_NeutronHitsVrs;
      /** Neutron Hits */
      TH2F* h_NeutronHitsWeightedVrs;
      /** Definite Neutron Hits */
      TH2F* h_DefNeutronHitsVrs;
      /** Definite Neutron Hits */
      TH2F* h_DefNeutronHitsWeightedVrs;
      /** Energy deposited by Proton and Tritium */
      TH1F* h_Edep1H3H;
      /** Energy deposited by Proton and Tritium in each tube */
      TH1F* h_Edep1H3H_detNB;
      /** Energy deposited by Protons */
      TH1F* h_Edep1H;
      /** Energy deposited by Tritiums */
      TH1F* h_Edep3H;
      /** Momentum of neutrons */
      TH1F* h_TotEdep;
      /** Energy deposited vs detector number  */
      TH1F* h_DetN_Edep;
      /** Neutron Hits per second */
      TH1F* h_NeutronRate;
      /** Definite Neutron Hits per second */
      TH1F* h_DefNeutronRate;
      /** Neutron Hits per second */
      TH2F* h_NeutronRateVrs;
      /** Definite Neutron Hits per second */
      TH2F* h_DefNeutronRateVrs;

      /** Pulse height of waveforms from neutrons*/
      TH1F* h_PulseHeights_Neutron;
      /** Pulse height of waveforms from definite neutrons*/
      TH1F* h_PulseHeights_DefNeutron;


      /** Pulse height of waveforms from non-neutrons */
      TH1F* h_PulseHeights_NotNeutron;
      /** Pulse heught of all waveforms */
      TH1F* h_PulseHeights_All;

      /** MC kin energy dis */
      TH1F* h_mche3_kinetic[10];

      /** Number of neutrons */
      int nNeutronHits = 0;
      /** Number of definte neutrons */
      int nDefiniteNeutron = 0;
      /** number of Tritium hits*/
      int n3Hhits = 0;
      /** number of proton hits*/
      int nPhits = 0;
      /** converts sample time to rate in s*/
      double rateCorrection;

    };

  }
}

#endif /* HE3TUBESTUDYMODULE_H */
