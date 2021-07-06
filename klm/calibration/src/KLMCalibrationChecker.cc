/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own include. */
#include <klm/calibration/KLMCalibrationChecker.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>
#include <klm/dataobjects/KLMChannelIndex.h>
#include <klm/dbobjects/bklm/BKLMAlignment.h>
#include <klm/dbobjects/eklm/EKLMAlignment.h>
#include <klm/dbobjects/eklm/EKLMSegmentAlignment.h>
#include <klm/dbobjects/KLMStripEfficiency.h>

/* Belle II headers. */
#include <framework/database/Database.h>
#include <framework/database/DBStore.h>
#include <framework/database/Configuration.h>
#include <framework/datastore/DataStore.h>

/* ROOT include. */
#include <TCanvas.h>
#include <TFile.h>
#include <TH1F.h>
#include <TString.h>
#include <TTree.h>

using namespace Belle2;

KLMCalibrationChecker::KLMCalibrationChecker() :
  m_experiment(0),
  m_run(0),
  m_ElementNumbers(&(KLMElementNumbers::Instance()))
{
}

KLMCalibrationChecker::~KLMCalibrationChecker()
{
}

void KLMCalibrationChecker::setExperimentRun(int experiment, int run)
{
  m_experiment = experiment;
  m_run = run;
  if (m_EventMetaData.isValid()) {
    m_EventMetaData->setExperiment(experiment);
    m_EventMetaData->setRun(run);
  }
}

void KLMCalibrationChecker::initializeDatabase()
{
  /* Mimic a module initialization. */
  DataStore::Instance().setInitializeActive(true);
  m_EventMetaData.registerInDataStore();
  DataStore::Instance().setInitializeActive(false);
  if (!m_EventMetaData.isValid())
    m_EventMetaData.construct(1, m_run, m_experiment);
  /* Database instance and configuration. */
  DBStore& dbStore = DBStore::Instance();
  dbStore.update();
  dbStore.updateEvent();
  auto& dbConfiguration = Conditions::Configuration::getInstance();
  if ((m_testingPayloadName != "") and (m_GlobalTagName == ""))
    dbConfiguration.prependTestingPayloadLocation(m_testingPayloadName);
  else if ((m_testingPayloadName == "") and (m_GlobalTagName != ""))
    dbConfiguration.prependGlobalTag(m_GlobalTagName);
  else
    B2FATAL("Setting both testing payload and Global Tag or setting no one of them.");
}

void KLMCalibrationChecker::resetDatabase()
{
  /* Reset both DataStore and Database. */
  DataStore::Instance().reset();
  Database::Instance().reset(false);
  DBStore::Instance().reset(false);
}

void KLMCalibrationChecker::checkAlignment()
{
  /* Initialize the database. */
  initializeDatabase();
  /* Now we can read the payload. */
  DBObjPtr<BKLMAlignment> bklmAlignment;
  DBObjPtr<EKLMAlignment> eklmAlignment;
  DBObjPtr<EKLMSegmentAlignment> eklmSegmentAlignment;
  DBObjPtr<BKLMAlignment> bklmAlignmentErrors("BKLMAlignment_ERRORS");
  DBObjPtr<EKLMAlignment> eklmAlignmentErrors("EKLMAlignment_ERRORS");
  DBObjPtr<EKLMSegmentAlignment> eklmSegmentAlignmentErrors("EKLMSegmentAlignment_ERRORS");
  DBObjPtr<BKLMAlignment> bklmAlignmentCorrections("BKLMAlignment_CORRECTIONS");
  DBObjPtr<EKLMAlignment> eklmAlignmentCorrections("EKLMAlignment_CORRECTIONS");
  DBObjPtr<EKLMSegmentAlignment> eklmSegmentAlignmentCorrections("EKLMSegmentAlignment_CORRECTIONS");
  if (!bklmAlignment.isValid() ||
      !eklmAlignment.isValid() ||
      !eklmSegmentAlignment.isValid() ||
      !bklmAlignmentErrors.isValid() ||
      !eklmAlignmentErrors.isValid() ||
      !eklmSegmentAlignmentErrors.isValid() ||
      !bklmAlignmentCorrections.isValid() ||
      !eklmAlignmentCorrections.isValid() ||
      !eklmSegmentAlignmentCorrections.isValid())
    B2FATAL("Alignment data are not valid.");
  if (m_GlobalTagName != "") {
    printPayloadInformation(bklmAlignment);
    printPayloadInformation(eklmAlignment);
    printPayloadInformation(eklmSegmentAlignment);
    printPayloadInformation(bklmAlignmentErrors);
    printPayloadInformation(eklmAlignmentErrors);
    printPayloadInformation(eklmSegmentAlignmentErrors);
    printPayloadInformation(bklmAlignmentCorrections);
    printPayloadInformation(eklmAlignmentCorrections);
    printPayloadInformation(eklmSegmentAlignmentCorrections);
  }
  /* Create trees with alignment results. */
  int section, sector, layer, plane, segment, param;
  float value, correction, error;
  TFile* alignmentResults = new TFile(m_AlignmentResultsFile.c_str(),
                                      "recreate");
  TTree* bklmModuleTree = new TTree("bklm_module",
                                    "BKLM module alignment data.");
  bklmModuleTree->Branch("experiment", &m_experiment, "experiment/I");
  bklmModuleTree->Branch("run", &m_run, "run/I");
  bklmModuleTree->Branch("section", &section, "section/I");
  bklmModuleTree->Branch("sector", &sector, "sector/I");
  bklmModuleTree->Branch("layer", &layer, "layer/I");
  bklmModuleTree->Branch("param", &param, "param/I");
  bklmModuleTree->Branch("value", &value, "value/F");
  bklmModuleTree->Branch("correction", &correction, "correction/F");
  bklmModuleTree->Branch("error", &error, "error/F");
  TTree* eklmModuleTree = new TTree("eklm_module",
                                    "EKLM module alignment data.");
  eklmModuleTree->Branch("experiment", &m_experiment, "experiment/I");
  eklmModuleTree->Branch("run", &m_run, "run/I");
  eklmModuleTree->Branch("section", &section, "section/I");
  eklmModuleTree->Branch("sector", &sector, "sector/I");
  eklmModuleTree->Branch("layer", &layer, "layer/I");
  eklmModuleTree->Branch("param", &param, "param/I");
  eklmModuleTree->Branch("value", &value, "value/F");
  eklmModuleTree->Branch("correction", &correction, "correction/F");
  eklmModuleTree->Branch("error", &error, "error/F");
  TTree* eklmSegmentTree = new TTree("eklm_segment",
                                     "EKLM segment alignment data.");
  eklmSegmentTree->Branch("experiment", &m_experiment, "experiment/I");
  eklmSegmentTree->Branch("run", &m_run, "run/I");
  eklmSegmentTree->Branch("section", &section, "section/I");
  eklmSegmentTree->Branch("sector", &sector, "sector/I");
  eklmSegmentTree->Branch("layer", &layer, "layer/I");
  eklmSegmentTree->Branch("plane", &plane, "plane/I");
  eklmSegmentTree->Branch("segment", &segment, "segment/I");
  eklmSegmentTree->Branch("param", &param, "param/I");
  eklmSegmentTree->Branch("value", &value, "value/F");
  eklmSegmentTree->Branch("correction", &correction, "correction/F");
  eklmSegmentTree->Branch("error", &error, "error/F");
  const KLMAlignmentData* alignment, *alignmentError, *alignmentCorrection;
  KLMAlignmentData zeroAlignment(0, 0, 0, 0, 0, 0);
  KLMChannelIndex klmModules(KLMChannelIndex::c_IndexLevelLayer);
  for (KLMChannelIndex& klmModule : klmModules) {
    KLMModuleNumber module = klmModule.getKLMModuleNumber();
    if (klmModule.getSubdetector() == KLMElementNumbers::c_BKLM) {
      alignment = bklmAlignment->getModuleAlignment(module);
      alignmentError = bklmAlignmentErrors->getModuleAlignment(module);
      alignmentCorrection =
        bklmAlignmentCorrections->getModuleAlignment(module);
    } else {
      alignment = eklmAlignment->getModuleAlignment(module);
      alignmentError = eklmAlignmentErrors->getModuleAlignment(module);
      alignmentCorrection =
        eklmAlignmentCorrections->getModuleAlignment(module);
    }
    if (alignment == nullptr)
      B2FATAL("Incomplete KLM alignment data.");
    if ((alignmentError == nullptr) && (alignmentCorrection == nullptr)) {
      B2WARNING("Alignment is not determined for KLM module."
                << LogVar("Module", module));
      alignmentError = &zeroAlignment;
      alignmentCorrection = &zeroAlignment;
    } else if ((alignmentError == nullptr) ||
               (alignmentCorrection == nullptr)) {
      B2FATAL("Inconsistent undtermined parameters.");
    }
    section = klmModule.getSection();
    sector = klmModule.getSector();
    layer = klmModule.getLayer();
    param = KLMAlignmentData::c_DeltaU;
    value = alignment->getDeltaU();
    /* cppcheck-suppress nullPointerRedundantCheck */
    error = alignmentError->getDeltaU();
    /* cppcheck-suppress nullPointerRedundantCheck */
    correction = alignmentCorrection->getDeltaU();
    if (klmModule.getSubdetector() == KLMElementNumbers::c_BKLM)
      bklmModuleTree->Fill();
    else
      eklmModuleTree->Fill();
    /* cppcheck-suppress redundantAssignment */
    param = KLMAlignmentData::c_DeltaV;
    /* cppcheck-suppress redundantAssignment */
    value = alignment->getDeltaV();
    /* cppcheck-suppress redundantAssignment */
    error = alignmentError->getDeltaV();
    /* cppcheck-suppress redundantAssignment */
    correction = alignmentCorrection->getDeltaV();
    if (klmModule.getSubdetector() == KLMElementNumbers::c_BKLM)
      bklmModuleTree->Fill();
    else
      eklmModuleTree->Fill();
    /* cppcheck-suppress redundantAssignment */
    param = KLMAlignmentData::c_DeltaW;
    /* cppcheck-suppress redundantAssignment */
    value = alignment->getDeltaW();
    /* cppcheck-suppress redundantAssignment */
    error = alignmentError->getDeltaW();
    /* cppcheck-suppress redundantAssignment */
    correction = alignmentCorrection->getDeltaW();
    if (klmModule.getSubdetector() == KLMElementNumbers::c_BKLM)
      bklmModuleTree->Fill();
    else
      eklmModuleTree->Fill();
    /* cppcheck-suppress redundantAssignment */
    param = KLMAlignmentData::c_DeltaAlpha;
    /* cppcheck-suppress redundantAssignment */
    value = alignment->getDeltaAlpha();
    /* cppcheck-suppress redundantAssignment */
    error = alignmentError->getDeltaAlpha();
    /* cppcheck-suppress redundantAssignment */
    correction = alignmentCorrection->getDeltaAlpha();
    if (klmModule.getSubdetector() == KLMElementNumbers::c_BKLM)
      bklmModuleTree->Fill();
    else
      eklmModuleTree->Fill();
    /* cppcheck-suppress redundantAssignment */
    param = KLMAlignmentData::c_DeltaBeta;
    /* cppcheck-suppress redundantAssignment */
    value = alignment->getDeltaBeta();
    /* cppcheck-suppress redundantAssignment */
    error = alignmentError->getDeltaBeta();
    /* cppcheck-suppress redundantAssignment */
    correction = alignmentCorrection->getDeltaBeta();
    if (klmModule.getSubdetector() == KLMElementNumbers::c_BKLM)
      bklmModuleTree->Fill();
    else
      eklmModuleTree->Fill();
    /* cppcheck-suppress redundantAssignment */
    param = KLMAlignmentData::c_DeltaGamma;
    /* cppcheck-suppress redundantAssignment */
    value = alignment->getDeltaGamma();
    /* cppcheck-suppress redundantAssignment */
    error = alignmentError->getDeltaGamma();
    /* cppcheck-suppress redundantAssignment */
    correction = alignmentCorrection->getDeltaGamma();
    if (klmModule.getSubdetector() == KLMElementNumbers::c_BKLM)
      bklmModuleTree->Fill();
    else
      eklmModuleTree->Fill();
  }
  KLMChannelIndex eklmSegments(KLMChannelIndex::c_IndexLevelStrip);
  KLMChannelIndex eklmSegment(KLMChannelIndex::c_IndexLevelStrip);
  eklmSegment = eklmSegments.beginEKLM();
  eklmSegment.useEKLMSegments();
  for (; eklmSegment != eklmSegments.endEKLM(); ++eklmSegment) {
    int eklmSegmentNumber = eklmSegment.getEKLMSegmentNumber();
    alignment = eklmSegmentAlignment->getSegmentAlignment(eklmSegmentNumber);
    alignmentError =
      eklmSegmentAlignmentErrors->getSegmentAlignment(eklmSegmentNumber);
    alignmentCorrection =
      eklmSegmentAlignmentCorrections->getSegmentAlignment(eklmSegmentNumber);
    if (alignment == nullptr)
      B2FATAL("Incomplete KLM alignment data.");
    if ((alignmentError == nullptr) && (alignmentCorrection == nullptr)) {
      /*
       * The segment alignment is not determined now.
       * TODO: If it will be turned on in the future, add a warning here.
       */
      alignmentError = &zeroAlignment;
      alignmentCorrection = &zeroAlignment;
    } else if ((alignmentError == nullptr) ||
               (alignmentCorrection == nullptr)) {
      B2FATAL("Inconsistent undtermined parameters.");
    }
    section = eklmSegment.getSection();
    sector = eklmSegment.getSector();
    layer = eklmSegment.getLayer();
    plane = eklmSegment.getPlane();
    segment = eklmSegment.getStrip();
    param = KLMAlignmentData::c_DeltaU;
    value = alignment->getDeltaU();
    error = alignmentError->getDeltaU();
    correction = alignmentCorrection->getDeltaU();
    eklmSegmentTree->Fill();
    /* cppcheck-suppress redundantAssignment */
    param = KLMAlignmentData::c_DeltaV;
    /* cppcheck-suppress redundantAssignment */
    value = alignment->getDeltaV();
    /* cppcheck-suppress redundantAssignment */
    error = alignmentError->getDeltaV();
    /* cppcheck-suppress redundantAssignment */
    correction = alignmentCorrection->getDeltaV();
    eklmSegmentTree->Fill();
    /* cppcheck-suppress redundantAssignment */
    param = KLMAlignmentData::c_DeltaW;
    /* cppcheck-suppress redundantAssignment */
    value = alignment->getDeltaW();
    /* cppcheck-suppress redundantAssignment */
    error = alignmentError->getDeltaW();
    /* cppcheck-suppress redundantAssignment */
    correction = alignmentCorrection->getDeltaW();
    eklmSegmentTree->Fill();
    /* cppcheck-suppress redundantAssignment */
    param = KLMAlignmentData::c_DeltaAlpha;
    /* cppcheck-suppress redundantAssignment */
    value = alignment->getDeltaAlpha();
    /* cppcheck-suppress redundantAssignment */
    error = alignmentError->getDeltaAlpha();
    /* cppcheck-suppress redundantAssignment */
    correction = alignmentCorrection->getDeltaAlpha();
    eklmSegmentTree->Fill();
    /* cppcheck-suppress redundantAssignment */
    param = KLMAlignmentData::c_DeltaBeta;
    /* cppcheck-suppress redundantAssignment */
    value = alignment->getDeltaBeta();
    /* cppcheck-suppress redundantAssignment */
    error = alignmentError->getDeltaBeta();
    /* cppcheck-suppress redundantAssignment */
    correction = alignmentCorrection->getDeltaBeta();
    eklmSegmentTree->Fill();
    /* cppcheck-suppress redundantAssignment */
    param = KLMAlignmentData::c_DeltaGamma;
    /* cppcheck-suppress redundantAssignment */
    value = alignment->getDeltaGamma();
    /* cppcheck-suppress redundantAssignment */
    error = alignmentError->getDeltaGamma();
    /* cppcheck-suppress redundantAssignment */
    correction = alignmentCorrection->getDeltaGamma();
    eklmSegmentTree->Fill();
  }
  bklmModuleTree->Write();
  eklmModuleTree->Write();
  eklmSegmentTree->Write();
  delete bklmModuleTree;
  delete eklmModuleTree;
  delete eklmSegmentTree;
  delete alignmentResults;
  /* Reset the database. Needed to avoid mess if we call this method multiple times with different GTs. */
  resetDatabase();
}

void KLMCalibrationChecker::checkStripEfficiency()
{
  /* Initialize the database. */
  initializeDatabase();
  /* Now we can read the payload. */
  DBObjPtr<KLMStripEfficiency> stripEfficiency;
  if (!stripEfficiency.isValid())
    B2FATAL("Strip efficiency data are not valid.");
  if (m_GlobalTagName != "")
    printPayloadInformation(stripEfficiency);
  /* Create trees with strip efficiency measurement results. */
  int subdetector, section, sector, layer, plane;
  float efficiency, error;
  TFile* stripEfficiencyResults =
    new TFile(m_StripEfficiencyResultsFile.c_str(), "recreate");
  TTree* efficiencyTree = new TTree("efficiency", "KLM strip efficiency data.");
  efficiencyTree->Branch("experiment", &m_experiment, "experiment/I");
  efficiencyTree->Branch("run", &m_run, "run/I");
  efficiencyTree->Branch("subdetector", &subdetector, "subdetector/I");
  efficiencyTree->Branch("section", &section, "section/I");
  efficiencyTree->Branch("sector", &sector, "sector/I");
  efficiencyTree->Branch("layer", &layer, "layer/I");
  efficiencyTree->Branch("plane", &plane, "plane/I");
  efficiencyTree->Branch("efficiency", &efficiency, "efficiency/F");
  efficiencyTree->Branch("error", &error, "error/F");
  KLMChannelIndex klmPlanes(KLMChannelIndex::c_IndexLevelPlane);
  for (KLMChannelIndex& klmPlane : klmPlanes) {
    subdetector = klmPlane.getSubdetector();
    section = klmPlane.getSection();
    sector = klmPlane.getSector();
    layer = klmPlane.getLayer();
    plane = klmPlane.getPlane();
    KLMChannelNumber channel = m_ElementNumbers->channelNumber(
                                 subdetector, section, sector, layer, plane, 1);
    efficiency = stripEfficiency->getEfficiency(channel);
    error = stripEfficiency->getEfficiencyError(channel);
    efficiencyTree->Fill();
  }
  efficiencyTree->Write();
  delete efficiencyTree;
  delete stripEfficiencyResults;
  /* Reset the database. Needed to avoid mess if we call this method multiple times with different GTs. */
  resetDatabase();
}

void KLMCalibrationChecker::createStripEfficiencyHistograms()
{
  /* Initialize the database. */
  initializeDatabase();
  /* Now we can read the payload. */
  DBObjPtr<KLMStripEfficiency> stripEfficiency;
  if (not stripEfficiency.isValid())
    B2FATAL("Strip efficiency data are not valid.");
  if (m_GlobalTagName != "")
    printPayloadInformation(stripEfficiency);
  /* Finally, loop over KLM sectors to check the efficiency. */
  KLMChannelIndex klmSectors(KLMChannelIndex::c_IndexLevelSector);
  TCanvas* canvas = new TCanvas();
  for (KLMChannelIndex& klmSector : klmSectors) {
    int subdetector = klmSector.getSubdetector();
    int section = klmSector.getSection();
    int sector = klmSector.getSector();
    /* Setup the histogram. */
    TH1F* hist = new TH1F("plane_histogram", "", 30, 0.5, 30.5);
    hist->GetYaxis()->SetTitle("Efficiency");
    hist->SetMinimum(0.4);
    hist->SetMaximum(1.);
    hist->SetMarkerStyle(20);
    hist->SetMarkerSize(0.5);
    TString title;
    if (subdetector == KLMElementNumbers::c_BKLM) {
      if (section == BKLMElementNumbers::c_BackwardSection)
        title.Form("BKLM backward sector %d", sector);
      else
        title.Form("BKLM forward sector %d", sector);
      hist->SetTitle(title.Data());
      hist->GetXaxis()->SetTitle("(Layer - 1) * 2 + plane + 1");
      for (int layer = 1; layer <= BKLMElementNumbers::getMaximalLayerNumber(); layer++) {
        for (int plane = 0; plane <= BKLMElementNumbers::getMaximalPlaneNumber(); plane++) {
          int bin = (layer - 1) * 2 + plane + 1;
          float efficiency = stripEfficiency->getBarrelEfficiency(section, sector, layer, plane, 2);
          float efficiencyError = stripEfficiency->getBarrelEfficiencyError(section, sector, layer, plane, 2);
          hist->SetBinContent(bin, efficiency);
          hist->SetBinError(bin, efficiencyError);
        }
      }
    } else {
      if (section == EKLMElementNumbers::c_BackwardSection) {
        hist->SetBins(24, 0.5, 24.5);
        title.Form("EKLM backward sector %d", sector);
      } else {
        hist->SetBins(28, 0.5, 28.5);
        title.Form("EKLM forward sector %d", sector);
      }
      hist->SetTitle(title.Data());
      hist->GetXaxis()->SetTitle("(Layer - 1) * 2 + plane");
      const EKLMElementNumbers* elementNumbersEKLM = &(EKLMElementNumbers::Instance());
      for (int layer = 1; layer <= elementNumbersEKLM->getMaximalDetectorLayerNumber(section); layer++) {
        for (int plane = 1; plane <= EKLMElementNumbers::getMaximalPlaneNumber(); plane++) {
          int bin = (layer - 1) * 2 + plane;
          float efficiency = stripEfficiency->getEndcapEfficiency(section, sector, layer, plane, 2);
          float efficiencyError = stripEfficiency->getEndcapEfficiencyError(section, sector, layer, plane, 2);
          hist->SetBinContent(bin, efficiency);
          hist->SetBinError(bin, efficiencyError);
        }
      }
    }
    hist->Draw("e");
    TString name;
    name.Form("efficiency_subdetector_%d_section_%d_sector_%d.pdf", subdetector, section, sector);
    canvas->Print(name.Data());
    canvas->Update();
    delete hist;
  }
  delete canvas;
  /* Reset the database. Needed to avoid mess if we call this method multiple times with different GTs. */
  resetDatabase();
}
