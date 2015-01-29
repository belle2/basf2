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

      /** Energy of Neutrons */
      TH1F* h_NeutronEnergy;
      /** Kinetic Energy of Neutrons */
      TH1F* h_NeutronKE;
      /** Energy deposited by Proton and Tritium */
      TH1F* h_Edep1H3H;
      /** Energy deposited by Proton and Tritium in each tube */
      TH1F* h_Edep1H3H_detNB;
      /** Energy deposited by Protons */
      TH1F* h_Edep1H;
      /** Energy deposited by Tritiums */
      TH1F* h_Edep3H;
      /** Momentum of neutrons */
      TH1F* h_NeutronMomentum;
      /** Total energy deposited */
      TH1F* h_TotEdep;
      /** Energy deposited vs detector number  */
      TH1F* h_DetN_Edep;
      /** Neutron Hits per second */
      TH1F* h_NeutronRate;
      /** Position of Tritium origin */
      TH3F* h_TritiumPositions;
      /** Phi of neutrons */
      TH1F* h_NeutronPhi;
      /** Theta of Neutrons */
      TH1F* h_NeutronTheta;
      /** X coordinate of neutron production vertex */
      TH1F* h_NeutronProdX;
      /** Y coordinate of neutron production vertex */
      TH1F* h_NeutronProdY;
      /** Z coordinate of neutron production vertex */
      TH1F* h_NeutronProdZ;
      /** Pulse height of waveforms */
      TH1F* h_PulseHeights;

      /** Number of neutrons */
      int nNeutronHits = 0;
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
