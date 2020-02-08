/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: giacomo                                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own include. */
#include <klm/calibration/KLMCalibrationChecker.h>

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>
#include <klm/dbobjects/KLMStripEfficiency.h>
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>

/* Belle II headers. */
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/database/Configuration.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>

/* ROOT include. */
#include <TCanvas.h>
#include <TFile.h>
#include <TH1F.h>
#include <TString.h>
#include <TTree.h>

using namespace Belle2;

KLMCalibrationChecker::KLMCalibrationChecker() :
  m_experiment(0),
  m_run(0)
{
}

KLMCalibrationChecker::~KLMCalibrationChecker()
{
}

void KLMCalibrationChecker::setExperimentRun(int experiment, int run)
{
  m_experiment = experiment;
  m_run = run;
}

void KLMCalibrationChecker::setTestingPayload(std::string testingPayloadName)
{
  m_testingPayloadName = testingPayloadName;
}

void KLMCalibrationChecker::prepareLocalDatabase()
{
  /* Mimic a module initialization. */
  StoreObjPtr<EventMetaData> eventMetaData;
  DataStore::Instance().setInitializeActive(true);
  eventMetaData.registerInDataStore();
  DataStore::Instance().setInitializeActive(false);
  eventMetaData.construct(1, m_run, m_experiment);
  /* Database instance and configuration. */
  DBStore& dbStore = DBStore::Instance();
  dbStore.update();
  dbStore.updateEvent();
  auto& dbConfiguration = Conditions::Configuration::getInstance();
  dbConfiguration.prependTestingPayloadLocation(m_testingPayloadName.c_str());
}

void KLMCalibrationChecker::checkStripEfficiency()
{
  /* Prepare the database. */
  prepareLocalDatabase();
  /* Now we can read the payload. */
  DBObjPtr<KLMStripEfficiency> stripEfficiency;
  if (not stripEfficiency.isValid())
    B2FATAL("Strip efficiency data are not valid.");
  TCanvas* canvas = new TCanvas();
  KLMChannelIndex klmSectors(KLMChannelIndex::c_IndexLevelSector);
  for (KLMChannelIndex& klmSector : klmSectors) {
    int subdetector = klmSector.getSubdetector();
    int section = klmSector.getSection();
    int sector = klmSector.getSector();
    if (subdetector == KLMElementNumbers::c_BKLM) {
      TH1F* hist = new TH1F("bklm_planes_0", "", 30, 0.5, 30.5);
      hist->GetXaxis()->SetTitle("(Layer - 1) * 2 + plane + 1");
      hist->GetYaxis()->SetTitle("Efficiency");
      hist->SetMinimum(0.4);
      hist->SetMaximum(1.);
      hist->SetMarkerStyle(20);
      hist->SetMarkerSize(0.5);
      for (int layer = 1; layer <= BKLMElementNumbers::getMaximalLayerNumber(); layer++) {
        for (int plane = 0; plane <= BKLMElementNumbers::getMaximalPlaneNumber(); plane++) {
          float efficiency = stripEfficiency->getBarrelEfficiency(section, sector, layer, plane, 2);
          float efficiencyError = stripEfficiency->getBarrelEfficiencyError(section, sector, layer, plane, 2);
          float bin = (layer - 1) * 2 + plane + 1;
          hist->SetBinContent(bin, efficiency);
          hist->SetBinContent(bin, efficiency);
          hist->SetBinError(bin, efficiencyError);
        }
      }
      TString title;
      if (section == BKLMElementNumbers::c_BackwardSection)
        title.Form("BKLM backward sector %d", sector);
      else
        title.Form("BKLM forward sector %d", sector);
      hist->SetTitle(title.Data());
      hist->Draw("e");
      TString name;
      name.Form("efficiency_subdetector_%d_section_%d_sector_%d.pdf", subdetector, section, sector);
      canvas->Print(name.Data());
      canvas->Update();
      delete hist;
    } else {
      TH1F* hist = new TH1F("eklm_planes_0", "", 30, 0.5, 30.5);
      if (section == EKLMElementNumbers::c_BackwardSection)
        hist->SetBins(24, 0.5, 24.5);
      else
        hist->SetBins(28, 0.5, 28.5);
      hist->GetXaxis()->SetTitle("(Layer - 1) * 2 + plane");
      hist->GetYaxis()->SetTitle("Efficiency");
      hist->SetMinimum(0.4);
      hist->SetMaximum(1.);
      hist->SetMarkerStyle(20);
      hist->SetMarkerSize(0.5);
      const EKLM::ElementNumbersSingleton* elementNumbersEKLM = &(EKLM::ElementNumbersSingleton::Instance());
      for (int layer = 1; layer <= elementNumbersEKLM->getMaximalDetectorLayerNumber(section); layer++) {
        for (int plane = 1; plane <= EKLMElementNumbers::getMaximalPlaneNumber(); plane++) {
          float efficiency = stripEfficiency->getEndcapEfficiency(section, sector, layer, plane, 2);
          float efficiencyError = stripEfficiency->getEndcapEfficiencyError(section, sector, layer, plane, 2);
          float bin = (layer - 1) * 2 + plane;
          hist->SetBinContent(bin, efficiency);
          hist->SetBinContent(bin, efficiency);
          hist->SetBinError(bin, efficiencyError);
        }
      }
      TString title;
      if (section == EKLMElementNumbers::c_BackwardSection)
        title.Form("EKLM backward sector %d", sector);
      else
        title.Form("EKLM forward sector %d", sector);
      hist->SetTitle(title.Data());
      hist->Draw("e");
      TString name;
      name.Form("efficiency_subdetector_%d_section_%d_sector_%d.pdf", subdetector, section, sector);
      canvas->Print(name.Data());
      canvas->Update();
      delete hist;
    }
  }
}
