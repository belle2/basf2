/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Giacomo De Pietro                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* C++ headers. */
#include <cmath>
#include <string>

/* External headers. */
#include <TFile.h>
#include <TTree.h>

/* Belle2 headers. */
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <klm/calibration/KLMDatabaseImporter.h>

using namespace Belle2;

KLMDatabaseImporter::KLMDatabaseImporter() :
  m_ExperimentLow(0),
  m_RunLow(0),
  m_ExperimentHigh(-1),
  m_RunHigh(-1)
{
}

KLMDatabaseImporter::~KLMDatabaseImporter()
{
}

void KLMDatabaseImporter::setIOV(int experimentLow, int runLow,
                                 int experimentHigh, int runHigh)
{
  m_ExperimentLow = experimentLow;
  m_RunLow = runLow;
  m_ExperimentHigh = experimentHigh;
  m_RunHigh = runHigh;
}

void KLMDatabaseImporter::importChannelStatus(
  const KLMChannelStatus* channelStatus)
{
  DBImportObjPtr<KLMChannelStatus> channelStatusImport;
  channelStatusImport.construct(*channelStatus);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  channelStatusImport.import(iov);
}

void KLMDatabaseImporter::importScintillatorDigitizationParameters(
  const KLMScintillatorDigitizationParameters* digitizationParameters)
{
  DBImportObjPtr<KLMScintillatorDigitizationParameters> digPar;
  digPar.construct(*digitizationParameters);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  digPar.import(iov);
}

void KLMDatabaseImporter::importTimeConversion(
  const KLMTimeConversion* timeConversion)
{
  DBImportObjPtr<KLMTimeConversion> timeConversionImport;
  timeConversionImport.construct(*timeConversion);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  timeConversionImport.import(iov);
}

void KLMDatabaseImporter::importStripEfficiency(std::string fileName)
{
  DBImportObjPtr<KLMStripEfficiency> stripEfficiency;
  stripEfficiency.construct();

  TFile* file = TFile::Open(fileName.c_str(), "r");
  if (!file) {
    B2ERROR("KLMDatabaseImporter: calibration file " << fileName << " *** failed to open");
  } else {
    TTree* tree = (TTree*)file->Get("tree");
    if (!tree) {
      B2ERROR("KLMDatabaseImporter: calibration file " << fileName << " *** no tree named 'tree' found");
      file->Close();
    } else {
      int isBarrel = 0;
      tree->SetBranchAddress("isBarrel", &isBarrel);
      int section = 0;
      tree->SetBranchAddress("isForward", &section);
      int sector = 0;
      tree->SetBranchAddress("sector", &sector);
      int layer = 0;
      tree->SetBranchAddress("layer", &layer);
      int plane = 0;
      tree->SetBranchAddress("plane", &plane);
      int strip = 0;
      tree->SetBranchAddress("strip", &strip);
      float efficiency = 1.;
      tree->SetBranchAddress("efficiency", &efficiency);
      float efficiencyError = 0.;
      tree->SetBranchAddress("efficiencyError", &efficiencyError);

      for (int i = 0; i < tree->GetEntries(); i++) {
        tree->GetEntry(i);
        if (isBarrel)
          stripEfficiency->setBarrelEfficiency(section, sector, layer, plane, strip, efficiency, efficiencyError);
        else
          stripEfficiency->setEndcapEfficiency(section, sector, layer, plane, strip, efficiency, efficiencyError);
      }
    }
    file->Close();
  }

  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  stripEfficiency.import(iov);

  B2INFO("KLMDatabaseImporter: strip efficiencies imported and calibration file " << fileName << " closed");
}

