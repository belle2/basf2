/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/calibration/KLMStripEfficiencyAlgorithm.h>

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>

/* ROOT headers. */
#include <TFile.h>
#include <TH1F.h>

using namespace Belle2;

KLMStripEfficiencyAlgorithm::Results::Results()
{
}

KLMStripEfficiencyAlgorithm::Results::Results(const Results& results)
{
  m_AchievedPrecision = results.m_AchievedPrecision;
  m_MatchedDigits = results.m_MatchedDigits;
  m_ExtHits = results.m_ExtHits;
  int nPlanes = KLMPlaneArrayIndex::Instance().getNElements();
  m_Efficiency = new float[nPlanes];
  m_ExtHitsPlane = new int[nPlanes];
  std::memcpy(m_Efficiency, results.m_Efficiency, nPlanes * sizeof(float));
  std::memcpy(m_ExtHitsPlane, results.m_ExtHitsPlane, nPlanes * sizeof(int));
}

KLMStripEfficiencyAlgorithm::Results::~Results()
{
  if (m_Efficiency != nullptr)
    delete m_Efficiency;
  if (m_ExtHitsPlane != nullptr)
    delete m_ExtHitsPlane;
}

KLMStripEfficiencyAlgorithm::KLMStripEfficiencyAlgorithm() :
  CalibrationAlgorithm("KLMStripEfficiencyCollector"),
  m_ElementNumbers(&(KLMElementNumbers::Instance())),
  m_PlaneArrayIndex(&(KLMPlaneArrayIndex::Instance())),
  m_StripEfficiency(nullptr)
{
  int nPlanes = m_PlaneArrayIndex->getNElements();
  m_Results.m_Efficiency = new float[nPlanes];
  m_Results.m_ExtHitsPlane = new int[nPlanes];
}

KLMStripEfficiencyAlgorithm::~KLMStripEfficiencyAlgorithm()
{
  if (m_StripEfficiency != nullptr)
    delete m_StripEfficiency;
}

CalibrationAlgorithm::EResult KLMStripEfficiencyAlgorithm::calibrate()
{
  /* Get collected results. */
  int nPlanes = m_PlaneArrayIndex->getNElements();
  TH1F* efficiencyHistogram =
    new TH1F("plane_efficiency", "KLM plane efficiency",
             nPlanes, -0.5, double(nPlanes) - 0.5);
  std::shared_ptr<TH1F> matchedDigitsInPlane;
  matchedDigitsInPlane = getObjectPtr<TH1F>("matchedDigitsInPlane");
  m_Results.m_MatchedDigits = matchedDigitsInPlane->Integral();
  std::shared_ptr<TH1F> allExtHitsInPlane;
  allExtHitsInPlane = getObjectPtr<TH1F>("allExtHitsInPlane");
  m_Results.m_ExtHits = allExtHitsInPlane->Integral();
  matchedDigitsInPlane.get()->Sumw2();
  allExtHitsInPlane.get()->Sumw2();
  efficiencyHistogram->Divide(matchedDigitsInPlane.get(),
                              allExtHitsInPlane.get(), 1, 1, "B");
  for (int i = 0; i < nPlanes; ++i) {
    m_Results.m_Efficiency[i] = efficiencyHistogram->GetBinContent(i + 1);
    m_Results.m_ExtHitsPlane[i] = allExtHitsInPlane->GetBinContent(i + 1);
  }
  /* Check whether the amount of data is sufficient. */
  bool notEnoughData = false;
  m_Results.m_AchievedPrecision = 0;
  KLMChannelIndex klmPlanes(KLMChannelIndex::c_IndexLevelPlane);
  for (KLMChannelIndex& klmPlane : klmPlanes) {
    KLMPlaneNumber plane = klmPlane.getKLMPlaneNumber();
    uint16_t planeIndex = m_PlaneArrayIndex->getIndex(plane);
    int extHits = allExtHitsInPlane->GetBinContent(planeIndex + 1);
    float efficiencyError = efficiencyHistogram->GetBinError(planeIndex + 1);
    if (efficiencyError > m_Results.m_AchievedPrecision)
      m_Results.m_AchievedPrecision = efficiencyError;
    /*
     * No hits is not considered as "not enough data", because this can
     * happen in case KLM is excluded.
     */
    switch (m_CalibrationStage) {
      case c_MeasurablePlaneCheck:
        if (extHits != 0 && extHits < m_MinimalExtHits)
          notEnoughData = true;
        break;
      case c_EfficiencyMeasurement:
        if (efficiencyError > m_RequestedPrecision)
          notEnoughData = true;
        break;
    }
  }
  /*
   * Fill the payload. A new object is created, because saveCalibration()
   * stores a pointer to KLMStripEfficiency, and it is necessary to save
   * the payloads to commit them at the end of calibration.
   */
  if ((m_CalibrationStage == c_EfficiencyMeasurement) &&
      (!notEnoughData || m_ForcedCalibration)) {
    m_StripEfficiency = new KLMStripEfficiency();
    KLMChannelIndex klmChannels;
    for (KLMChannelIndex& klmChannel : klmChannels) {
      int subdetector = klmChannel.getSubdetector();
      int section = klmChannel.getSection();
      int sector = klmChannel.getSector();
      int layer = klmChannel.getLayer();
      int plane = klmChannel.getPlane();
      int strip = klmChannel.getStrip();
      KLMPlaneNumber planeKLM = 0;
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
  }
  /* Write histograms to output file. */
  TFile* outputFile = new TFile(m_OutputFileName.c_str(), "recreate");
  outputFile->cd();
  matchedDigitsInPlane.get()->Write();
  allExtHitsInPlane.get()->Write();
  efficiencyHistogram->Write();
  delete efficiencyHistogram;
  delete outputFile;
  /* Set output status. */
  if (notEnoughData && !m_ForcedCalibration)
    return CalibrationAlgorithm::c_NotEnoughData;
  return CalibrationAlgorithm::c_OK;
}

int KLMStripEfficiencyAlgorithm::Results::newMeasuredPlanes(
  float* efficiency) const
{
  const int nPlanes = KLMPlaneArrayIndex::Instance().getNElements();
  int newPlanes = 0;
  for (int i = 0; i < nPlanes; ++i) {
    if (m_Efficiency[i] > 0 && efficiency[i] == 0)
      newPlanes++;
  }
  return newPlanes;
}

int KLMStripEfficiencyAlgorithm::Results::newExtHitsPlanes(
  int* extHitsPlane) const
{
  const int nPlanes = KLMPlaneArrayIndex::Instance().getNElements();
  int newPlanes = 0;
  for (int i = 0; i < nPlanes; ++i) {
    if (m_ExtHitsPlane[i] > 0 && extHitsPlane[i] == 0)
      newPlanes++;
  }
  return newPlanes;
}
