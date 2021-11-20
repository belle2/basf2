/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

    /**   This module simulates ECL Luminosity Online Monitor logic,
        i.e. counts events with back-to-back energy depositions
        in the ECL endcaps utilizing coincedence scheme. */
    class ECLLOMModule : public Module {
    public:

      /** Constructor. */
      ECLLOMModule();

      /** Destructor. */
      virtual ~ECLLOMModule();

      /** Initialize variables. */
      virtual void initialize();

      /** begin run.*/
      virtual void beginRun();

      /** event per event. */
      virtual void event();

      /** end run. */
      virtual void endRun();

      /** terminate.*/
      virtual void terminate();

    protected:

    private:

      /** Get MC particles parameters.
       * Requires MCParticle to be stored.
      */
      void get_MCparticles();

      /** Get ECL waveforms comdined into sectors.
       * For both endcaps there are 16 sectors.
       * Requires TRGECLWaveform to be stored.
      */
      void get_waveforms();

      /** Transforms waveforms into discriminators output.
        * Calculates running sums (energy deposition in two consecutive sectors) for endcaps.
        * Unit output is emmited for &DiscrTime [ns] when signal exceeds given threshold.
        * Also calculates and substracts pedestals,
        * and linearly interpolates waveforms from 100ns sampling interval to 10ns.
      */
      void calculate_discr_output();

      /** Calculates amplitude [GeV] in an event for each sector.*/
      void calculate_amplitudes();

      /** Return Quality (topology) flag at sample point, iSample, for Backward Endcap.*/
      bool calculate_BE_quality(int iSample);

      /** Return Quality (topology) flag at sample point, iSample, for Forward Endcap.*/
      bool calculate_FE_quality(int iSample);

      /** Calculates Coincidence Matrix at sample point, iSample.
        * Performs logical multiplication of disciminators signals for each sector (16x16 matrix).
        * Should be called successively on iSample.
      */
      void calculate_coincidence(int iSample);

      /** Clear internal data.*/
      void clear_lom_data();

      //module parameters
      std::string m_lomtestFilename; /**< Name of file to save output.*/
      double BhabhaHitThresholdFE; /**< Threshold [GeV] on signal for Forward Endcap .*/
      double BhabhaHitThresholdBE; /**< Threshold [GeV] on signal for Backward Endcap .*/
      double BackgroundThreshold; /**< Threshold [GeV] on signal when sector considered as lighted. For quality signal calculation.*/
      double DiscrTime; /**< Discriminator's signal duration in ns.*/
      bool includeFEInnerTC; /**< Flag to include Inner part of the Forward Endcap.*/
      bool saveSignal; /**< Flag to save signal wavefroms into file.*/

      //input data
      int m_EvtNum; /**< Event number.*/
      double BE_Waveform_100ns[16][64]; /**< Waveforms with 100ns sampling for Backward Endcap sectors.*/
      double FE_Waveform_100ns[16][64]; /**< Waveforms with 100ns sampling for Forward Endcap sectors.*/
      double mcen[2]; /**< Monte Carlo energy of the final state particles in main frame.*/
      double mcth[2]; /**< Monte Carlo thetha of the final state particles in main frame.*/
      double mcph[2]; /**< Monte Carlo phi of the final state particles in main frame.*/
      double com_en[2]; /**< Monte Carlo energy of the final state particles in CMS frame.*/
      double com_th[2]; /**< Monte Carlo thetha of the final state particles in CMS frame.*/
      double com_ph[2]; /**< Monte Carlo phi of the final state particles in CMS frame.*/

      //important output
      bool isBhabha; /**< Bha-bha signal for an event.*/

      int BhNum; /**< Number of Bha-bha signals in an event. Could be >1.*/
      double BE_Amplitude[16]; /**< Calculated amplitudes in sectors of Backward Endcap.*/
      double FE_Amplitude[16]; /**< Calculated amplitudes in sectors of Forward Endcap.*/
      double BESum_Amplitude[16]; /**< Calculated amplitudes in running sums of Backward Endcap.*/
      double FESum_Amplitude[16]; /**< Calculated amplitudes in running sums of Forward Endcap.*/
      double BE_Pedal[16]; /**< Calculated pedestal values for Backward Endcap.*/
      double FE_Pedal[16]; /**< Calculated pedestal values for Forward Endcap.*/
      double FESum_MaxAmp; /**< Maximum runing sum amplitude in an event for Forward endcap.*/
      double BESum_MaxAmp; /**< Maximum runing sum amplitude in an event for Backward endcap.*/
      int FESum_MaxId; /**< Id of a sector with maximum aplitude in Forward endcap.*/
      int BESum_MaxId; /**< Id of a sector with maximum aplitude in Backward endcap.*/

      //validation output
      TH2D* h2Coin; /**< Store number of coincedencies for i:j sectors (Forward:Backward) over all events.*/
      TH2D* h2SumCoin; /**< Store number of coincedencies in running sums for i:j sectors (Forward:Backward) over all events.*/
      TH2D* h2FEAmp; /**< Store sectors amplitudes for Forward endcap over all events.*/
      TH2D* h2BEAmp; /**< Store sectors amplitudes for Backward endcap over all events.*/
      TH1D* h1BEHits; /**< Store number of events when Backward sector i has signal exceeding Bha-Bha threshold over all events.*/
      TH1D* h1FEHits; /**< Store number of events when Forward sector i has signal exceeding Bha-Bha threshold over all events.*/

      //internal variables
      TFile* testfile; /**< File to save output.*/
      TTree* testtree; /**< Tree to store output.*/
      int NSamples; /**< NSamples=631, number of samples for 10ns sampling.*/
      double BE_Waveform_10ns[16][631]; /**< Waveforms with 10ns sampling for Backward Endcap sectors.*/
      double FE_Waveform_10ns[16][631]; /**< Waveforms with 10ns sampling for Forward Endcap sectors.*/
      double BESum_Waveform_10ns[16][631]; /**< Running sum's waveforms with 10ns sampling for Backward Endcap sectors.*/
      double FESum_Waveform_10ns[16][631]; /**< Running sum's waveforms with 10ns sampling for Forward Endcap sectors.*/
      bool FESum_Discr[16][631]; /**< Discriminators values for running sums of Forward Endcap.*/
      bool BESum_Discr[16][631]; /**< Discriminators values for running sums of Backward Endcap.*/
      bool FEQual_Discr[16][631]; /**< Discriminators values for Quality signal of Forward Endcap.*/
      bool BEQual_Discr[16][631]; /**< Discriminators values for Quality signal of Backward Endcap.*/
      bool isBhabhaPatternFE; /**< Quality signal for Forward endcap.*/
      bool isBhabhaPatternBE; /**< Quality signal for Backward endcap.*/
      int CoincidenceMatrix[16][16]; /**< Stores current coincidence duration [in samples] between waveforms exceeding threshold in i:j sectors (Forward:Backward).*/
      int SumCoincidenceMatrix[16][16]; /**< Stores current coincidence duration [in samples] between runnig sums discriminators in i:j sectors (Forward:Backward).*/
      int CoincidenceCounterMatrix[16][16]; /**< Stores number of concidences between waveforms exceeding threshold in i:j sectors (Forward:Backward).*/
      int SumCoincidenceCounterMatrix[16][16]; /**< Stores number of concidences between runnig sums discriminators in i:j sectors (Forward:Backward).*/
    };
  }//namespace ECL
}//namespace Belle2

#endif


