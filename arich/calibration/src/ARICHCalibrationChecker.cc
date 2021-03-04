/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own include. */
#include <arich/calibration/ARICHCalibrationChecker.h>

/* ARICH headers. */
#include <arich/utility/ARICHChannelHist.h>
#include <arich/dbobjects/ARICHChannelMask.h>

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

ARICHCalibrationChecker::ARICHCalibrationChecker() :
  m_experiment(0),
  m_run(0)
{
}

ARICHCalibrationChecker::~ARICHCalibrationChecker()
{
}

void ARICHCalibrationChecker::setExperimentRun(int experiment, int run)
{
  m_experiment = experiment;
  m_run = run;
  if (m_EventMetaData.isValid()) {
    m_EventMetaData->setExperiment(experiment);
    m_EventMetaData->setRun(run);
  }
}

void ARICHCalibrationChecker::initializeDatabase()
{
  /* Mimic a module initialization. */
  StoreObjPtr<EventMetaData> eventMetaData;
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

void ARICHCalibrationChecker::resetDatabase()
{
  /* Reset both DataStore and Database. */
  DataStore::Instance().reset();
  Database::Instance().reset(false);
  DBStore::Instance().reset(false);
}

int  ARICHCalibrationChecker::getRing(int modID)
{
  if (modID <= 42)  return 1;
  if (modID <= 90)  return 2;
  if (modID <= 144) return 3;
  if (modID <= 204) return 4;
  if (modID <= 270) return 5;
  if (modID <= 342) return 6;
  if (modID <= 420) return 7;
  return 0;
}

int ARICHCalibrationChecker::getSector(int modID)
{
  if (getRing(modID) == 1) return (modID - 1) / 7 + 1;
  if (getRing(modID) == 2) return (modID - 43) / 8 + 1;
  if (getRing(modID) == 3) return (modID - 91) / 9 + 1;
  if (getRing(modID) == 4) return (modID - 145) / 10 + 1;
  if (getRing(modID) == 5) return (modID - 205) / 11 + 1;
  if (getRing(modID) == 6) return (modID - 271) / 12 + 1;
  if (getRing(modID) == 7) return (modID - 343) / 13 + 1;
  return 0;
}

void ARICHCalibrationChecker::checkChannelMask()
{
  /* Initialize the database. */
  initializeDatabase();
  /* Now we can read the payload. */
  DBObjPtr<ARICHChannelMask> channelMask;
  if (!channelMask.isValid())
    B2FATAL("ARICHChannelMask is not valid.");
  if (m_GlobalTagName != "")
    printPayloadInformation(channelMask);
  /* Create trees with strip efficiency measurement results. */
  float frac_masked_sector[6] = {0.};
  float frac_masked = 0.;
  TFile* channelMaskResults =
    new TFile(m_channelMaskResultsFile.c_str(), "recreate");
  TTree* maskTree = new TTree("arich_masked", "ARICH channel masking");
  maskTree->Branch("experiment", &m_experiment, "experiment/I");
  maskTree->Branch("run", &m_run, "run/I");
  maskTree->Branch("frac_masked_sector", &frac_masked_sector, "frac_masked_sector[6]/F");
  maskTree->Branch("frac_masked", &frac_masked, "frac_masked/F");

  for (int mod = 1; mod < 421; mod++) {
    int sector = getSector(mod);
    for (int chn = 0; chn < 144; chn++) {
      if (!channelMask->isActive(mod, chn)) { frac_masked_sector[sector - 1]++; frac_masked++;}
    }
  }

  for (int sec = 0; sec < 6; sec++) frac_masked_sector[sec] /= 10080.;
  frac_masked /= 60480.;
  maskTree->Fill();

  maskTree->Write();
  delete maskTree;
  delete channelMaskResults;
  /* Reset the database. Needed to avoid mess if we call this method multiple times with different GTs. */
  resetDatabase();
}

void ARICHCalibrationChecker::createChannelMaskHistograms()
{
  /* Initialize the database. */
  initializeDatabase();
  /* Now we can read the payload. */
  /* DBObjPtr<ARICHStripEfficiency> stripEfficiency;
  if (not stripEfficiency.isValid())
    B2FATAL("Strip efficiency data are not valid.");
  if (m_GlobalTagName != "")
    printPayloadInformation(stripEfficiency);

  ARICHChannelIndex arichSectors(ARICHChannelIndex::c_IndexLevelSector);
  TCanvas* canvas = new TCanvas();
  for (ARICHChannelIndex& arichSector : arichSectors) {
    int subdetector = arichSector.getSubdetector();
    int section = arichSector.getSection();
    int sector = arichSector.getSector();

    TH1F* hist = new TH1F("plane_histogram", "", 30, 0.5, 30.5);
    hist->GetYaxis()->SetTitle("Efficiency");
    hist->SetMinimum(0.4);
    hist->SetMaximum(1.);
    hist->SetMarkerStyle(20);
    hist->SetMarkerSize(0.5);
    TString title;
    if (subdetector == ARICHElementNumbers::c_BARICH) {
      if (section == BARICHElementNumbers::c_BackwardSection)
        title.Form("BARICH backward sector %d", sector);
      else
        title.Form("BARICH forward sector %d", sector);
      hist->SetTitle(title.Data());
      hist->GetXaxis()->SetTitle("(Layer - 1) * 2 + plane + 1");
      for (int layer = 1; layer <= BARICHElementNumbers::getMaximalLayerNumber(); layer++) {
        for (int plane = 0; plane <= BARICHElementNumbers::getMaximalPlaneNumber(); plane++) {
          int bin = (layer - 1) * 2 + plane + 1;
          float efficiency = stripEfficiency->getBarrelEfficiency(section, sector, layer, plane, 2);
          float efficiencyError = stripEfficiency->getBarrelEfficiencyError(section, sector, layer, plane, 2);
          hist->SetBinContent(bin, efficiency);
          hist->SetBinError(bin, efficiencyError);
        }
      }
    } else {
      if (section == EARICHElementNumbers::c_BackwardSection) {
        hist->SetBins(24, 0.5, 24.5);
        title.Form("EARICH backward sector %d", sector);
      } else {
        hist->SetBins(28, 0.5, 28.5);
        title.Form("EARICH forward sector %d", sector);
      }
      hist->SetTitle(title.Data());
      hist->GetXaxis()->SetTitle("(Layer - 1) * 2 + plane");
      const EARICHElementNumbers* elementNumbersEARICH = &(EARICHElementNumbers::Instance());
      for (int layer = 1; layer <= elementNumbersEARICH->getMaximalDetectorLayerNumber(section); layer++) {
        for (int plane = 1; plane <= EARICHElementNumbers::getMaximalPlaneNumber(); plane++) {
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
  */
  resetDatabase();
}
