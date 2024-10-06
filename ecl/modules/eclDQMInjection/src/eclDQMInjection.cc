/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclDQMInjection/eclDQMInjection.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/utility/ECLDspUtilities.h>

/* Boost headers. */
#include <boost/format.hpp>
#include <boost/range/combine.hpp>

/* ROOT headers. */
#include <TDirectory.h>

/* C++ headers. */
#include <stdexcept>

using namespace std;
using namespace Belle2;
using namespace Belle2::ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLDQMInjection);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLDQMInjectionModule::ECLDQMInjectionModule()
  : HistoModule(),
    m_calibrationThrApsd("ECL_FPGA_StoreWaveform")
{
  //Set module properties
  setDescription("Monitor Occupancy after Injection");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("ECLINJ"));
  addParam("ECLDigitsName", m_ECLDigitsName, "Name of ECL hits", std::string(""));
  // BeamRevolutionCycle is set based on 'Timing distribution for the Belle II
  // data acquisition system'. RF clock of 508 MHz is synchronized to
  // beam-revolution cycle (5120 RF bunches in one cycle).
  addParam("BeamRevolutionCycle", m_revolutionTime, "Beam revolution cycle in musec", 5120 / 508.);
  addParam("ECLThresholdforVetoTuning", m_ECLThresholdforVetoTuning, "ECL Threshold for injection veto tuning, ADC channels", 400.);
  addParam("DPHYTTYP", m_DPHYTTYP,
           "Flag to control trigger of delayed bhabha events; 0 - select events by 'bha_delay' trigger bit, 1 - select by TTYP_DPHY", false);

}

void ECLDQMInjectionModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str());// do not rely on return value, might be ZERO
  oldDir->cd(m_histogramDirectoryName.c_str());//changing to the right directory

  hHitsAfterInjLER  = new TH1F("ECLHitsInjLER", "ECLHitsInjLER/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
  hHitsAfterInjHER  = new TH1F("ECLHitsInjHER", "ECLHitsInjHER/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
  hEHitsAfterInjLER  = new TH1F("ECLEHitsInjLER", "ECLEHitsInjLER/Time;Time in #mus;Triggers/Time (5 #mus bins)", 4000, 0, 20000);
  hEHitsAfterInjHER  = new TH1F("ECLEHitsInjHER", "ECLEHitsInjHER/Time;Time in #mus;Triggers/Time (5 #mus bins)", 4000, 0, 20000);
  hBurstsAfterInjLER = new TH1F("ECLBurstsInjLER", "ECLBurstsInjLER/Time;Time in #mus;Count/Time (1 #mus bins)", 20000, 0, 20000);
  hBurstsAfterInjHER = new TH1F("ECLBurstsInjHER", "ECLBurstsInjHER/Time;Time in #mus;Count/Time (1 #mus bins)", 20000, 0, 20000);
  hEBurstsAfterInjLER = new TH1F("ECLEBurstsInjLER", "ECLEBurstsInjLER/Time;Time in #mus;Triggers/Time (1 #mus bins)", 20000, 0,
                                 20000);
  hEBurstsAfterInjHER = new TH1F("ECLEBurstsInjHER", "ECLEBurstsInjHER/Time;Time in #mus;Triggers/Time (1 #mus bins)", 20000, 0,
                                 20000);
  hVetoAfterInjLER = new TH2F("ECLVetoAfterInjLER",
                              "ECL Hits for LER veto tuning (E > 20 MeV);"
                              "Time since last injection in #mus;Time within beam cycle in #mus", 500, 0, 30000, 100, 0,
                              m_revolutionTime);
  hVetoAfterInjHER = new TH2F("ECLVetoAfterInjHER",
                              "ECL Hits for HER veto tuning (E > 20 MeV);"
                              "Time since last injection in #mus;Time within beam cycle in #mus", 500, 0, 30000, 100, 0,
                              m_revolutionTime);
  hOccAfterInjLER = new TH2F("ECLOccAfterInjLER",
                             "ECL Occupancy after LER injection (E > 1 MeV);Time since last injection in #mus;Occupancy (Nhits/8736) [%]",
                             100, 0, 20000, 98, 2, 100);
  hOccAfterInjHER = new TH2F("ECLOccAfterInjHER",
                             "ECL Occupancy after HER injection (E > 1 MeV);Time since last injection in #mus;Occupancy (Nhits/8736) [%]",
                             100, 0, 20000, 98, 2, 100);

  hInjkickTimeShift[0] = new TH2F("ECLInjkickTimeShiftLER",
                                  "LER Injection peak position in ECL data;"
                                  "Time within beam cycle [ADC ticks];"
                                  "Inj peak position [ADC ticks]",
                                  18, 0, 18, 16, 0, 16);
  hInjkickTimeShift[1] = new TH2F("ECLInjkickTimeShiftHER",
                                  "HER Injection peak position in ECL data;"
                                  "Time within beam cycle [ADC ticks];"
                                  "Inj peak position [ADC ticks]",
                                  18, 0, 18, 16, 0, 16);

  //== Fill h_ped_peak vector

  m_ped_peak_range = {
    1.0, 1.5, 2.0, 4, 6, 8, 10
  };
  int ped_peak_range_count = m_ped_peak_range.size() - 1;

  static const std::string part_names[] = {"fwd", "bar", "bwd"};
  static const std::string title_suffix[] = {
    "in fwd endcap", "in barrel", "in bwd endcap"
  };

  for (int ler_her = 0; ler_her < 2; ler_her++) {
    std::string ring_name = (ler_her == 0) ? "LER" : "HER";
    for (int part = 0; part < 3; part++) {
      std::string suffix = title_suffix[part];
      for (int i = 0; i < ped_peak_range_count; i++) {
        float min_time = m_ped_peak_range[i];
        float max_time = m_ped_peak_range[i + 1];
        std::string name, title;
        name  = str(boost::format("ped_peak_%s_%s_%d") %
                    ring_name % part_names[part] % i);
        title = str(boost::format("Peak height %.1f-%.1f ms after %s inj %s") %
                    min_time % max_time % ring_name % suffix);

        auto h = new TH1F(name.c_str(), title.c_str(), 300, 0.0, 0.3);
        h->GetXaxis()->SetTitle("Peak height in first 16 points [GeV]");

        h_ped_peak.push_back(h);
      }
    }
  }

  // Initialize coefficients used by pedestalFit function
  ECLDspUtilities::initPedestalFit();

  // cd back to root directory
  oldDir->cd();
}

void ECLDQMInjectionModule::initialize()
{
  REG_HISTOGRAM
  m_rawTTD.isOptional(); /// TODO better use isRequired(), but RawFTSW is not in sim, thus tests are failin
  m_storeHits.isRequired(m_ECLDigitsName);
  m_ECLTrigs.isOptional();
  m_ECLDsps.isOptional();
  m_l1Trigger.isOptional();

  if (!mapper.initFromDB()) B2FATAL("ECL Display:: Can't initialize eclChannelMapper");

  v_totalthrApsd.resize((m_calibrationThrApsd->getCalibVector()).size());
  for (size_t i = 0; i < v_totalthrApsd.size(); i++) v_totalthrApsd[i] = (int)(m_calibrationThrApsd->getCalibVector())[i];
}

void ECLDQMInjectionModule::beginRun()
{
  // Assume that everthing is non-yero ;-)
  hHitsAfterInjLER->Reset();
  hHitsAfterInjHER->Reset();
  hEHitsAfterInjLER->Reset();
  hEHitsAfterInjHER->Reset();
  hBurstsAfterInjLER->Reset();
  hBurstsAfterInjHER->Reset();
  hEBurstsAfterInjLER->Reset();
  hEBurstsAfterInjHER->Reset();
  hVetoAfterInjLER->Reset();
  hVetoAfterInjHER->Reset();
  hOccAfterInjHER->Reset();
  hOccAfterInjLER->Reset();
}

void ECLDQMInjectionModule::event()
{
  bool bhatrig = false;

  if (m_l1Trigger.isValid() && m_DPHYTTYP) bhatrig = m_l1Trigger->getTimType() == TRGSummary::ETimingType::TTYP_DPHY;
  else if (m_l1Trigger.isValid() && !m_DPHYTTYP) {
    try { bhatrig = m_l1Trigger->testInput("bha_delay"); }
    catch (const std::exception&) { bhatrig = false; }
  }

  if (m_eventmetadata.isValid() && m_eventmetadata->getErrorFlag() != 0x10) {
    m_iEvent = m_eventmetadata->getEvent();
  } else m_iEvent = -1;

  int amps[ECLElementNumbers::c_NCrystals] = {};
  unsigned int ECLDigitsAboveThr = 0; // Threshold is set to 20 MeV
  unsigned int ECLDigitsAboveThr1MeV = 0;
  for (const auto& aECLDigit : m_storeHits) {
    int amp = aECLDigit.getAmp();
    amps[aECLDigit.getCellId() - 1] = amp;
    if (amp > m_ECLThresholdforVetoTuning) ECLDigitsAboveThr += 1;
    if (amp > 20) ECLDigitsAboveThr1MeV += 1;
  }

  int discarded_wfs = 0;
  for (auto& aECLTrig : m_ECLTrigs) {
    int crate = aECLTrig.getTrigId();
    int suppress = aECLTrig.getBurstSuppressionMask();
    int shaper_pos = 0;
    while (suppress) {
      shaper_pos ++;
      bool shaper_bit = suppress & 1;
      if (shaper_bit) {
        if (m_iEvent % 1000 == 999 || (m_l1Trigger.isValid() &&  m_l1Trigger->getTimType() == TRGSummary::ETimingType::TTYP_RAND) ||
            (m_l1Trigger.isValid() && bhatrig)) {
          for (int channel_pos = 0; channel_pos < 16; channel_pos ++) {
            if (mapper.getCellId(crate, shaper_pos, channel_pos) > 0) discarded_wfs += 1;
          }
        } else {
          for (int channel_pos = 0; channel_pos < 16; channel_pos ++) {
            int cid = mapper.getCellId(crate, shaper_pos, channel_pos);
            if (cid > 0 && amps[cid - 1] >= (v_totalthrApsd[cid - 1] / 4 * 4)) discarded_wfs += 1;
          }
        }
      }
      suppress >>= 1;
    }
  }

  for (auto& it : m_rawTTD) {
    B2DEBUG(29, "TTD FTSW : " << hex << it.GetTTUtime(0) << " " << it.GetTTCtime(0) << " EvtNr " << it.GetEveNo(0)  << " Type " <<
            (it.GetTTCtimeTRGType(0) & 0xF) << " TimeSincePrev " << it.GetTimeSincePrevTrigger(0) << " TimeSinceInj " <<
            it.GetTimeSinceLastInjection(0) << " IsHER " << it.GetIsHER(0) << " Bunch " << it.GetBunchNumber(0));

    // get last injection time
    auto difference = it.GetTimeSinceLastInjection(0);
    // check time overflow, too long ago
    if (difference == 0x7FFFFFFF) continue;

    unsigned int all = m_storeHits.getEntries();
    float diff2 = difference / 127.; //  127MHz clock ticks to us, inexact rounding

    // Time within beam revolution (in 127 MHz ticks)
    int time_within_cycle = difference % 1280;
    // Time within beam revolution (in microseconds)
    double time_in_cycle_us = time_within_cycle / 127.;
    // Time within beam revolution (in ADC ticks)
    // https://xwiki.desy.de/xwiki/rest/p/4630a
    int time_within_cycle_adc_ticks = (1280 - time_within_cycle) / 72;

    int is_her = it.GetIsHER(0);

    if (is_her < 0 || is_her > 1) continue;

    if (is_her) {
      hHitsAfterInjHER->Fill(diff2, all);
      hEHitsAfterInjHER->Fill(diff2);
      hBurstsAfterInjHER->Fill(diff2, discarded_wfs);
      hEBurstsAfterInjHER->Fill(diff2);
      hVetoAfterInjHER->Fill(diff2, time_in_cycle_us, ECLDigitsAboveThr);
      if (all > 0) hOccAfterInjHER->Fill(diff2, ECLDigitsAboveThr1MeV * 100.0 / ECLElementNumbers::c_NCrystals);
    } else {
      hHitsAfterInjLER->Fill(diff2, all);
      hEHitsAfterInjLER->Fill(diff2);
      hBurstsAfterInjLER->Fill(diff2, discarded_wfs);
      hEBurstsAfterInjLER->Fill(diff2);
      hVetoAfterInjLER->Fill(diff2, time_in_cycle_us, ECLDigitsAboveThr);
      if (all > 0) hOccAfterInjLER->Fill(diff2, ECLDigitsAboveThr1MeV * 100.0 / ECLElementNumbers::c_NCrystals);
    }

    //== Filling h_ped_peak histograms
    int range_count = m_ped_peak_range.size() - 1;
    if (diff2 < m_ped_peak_range[range_count] * 1000) {
      //== Identify which histogram to fill (according to inj time range)
      int range_id;
      for (range_id = 0; range_id < range_count; range_id++) {
        // Converting from ms to us
        float min_time = m_ped_peak_range[range_id    ] * 1000;
        float max_time = m_ped_peak_range[range_id + 1] * 1000;
        if (diff2 > min_time && diff2 < max_time) break;
      }
      //== Find pedestal peaks in all available waveforms
      if (range_id < range_count) {
        for (auto& aECLDsp : m_ECLDsps) {
          auto result = ECLDspUtilities::pedestalFit(aECLDsp.getDspA());

          //== Identify which histogram to fill (HER/LER,FWD/BAR/BWD)
          int cid = aECLDsp.getCellId();
          int part_id = 0;              // forward endcap
          if (ECLElementNumbers::isBarrel(cid)) part_id = 1; // barrel
          if (ECLElementNumbers::isBackward(cid)) part_id = 2; // backward endcap

          int hist_id = is_her * 3 * range_count + part_id * range_count + range_id;
          // NOTE: We are using the approximate conversion to energy here.
          // (20'000 ADC counts ~= 1 GeV)
          h_ped_peak[hist_id]->Fill(result.amp / 2e4);
        }
      }
    }

    //== Filling hInjkickTimeShift histograms

    if (diff2 < 10e3) {
      for (auto& aECLDsp : m_ECLDsps) {
        int adc[31];
        aECLDsp.getDspA(adc);
        // Do a naive estimate of inj peak position by
        // searching for the maximum ADC sample in the
        // pedestal part of the waveform.
        int* ped_max = std::max_element(adc, adc + 16);
        int* ped_min = std::min_element(adc, adc + 16);
        // The waveform should have at least ~10 MeV peak amplitude
        if (*ped_max - *ped_min < 200) continue;
        int max_ped_id = ped_max - adc;
        hInjkickTimeShift[is_her]->Fill(time_within_cycle_adc_ticks, max_ped_id);
      }
    }


    break;
  }
}
