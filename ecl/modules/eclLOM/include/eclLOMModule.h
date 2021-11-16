/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Shebalin Vasily, Evgeniy Kovalenko                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef ECLLOMMODULE_H
#define ECLLOMMODULE_H


#include <framework/datastore/StoreArray.h>

#include <framework/core/Module.h>
#include "TTree.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"

namespace Belle2 {
  namespace ECL {
    class ECLLOMModule : public Module {
    public:
      ECLLOMModule();
      virtual ~ECLLOMModule();
      virtual void initialize();
      virtual void beginRun();
      virtual void event();
      virtual void endRun();
      virtual void terminate();

    protected:

    private:
      void get_MCparticles();
      void get_waveforms();
      void calculate_discr_output();
      void calculate_amplitudes();
      bool calculate_BE_quality(int iSample);
      bool calculate_FE_quality(int iSample);
      void calculate_coincidence(int iSample);
      void clear_lom_data();
      /** Event number */
      int    m_EvtNum;
      // for tests:
      TFile* testfile;
      TTree* testtree;

      std::string m_lomtestFilename;

      TH2D* h2Coin;
      TH2D* h2SumCoin;
      TH2D* h2FEAmp;
      TH2D* h2BEAmp;

      TH1D* h1BEHits;
      TH1D* h1FEHits;

      int NSamples;
      double m_wf[64];
      int    m_tcid;

      double BE_Waveform_100ns[16][64];
      double FE_Waveform_100ns[16][64];
      double BE_Waveform_10ns[16][631];
      double FE_Waveform_10ns[16][631];

      //double BESumWaveform_100ns[16][64]; // in FAM simulation sampling interval is 100 ns (or 125?)
      //double FESumWaveform_100ns[16][64];
      double BESum_Waveform_10ns[16][631];
      double FESum_Waveform_10ns[16][631];
      bool FESum_Discr[16][631]; // massives for main discriminator's signals
      bool BESum_Discr[16][631];
      bool FEQual_Discr[16][631]; //for quality discr
      bool BEQual_Discr[16][631];

      double BE_Amplitude[16];
      double FE_Amplitude[16];
      double BESum_Amplitude[16];
      double FESum_Amplitude[16];

      int CoincidenceMatrix[16][16];
      int SumCoincidenceMatrix[16][16];
      int SumCoincidenceMatrixMax[16][16];

      double BhabhaHitThresholdFE; // threshold on the endcap sector signal  amplitude
      double BhabhaHitThresholdBE; // threshold on the endcap sector signal  amplitude
      double BackgroundThreshold; // Quality threshold

      double DiscrTime; // Discriminator's signal duration in ns

      bool includeFEInnerTC;
      bool saveSignal;

      bool isBhabhaPatternFE; //quality signal G_f
      bool isBhabhaPatternBE; //quality signal G_b

      bool isBhabha; // Bhabha signal BH signal
      int BhNum;
      // scalers
      int CoincidenceCounterMatrix[16][16];
      int SumCoincidenceCounterMatrix[16][16];

      double FESum_MaxAmp;
      double BESum_MaxAmp;
      int FESum_MaxId;
      int BESum_MaxId;
      int maxcoidur;

      double mcen[2];
      double mcth[2];
      double mcph[2];

      double com_en[2];
      double com_th[2];
      double com_ph[2];

      double BE_Pedal[16];
      double FE_Pedal[16];
    };
  }//namespace ECL
}//namespace Belle2

#endif


