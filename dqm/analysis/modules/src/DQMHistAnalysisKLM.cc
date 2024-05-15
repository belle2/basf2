/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <dqm/analysis/modules/DQMHistAnalysisKLM.h>

/* Basf2 headers. */
#include <klm/dataobjects/KLMChannelIndex.h>

/* ROOT headers. */
#include <TClass.h>
#include <TROOT.h>
#include <TStyle.h>

/* C++ headers. */
#include <algorithm>

using namespace Belle2;

REG_MODULE(DQMHistAnalysisKLM);

DQMHistAnalysisKLMModule::DQMHistAnalysisKLMModule()
  : DQMHistAnalysisModule(),
    m_ProcessedEvents{0},
    m_IsNullRun{false},
    m_ChannelArrayIndex{&(KLMChannelArrayIndex::Instance())},
    m_SectorArrayIndex{&(KLMSectorArrayIndex::Instance())},
    m_ElementNumbers{&(KLMElementNumbers::Instance())},
    m_EklmElementNumbers{&(EKLMElementNumbers::Instance())}
{
  setDescription("Module used to analyze KLM DQM histograms.");
  addParam("ThresholdForMasked", m_ThresholdForMasked,
           "Threshold X for masked channels: if a channel has an occupancy X times larger than the average, it will be masked.", 100);
  addParam("ThresholdForHot", m_ThresholdForHot,
           "Threshold Y for hot channels: if a channel has an occupancy Y times larger than the average, it will be marked as hot (but not masked).",
           10);
  addParam("ThresholdForLog", m_ThresholdForLog,
           "Threshold Z for log scale view: if a channel has an occupancy Z times larger than the average, canvas shifts to log scale.",
           20);
  addParam("MinHitsForFlagging", m_MinHitsForFlagging, "Minimal number of hits in a channel required to flag it as 'Masked' or 'Hot'",
           50);
  addParam("MinProcessedEventsForMessages", m_MinProcessedEventsForMessagesInput,
           "Minimal number of processed events required to print error messages", 10000.);
  addParam("MinEntries", m_minEntries,
           "Minimal number for delta histogram updates", 50000.);
  addParam("MessageThreshold", m_MessageThreshold,
           "Max number of messages to show up in channel occupancy plots", 12);
  addParam("HistogramDirectoryName", m_histogramDirectoryName, "Name of histogram directory", std::string("KLM"));
  addParam("RefHistogramDirectoryPrefix", m_refHistogramDirectoryPrefix, "Prefix to account for reference file", std::string("ref/"));
  addParam("RefHistoFile", m_refFileName, "Reference histogram file name", std::string("KLM_DQM_REF_BEAM.root"));

  m_MinProcessedEventsForMessages = m_MinProcessedEventsForMessagesInput;
  m_2DHitsLine.SetLineColor(kRed);
  m_2DHitsLine.SetLineWidth(3);
  m_2DHitsLine.SetLineStyle(2); // dashed
  m_PlaneLine.SetLineColor(kMagenta);
  m_PlaneLine.SetLineWidth(1);
  m_PlaneLine.SetLineStyle(2); // dashed
  m_PlaneText.SetTextAlign(22); // centered, middle
  m_PlaneText.SetTextColor(kMagenta);
  m_PlaneText.SetTextFont(42); // Helvetica regular
  m_PlaneText.SetTextSize(0.02); // 2% of TPad's full height
}


void DQMHistAnalysisKLMModule::initialize()
{
  m_monObj = getMonitoringObject("klm");

  if (m_ThresholdForHot > m_ThresholdForMasked)
    B2FATAL("The threshold used for hot channels is larger than the one for masked channels."
            << LogVar("Threshold for hot channels", m_ThresholdForHot)
            << LogVar("Threshold for masked channels", m_ThresholdForMasked));

  if (m_refFileName != "") {
    m_refFile = TFile::Open(m_refFileName.data(), "READ");
  }

  // register plots for delta histogramming
  addDeltaPar(m_histogramDirectoryName, "time_rpc", HistDelta::c_Entries, m_minEntries, 1);
  addDeltaPar(m_histogramDirectoryName, "time_scintillator_bklm", HistDelta::c_Entries, m_minEntries, 1);
  addDeltaPar(m_histogramDirectoryName, "time_scintillator_eklm", HistDelta::c_Entries, m_minEntries, 1);

  //register EPICS PVs
  registerEpicsPV("KLM:MaskedChannels", "MaskedChannels");
  registerEpicsPV("KLM:DeadBarrelModules", "DeadBarrelModules");
  registerEpicsPV("KLM:DeadEndcapModules", "DeadEndcapModules");

  gROOT->cd();

  std::string str;
  KLMChannelIndex klmIndex(KLMChannelIndex::c_IndexLevelSector);
  for (KLMChannelIndex& klmSector : klmIndex) {
    int nHistograms;
    if (klmSector.getSubdetector() == KLMElementNumbers::c_BKLM)
      nHistograms = 2;
    else
      nHistograms = 3;
    for (int j = 0; j < nHistograms; j++) {
      str = "strip_hits_subdetector_" +
            std::to_string(klmSector.getSubdetector()) +
            "_section_" + std::to_string(klmSector.getSection()) +
            "_sector_" + std::to_string(klmSector.getSector()) +
            "_" + std::to_string(j);
      addDeltaPar(m_histogramDirectoryName, str, HistDelta::c_Entries, m_minEntries, 1);

    }
  }


  //search for reference
  if (m_refFile && m_refFile->IsOpen()) {
    B2INFO("DQMHistAnalysisKLM: reference root file (" << m_refFileName << ") FOUND, able to read ref histograms");

  } else
    B2WARNING("DQMHistAnalysisKLM: reference root file (" << m_refFileName << ") not found, or closed");
}

void DQMHistAnalysisKLMModule::terminate()
{
  if (m_refFile) {
    m_refFile->Close();
    delete m_refFile;
  }
}

void DQMHistAnalysisKLMModule::beginRun()
{
  if (!m_ElectronicsMap.isValid())
    B2FATAL("No KLM electronics map.");
  m_MinProcessedEventsForMessages = m_MinProcessedEventsForMessagesInput;
  m_ProcessedEvents = 0.;
  m_DeadBarrelModules.clear();
  m_DeadEndcapModules.clear();
  m_MaskedChannels.clear();

  m_IsNullRun = (getRunType() == "null");
}

void DQMHistAnalysisKLMModule::endRun()
{
  int hist_max_bin; double max_position;
  TH1* time_rpc = findHist(m_histogramDirectoryName + "/time_rpc");
  if (time_rpc) {
    hist_max_bin = time_rpc->GetMaximumBin();
    max_position = time_rpc->GetXaxis()->GetBinCenter(hist_max_bin);
    m_monObj->setVariable("RPC_Time_Peak", max_position);
  }

  TH1* time_scint_bklm = findHist(m_histogramDirectoryName + "/time_scintillator_bklm");
  if (time_scint_bklm) {
    hist_max_bin = time_scint_bklm->GetMaximumBin();
    max_position = time_scint_bklm->GetXaxis()->GetBinCenter(hist_max_bin);
    m_monObj->setVariable("BKLM_Scint_Time_Peak", max_position);
  }

  TH1* time_scint_eklm = findHist(m_histogramDirectoryName + "/time_scintillator_eklm");
  if (time_scint_eklm) {
    hist_max_bin = time_scint_eklm->GetMaximumBin();
    max_position = time_scint_eklm->GetXaxis()->GetBinCenter(hist_max_bin);
    m_monObj->setVariable("EKLM_Scint_Time_Peak", max_position);
  }
}

double DQMHistAnalysisKLMModule::getProcessedEvents()
{
  if (DQMHistAnalysisModule::getEventProcessed() == 0.) {
    B2WARNING("Either DAQ/Nevent is not found or Nevent = 0.");
    /* Set the minimal number of processed events to 0 if we can't determine the processed events. */
    m_MinProcessedEventsForMessages = 0.;
  }
  return (double) DQMHistAnalysisModule::getEventProcessed();
}

void DQMHistAnalysisKLMModule::deltaDrawer(TH1* delta, TH1* histogram, TCanvas* canvas)
{
  if (delta != nullptr) {
    Double_t scale = (Double_t) histogram->Integral(); //want delta and histo to have same norm

    // delta != nullptr should take care of whether update condition is met.
    delta->SetLineColor(kBlackBody); //random choice of not green or blue
    delta->SetLineStyle(4);
    delta->DrawNormalized("SAME", scale); //normalize delta to histo
    canvas->Modified();
    canvas->Update();
  }
}

void DQMHistAnalysisKLMModule::analyseChannelHitHistogram(
  int subdetector, int section, int sector, int index,
  TH1* histogram, TH1* delta, TCanvas* canvas, TLatex& latex)
{
  double x = 0.15;
  double y = 0.85;
  int i, n;
  std::map<KLMModuleNumber, double> moduleHitMap;
  std::map<KLMModuleNumber, double>::iterator it;
  double average = 0;
  int channelSubdetector, channelSection, channelSector;
  int layer, plane, strip;
  std::string str;
  canvas->Clear();
  canvas->cd();
  histogram->SetStats(false);
  canvas->SetLogy(0); //initialize to start without logscale
  histogram->Draw();
  deltaDrawer(delta, histogram, canvas); //draw normalized delta on top
  n = histogram->GetXaxis()->GetNbins();

  /* call reference histograms */
  TH1* ref_histogram = nullptr;
  float ref_average = 0;
  if (m_refFile && m_refFile->IsOpen()) {
    ref_histogram = (TH1*)m_refFile->Get((m_refHistogramDirectoryPrefix + histogram->GetName()).data());
    if (!ref_histogram) {
      B2WARNING("Unable to find " << m_refHistogramDirectoryPrefix << histogram->GetName() << " in reference file.");
    }
  }

  if (ref_histogram != nullptr) {
    for (i = 1; i <= n; i++) {
      double nHitsPerModuleRef = ref_histogram->GetBinContent(i);
      ref_average = ref_average + nHitsPerModuleRef;
    }
  }

  for (i = 1; i <= n; i++) {
    KLMChannelNumber channelIndex = std::round(histogram->GetBinCenter(i));
    KLMChannelNumber channelNumber =
      m_ChannelArrayIndex->getNumber(channelIndex);
    double nHitsPerModule = histogram->GetBinContent(i);
    average = average + nHitsPerModule;
    m_ElementNumbers->channelNumberToElementNumbers(
      channelNumber, &channelSubdetector, &channelSection, &channelSector,
      &layer, &plane, &strip);
    if ((channelSubdetector != subdetector) ||
        (channelSection != section) ||
        (channelSector != sector))
      B2FATAL("Inconsistent element numbers.");
    KLMModuleNumber module = m_ElementNumbers->moduleNumber(
                               subdetector, section, sector, layer);
    it = moduleHitMap.find(module);
    if (it == moduleHitMap.end()) {
      moduleHitMap.insert(std::pair<KLMModuleNumber, double>(
                            module, nHitsPerModule));
    } else {
      it->second += nHitsPerModule;
    }
  }
  unsigned int activeModuleChannels = 0;
  int message_counter = 0;
  for (it = moduleHitMap.begin(); it != moduleHitMap.end(); ++it) {
    KLMModuleNumber moduleNumber = it->first;
    if (it->second != 0) {
      activeModuleChannels += m_ElementNumbers->getNChannelsModule(moduleNumber);
      continue;
    }
    m_ElementNumbers->moduleNumberToElementNumbers(
      moduleNumber, &channelSubdetector, &channelSection, &channelSector, &layer);
    /* Channel with plane = 1, strip = 1 exists for any BKLM or EKLM module. */
    KLMChannelNumber channel =
      m_ElementNumbers->channelNumber(
        channelSubdetector, channelSection, channelSector, layer, 1, 1);
    const KLMElectronicsChannel* electronicsChannel =
      m_ElectronicsMap->getElectronicsChannel(channel);
    if (electronicsChannel == nullptr)
      B2FATAL("Incomplete KLM electronics map.");
    str = "No data from lane " + std::to_string(electronicsChannel->getLane());
    latex.DrawLatexNDC(x, y, str.c_str());
    y -= 0.05;
    message_counter++;
    /* Store the module number, used later in processPlaneHistogram
     * to color the canvas with red and to raise up an alarm. */
    if (channelSubdetector == KLMElementNumbers::c_BKLM) {
      std::vector<KLMModuleNumber>::iterator ite =
        std::find(m_DeadBarrelModules.begin(),
                  m_DeadBarrelModules.end(),
                  moduleNumber);
      if (ite == m_DeadBarrelModules.end())
        m_DeadBarrelModules.push_back(moduleNumber);
    } else {
      std::vector<KLMModuleNumber>::iterator ite = std::find(m_DeadEndcapModules.begin(),
                                                             m_DeadEndcapModules.end(),
                                                             moduleNumber);
      if (ite == m_DeadEndcapModules.end())
        m_DeadEndcapModules.push_back(moduleNumber);
    }
  }
  if (activeModuleChannels == 0)
    return;
  average /= activeModuleChannels;
  ref_average /= activeModuleChannels;

  for (i = 1; i <= n; ++i) {
    KLMChannelNumber channelIndex = std::round(histogram->GetBinCenter(i));
    KLMChannelNumber channelNumber =
      m_ChannelArrayIndex->getNumber(channelIndex);
    double nHits = histogram->GetBinContent(i);
    m_ElementNumbers->channelNumberToElementNumbers(
      channelNumber, &channelSubdetector, &channelSection, &channelSector,
      &layer, &plane, &strip);
    std::string channelStatus = "Normal";
    if ((nHits > average * m_ThresholdForMasked) && (nHits > m_MinHitsForFlagging)) {
      channelStatus = "Masked";
      std::vector<KLMModuleNumber>::iterator ite =
        std::find(m_MaskedChannels.begin(),
                  m_MaskedChannels.end(),
                  channelNumber);
      if (ite == m_MaskedChannels.end())
        m_MaskedChannels.push_back(channelNumber);
      B2DEBUG(20, "KLM@MaskMe " << channelNumber);
    } else if ((nHits > average * m_ThresholdForHot) && (nHits > m_MinHitsForFlagging)) {
      channelStatus = "Hot";
    }
    if (channelStatus != "Normal") {
      const KLMElectronicsChannel* electronicsChannel =
        m_ElectronicsMap->getElectronicsChannel(channelNumber);
      if (electronicsChannel == nullptr)
        B2FATAL("Incomplete BKLM electronics map.");
      if (channelStatus == "Masked") {
        histogram->SetBinContent(i, 0);
        if (delta != nullptr)
          delta->SetBinContent(i, 0);
      }
      str = channelStatus + " channel: ";
      // lane, axis, channel
      str += ("L" + std::to_string(electronicsChannel->getLane()) +
              " A" + std::to_string(electronicsChannel->getAxis()) +
              " Ch" + std::to_string(electronicsChannel->getChannel()));
      message_counter++;
      if (message_counter <= m_MessageThreshold) {
        latex.DrawLatexNDC(x, y, str.c_str());
        y -= 0.05;
      }
    }
  }
  if (message_counter > m_MessageThreshold) {
    std::string verbose_message = " more messages";
    verbose_message = std::to_string(message_counter - m_MessageThreshold) + verbose_message;
    latex.DrawLatexNDC(x, y, verbose_message.c_str());
  }


  // for hot/masked channels, log scale plots (reference and main)
  if (histogram->GetMaximum()*n > histogram->Integral()*m_ThresholdForLog && average * activeModuleChannels > m_MinHitsForFlagging) {
    histogram->SetMinimum(1);
    canvas->SetLogy();
  } else if (ref_histogram != nullptr) {
    if (ref_histogram->GetMaximum()*n > ref_histogram->Integral()*m_ThresholdForLog
        && ref_average * activeModuleChannels > m_MinHitsForFlagging) {
      histogram->SetMinimum(1);
      canvas->SetLogy();
    } else
      canvas->SetLogy(0);
  } else
    canvas->SetLogy(0);

  canvas->Modified();
  canvas->Update();

  /* Drawing dividing lines */
  int divisions;
  int bin = 1;
  double xLine;
  // drawing lines for BKLM sectors
  if (subdetector == 1) {
    int shift;
    if (index == 0) {
      divisions = 7;
      shift = 1;
    } else {
      divisions = 8;
      shift = 8;
    }
    for (int k = 0; k < divisions; k++) {
      xLine = (histogram->GetXaxis()->GetBinLowEdge(bin) - canvas->GetX1()) / (canvas->GetX2() - canvas->GetX1());
      m_PlaneLine.DrawLineNDC(xLine, 0.1, xLine, 0.9);
      bin += BKLMElementNumbers::getNStrips(section, sector, k + shift, 0)
             + BKLMElementNumbers::getNStrips(section, sector, k + shift, 1);
    }
  } else { // drawing lines for EKLM sectors
    if ((section == 2) && (index == 0 || index == 1))
      divisions = 5;
    else
      divisions = 4;
    for (int k = 0; k < divisions; k++) {
      xLine = (histogram->GetXaxis()->GetBinLowEdge(bin) - canvas->GetX1()) / (canvas->GetX2() - canvas->GetX1());
      m_PlaneLine.DrawLineNDC(xLine, 0.1, xLine, 0.9);
      bin += EKLMElementNumbers::getNStripsSector();
    }
  }
  canvas->Modified();
  canvas->Update();

}

void DQMHistAnalysisKLMModule::processSpatial2DHitEndcapHistogram(
  KLMSectionNumber section, TH2F* histogram, TCanvas* canvas)
{
  canvas->Clear();
  canvas->cd();
  histogram->SetStats(false);
  histogram->Draw("COLZ");
  /* Draw the lines only for the backward layers. */
  if (section == EKLMElementNumbers::c_ForwardSection) {
    m_2DHitsLine.DrawLine(-110, 80, -110, 190);
    m_2DHitsLine.DrawLine(-110, 190, 110, 190);
    m_2DHitsLine.DrawLine(110, 80, 110, 190);
    m_2DHitsLine.DrawLine(-110, 80, 110, 80);
  }
  canvas->Modified();
  canvas->Update();
}

void DQMHistAnalysisKLMModule::processTimeHistogram(
  const std::string& histName)
{
  TH1* histogram = findHist(m_histogramDirectoryName + "/" + histName);
  if (histogram == nullptr) {
    B2WARNING("KLM DQM histogram " + m_histogramDirectoryName + "/" << histName << " is not found.");
    return;
  }

  TCanvas* canvas = findCanvas(m_histogramDirectoryName + "/c_" + histName);
  if (canvas == nullptr) {
    B2WARNING("KLM DQM histogram canvas " + m_histogramDirectoryName + "/c_" << histName << " is not found.");
    return;
  }

  else {
    canvas->Clear();
    canvas->cd();
    histogram->Draw();
    /* calling on delta histogram*/
    TH1* delta = getDelta(m_histogramDirectoryName, histName);
    UpdateCanvas(canvas->GetName(), delta != nullptr); //keeping this for testing purposes
    if (delta != nullptr) {
      B2INFO("DQMHistAnalysisKLM: Time Delta Entries is " << delta->GetEntries());
      deltaDrawer(delta, histogram, canvas);
    }
  }
}

void DQMHistAnalysisKLMModule::fillMaskedChannelsHistogram(
  const std::string& histName)
{
  TH1* histogram = findHist(m_histogramDirectoryName + "/" + histName);
  if (histogram == nullptr) {
    B2WARNING("KLM DQM histogram " + m_histogramDirectoryName + "/" << histName << " is not found.");
    return;
  }
  TCanvas* canvas = findCanvas(m_histogramDirectoryName + "/c_" + histName);
  if (canvas == nullptr) {
    B2WARNING("KLM DQM histogram canvas " + m_histogramDirectoryName + "/c_" << histName << " is not found.");
    return;
  }

  histogram->Clear();
  canvas->Clear();
  canvas->cd();
  if (m_MaskedChannels.size() > 0) {
    int channelSubdetector, channelSection, channelSector;
    int layer, plane, strip;
    for (KLMChannelNumber channel : m_MaskedChannels) {
      m_ElementNumbers->channelNumberToElementNumbers(
        channel, &channelSubdetector, &channelSection, &channelSector,
        &layer, &plane, &strip);
      KLMSectorNumber sectorNumber;
      if (channelSubdetector == KLMElementNumbers::c_BKLM)
        sectorNumber = m_ElementNumbers->sectorNumberBKLM(channelSection, channelSector);
      else
        sectorNumber = m_ElementNumbers->sectorNumberEKLM(channelSection, channelSector);
      KLMSectorNumber sectorIndex = m_SectorArrayIndex->getIndex(sectorNumber);
      histogram->Fill(sectorIndex);
    }
  }
  histogram->SetStats(false);
  histogram->Draw();
  canvas->Modified();
  canvas->Update();
}

void DQMHistAnalysisKLMModule::processPlaneHistogram(
  const std::string& histName, TLatex& latex)
{

  TH1* histogram = findHist(m_histogramDirectoryName + "/" + histName);
  if (histogram == nullptr) {
    B2WARNING("KLM DQM histogram " + m_histogramDirectoryName + "/" << histName << " is not found.");
    return;
  }
  TCanvas* canvas = findCanvas(m_histogramDirectoryName + "/c_" + histName);
  if (canvas == nullptr) {
    B2WARNING("KLM DQM histogram canvas " + m_histogramDirectoryName + "/c_" << histName << " is not found.");
    return;
  } else {
    std::string name, alarm;
    int moduleSubdetector, moduleSection, moduleSector, moduleLayer;
    double xAlarm = 0.15;
    double yAlarm = 0.8;
    canvas->Clear();
    canvas->cd();
    histogram->SetStats(false);
    histogram->Draw();

    int message_counter = 0;
    if (histName.find("bklm") != std::string::npos) {
      /* First draw the vertical lines and the sector names. */
      const int maximalLayer = BKLMElementNumbers::getMaximalLayerNumber();
      for (int sector = 0; sector < BKLMElementNumbers::getMaximalSectorGlobalNumber(); ++sector) {
        int bin = maximalLayer * sector + 1;
        double xLine = histogram->GetXaxis()->GetBinLowEdge(bin);
        double xText = histogram->GetXaxis()->GetBinLowEdge(bin + maximalLayer / 2);
        double yText = gPad->GetUymin() + 0.98 * (gPad->GetUymax() - gPad->GetUymin());
        if (sector > 0)
          m_PlaneLine.DrawLine(xLine, gPad->GetUymin(), xLine, gPad->GetUymax());
        name = "B";
        if (sector < 8)
          name += "B";
        else
          name += "F";
        name += std::to_string(sector % 8);
        m_PlaneText.DrawText(xText, yText, name.c_str());
      }
      /* Then, color the canvas with red if there is a dead module
      * and write an error message. */
      if (m_DeadBarrelModules.size() == 0) {
        colorizeCanvas(canvas, c_StatusGood);
      } else if (m_ProcessedEvents > m_MinProcessedEventsForMessages) {
        for (KLMModuleNumber module : m_DeadBarrelModules) {
          m_ElementNumbers->moduleNumberToElementNumbers(
            module, &moduleSubdetector, &moduleSection, &moduleSector, &moduleLayer);
          alarm = "No data from " + m_ElementNumbers->getSectorDAQName(moduleSubdetector, moduleSection, moduleSector);
          alarm += ", layer " + std::to_string(moduleLayer);
          message_counter++;
          if (message_counter <= m_MessageThreshold) {
            latex.DrawLatexNDC(xAlarm, yAlarm, alarm.c_str());
            yAlarm -= 0.05;
          }
        }
        if (m_IsNullRun == false) {
          colorizeCanvas(canvas, c_StatusError);
        }
      } //end of enough statistics condition
      else {
        colorizeCanvas(canvas, c_StatusTooFew);
      }
    } else {
      /* First draw the vertical lines and the sector names. */
      const double maximalLayer = EKLMElementNumbers::getMaximalLayerGlobalNumber();
      const double maxPlane = EKLMElementNumbers::getMaximalPlaneNumber() * EKLMElementNumbers::getMaximalSectorNumber();
      for (int layerGlobal = 1; layerGlobal <= maximalLayer; ++layerGlobal) {
        int bin = maxPlane * layerGlobal + 1;
        double xLine = histogram->GetXaxis()->GetBinLowEdge(bin);
        double xText = histogram->GetXaxis()->GetBinLowEdge(bin - maxPlane / 2);
        double yText = gPad->GetUymin() + 0.98 * (gPad->GetUymax() - gPad->GetUymin());
        if (layerGlobal < maximalLayer)
          m_PlaneLine.DrawLine(xLine, gPad->GetUymin(), xLine, gPad->GetUymax());
        int section, layer;
        m_EklmElementNumbers->layerNumberToElementNumbers(
          layerGlobal, &section, &layer);
        if (section == EKLMElementNumbers::c_BackwardSection)
          name = "B";
        else
          name = "F";
        name += std::to_string(layer);
        m_PlaneText.DrawText(xText, yText, name.c_str());
      }
      /* Then, color the canvas with red if there is a dead module
      * and write an error message. */
      if (m_DeadEndcapModules.size() == 0) {
        colorizeCanvas(canvas, c_StatusGood);
      } else if (m_ProcessedEvents > m_MinProcessedEventsForMessages) {
        for (KLMModuleNumber module : m_DeadEndcapModules) {
          m_ElementNumbers->moduleNumberToElementNumbers(
            module, &moduleSubdetector, &moduleSection, &moduleSector, &moduleLayer);
          alarm = "No data from " + m_ElementNumbers->getSectorDAQName(moduleSubdetector, moduleSection, moduleSector);
          alarm += ", layer " + std::to_string(moduleLayer);
          message_counter++;
          if (message_counter <= m_MessageThreshold) {
            latex.DrawLatexNDC(xAlarm, yAlarm, alarm.c_str());
            yAlarm -= 0.05;
          }
        }
        if (m_IsNullRun == false) {
          colorizeCanvas(canvas, c_StatusError);
        }
      } //end of high statistics condition
      else {
        colorizeCanvas(canvas, c_StatusTooFew);
      }
    }
    if (message_counter > m_MessageThreshold) {
      std::string verbose_string = " more messages";
      verbose_string = std::to_string(message_counter - m_MessageThreshold) + verbose_string;
      latex.DrawLatexNDC(xAlarm, yAlarm, verbose_string.c_str());
    }
    canvas->Modified();
    canvas->Update();
  }
}

void DQMHistAnalysisKLMModule::event()
{
  /* If KLM is not included, stop here and return. */
  TH1* daqInclusion = findHist(m_histogramDirectoryName + "/daq_inclusion");
  if (not(daqInclusion == nullptr)) {
    int isKlmIncluded = daqInclusion->GetBinContent(daqInclusion->GetXaxis()->FindBin("Yes"));
    if (isKlmIncluded == 0)
      return;
  }
  /* Make sure that the vectors are cleared at each DQM refresh. */
  m_DeadBarrelModules.clear();
  m_DeadEndcapModules.clear();
  m_MaskedChannels.clear();
  m_ProcessedEvents = getProcessedEvents();
  std::string str, histogramName, canvasName;
  TLatex latex;
  latex.SetTextColor(kRed);
  latex.SetTextAlign(11);
  KLMChannelIndex klmIndex(KLMChannelIndex::c_IndexLevelSector);
  // gathering relevant info for analyseChannelHitHistogram
  for (KLMChannelIndex& klmSector : klmIndex) {
    int nHistograms;
    if (klmSector.getSubdetector() == KLMElementNumbers::c_BKLM)
      nHistograms = 2;
    else
      nHistograms = 3;
    for (int j = 0; j < nHistograms; j++) {
      str = "strip_hits_subdetector_" +
            std::to_string(klmSector.getSubdetector()) +
            "_section_" + std::to_string(klmSector.getSection()) +
            "_sector_" + std::to_string(klmSector.getSector()) +
            "_" + std::to_string(j);
      histogramName = m_histogramDirectoryName + "/" + str;
      canvasName = m_histogramDirectoryName + "/c_" + str;
      TH1* histogram = findHist(histogramName);
      //get delta histogram (should we work with a clone instead?)
      auto delta = getDelta("", histogramName);

      if (histogram == nullptr) {
        B2WARNING("KLM DQM histogram " << histogramName << " is not found.");
        continue;
      }
      TCanvas* canvas = findCanvas(canvasName);
      if (canvas == nullptr) {
        B2WARNING("KLM DQM histogram canvas " << canvasName << " is not found.");
        continue;
      }
      // Add this canvas that it is time to update
      // not sure if this is interfering with the generation of some features
      // after testing, switch condition back to delta != nullptr || histogram != nullptr
      UpdateCanvas(canvas->GetName(), true);
      analyseChannelHitHistogram(
        klmSector.getSubdetector(), klmSector.getSection(),
        klmSector.getSector(), j, histogram, delta, canvas, latex);

    }
  }
  /* Temporary change the color palette. */
  gStyle->SetPalette(kLightTemperature);
  klmIndex.setIndexLevel(KLMChannelIndex::c_IndexLevelSection);
  for (KLMChannelIndex& klmSection : klmIndex) {
    KLMSubdetectorNumber subdetector = klmSection.getSubdetector();
    if (subdetector == KLMElementNumbers::c_EKLM) {
      KLMSubdetectorNumber section = klmSection.getSection();
      int maximalLayerNumber = m_EklmElementNumbers->getMaximalDetectorLayerNumber(section);
      for (int j = 1; j <= maximalLayerNumber; ++j) {
        str = "spatial_2d_hits_subdetector_" + std::to_string(subdetector) +
              "_section_" + std::to_string(section) +
              "_layer_" + std::to_string(j);
        histogramName = m_histogramDirectoryName + "/" + str;
        canvasName = m_histogramDirectoryName + "/c_" + str;
        TH2F* histogram = static_cast<TH2F*>(findHist(histogramName));
        if (histogram == nullptr) {
          B2ERROR("KLM DQM histogram " << histogramName << " is not found.");
          continue;
        }
        TCanvas* canvas = findCanvas(canvasName);
        if (canvas == nullptr) {
          B2ERROR("KLM DQM histogram canvas " << canvasName << " is not found.");
          continue;
        }
        processSpatial2DHitEndcapHistogram(section, histogram, canvas);
      }
    }
  }
  /* Reset the color palette to the default one. */
  gStyle->SetPalette(kBird);
  fillMaskedChannelsHistogram("masked_channels");
  latex.SetTextColor(kBlue);
  processPlaneHistogram("plane_bklm_phi", latex);
  processPlaneHistogram("plane_bklm_z", latex);
  processPlaneHistogram("plane_eklm", latex);

  processTimeHistogram("time_rpc");
  processTimeHistogram("time_scintillator_bklm");
  processTimeHistogram("time_scintillator_eklm");

  B2DEBUG(20, "Updating EPICS PVs for DQMHistAnalysisKLM");
  // only update PVs if there's enough statistics and datasize != 0
  // Check if it's a null run, if so, don't update EPICS PVs
  if (m_IsNullRun) {
    B2INFO("DQMHistAnalysisKLM: Null run detected. No PV Update.");
    return;
  }
  auto* daqDataSize = findHist("DAQ/KLMDataSize");
  double meanDAQDataSize = 0.;
  if (daqDataSize != nullptr) {
    meanDAQDataSize = daqDataSize->GetMean();
  } else
    B2WARNING("DQMHistAnalysisKLM: Cannot find KLMDataSize");
  if ((daqDataSize != nullptr) and (meanDAQDataSize != 0.)) {
    if (m_ProcessedEvents > m_MinProcessedEventsForMessages) {
      setEpicsPV("MaskedChannels", (double)m_MaskedChannels.size());
      setEpicsPV("DeadBarrelModules", (double)m_DeadBarrelModules.size());
      setEpicsPV("DeadEndcapModules", (double)m_DeadEndcapModules.size());
      B2DEBUG(20, "DQMHistAnalysisKLM: MaskedChannels " << m_MaskedChannels.size());
      B2DEBUG(20, "DQMHistAnalysisKLM: DeadBarrelModules " << m_DeadBarrelModules.size());
      B2DEBUG(20, "DQMHistAnalysisKLM: DeadEndcapModules " << m_DeadEndcapModules.size());
    }
  } else
    B2INFO("DQMHistAnalysisKLM: KLM Not included. No PV Update. ");


}
