/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vitaliy Popov, Dmytro Minchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <klm/eklm/calibration/EKLMTrackMatchAlgorithm.h>
#include <klm/dataobjects/KLMChannelIndex.h>

using namespace Belle2;

EKLMTrackMatchAlgorithm::EKLMTrackMatchAlgorithm() : CalibrationAlgorithm("EKLMTrackMatchCollector")
{
  m_planesEff = new TH1F(
    "plane_effiiency", "",
    EKLMElementNumbers::getMaximalPlaneGlobalNumber(),
    0.5, EKLMElementNumbers::getMaximalPlaneGlobalNumber() + 0.5);
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
  m_StripEfficiency = new KLMStripEfficiency();
  m_file = new TFile("TrackMAtchedResult.root", "recreate");
  m_file->cd();
}

EKLMTrackMatchAlgorithm::~EKLMTrackMatchAlgorithm()
{
}

CalibrationAlgorithm::EResult EKLMTrackMatchAlgorithm::calibrate()
{
  std::shared_ptr<TH1F> MatchedDigitsInPlane;
  MatchedDigitsInPlane = getObjectPtr<TH1F>("MatchedDigitsInPlane");
  std::shared_ptr<TH1F> AllExtHitsInPlane;
  AllExtHitsInPlane = getObjectPtr<TH1F>("AllExtHitsInPlane");

  MatchedDigitsInPlane.get()->Sumw2();
  AllExtHitsInPlane.get()->Sumw2();
  MatchedDigitsInPlane.get()->Write();
  AllExtHitsInPlane.get()->Write();

  m_planesEff->Divide(MatchedDigitsInPlane.get(), AllExtHitsInPlane.get(), 1, 1, "B");
  m_planesEff->Write();

  KLMChannelIndex klmChannels;
  for (KLMChannelIndex klmChannel = klmChannels.beginEKLM(); klmChannel != klmChannels.endEKLM(); ++klmChannel) {
    int idSection = klmChannel.getSection(); // Section
    int idSector = klmChannel.getSector(); // Sector
    int idLayer = klmChannel.getLayer(); // Layer
    int idPlane = klmChannel.getPlane(); // Plane
    int idStrip = klmChannel.getStrip(); // Strip
    int planeNum = m_ElementNumbers->planeNumber(idSection, idLayer, idSector, idPlane);

    // Bin number is equal to plane planeNumber
    float efficiency = m_planesEff->GetBinContent(planeNum);
    float efficiencyError = m_planesEff->GetBinError(planeNum);

    // Fill the efficiency for this strip.
    m_StripEfficiency->setEndcapEfficiency(idSection, idSector, idLayer, idPlane, idStrip, efficiency, efficiencyError);
  }

  // Where can I get IoV?
  saveCalibration(m_StripEfficiency, "KLMStripEfficiency");
  return CalibrationAlgorithm::c_OK;
}
