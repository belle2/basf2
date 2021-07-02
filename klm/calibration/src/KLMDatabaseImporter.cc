/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Giacomo De Pietro                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/calibration/KLMDatabaseImporter.h>

/* Belle 2 headers. */
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/logging/Logger.h>

/* ROOT headers. */
#include <TFile.h>
#include <TTree.h>

/* C++ headers. */
#include <string>

using namespace Belle2;

KLMDatabaseImporter::KLMDatabaseImporter()
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

void KLMDatabaseImporter::importReconstructionParameters(
  const KLMReconstructionParameters* digitizationParameters)
{
  DBImportObjPtr<KLMReconstructionParameters> digPar;
  digPar.construct(*digitizationParameters);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  digPar.import(iov);
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

void KLMDatabaseImporter::importTimeWindow(KLMTimeWindow* timeWindow)
{
  DBImportObjPtr<KLMTimeWindow> timeWindowImport;
  timeWindowImport.construct(*timeWindow);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  timeWindowImport.import(iov);
}

void KLMDatabaseImporter::loadStripEfficiency(
  KLMStripEfficiency* stripEfficiency, std::string fileName)
{
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
}

void KLMDatabaseImporter::importStripEfficiency(
  const KLMStripEfficiency* stripEfficiency)
{
  DBImportObjPtr<KLMStripEfficiency> stripEfficiencyImport;
  stripEfficiencyImport.construct(*stripEfficiency);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  stripEfficiencyImport.import(iov);
}

void KLMDatabaseImporter::importBKLMAlignment(
  const BKLMAlignment* bklmAlignment, bool displacement)
{
  std::string payloadName;
  if (displacement)
    payloadName = "BKLMDisplacement";
  else
    payloadName = "BKLMAlignment";
  DBImportObjPtr<BKLMAlignment> bklmAlignmentImport(payloadName);
  bklmAlignmentImport.construct(*bklmAlignment);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  bklmAlignmentImport.import(iov);
}

void KLMDatabaseImporter::importEKLMAlignment(
  const EKLMAlignment* eklmAlignment, bool displacement)
{
  std::string payloadName;
  if (displacement)
    payloadName = "EKLMDisplacement";
  else
    payloadName = "EKLMAlignment";
  DBImportObjPtr<EKLMAlignment> eklmAlignmentImport(payloadName);
  eklmAlignmentImport.construct(*eklmAlignment);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  eklmAlignmentImport.import(iov);
}

void KLMDatabaseImporter::importEKLMSegmentAlignment(
  const EKLMSegmentAlignment* eklmSegmentAlignment, bool displacement)
{
  std::string payloadName;
  if (displacement)
    payloadName = "EKLMSegmentDisplacement";
  else
    payloadName = "EKLMSegmentAlignment";
  DBImportObjPtr<EKLMSegmentAlignment> eklmSegmentAlignmentImport(payloadName);
  eklmSegmentAlignmentImport.construct(*eklmSegmentAlignment);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  eklmSegmentAlignmentImport.import(iov);
}

void KLMDatabaseImporter::importAlignment(
  const BKLMAlignment* bklmAlignment, const EKLMAlignment* eklmAlignment,
  const EKLMSegmentAlignment* eklmSegmentAlignment, bool displacement)
{
  importBKLMAlignment(bklmAlignment, displacement);
  importEKLMAlignment(eklmAlignment, displacement);
  importEKLMSegmentAlignment(eklmSegmentAlignment, displacement);
}
