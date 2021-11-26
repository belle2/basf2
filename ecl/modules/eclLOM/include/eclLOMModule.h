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
      virtual void initialize() override;

      /** begin run.*/
      virtual void beginRun() override;

      /** event per event. */
      virtual void event() override;

      /** end run. */
      virtual void endRun() override;

      /** terminate.*/
      virtual void terminate() override;

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
      std::string m_testFileName; /**< Name of file to save output.*/
      double m_thresholdFE; /**< Threshold [GeV] on signal for Forward Endcap .*/
      double m_thresholdBE; /**< Threshold [GeV] on signal for Backward Endcap .*/
      double m_thresholdBkg; /**< Threshold [GeV] on signal when sector considered as lighted. For quality signal calculation.*/
      double m_discrTime; /**< Discriminator's signal duration in ns.*/
      bool m_includeInnerFE; /**< Flag to include Inner part of the Forward Endcap.*/
      bool m_saveSignal; /**< Flag to save signal wavefroms into file.*/

      //input data
      int m_evtNum; /**< Event number.*/
      double m_BE_Waveform_100ns[16][64]; /**< Waveforms with 100ns sampling for Backward Endcap sectors.*/
      double m_FE_Waveform_100ns[16][64]; /**< Waveforms with 100ns sampling for Forward Endcap sectors.*/
      double m_mcen[2]; /**< Monte Carlo energy of the final state particles in main frame.*/
      double m_mcth[2]; /**< Monte Carlo thetha of the final state particles in main frame.*/
      double m_mcph[2]; /**< Monte Carlo phi of the final state particles in main frame.*/
      double m_com_en[2]; /**< Monte Carlo energy of the final state particles in CMS frame.*/
      double m_com_th[2]; /**< Monte Carlo thetha of the final state particles in CMS frame.*/
      double m_com_ph[2]; /**< Monte Carlo phi of the final state particles in CMS frame.*/

      //important output
      bool m_isBhabha; /**< Bha-bha signal for an event.*/

      int m_BhNum; /**< Number of Bha-bha signals in an event. Could be >1.*/
      double m_BE_Amplitude[16]; /**< Calculated amplitudes in sectors of Backward Endcap.*/
      double m_FE_Amplitude[16]; /**< Calculated amplitudes in sectors of Forward Endcap.*/
      double m_BESum_Amplitude[16]; /**< Calculated amplitudes in running sums of Backward Endcap.*/
      double m_FESum_Amplitude[16]; /**< Calculated amplitudes in running sums of Forward Endcap.*/
      double m_BE_Pedal[16]; /**< Calculated pedestal values for Backward Endcap.*/
      double m_FE_Pedal[16]; /**< Calculated pedestal values for Forward Endcap.*/
      double m_FESum_MaxAmp; /**< Maximum runing sum amplitude in an event for Forward endcap.*/
      double m_BESum_MaxAmp; /**< Maximum runing sum amplitude in an event for Backward endcap.*/
      int m_FESum_MaxId; /**< Id of a sector with maximum aplitude in Forward endcap.*/
      int m_BESum_MaxId; /**< Id of a sector with maximum aplitude in Backward endcap.*/

      //validation output
      TH2D* m_h2Coin; /**< Store number of coincedencies for i:j sectors (Forward:Backward) over all events.*/
      TH2D* m_h2SumCoin; /**< Store number of coincedencies in running sums for i:j sectors (Forward:Backward) over all events.*/
      TH2D* m_h2FEAmp; /**< Store sectors amplitudes for Forward endcap over all events.*/
      TH2D* m_h2BEAmp; /**< Store sectors amplitudes for Backward endcap over all events.*/
      TH1D* m_h1BEHits; /**< Store number of events when Backward sector i has signal exceeding Bha-Bha threshold over all events.*/
      TH1D* m_h1FEHits; /**< Store number of events when Forward sector i has signal exceeding Bha-Bha threshold over all events.*/

      //internal variables
      TFile* m_testfile; /**< File to save output.*/
      TTree* m_testtree; /**< Tree to store output.*/
      int m_NSamples; /**< m_NSamples=631, number of samples for 10ns sampling.*/
      double m_BE_Waveform_10ns[16][631]; /**< Waveforms with 10ns sampling for Backward Endcap sectors.*/
      double m_FE_Waveform_10ns[16][631]; /**< Waveforms with 10ns sampling for Forward Endcap sectors.*/
      double m_BESum_Waveform_10ns[16][631]; /**< Running sum's waveforms with 10ns sampling for Backward Endcap sectors.*/
      double m_FESum_Waveform_10ns[16][631]; /**< Running sum's waveforms with 10ns sampling for Forward Endcap sectors.*/
      bool m_FESum_Discr[16][631]; /**< Discriminators values for running sums of Forward Endcap.*/
      bool m_BESum_Discr[16][631]; /**< Discriminators values for running sums of Backward Endcap.*/
      bool m_FEQual_Discr[16][631]; /**< Discriminators values for Quality signal of Forward Endcap.*/
      bool m_BEQual_Discr[16][631]; /**< Discriminators values for Quality signal of Backward Endcap.*/
      bool m_isBhabhaPatternFE; /**< Quality signal for Forward endcap.*/
      bool m_isBhabhaPatternBE; /**< Quality signal for Backward endcap.*/
      int m_CoincidenceMatrix[16][16]; /**< Stores current coincidence duration [in samples] between waveforms exceeding threshold in i:j sectors (Forward:Backward).*/
      int m_SumCoincidenceMatrix[16][16]; /**< Stores current coincidence duration [in samples] between runnig sums discriminators in i:j sectors (Forward:Backward).*/
      int m_CoincidenceCounterMatrix[16][16]; /**< Stores number of concidences between waveforms exceeding threshold in i:j sectors (Forward:Backward).*/
      int m_SumCoincidenceCounterMatrix[16][16]; /**< Stores number of concidences between runnig sums discriminators in i:j sectors (Forward:Backward).*/
    };
  }//namespace ECL
}//namespace Belle2

#endif


