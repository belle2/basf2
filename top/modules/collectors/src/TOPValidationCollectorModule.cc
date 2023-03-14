/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/modules/collectors/TOPValidationCollectorModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/reconstruction_cpp/PDFConstructor.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>

// framework aux
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventMetaData.h>

// root
#include <TRandom.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>

#include <limits>

using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  ///                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPValidationCollector);

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPValidationCollectorModule::TOPValidationCollectorModule()
  {
    // set module description and processing properties
    setDescription("A collector for automatic validation of TOP calibration");
    setPropertyFlags(c_ParallelProcessingCertified);

    // module parameters
    addParam("numBins", m_numBins, "number of bins of the search region", 100);
    addParam("timeRange", m_timeRange,
             "time range in which to search for the minimum [ns]", 10.0);
    addParam("sigmaSmear", m_sigmaSmear,
             "sigma in [ns] for additional smearing of PDF", 0.0);
    addParam("sample", m_sample,
             "sample type: one of dimuon or bhabha", std::string("dimuon"));
    addParam("deltaEcms", m_deltaEcms,
             "c.m.s energy window (half size) if sample is dimuon or bhabha", 0.1);
    addParam("dr", m_dr, "cut on POCA in r", 2.0);
    addParam("dz", m_dz, "cut on POCA in abs(z)", 4.0);
    addParam("minZ", m_minZ,
             "minimal local z of extrapolated hit", -130.0);
    addParam("maxZ", m_maxZ,
             "maximal local z of extrapolated hit", 130.0);
    addParam("pdfOption", m_pdfOption,
             "PDF option, one of 'rough', 'fine', 'optimal'", std::string("rough"));

  }


  void TOPValidationCollectorModule::prepare()
  {
    // input collections

    m_digits.isRequired();
    m_tracks.isRequired();
    m_extHits.isRequired();
    m_recBunch.isRequired();

    // Parse PDF option

    if (m_pdfOption == "rough") {
      m_PDFOption = PDFConstructor::c_Rough;
    } else if (m_pdfOption == "fine") {
      m_PDFOption = PDFConstructor::c_Fine;
    } else if (m_pdfOption == "optimal") {
      m_PDFOption = PDFConstructor::c_Optimal;
    } else {
      B2ERROR("Unknown PDF option '" << m_pdfOption << "'");
    }

    // set track selector

    if (m_sample == "dimuon" or m_sample == "bhabha") {
      m_selector = TrackSelector(m_sample);
      m_selector.setDeltaEcms(m_deltaEcms);
      m_selector.setCutOnPOCA(m_dr, m_dz);
      m_selector.setCutOnLocalZ(m_minZ, m_maxZ);
    } else {
      B2ERROR("Invalid sample type '" << m_sample << "'");
    }

    // create chi2 minimum finders

    double tmin = -m_timeRange / 2;
    double tmax =  m_timeRange / 2;
    for (int set = 0; set < c_numSets; set++) {
      for (int slot = 1; slot <= c_numModules; slot++) {
        m_finders[set].push_back(Chi2MinimumFinder1D(m_numBins, tmin, tmax));
      }
    }

    // create and register histograms and tree

    for (int slot = 1; slot <= c_numModules; slot++) {
      string slotName = to_string(slot);
      if (slot < 10) slotName.insert(0, "0");
      string name = "chi2_slot" + slotName;
      string title = "Chi2 scan, slot" + slotName + "; channel; t0 [ns]";
      auto h = new TH2F(name.c_str(), title.c_str(),  c_numChannels, 0, c_numChannels, m_numBins, tmin, tmax);
      registerObject<TH2F>(name, h);
      m_namesChi.push_back(name);
    }

    for (int slot = 1; slot <= c_numModules; slot++) {
      string slotName = to_string(slot);
      if (slot < 10) slotName.insert(0, "0");
      string name = "hits_slot" + slotName;
      string title = "Photon hits, slot" + slotName + "; channel; time [ns]";
      auto h = new TH2F(name.c_str(), title.c_str(),  c_numChannels, 0, c_numChannels, 100, 0., 20.);
      registerObject<TH2F>(name, h);
      m_namesHit.push_back(name);
    }

    auto h = new TH1F("moduleT0_pulls", "Module T0 pulls; pulls", 200, -15.0, 15.0);
    registerObject<TH1F>("moduleT0_pulls", h);

    auto tree = new TTree("tree", "TOP calibration validation tree");
    tree->Branch("expNo", &m_treeEntry.expNo, "expNo/I");
    tree->Branch("runNo", &m_treeEntry.runNo, "runNo/I");
    tree->Branch("numTracks", &m_treeEntry.numTracks, "numTracks/I");
    tree->Branch("commonT0", &m_treeEntry.commonT0, "commonT0/F");
    tree->Branch("commonT0Err", &m_treeEntry.commonT0Err, "commonT0Err/F");
    tree->Branch("moduleT0", &m_treeEntry.moduleT0, "moduleT0[16]/F");
    tree->Branch("moduleT0Err", &m_treeEntry.moduleT0Err, "moduleT0Err[16]/F");
    tree->Branch("numTBCalibrated", &m_treeEntry.numTBCalibrated, "numTBCalibrated[16]/I");
    tree->Branch("numT0Calibrated", &m_treeEntry.numT0Calibrated, "numT0Calibrated[16]/I");
    tree->Branch("numActive", &m_treeEntry.numActive, "numActive[16]/I");
    tree->Branch("numActiveCalibrated", &m_treeEntry.numActiveCalibrated, "numActiveCalibrated[16]/I");
    tree->Branch("thrEffi", &m_treeEntry.thrEffi, "thrEffi[16]/F");
    tree->Branch("asicShifts", &m_treeEntry.asicShifts, "asicShifts[4]/F");
    tree->Branch("svdOffset", &m_treeEntry.svdOffset, "svdOffset/F");
    tree->Branch("svdSigma", &m_treeEntry.svdSigma, "svdSigma/F");
    tree->Branch("cdcOffset", &m_treeEntry.cdcOffset, "cdcOffset/F");
    tree->Branch("cdcSigma", &m_treeEntry.cdcSigma, "cdcSigma/F");
    tree->Branch("fillPatternOffset", &m_treeEntry.fillPatternOffset, "fillPatternOffset/F");
    tree->Branch("fillPatternFraction", &m_treeEntry.fillPatternFraction, "fillPatternFraction/F");
    registerObject<TTree>("tree", tree);
  }


  void TOPValidationCollectorModule::startRun()
  {
    // initialize tree variables

    m_treeEntry.clear();
    StoreObjPtr<EventMetaData> evtMetaData;
    m_treeEntry.expNo = evtMetaData->getExperiment();
    m_treeEntry.runNo = evtMetaData->getRun();

    // clear minimum finders

    for (auto& finders : m_finders) {
      for (auto& finder : finders) finder.clear();
    }

    // pass payload summaries to tree

    const auto& fe_mapper = TOPGeometryPar::Instance()->getFrontEndMapper();
    for (unsigned module = 0; module < c_numModules; module++) {
      auto& numTBCalibrated = m_treeEntry.numTBCalibrated[module];
      auto& numT0Calibrated = m_treeEntry.numT0Calibrated[module];
      auto& numActive = m_treeEntry.numActive[module];
      auto& numActiveCalibrated = m_treeEntry.numActiveCalibrated[module];
      auto& thrEffi = m_treeEntry.thrEffi[module];
      int slot = module + 1;
      for (unsigned channel = 0; channel < c_numChannels; channel++) {
        bool tbCalibrated = false;
        const auto* fe = fe_mapper.getMap(slot, channel / 128);
        if (fe) {
          tbCalibrated = m_timebase->isAvailable(fe->getScrodID(), channel);
        } else {
          B2ERROR("No front-end map found");
        }
        bool t0Calibrated = m_channelT0->isCalibrated(slot, channel);
        bool active = m_channelMask->isActive(slot, channel);
        if (tbCalibrated) numTBCalibrated++;
        if (t0Calibrated) numT0Calibrated++;
        if (active) numActive++;
        if (tbCalibrated and t0Calibrated and active) {
          numActiveCalibrated++;
          thrEffi += m_thresholdEff->getThrEff(slot, channel);
        }
      }
      if (numActiveCalibrated > 0) thrEffi /= numActiveCalibrated;
    }

    for (unsigned carrier = 0; carrier < 4; carrier++) {
      unsigned asic = (3 * 4 + carrier) * 4;
      m_treeEntry.asicShifts[carrier] = m_asicShift->isCalibrated(13, asic) ? m_asicShift->getT0(13, asic) :
                                        std::numeric_limits<float>::quiet_NaN();
    }

    const auto& svd = m_eventT0Offset->get(Const::SVD);
    m_treeEntry.svdOffset = svd.offset;
    m_treeEntry.svdSigma = svd.sigma;

    const auto& cdc = m_eventT0Offset->get(Const::CDC);
    m_treeEntry.cdcOffset = cdc.offset;
    m_treeEntry.cdcSigma = cdc.sigma;

    m_treeEntry.fillPatternOffset = m_fillPatternOffset->isCalibrated() ? m_fillPatternOffset->get() :
                                    std::numeric_limits<float>::quiet_NaN();
    m_treeEntry.fillPatternFraction = m_fillPatternOffset->getFraction();
  }


  void TOPValidationCollectorModule::collect()
  {
    // bunch must be reconstructed

    if (not m_recBunch.isValid()) return;
    if (not m_recBunch->isReconstructed()) return;

    TOPRecoManager::setDefaultTimeWindow();
    const auto& chMapper = TOPGeometryPar::Instance()->getChannelMapper();

    // loop over reconstructed tracks, make a selection and accumulate log likelihoods

    for (const auto& track : m_tracks) {

      // track selection
      TOPTrack trk(track);
      if (not trk.isValid()) continue;

      if (not m_selector.isSelected(trk)) continue;

      // construct PDF
      PDFConstructor pdfConstructor(trk, m_selector.getChargedStable(), m_PDFOption);
      if (not pdfConstructor.isValid()) continue;

      // minimization procedure: accumulate
      unsigned module = trk.getModuleID() - 1;
      if (module >= m_namesChi.size()) continue;
      auto h = getObjectPtr<TH2F>(m_namesChi[module]);
      int set = gRandom->Integer(c_numSets); // generate sub-sample number
      auto& finder = m_finders[set][module];
      for (int ibin = 0; ibin < h->GetNbinsY(); ibin++) {
        double t0 = h->GetYaxis()->GetBinCenter(ibin + 1);
        const auto& pixelLogLs = pdfConstructor.getPixelLogLs(t0, m_sigmaSmear);
        for (unsigned channel = 0; channel < c_numChannels; channel++) {
          int pix = chMapper.getPixelID(channel) - 1;
          double chi = h->GetBinContent(channel + 1, ibin + 1);
          chi += -2 * pixelLogLs[pix].logL;
          h->SetBinContent(channel + 1, ibin + 1, chi);
        }
        double logL = 0;
        for (auto& LL : pixelLogLs) logL += LL.logL;
        finder.add(ibin, -2 * logL);
      }
      m_treeEntry.numTracks++;

      auto h1 = getObjectPtr<TH2F>(m_namesHit[module]);
      for (const auto& digit : m_digits) {
        if (digit.getHitQuality() != TOPDigit::c_Good) continue;
        if (digit.getModuleID() != trk.getModuleID()) continue;
        h1->Fill(digit.getChannel(), digit.getTime());
      }

    }
  }

  void TOPValidationCollectorModule::closeRun()
  {

    // module T0 pulls

    for (int module = 0; module < c_numModules; module++) {
      std::vector<double> pos, err;
      for (int set = 0; set < c_numSets; set++) {
        const auto& minimum = m_finders[set][module].getMinimum();
        if (minimum.valid) {
          pos.push_back(minimum.position);
          err.push_back(minimum.error);
        }
      }
      auto h_pulls = getObjectPtr<TH1F>("moduleT0_pulls");
      for (unsigned i = 0; i < pos.size(); i++) {
        for (unsigned j = i + 1; j < pos.size(); j++) {
          double pull = (pos[i] - pos[j]) / sqrt(err[i] * err[i] + err[j] * err[j]);
          h_pulls->Fill(pull);
        }
      }
    }

    // module T0 residuals

    for (int module = 0; module < c_numModules; module++) {
      auto& finder = m_finders[0][module];
      for (int set = 1; set < c_numSets; set++) {
        finder.add(m_finders[set][module]);
      }
      const auto& minimum = finder.getMinimum();
      if (minimum.valid) {
        m_treeEntry.moduleT0[module] = minimum.position;
        m_treeEntry.moduleT0Err[module] = minimum.error;
      }
    }

    // common T0 residual

    auto& finder = m_finders[0][0];
    for (int module = 1; module < c_numModules; module++) {
      finder.add(m_finders[0][module]);
    }
    const auto& minimum = finder.getMinimum();
    if (minimum.valid) {
      m_treeEntry.commonT0 = minimum.position;
      m_treeEntry.commonT0Err = minimum.error;
    }

    // fill the tree

    auto tree = getObjectPtr<TTree>("tree");
    tree->Fill();
  }

}
