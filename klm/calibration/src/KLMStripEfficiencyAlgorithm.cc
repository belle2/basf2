/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vitaliy Popov, Dmytro Minchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/calibration/KLMStripEfficiencyAlgorithm.h>

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>

/* ROOT headers. */
#include <TFile.h>
#include <TH1F.h>

using namespace Belle2;

KLMStripEfficiencyAlgorithm::KLMStripEfficiencyAlgorithm() : CalibrationAlgorithm("KLMStripEfficiencyCollector")
{
  m_ElementNumbers = &(KLMElementNumbers::Instance());
  m_PlaneArrayIndex = &(KLMPlaneArrayIndex::Instance());
  m_StripEfficiency = new KLMStripEfficiency();
}

KLMStripEfficiencyAlgorithm::~KLMStripEfficiencyAlgorithm()
{
  if (m_StripEfficiency != nullptr)
    delete m_StripEfficiency;
}

CalibrationAlgorithm::EResult KLMStripEfficiencyAlgorithm::calibrate()
{
  TFile* outputFile = new TFile(m_OutputFileName.c_str(), "recreate");
  int nPlanes = m_PlaneArrayIndex->getNPlanes();
  TH1F* efficiencyHistogram =
    new TH1F("plane_efficiency", "KLM plane efficiency",
             nPlanes, -0.5, double(nPlanes) - 0.5);
  std::shared_ptr<TH1F> matchedDigitsInPlane;
  matchedDigitsInPlane = getObjectPtr<TH1F>("matchedDigitsInPlane");
  std::shared_ptr<TH1F> allExtHitsInPlane;
  allExtHitsInPlane = getObjectPtr<TH1F>("allExtHitsInPlane");
  matchedDigitsInPlane.get()->Sumw2();
  allExtHitsInPlane.get()->Sumw2();
  matchedDigitsInPlane.get()->Write();
  allExtHitsInPlane.get()->Write();
  efficiencyHistogram->Divide(matchedDigitsInPlane.get(), allExtHitsInPlane.get(), 1, 1, "B");
  efficiencyHistogram->Write();
  KLMChannelIndex klmChannels;
  for (KLMChannelIndex& klmChannel : klmChannels) {
    int subdetector = klmChannel.getSubdetector();
    int section = klmChannel.getSection();
    int sector = klmChannel.getSector();
    int layer = klmChannel.getLayer();
    int plane = klmChannel.getPlane();
    int strip = klmChannel.getStrip();
    uint16_t planeKLM = 0;
    if (subdetector == KLMElementNumbers::c_BKLM) {
      planeKLM = m_ElementNumbers->planeNumberBKLM(
                   section, sector, layer, plane);
    } else {
      planeKLM = m_ElementNumbers->planeNumberEKLM(
                   section, sector, layer, plane);
    }
    uint16_t planeIndex = m_PlaneArrayIndex->getIndex(planeKLM);
    float efficiency = efficiencyHistogram->GetBinContent(planeIndex + 1);
    float efficiencyError = efficiencyHistogram->GetBinError(planeIndex + 1);
    /* Fill the efficiency for this strip. */
    if (subdetector == KLMElementNumbers::c_BKLM) {
      m_StripEfficiency->setBarrelEfficiency(
        section, sector, layer, plane, strip, efficiency, efficiencyError);
    } else {
      m_StripEfficiency->setEndcapEfficiency(
        section, sector, layer, plane, strip, efficiency, efficiencyError);
    }
  }
  saveCalibration(m_StripEfficiency, "KLMStripEfficiency");
  outputFile->cd();
  efficiencyHistogram->Write();
  delete efficiencyHistogram;
  delete outputFile;
  return CalibrationAlgorithm::c_OK;
}
