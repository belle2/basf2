/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/



#include <TH1.h>
#include <eklm/calibration/EKLMTrackMatchAlgorithm.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

using namespace Belle2;

EKLMTrackMatchAlgorithm::EKLMTrackMatchAlgorithm() : CalibrationAlgorithm("EKLMTrackMatchCollector")
{
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

  return CalibrationAlgorithm::c_OK;
}
