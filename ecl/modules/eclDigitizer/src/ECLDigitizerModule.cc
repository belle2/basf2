/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclDigitizer/ECLDigitizerModule.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLDspWithExtraMCInfo.h>
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLTrig.h>
#include <ecl/dataobjects/ECLWaveforms.h>
#include <ecl/dbobjects/ECLWaveformData.h>
#include <ecl/digitization/BitStream.h>
#include <ecl/digitization/ECLCompress.h>
#include <ecl/digitization/shaperdsp.h>
#include <ecl/digitization/shaperdspUp.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/utility/ECLDspUtilities.h>

/* Basf2 headers. */
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

/* ROOT headers. */
#include <TFile.h>
#include <TRandom.h>
#include <TTree.h>
#include <array>
#include <type_traits>

using namespace std;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLDigitizer);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLDigitizerModule::ECLDigitizerModule() : Module()
{
  //Set module properties
  setDescription("Creates ECLDigiHits from ECLHits.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("TriggerTime", m_trigTime,
           "Flag to use crate trigger times from beam background overlay if there are any (default: false)", false);
  addParam("Background", m_background, "Flag to use the Digitizer configuration with backgrounds (default: false)", false);
  addParam("Calibration", m_calibration, "Flag to use the Digitizer for Waveform fit Covariance Matrix calibration (default: false)",
           false);
  addParam("DiodeDeposition", m_inter,
           "Flag to take into account energy deposition in photodiodes; Default diode is sensitive detector (default: true)", true);
  addParam("NoElectronicNoise", m_noElectronicNoise,
           "Flag to disable electronic noise when generating waveforms (default: false)", false);
  addParam("WaveformMaker", m_waveformMaker, "Flag to produce background waveform digits (default: false)", false);
  addParam("CompressionAlgorithm", m_compAlgo,
           "Waveform compression algorithm (-1: automatic, 0 for current electronics, 1 for upgraded electronics)", -1);
  addParam("eclWaveformsName", m_eclWaveformsName, "Name of the output/input collection (digitized waveforms)", string(""));
  addParam("HadronPulseShapes", m_hadronPulseShape, "Flag to include hadron component in pulse shape construction (default: true)",
           true);
  addParam("ADCThreshold", m_adcThreshold, "ADC threshold for waveform fits (default: 25)", 25);
  addParam("WaveformThresholdOverride", m_waveformThresholdOverride,
           "If gt 0 value is applied to all crystals for waveform saving threshold. If lt 0 dbobject is used. (GeV)", -1.0);
  addParam("StoreDspWithExtraMCInfo", m_storeDspWithExtraMCInfo,
           "Flag to store Dsp with extra MC information in addition to normal Dsp (default: false)", false);
  addParam("DspWithExtraMCInfoThreshold", m_dspWithExtraMCInfoThreshold,
           "Threshold above with to store Dsp with extra MC information [GeV]",
           0.02);
  addParam("DspDataTest", m_dspDataTest,
           "Use DSP coefficients from the database for the processing. This "
           "will significantly reduce performance so this option is to be "
           "used for testing only.", false);
  addParam("unitscale", m_unitscale,
           "Normalization coefficient for ECL signal shape. "
           "If positive, use same static value for all ECL channels. "
           "If negative, calculate dynamically at beginRun().", -1.0);
  addParam("ECLupgrade", m_eclUpgradeOverride,
           "Simulate ECL upgraded electronics digitization: -1 from the payload ECLElectronicsConfiguration, "
           "0 current electronics, 1 upgraded electronics (default: -1)", -1);

}

void ECLDigitizerModule::initialize()
{
  if (m_eclUpgradeOverride < -1 || m_eclUpgradeOverride > 1)
    B2FATAL("ECLDigitizer: ECLupgrade must be -1, 0 or 1, not " << m_eclUpgradeOverride);
  /* The payloads of the first run are already available here. */
  if (m_eclUpgradeOverride == -1)
    m_eclUpgrade = m_electronicsConfiguration.isValid() && m_electronicsConfiguration->getUpgrade();
  else
    m_eclUpgrade = (m_eclUpgradeOverride == 1);
  if (m_eclUpgrade) {
    m_backend.emplace<UpgradeBackend>();
    /* Pulse shape discrimination not implemented yet for upgraded electronics */
    m_hadronPulseShape = false;
  } else {
    m_backend.emplace<CurrentBackend>();
  }

  if (m_compAlgo == -1) {
    m_compAlgo = m_eclUpgrade ? 1 : 0;
  }

  m_eclDsps.registerInDataStore();
  if (m_storeDspWithExtraMCInfo) m_eclDspsWithExtraMCInfo.registerInDataStore();
  m_eclDigits.registerInDataStore();
  m_eclTrigs.registerInDataStore();

  if (m_hadronPulseShape) {
    B2DEBUG(20,
            "Hadron pulse shapes for ECL simulations are enabled.  Pulse shape simulations use techniques validated with test beam data documented in: S. Longo and J. M. Roney 2018 JINST 13 P03018");
  } else {
    B2DEBUG(20, "Hadron pulse shapes for ECL simulations are disabled.");
  }

  if (m_inter) {
    B2DEBUG(20, "Diode-crossing pulse shapes for ECL simulations are enabled.");
  } else {
    B2DEBUG(20, "Diode-crossing pulse shapes for ECL simulations are disabled.");
  }

  m_eclDiodeHits.registerInDataStore("ECLDiodeHits");

  m_eclDsps.registerRelationTo(m_eclDigits);
  if (m_storeDspWithExtraMCInfo)
    m_eclDspsWithExtraMCInfo.registerRelationTo(m_eclDigits);
  m_eclDigits.registerRelationTo(m_eclHits);
  if (m_waveformMaker)
    m_eclWaveforms.registerInDataStore(m_eclWaveformsName);

  std::visit(
  [&](auto & backend) {
    backend.adc.resize(EclConfiguration::c_nChannels);
  },
  m_backend);

  EclConfiguration::get().setBackground(m_background);
}

void ECLDigitizerModule::beginRun()
{
  const bool eclUpgrade = m_electronicsConfiguration.isValid() && m_electronicsConfiguration->getUpgrade();
  if (m_eclUpgradeOverride == -1 && eclUpgrade != m_eclUpgrade)
    B2FATAL("ECLDigitizer: the ECL electronics (ECLElectronicsConfiguration) changes within the job, this is not supported.");

  const EclConfiguration& ec = EclConfiguration::get();
  double ns_per_tick = ec.c_clock / (ec.c_nFitPointsPerAdcTick * ec.c_nSubTimeCorr *
                                     ec.getRF()) *
                       1e3; // convert to ns

  if (m_loadOnce) {
    readDSPDB();
    m_loadOnce = false;
  }

  Calibration def = {1, 0};
  m_calib.assign(ECLElementNumbers::c_NCrystals, def);

  if (m_crystalElectronics.isValid()) {
    for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++)
      m_calib[i].ascale /= m_crystalElectronics->getCalibVector()[i];
  }
  if (m_crystalEnergy.isValid()) {
    for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++)
      m_calib[i].ascale /= m_crystalEnergy->getCalibVector()[i] * 20000.0;
  }
  if (m_crystalElectronicsTime.isValid()) {
    for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++)
      m_calib[i].tshift += m_crystalElectronicsTime->getCalibVector()[i] * ns_per_tick;
  }
  if (m_crystalTimeOffset.isValid()) {
    for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++)
      m_calib[i].tshift += m_crystalTimeOffset->getCalibVector()[i] * ns_per_tick;
  }
  if (m_crateTimeOffset.isValid()) {
    for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++)
      m_calib[i].tshift += m_crateTimeOffset->getCalibVector()[i] * ns_per_tick;
  }
  if (m_mcTimeOffset.isValid()) {
    for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++)
      m_calib[i].tshift += m_mcTimeOffset->getCalibVector()[i] * ns_per_tick;
  }
  if (m_eclUpgrade) {
    // Set to zero all time calibrations for Upgrade electronics
    for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++)
      m_calib[i].tshift = 0;
  }
  m_waveformThresholds.assign(ECLElementNumbers::c_NCrystals, -1);
  if (m_waveformThresholdOverride < 0) {
    if (m_fpgaWaveform.isValid()) {
      for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++)
        m_waveformThresholds[i] = m_fpgaWaveform->getCalibVector()[i];
    }
  } else {
    //If m_waveformThresholdOverride > 0 override ECL_FPGA_StoreWaveform;
    for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++)
      m_waveformThresholds[i] = m_waveformThresholdOverride * m_calib[i].ascale; // convert GeV to ADC
  }

  if (m_hadronPulseShape)
    callbackHadronSignalShapes();

  // Initialize channel mapper at run start to account for possible
  // changes in ECL mapping between runs.
  if (!m_eclMapper.initFromDB()) {
    B2FATAL("ECLDigitizer: Can't initialize eclChannelMapper!");
  }
}

// interface to C shape fitting function function
void ECLDigitizerModule::shapeFitterWrapper(
  const int j, const int* fitA, const int triggerTime,
  int& amplitude, int& time, int& quality, int& chi2) const
{
  const CrystalLinks& t = m_tbl[j]; //lookup table [0,8735]

  const int* f = nullptr;
  const int* f1 = nullptr;
  const int* fg41 = nullptr;
  const int* fg43 = nullptr;
  const int* fg31 = nullptr;
  const int* fg32 = nullptr;
  const int* fg33 = nullptr;

  std::visit(
  [&](const auto & backend) {
    const auto& r = backend.fitParams[t.ifunc];

    f = &r.f[0][0];
    f1 = &r.f1[0][0];
    fg41 = &r.fg41[0][0];
    fg43 = &r.fg43[0][0];
    fg31 = &r.fg31[0][0];
    fg32 = &r.fg32[0][0];
    fg33 = &r.fg33[0][0];
  },
  m_backend);

  const int nSamples = m_eclUpgrade
                       ? EclConfigurationUp::c_nAdcSamples
                       : EclConfiguration::c_nAdcSamples;

  ECLShapeFit result;

  if (!m_dspDataTest) {
    const short int* id = std::visit(
    [&](const auto & backend) -> const short int* {
      return backend.idn[t.idn].id;
    },
    m_backend);

    int A0  = (int) * (id + 0) - 128;
    int Askip  = (int) * (id + 1) - 128;

    int Ahard  = (int) * (id + 2);
    int k_a = static_cast<int>(*(reinterpret_cast<const unsigned char*>(id) + 26));
    int k_b = static_cast<int>(*(reinterpret_cast<const unsigned char*>(id) + 27));
    int k_c = static_cast<int>(*(reinterpret_cast<const unsigned char*>(id) + 28));
    int k1_chi = static_cast<int>(*(reinterpret_cast<const unsigned char*>(id) + 24));
    int k2_chi = static_cast<int>(*(reinterpret_cast<const unsigned char*>(id) + 25));

    int chi_thres = (int) * (id + 15);

    int trg_time = triggerTime;

    if (m_eclUpgrade) {
      result = lftdaUp_(
                 f, f1, fg41, fg43, fg31, fg32, fg33,
                 const_cast<int*>(fitA),
                 trg_time, A0, Ahard, Askip,
                 k_a, k_b, k_c,
                 k1_chi, k2_chi, chi_thres);
    } else {
      int k_16 =
        static_cast<int>(*(reinterpret_cast<const unsigned char*>(id) + 29));

      result = lftda_(
                 f, f1, fg41, fg43, fg31, fg32, fg33,
                 const_cast<int*>(fitA),
                 trg_time, A0, Ahard, Askip,
                 k_a, k_b, k_c, k_16,
                 k1_chi, k2_chi, chi_thres);
    }
  } else {
    std::vector<int> adc(nSamples);
    for (int i = 0; i < nSamples; i++) adc[i] = fitA[i];
    // NOTE: triggerTime_packed is any even number in set {8*Z, 8*Z + 2, 8*Z + 4}
    //       where Z is an integer number in 0..23 range
    // triggerTime = triggerTime_packed - 2 * (triggerTime_packed / 8);
    // triggerTime = 6*Z + q
    int triggerTime_packed = triggerTime / 6 * 8 + triggerTime % 6;
    result = ECLDspUtilities::shapeFitter(j + 1, adc, triggerTime_packed, true, m_eclUpgrade);
  }

  amplitude = result.amp;
  time = result.time;
  quality = result.quality;
  chi2 = result.chi2;

  //== Set precision of chi^2 to be the same as in the raw data.
  int discarded_bits = 0;
  if ((chi2 & 0x7800000) != 0) {
    chi2 = 0x7800000;
  } else if ((chi2 & 0x0600000) != 0) {
    discarded_bits = 14;
  } else if ((chi2 & 0x0180000) != 0) {
    discarded_bits = 12;
  } else if ((chi2 & 0x0060000) != 0) {
    discarded_bits = 10;
  } else if ((chi2 & 0x0018000) != 0) {
    discarded_bits = 8;
  } else if ((chi2 & 0x0006000) != 0) {
    discarded_bits = 6;
  } else if ((chi2 & 0x0001800) != 0) {
    discarded_bits = 4;
  } else if ((chi2 & 0x0000600) != 0) {
    discarded_bits = 2;
  }
  if (discarded_bits > 0) {
    chi2 >>= discarded_bits;
    chi2 <<= discarded_bits;
  }
}

void ECLDigitizerModule::shapeSignals()
{
  std::visit(
  [&](auto & backend) {
    shapeSignalsImpl(backend);
  },
  m_backend);
}

template <typename BackendType>
void ECLDigitizerModule::shapeSignalsImpl(BackendType& backend)
{
  using Config = typename BackendType::Configuration;

  constexpr bool isUpgrade =
    std::is_same_v<Config, ECL::EclConfigurationUp>;

  const Config& ec = Config::get();

  auto& adc = backend.adc;
  auto& shapeSignalSamples = backend.shapeSignalSamples;

  ECLGeometryPar* eclp = ECLGeometryPar::Instance();

  const double trgtick = ec.c_clock / ec.getRF() / ec.c_nTriggerCounts;   // trigger tick
  const double tscale = 2 * trgtick, toff = ec.c_clock / (2 * ec.getRF());

  // clear the storage for the event
  memset(adc.data(), 0, adc.size()*sizeof(typename BackendType::AdcCounts));

  const double e2GeV = 1 / Unit::GeV; // convert Geant energy units to GeV
  const double t2us = 1 / Unit::us; // convert Geant time units to microseconds

  // emulate response for ECL hits after ADC measurements
  for (const auto& hit : m_eclSimHits) {
    int cellId = hit.getCellId(); // 1 .. 8736
    int j = cellId - 1; // 0 .. 8735
    int id = m_eclMapper.getCrateID(cellId) - 1; // 0 .. 51
    double timeOffset = tscale * m_ttime[id] - toff;
    double hitE       = hit.getEnergyDep() * m_calib[j].ascale * e2GeV;
    double hitTimeAve = (hit.getFlightTime() + m_calib[j].tshift + eclp->time2sensor(j, hit.getPosition())) * t2us;

    adc[j].m_energyConversion = m_calib[j].ascale * e2GeV * 20000;
    adc[j].m_flightTime += hit.getFlightTime() * hit.getEnergyDep(); //true time weighted by energy
    adc[j].m_timeShift += m_calib[j].tshift * hit.getEnergyDep();
    adc[j].m_timeToSensor += eclp->time2sensor(j, hit.getPosition()) * hit.getEnergyDep();
    adc[j].m_totalHadronDep += hit.getHadronEnergyDep(); // true deposited energy hadron component (in GeV)
    adc[j].m_totalDep += hit.getEnergyDep(); //true deposited energy (in GeV)
    if constexpr(!isUpgrade) {
      if (m_hadronPulseShape) {
        double hitHadronE       = hit.getHadronEnergyDep() * m_calib[j].ascale * e2GeV;
        adc[j].addHit(hitE - hitHadronE, hitTimeAve + timeOffset, m_hadronShapeSignalSamples[0]);//Gamma Component
        adc[j].addHit(hitHadronE, hitTimeAve + timeOffset, m_hadronShapeSignalSamples[1]); //Hadron Component
        continue;
      }
    }
    adc[j].addHit(hitE, hitTimeAve + timeOffset, shapeSignalSamples[m_tbl[j].iss]);
  }

  // add only background hits
  for (const auto& hit : m_eclHits) {
    if (hit.getBackgroundTag() == BackgroundMetaData::bg_none) continue;
    int cellId = hit.getCellId(); // 1 .. 8736
    int j = cellId - 1; // 0 .. 8735
    int id = m_eclMapper.getCrateID(cellId) - 1; // 0 .. 51
    double timeOffset = tscale * m_ttime[id] - toff;
    double hitE       = hit.getEnergyDep() * m_calib[j].ascale * e2GeV;
    double hitTimeAve = (hit.getTimeAve() + m_calib[j].tshift) * t2us;
    adc[j].addHit(hitE, hitTimeAve + timeOffset, shapeSignalSamples[m_tbl[j].iss]);
  }

  // internuclear counter effect -- charged particle crosses diode and produces signal
  if (m_inter) {
    // ionisation energy in Si is I = 3.6x10^-6 MeV for electron-hole pair
    // 5000 pairs in the diode per 1 MeV deposited in the crystal attached to the diode
    // conversion factor to get equivalent energy deposition in the crystal to sum up it with deposition in crystal
    const double diodeEdep2crystalEdep = e2GeV * (1 / (5000 * 3.6e-6));
    for (const auto& hit : m_eclDiodeHits) {
      int cellId = hit.getCellId(); // 1 .. 8736
      int j = cellId - 1; // 0 .. 8735
      int id = m_eclMapper.getCrateID(cellId) - 1; // 0 .. 51
      double timeOffset = tscale * m_ttime[id] - toff;
      double hitE       = hit.getEnergyDep() * m_calib[j].ascale * diodeEdep2crystalEdep;
      double hitTimeAve = (hit.getTimeAve() + m_calib[j].tshift) * t2us;

      if constexpr(!isUpgrade) {
        if (m_hadronPulseShape) {
          adc[j].addHit(hitE, hitTimeAve + timeOffset, m_hadronShapeSignalSamples[2]); // diode component
          continue;
        }
      }
      adc[j].addHit(hitE, hitTimeAve + timeOffset, shapeSignalSamples[1]); // shapeSignalSamples[1] is the sampled diode response
    }
  }

  if (m_calibration) {
    // This has been added by Alex Bobrov for calibration
    // of covariance matrix artificially generate 100 MeV in time for each crystal
    double hitE = 0.1, hitTimeAve = 0.0;
    for (int j = 0; j < ec.c_nChannels; j++) {
      int cellId = j + 1; // 1 .. 8736
      int id = m_eclMapper.getCrateID(cellId) - 1; // 0 .. 51
      double timeOffset = tscale * m_ttime[id] - toff;
      adc[j].addHit(hitE, hitTimeAve + timeOffset, shapeSignalSamples[m_tbl[j].iss]);
    }
  }
}

void ECLDigitizerModule::makeElectronicNoiseAndPedestal(int j, int* fitA)
{
  const int nSamples = m_eclUpgrade
                       ? EclConfigurationUp::c_nAdcSamples
                       : EclConfiguration::c_nAdcSamples;
  const double mevToAdc = m_eclUpgrade ? 1.0 : 20.0;
  if (m_noElectronicNoise) {
    for (int i = 0; i < nSamples; ++i) fitA[i] = 3000;
    return;
  }
  std::vector<float> z(nSamples), adcNoise(nSamples); // buffers with electronic noise
  // Noise generation
  for (int i = 0; i < nSamples; ++i) z[i] = gRandom->Gaus(0, 1);
  m_noise[m_tbl[j].inoise].generateCorrelatedNoise(z.data(), adcNoise.data());
  for (int i = 0; i < nSamples; ++i) fitA[i] = mevToAdc * adcNoise[i] + 3000;
}

void ECLDigitizerModule::makeWaveforms()
{
  const int nChannels = EclConfiguration::c_nChannels;
  const int nSamples = m_eclUpgrade
                       ? EclConfigurationUp::c_nAdcSamples
                       : EclConfiguration::c_nAdcSamples;
  BitStream out(nChannels * nSamples * 18 / 32);
  out.putNBits(m_compAlgo & 0xff, 8);
  ECLCompress* comp = selectAlgo(m_compAlgo & 0xff, m_eclUpgrade);
  if (comp == nullptr)
    B2FATAL("Unknown compression algorithm: " << m_compAlgo);

  std::vector<int> fitA(nSamples); // buffer for the waveform fitter
  std::visit(
  [&](auto & backend) {
    auto& adc = backend.adc;
    // loop over entire calorimeter
    for (int j = 0; j < nChannels; j++) {
      auto& a = adc[j];
      makeElectronicNoiseAndPedestal(j, fitA.data());
      for (int  i = 0; i < nSamples; i++) {
        int A = 20000 * a.m_adcCounts[i] + fitA[i];
        fitA[i] = max(0, min(A, (1 << 18) - 1));
      }
      comp->compress(out, fitA.data());
    }
  },
  m_backend);
  out.resize();

  ECLWaveforms* wf = new ECLWaveforms;
  m_eclWaveforms.assign(wf);

  std::swap(out.getStore(), wf->getStore());
  if (comp) delete comp;
}

void ECLDigitizerModule::event()
{
  const int nChannels = EclConfiguration::c_nChannels;
  const int nSamples = m_eclUpgrade
                       ? EclConfigurationUp::c_nAdcSamples
                       : EclConfiguration::c_nAdcSamples;
  const int nTriggerCounts = m_eclUpgrade
                             ? EclConfigurationUp::c_nTriggerCounts
                             : EclConfiguration::c_nTriggerCounts;

  // make relation between cellid and eclhits
  struct HitMapEntry {
    int cell;
    int id;
  };
  vector<HitMapEntry> hitMap;
  for (const auto& hit : m_eclHits) {
    int j = hit.getCellId() - 1; //0~8735
    if (hit.getBackgroundTag() == BackgroundMetaData::bg_none) hitMap.push_back({j, hit.getArrayIndex()});
  }

  bool isBGOverlay = m_eclWaveforms.isValid(), isTrigTime = false;
  BitStream out;
  ECLCompress* comp = nullptr;

  // check background overlay
  if (isBGOverlay) {
    std::swap(out.getStore(), m_eclWaveforms->getStore());
    out.setPos(0);
    unsigned int compAlgo = out.getNBits(8);
    comp = selectAlgo(compAlgo & 0x7f, m_eclUpgrade);
    if (comp == nullptr)
      B2FATAL("Unknown compression algorithm: " << compAlgo);
    isTrigTime = compAlgo >> 7; // crate trigger times are stored and retrieved
    if (isTrigTime) {
      for (int i = 0; i < ECL::ECL_CRATES; i++) {
        unsigned char t = out.getNBits(7); // [0..72)
        m_ttime[i] = t;
      }
    }
  }

  if (!m_trigTime || !isTrigTime) { // reproduce previous logic -- one trigger time for all crates
    int deltaT = gRandom->Uniform(0, double(nTriggerCounts) * 0.5); // trigger decision can come in any time [0..n_trg/2)
    for (int id = 0; id < ECL::ECL_CRATES; id++) m_ttime[id] = deltaT;
  }

  int triggerTag0 = m_eventMetaData->getEvent();
  for (int id = 0; id < ECL::ECL_CRATES; id++) {
    auto eclTrig = m_eclTrigs.appendNew();
    int triggerPhase0 = 2 * (m_ttime[id] + m_ttime[id] / 3);
    eclTrig->setTrigId(id);
    eclTrig->setTimeTrig(triggerPhase0);
    eclTrig->setTrigTag(triggerTag0);
  }

  shapeSignals();

  // We want to produce background waveforms in simulation first than
  // dump to a disk, read from the disk to test before real data
  if (m_waveformMaker) { makeWaveforms(); return; }

  std::vector<int> fitA(nSamples); // buffer for the waveform fitter

  std::visit(
  [&](auto & backend) {
    auto& adc = backend.adc;
    // loop over entire calorimeter
    for (int j = 0; j < nChannels; j++) {
      auto& a = adc[j];

      // Normalize MC true arrival times.
      if (a.m_totalDep > 0) {
        a.m_flightTime /= a.m_totalDep;
        a.m_timeShift /= a.m_totalDep;
        a.m_timeToSensor /= a.m_totalDep;
      }

      // If a background waveform is present, electronic noise
      // is already included in it.
      if (isBGOverlay) {
        comp->uncompress(out, fitA.data());
      } else {
        // Signal amplitude should be above 100 keV.
        if (a.m_total < 0.0001) continue;
        makeElectronicNoiseAndPedestal(j, fitA.data());
      }

      for (int i = 0; i < nSamples; i++) {
        int A = 20000 * a.m_adcCounts[i] + fitA[i];
        fitA[i] = max(0, min(A, (1 << 18) - 1));
      }

      int  energyFit = 0; // fit output : fitted amplitude 18 bits
      int       tFit = 0; // fit output : fitted time      12 bits
      int qualityFit = 0; // fit output : fit quality      2 bits
      int        chi = 0; // fit output : chi square

      int id = m_eclMapper.getCrateID(j + 1) - 1; // 0 .. 51
      int triggerTime = 2 * m_ttime[id];

      shapeFitterWrapper(j, fitA.data(), triggerTime, energyFit, tFit, qualityFit, chi);

      if (energyFit > m_adcThreshold) {
        //store each crystal hit
        const int cellId = j + 1;

        //note energyFit and m_waveformThresholds is in ADC units
        if (energyFit > m_waveformThresholds[cellId - 1]) {
          //only save waveforms above ADC threshold
          const auto eclDsp = m_eclDsps.appendNew();
          eclDsp->setCellId(cellId);
          eclDsp->setDspA(fitA);
        }

        // only store extra MC info if requested and above threshold
        if (m_storeDspWithExtraMCInfo && a.m_totalDep >= m_dspWithExtraMCInfoThreshold) {
          const auto eclDspWithExtraMCInfo = m_eclDspsWithExtraMCInfo.appendNew();
          eclDspWithExtraMCInfo->setCellId(cellId);
          eclDspWithExtraMCInfo->setDspA(fitA);
          eclDspWithExtraMCInfo->setEnergyDep(a.m_totalDep);
          eclDspWithExtraMCInfo->setHadronEnergyDep(a.m_totalHadronDep);
          eclDspWithExtraMCInfo->setFlightTime(a.m_flightTime);
          eclDspWithExtraMCInfo->setTimeShift(a.m_timeShift);
          eclDspWithExtraMCInfo->setTimeToSensor(a.m_timeToSensor);
          eclDspWithExtraMCInfo->setEnergyConversion(a.m_energyConversion * 20000);
        }

        const auto eclDigit = m_eclDigits.appendNew();
        eclDigit->setCellId(cellId); // cellId in range from 1 to 8736
        eclDigit->setAmp(energyFit); // E (GeV) = energyFit/20000;
        eclDigit->setTimeFit(tFit);  // in TDC units;
        eclDigit->setQuality(qualityFit);
        if (qualityFit == 2)
          eclDigit->setChi(chi);
        else eclDigit->setChi(0);
        for (const auto& hit : hitMap)
          if (hit.cell == j) eclDigit->addRelationTo(m_eclHits[hit.id]);

        // set relation to DspWithExtraInfo
        for (auto& dspWithExtraMCInfo : m_eclDspsWithExtraMCInfo) {
          if (eclDigit->getCellId() == dspWithExtraMCInfo.getCellId()) dspWithExtraMCInfo.addRelationTo(eclDigit);
        }
      }
    }
  },
  m_backend);

  if (comp) delete comp;
}

void ECLDigitizerModule::endRun()
{
}

void ECLDigitizerModule::terminate()
{
}

void ECLDigitizerModule::callbackHadronSignalShapes()
{
  const EclConfiguration& ec = EclConfiguration::get();
  const int nWaveformParameters = ec.c_nWaveformParameters;

  if (m_waveformParametersMC.hasChanged()) {

    m_hadronShapeSignalSamples.resize(3);

    //read MC templates from database
    float photonParsPSD[nWaveformParameters];
    float hadronParsPSD[nWaveformParameters];
    float diodeParsPSD[nWaveformParameters];
    for (int i = 0; i < nWaveformParameters; i++) {
      photonParsPSD[i] = m_waveformParametersMC->getPhotonParameters()[i + 1];
      hadronParsPSD[i] = m_waveformParametersMC->getHadronParameters()[i + 1];
      diodeParsPSD[i] = m_waveformParametersMC->getDiodeParameters()[i + 1];
    }

    //Initialize templates for hadron shape simulations
    m_hadronShapeSignalSamples[0].initSample(photonParsPSD, m_waveformParametersMC->getPhotonParameters()[0]);
    m_hadronShapeSignalSamples[1].initSample(hadronParsPSD, m_waveformParametersMC->getHadronParameters()[0]);
    m_hadronShapeSignalSamples[2].initSample(diodeParsPSD, m_waveformParametersMC->getDiodeParameters()[0]);

  }

}

void ECLDigitizerModule::readDSPDB()
{
  const int nChannels = EclConfiguration::c_nChannels;

  TTree* tree  = nullptr;
  TTree* tree2 = nullptr;
  TTree* tree3 = nullptr;

  if (m_eclUpgrade) {
    if (!m_waveformParametersUp.isValid())
      B2FATAL("Missing required payload: ECLWFParametersUp");
    if (!m_algoParametersUp.isValid())
      B2FATAL("Missing required payload: ECLWFAlgoParamsUp");
    if (!m_noiseParametersUp.isValid())
      B2FATAL("Missing required payload: ECLWFNoiseParamsUp");
  }

  auto& waveformParametersPayLoad =
    m_eclUpgrade ? m_waveformParametersUp : m_waveformParameters;

  auto& algoParametersPayLoad =
    m_eclUpgrade ? m_algoParametersUp : m_algoParameters;

  auto& noiseParametersPayLoad =
    m_eclUpgrade ? m_noiseParametersUp : m_noiseParameters;

  bool hasChanged = false;
  hasChanged |= waveformParametersPayLoad.hasChanged();
  hasChanged |= algoParametersPayLoad.hasChanged();
  hasChanged |= noiseParametersPayLoad.hasChanged();

  if (!hasChanged) return;

  tree  = const_cast<TTree*>(&*waveformParametersPayLoad);
  tree2 = const_cast<TTree*>(&*algoParametersPayLoad);
  tree3 = const_cast<TTree*>(&*noiseParametersPayLoad);

  if (tree == 0 || tree2 == 0 || tree3 == 0) B2FATAL("Data not found");

  m_tbl.resize(nChannels);

  const int maxncellid = 512;
  int ncellId;
  vector<int> cellId(maxncellid);//[ncellId] buffer for crystal identification number

  tree->SetBranchAddress("ncellId", &ncellId);
  tree->SetBranchAddress("cellId", cellId.data());

  vector<int> eclWaveformDataTable(nChannels);
  for (Long64_t j = 0, jmax = tree->GetEntries(); j < jmax; j++) {
    tree->GetEntry(j);
    assert(ncellId <= maxncellid);
    for (int i = 0; i < ncellId; ++i)
      eclWaveformDataTable[cellId[i] - 1] = j;
  }
  B2DEBUG(150, "ECLDigitizer: " << tree->GetEntries() << " sets of wave form covariance matrices will be used.");

  ECLWFAlgoParams* algo = new ECLWFAlgoParams;
  tree2->SetBranchAddress("Algopars", &algo);
  tree2->SetBranchAddress("ncellId", &ncellId);
  tree2->SetBranchAddress("cellId", cellId.data());
  Long64_t jmax2 = tree2->GetEntries();
  vector<ECLWFAlgoParams> eclWFAlgoParams;
  eclWFAlgoParams.reserve(jmax2);
  for (Long64_t j = 0; j < jmax2; j++) {
    tree2->GetEntry(j);
    assert(ncellId <= maxncellid);
    eclWFAlgoParams.push_back(*algo);
    for (int i = 0; i < ncellId; ++i)
      m_tbl[cellId[i] - 1].idn = j;
  }
  if (algo) delete algo;
  B2DEBUG(150, "ECLDigitizer: " << eclWFAlgoParams.size() << " parameter sets of fitting algorithm were read.");

  ECLNoiseData* noise = new ECLNoiseData;
  tree3->SetBranchAddress("NoiseM", &noise);
  tree3->SetBranchAddress("ncellId", &ncellId);
  tree3->SetBranchAddress("cellId", cellId.data());

  Long64_t jmax3 = tree3->GetEntries();
  m_noise.reserve(jmax3);
  for (Long64_t j = 0; j < jmax3; j++) {
    tree3->GetEntry(j);
    assert(ncellId <= maxncellid);
    m_noise.push_back(*noise);
    if (ncellId == 0) {
      for (int i = 0; i < nChannels; i++)
        m_tbl[i].inoise = 0;
      break;
    } else {
      for (int i = 0; i < ncellId; ++i)
        m_tbl[cellId[i] - 1].inoise = j;
    }
  }
  if (noise) delete noise;
  B2DEBUG(150, "ECLDigitizer: " << eclWFAlgoParams.size() << " noise matrices were loaded.");

  // repack fitting algorithm parameters
  std::visit(
  [&](auto & backend) {
    backend.idn.resize(eclWFAlgoParams.size());
    for (int i = 0, imax = eclWFAlgoParams.size(); i < imax; ++i) {
      repack(eclWFAlgoParams[i], backend.idn[i]);
    }
  },
  m_backend);

  vector<UIntPair> pairIdx;
  for (int i = 0; i < nChannels; i++) {
    unsigned int   wfIdx = eclWaveformDataTable[i];
    unsigned int algoIdx = m_tbl[i].idn;
    UIntPair p(wfIdx, algoIdx);
    vector<UIntPair>::iterator ip = find(pairIdx.begin(), pairIdx.end(), p);
    if (ip != pairIdx.end()) { // crystal i already have the same parameters as before
      m_tbl[i].ifunc = ip - pairIdx.begin();
    } else {                  // new combination of parameters
      m_tbl[i].ifunc = pairIdx.size();
      pairIdx.push_back(p);
    }
  }


  ECLWaveformData* eclWFData = new ECLWaveformData;
  tree->SetBranchAddress("CovarianceM", &eclWFData);
  tree->SetBranchStatus("ncellId", 0); // do not read it at the moment
  tree->SetBranchStatus("cellId", 0); // do not read it at the moment

  std::visit(
  [&](auto & backend) {
    using BackendType = std::decay_t<decltype(backend)>;
    using Config = typename BackendType::Configuration;
    // now we know how many distinct elements of the (Waveform x AlgoParams) matrix should be
    backend.fitParams.resize(pairIdx.size());
    for (unsigned int ip = 0; ip < pairIdx.size(); ++ip) {
      const UIntPair& p = pairIdx[ip];
      tree->GetEntry(p.first);
      getFitParams<Config>(
        *eclWFData,
        eclWFAlgoParams[p.second],
        backend.fitParams[ip]);
    }
  },
  m_backend);

  std::visit(
  [&](const auto & backend) {
    B2DEBUG(150, "ECLDigitizer: " << backend.fitParams.size() << " fitting crystals groups were created.");
  },
  m_backend);

  std::visit(
  [&](auto & backend) {
    using BackendType = std::decay_t<decltype(backend)>;
    using Config = typename BackendType::Configuration;

    // at the moment there is only one sampled signal shape in the pool
    // since all shaper parameters are the same for all crystals
    backend.shapeSignalSamples.resize(2);

    std::array<float, Config::c_nWaveformParameters> waveformParameters{};
    eclWFData->getWaveformParArray(waveformParameters.data());

    const auto diodeParameters = Config::getDefaultDiodeParameters();

    backend.shapeSignalSamples[0].initSample(
      waveformParameters.data(), Config::c_signalUnitScale);
    backend.shapeSignalSamples[1].initSample(
      diodeParameters.data(), Config::c_diodeUnitScale);

    B2DEBUG(150, "ECLDigitizer: "
            << backend.shapeSignalSamples.size()
            << " backend sampled signal templates were created.");
  },
  m_backend);

  for (int i = 0; i < nChannels; i++) m_tbl[i].iss = 0;

  if (eclWFData) delete eclWFData;
}

template <typename AlgoParamsType>
void ECLDigitizerModule::repack(const ECLWFAlgoParams& eclWFAlgo, AlgoParamsType& t)
{
  // filling short int array
  t.id[ 0] = eclWFAlgo.getlAT() + 128;
  t.id[ 1] = eclWFAlgo.getsT()  + 128;
  t.id[ 2] = eclWFAlgo.gethT();
  t.id[ 3] = 17952;
  t.id[ 4] = 19529;
  t.id[ 5] = 69;
  t.id[ 6] = 0;
  t.id[ 7] = 0;
  t.id[ 8] = 257;
  t.id[ 9] = -1;
  t.id[10] = 0;
  t.id[11] = 0;

  // filling unsigned char array
  t.ic[12 * 2 + 0] = eclWFAlgo.getk1();
  t.ic[12 * 2 + 1] = eclWFAlgo.getk2();
  t.ic[13 * 2 + 0] = eclWFAlgo.getka();
  t.ic[13 * 2 + 1] = eclWFAlgo.getkb();
  t.ic[14 * 2 + 0] = eclWFAlgo.getkc();
  t.ic[14 * 2 + 1] = eclWFAlgo.gety0s() - 16;

  // again, filling short int array
  t.id[15] = eclWFAlgo.getcT();
}

template <typename Config>
void ECLDigitizerModule::getFitParams(const ECLWaveformData& eclWFData,
                                      const ECLWFAlgoParams& eclWFAlgo,
                                      typename Config::FitParams& p)
{
  const Config& ec = Config::get();

  constexpr int nFitSamples = ec.c_nFitSamples;
  constexpr int nFitPointsPerAdcTick = ec.c_nFitPointsPerAdcTick;
  constexpr int nFitTimeShifts = ec.c_nFitTimeShifts;
  constexpr int nLowAmpFitPoints = ec.c_nLowAmpFitPoints;
  constexpr int nFitGridPoints = nFitTimeShifts * nFitPointsPerAdcTick;
  constexpr bool isUpgrade = std::is_same_v<Config, ECL::EclConfigurationUp>;

  using FitParamsType = typename Config::FitParams;
  using IntArrayGridXAdcType = typename FitParamsType::IntArrayGridXAdc;
  using IntArraySubXAdcType = typename FitParamsType::IntArraySubXAdc;
  using ShaperType = std::conditional_t<isUpgrade, ShaperDSPUp_t, ShaperDSP_t>;

  using FitRow = std::array<double, nFitSamples>;

  double ssd[nFitSamples][nFitSamples];
  eclWFData.getMatrix(ssd);
  vector<double> MP(ec.c_nWaveformParameters);
  eclWFData.getWaveformParArray(MP.data());

  // shape function parameters
  int iff = 1 << 14;   //Alex Bobrov for correct chi square

  int ia = 1 << eclWFAlgo.getka();
  int ib = 1 << eclWFAlgo.getkb();
  int ic = 1 << eclWFAlgo.getkc();

  std::vector<FitRow> dblF(nFitGridPoints);
  std::vector<FitRow> dblF1(nFitGridPoints);
  std::vector<FitRow> dblFg31(nFitGridPoints);
  std::vector<FitRow> dblFg32(nFitGridPoints);
  std::vector<FitRow> dblFg33(nFitGridPoints);
  std::vector<FitRow> dblFg41(nLowAmpFitPoints);
  std::vector<FitRow> dblFg43(nLowAmpFitPoints);

  IntArrayGridXAdcType& ref_f    = p.f;
  IntArrayGridXAdcType& ref_f1   = p.f1;
  IntArrayGridXAdcType& ref_fg31 = p.fg31;
  IntArrayGridXAdcType& ref_fg32 = p.fg32;
  IntArrayGridXAdcType& ref_fg33 = p.fg33;
  IntArraySubXAdcType&  ref_fg41 = p.fg41;
  IntArraySubXAdcType&  ref_fg43 = p.fg43;

  // Dynamic normalization coefficient for shape function
  double fm = 0;

  double unitscale = 1.0;
  if (m_unitscale > 0) unitscale = m_unitscale;

  const double isd = isUpgrade ? 1.0 : 3. / 4.;
  const double sd = 1. / isd;

  /* Parameters used for low amplitude approximations */
  int center = nFitPointsPerAdcTick * (nFitTimeShifts - 1) / 2;
  int step   = nFitPointsPerAdcTick / nLowAmpFitPoints;

  ShaperType dsp(MP, unitscale);
  dsp.settimestride(ec.c_step);
  dsp.setseedoffset(ec.c_step / nFitPointsPerAdcTick);
  if constexpr(isUpgrade) {
    dsp.settimeseed(ec.c_step * (ec.c_fitTimeMin - (nFitTimeShifts / 2)));
  } else {
    dsp.settimeseed(-(ec.c_step - (ec.c_step / nFitPointsPerAdcTick)));
  }
  vector<dd_t> f(nFitSamples);
  for (int k = 0; k < nFitTimeShifts * nFitPointsPerAdcTick;
       k++, dsp.nextseed()) { // calculate fit parameters around 0 +- nFitTimeShifts/2 ADC tick
    dsp.fillvector(f);

    double g0g0 = 0, g0g1 = 0, g1g1 = 0, g0g2 = 0, g1g2 = 0, g2g2 = 0;
    double sg0[nFitSamples], sg1[nFitSamples], sg2[nFitSamples];
    for (int j = 0; j < nFitSamples; j++) {
      double g0 = 0, g1 = 0, g2 = 0;
      for (int i = 0; i < nFitSamples; i++) {
        if (fm < f[i].first) fm = f[i].first;
        g0 += ssd[j][i] * f[i].first;
        g1 += ssd[j][i] * f[i].second;
        g2 += ssd[j][i];
      }
      g0g0 += g0 * f[j].first;
      g0g1 += g1 * f[j].first;
      g1g1 += g1 * f[j].second;
      g0g2 += g0;
      g1g2 += g1;
      g2g2 += g2;
      sg0[j] = g0;
      sg1[j] = g1;
      sg2[j] = g2;
    }

    double a00 = g1g1 * g2g2 - g1g2 * g1g2;
    double a11 = g0g0 * g2g2 - g0g2 * g0g2;
    double a22 = g0g0 * g1g1 - g0g1 * g0g1;
    double a01 = g1g2 * g0g2 - g0g1 * g2g2;
    double a02 = g0g1 * g1g2 - g1g1 * g0g2;
    double a12 = g0g1 * g0g2 - g0g0 * g1g2;

    double igg2 = 1 / (a11 * g1g1 + g0g1 * a01 + g1g2 * a12);

    // to fixed point representation
    for (int i = 0; i < nFitSamples; i++) {
      const double w = isUpgrade ? 1.0 : (i ? 1.0 : 1. / 16.);

      dblF   [k][i] = f[i].first  * iff * w;
      dblF1  [k][i] = f[i].second * iff * w * sd;

      double fg31 = (a00 * sg0[i] + a01 * sg1[i] + a02 * sg2[i]) * igg2;
      double fg32 = (a01 * sg0[i] + a11 * sg1[i] + a12 * sg2[i]) * igg2;
      double fg33 = (a02 * sg0[i] + a12 * sg1[i] + a22 * sg2[i]) * igg2;

      dblFg31[k][i] = fg31 * ia * w;
      dblFg32[k][i] = fg32 * ib * w * isd;
      dblFg33[k][i] = fg33 * ic * w;
    }

    //first approximation without time correction
    int rel = center - k;
    if (rel % step == 0) {
      int jk = (nLowAmpFitPoints - 1) + rel / step;
      if (jk >= 0 && jk < nLowAmpFitPoints) {

        double igg1 = 1 / a11;
        // to fixed point
        for (int i = 0; i < nFitSamples; i++) {
          const double w = isUpgrade ? 1.0 : (i ? 1.0 : 1. / 16.);

          double fg41 = (g2g2 * sg0[i] - g0g2 * sg2[i]) * igg1;
          double fg43 = (g0g0 * sg2[i] - g0g2 * sg0[i]) * igg1;
          dblFg41[jk][i] = fg41 * ia * w;
          dblFg43[jk][i] = fg43 * ic * w;
        }
      }
    }
  }
  // Ignore dynamically calculated normalization coefficient if
  // unitscale is set to a static positive value.
  if (m_unitscale > 0 || isUpgrade) fm = 1.0;
  for (int k = 0; k < nFitTimeShifts * nFitPointsPerAdcTick; k++) {
    for (int j = 0; j < nFitSamples; j++) {
      ref_f   [k][j] = lrint(dblF   [k][j] / fm);
      ref_f1  [k][j] = lrint(dblF1  [k][j] / fm);
      ref_fg31[k][j] = lrint(dblFg31[k][j] * fm);
      ref_fg32[k][j] = lrint(dblFg32[k][j] * fm);
      ref_fg33[k][j] = lrint(dblFg33[k][j]);
      if (k >= nLowAmpFitPoints) continue;
      ref_fg41[k][j] = lrint(dblFg41[k][j] * fm);
      ref_fg43[k][j] = lrint(dblFg43[k][j]);
    }
  }
}
