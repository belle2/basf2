/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/ClusterPreparer.h>
#include <tracking/modules/cdcHitBasedT0Extraction/cdcHitBasedT0Extraction.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParam.h>

#include <framework/logging/Logger.h>

#include <TFile.h>
#include <TH1D.h>
#include <TF1.h>
#include <TFitResult.h>
#include <TCanvas.h>
#include <TROOT.h>

#include <boost/lexical_cast.hpp>

#include <memory>
#include <sstream>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string CDCHitBasedT0Extraction::getDescription()
{
  return "Extracts the T0 time of an event only using CDC Hits";
}

void CDCHitBasedT0Extraction::initialize()
{
  m_eventT0.registerInDataStore();
  m_eventMetaData.isRequired();

  Super::initialize();
}

void CDCHitBasedT0Extraction::exposeParameters(ModuleParamList* moduleParamList,
                                               const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "searchWindow"),
                                m_param_searchWindow,
                                "Size of the time distance (in ns) to search for t0 in both direction from the current best t0",
                                m_param_searchWindow);

  moduleParamList->addParameter(prefixed(prefix, "fitWindow"),
                                m_param_fitWindow,
                                "Size of the time distance (in ns) to used for fitting",
                                m_param_fitWindow);

  moduleParamList->addParameter(prefixed(prefix, "refitWindow"),
                                m_param_refitWindow,
                                "Size of the time distance (in ns) to used for re-fitting",
                                m_param_refitWindow);

  moduleParamList->addParameter(prefixed(prefix, "binCountTimeHistogram"),
                                m_param_binCountTimeHistogram,
                                "Number of bins in the timing histogram used for fitting",
                                m_param_binCountTimeHistogram);

  moduleParamList->addParameter(prefixed(prefix, "rejectByBackgroundFlag"),
                                m_param_rejectByBackgroundFlag,
                                "Don't consider hits if they have the background flag set",
                                m_param_rejectByBackgroundFlag);

  moduleParamList->addParameter(prefixed(prefix, "rejectIfNotTakenFlag"),
                                m_param_rejectIfNotTakenFlag,
                                "Don't consider hits which have not been assigned during track finding. The CDC track finding has "
                                "to be run before for this flag to be useful.",
                                m_param_rejectIfNotTakenFlag);

  moduleParamList->addParameter(prefixed(prefix, "rejectIfChiSquareLargerThan"),
                                m_param_rejectIfChiSquareLargerThan,
                                "consider all t0 fit failed which have larger chi2 than this number",
                                m_param_rejectIfChiSquareLargerThan);

  moduleParamList->addParameter(prefixed(prefix, "storeAllFits"),
                                m_param_storeAllFits,
                                "store images for all fits",
                                m_param_storeAllFits);
}

void CDCHitBasedT0Extraction::apply(std::vector<CDCWireHit>& inputWireHits)
{
  // make sure the EventT0 object is available and created (even if we don't fill it later)
  if (!m_eventT0.isValid()) {
    m_eventT0.create();
  }

  const auto timeHistogramName = "CDCHitBasedT0Extraction_time_hist";
  const std::string debugImageName = "CDCHitBasedT0Extraction_debug_" + boost::lexical_cast<std::string>
                                     (m_eventMetaData->getEvent()) + ".png";

  auto timingHistgram = TH1D(timeHistogramName, timeHistogramName,
                             m_param_binCountTimeHistogram, -m_param_fitWindow,
                             m_param_fitWindow);

  gROOT->SetBatch();
  TCanvas canvas(debugImageName.c_str(), debugImageName.c_str(), 800, 600);

  if (inputWireHits.size() == 0) {
    B2WARNING("No input CDC hits available for the CDC hit based t0 extraction.");
    return;
  }

  for (auto const& wireHit : inputWireHits) {

    if (m_param_rejectByBackgroundFlag
        && wireHit.getAutomatonCell().hasBackgroundFlag())
      continue;

    if (m_param_rejectIfNotTakenFlag
        && !wireHit.getAutomatonCell().hasTakenFlag())
      continue;

    // the taken flag is also set for background hits. Therefore we must
    // also hits to be also classified as background to not get flooded
    // with background hits
    if (m_param_rejectIfNotTakenFlag
        && wireHit.getAutomatonCell().hasTakenFlag()
        && wireHit.getAutomatonCell().hasBackgroundFlag())
      continue;

    timingHistgram.Fill(wireHit.getDriftTime());
  }

  // add an overall offset of 1 to not have to care about empty bins in
  // the cumulated plot
  timingHistgram.SetBinContent(1, timingHistgram.GetBinContent(1) + 1);

  B2DEBUG(50,
          "Filled histogram with " << timingHistgram.GetEntries() << " Entries");

  std::unique_ptr<TH1> cumTimingHistogram(timingHistgram.GetCumulative());
  // detach histogram from any directory so we can delete it ourselves
  cumTimingHistogram->SetDirectory(0);

  // set the error for each bin taking into account that the bin content is the
  // cumulated content of previous bins
  double errPrevBin = 0.0f;
  double errThisBin = 0.0f;
  for (int i = 0; i < cumTimingHistogram->GetNbinsX(); i++) {
    if (i > 0) {
      const double prevEntries = cumTimingHistogram->GetBinContent(i - 1);
      const double addedEntries = cumTimingHistogram->GetBinContent(i) - prevEntries;
      if (addedEntries > 0.0) {
        // combine the error of the previous bin with the new entries of this bin
        const double errNewEntries = 1.0 / std::sqrt(addedEntries);
        errThisBin = errPrevBin + errNewEntries;
      }

    } else {
      if (cumTimingHistogram->GetBinContent(i) > 0) {
        errThisBin = 1.0 / std::sqrt(cumTimingHistogram->GetBinContent(i));
      }
    }

    cumTimingHistogram->SetBinError(i, errThisBin);
    errPrevBin = errThisBin;
  }

  if (m_param_storeAllFits) {
    cumTimingHistogram->Draw();
  }

  auto rangeBkg = std::make_pair(-m_param_fitWindow, -m_param_searchWindow);
  auto rangeSig = std::make_pair(m_param_searchWindow, m_param_fitWindow);
  // fit the background and signal side of the time distribution
  TF1 fitfuncBkg = TF1("CDCHitBasedT0Extraction_fit_bkg", "[0]*x + [1]",
                       rangeBkg.first, rangeBkg.second);
  TF1 fitfuncSig = TF1("CDCHitBasedT0Extraction_fit_sig", "[0]*x + [1]",
                       rangeSig.first, rangeSig.second);

  auto fitresBkg = cumTimingHistogram->Fit(&fitfuncBkg, "LQS", "",
                                           rangeBkg.first, rangeBkg.second);
  auto fitresSig = cumTimingHistogram->Fit(&fitfuncSig, "LQS", "",
                                           rangeSig.first, rangeSig.second);

  if (m_param_storeAllFits) {
    fitfuncBkg.Draw("SAME");
    fitfuncSig.Draw("SAME");
  }

  TF1 fitfuncSegmented = TF1("CDCHitBasedT0Extraction_fit_seg",
                             "[1]*(x+TMath::Abs(x+[3])) + [2]*(x-TMath::Abs(x+[3])) + [0]",
                             -m_param_fitWindow, m_param_fitWindow);

  if (fitresSig->IsValid() && fitresBkg->IsValid()) {
    double t0_estimate = (fitresBkg->Parameter(1) - fitresSig->Parameter(1))
                         / (fitresSig->Parameter(0) - fitresBkg->Parameter(0));

    // apply segmented fit
    std::array<double, 4> fit_params = { 0,  // = overall background offset
                                         fitresSig->Parameter(0),  // signal hits slope
                                         fitresBkg->Parameter(0), // background hits slope
                                         0.0 // breaking point shift
                                       };

    // use t0 estimate if it is something useful
    if (std::abs(t0_estimate) < m_param_searchWindow) {
      fit_params[3] = t0_estimate;
    }

    fitfuncSegmented.SetParameters(fit_params.data());

    auto fitresFull = cumTimingHistogram->Fit(&fitfuncSegmented, "QS", "", -m_param_fitWindow, m_param_fitWindow);
    if (m_param_storeAllFits) {
      fitfuncSegmented.Draw("SAME");
    }

    // refit with a fixed window around the extracted t0 to remove a possible bias
    // because we fitted a large part of the signal side in the first iteration
    const double fitted_t0_first = -fitresFull->Parameter(3);
    bool refitSuccess = false;
    if (std::abs(fitted_t0_first) < m_param_searchWindow) {
      fitresFull = cumTimingHistogram->Fit(&fitfuncSegmented, "QS", "",
                                           fitted_t0_first - m_param_refitWindow, fitted_t0_first + m_param_refitWindow);
      refitSuccess = true;
    } else {
      B2DEBUG(50, "First t0 estimate not in proper range" << fitted_t0_first);
    }

    if (m_param_storeAllFits) {
      fitfuncSegmented.Draw("SAME");
    }

    // actually store this result ?
    if (refitSuccess && fitresFull->IsValid()) {
      const double fitted_t0 = -fitresFull->Parameter(3);
      const double fitted_t0_error = fitresFull->Error(3);

      if (fitresFull->Chi2() > m_param_rejectIfChiSquareLargerThan) {
        B2DEBUG(50,
                "T0 fit has too large Chi2 " << fitresFull->Chi2());
      } else {

        m_eventT0->addTemporaryEventT0(fitted_t0, fitted_t0_error, Const::CDC);
        // TODO: until now, we have no combination of different t0s in place, so we just set the final one here.
        m_eventT0->setEventT0(fitted_t0, fitted_t0_error, Const::CDC);
        B2DEBUG(50,
                "Successful t0 extraction with CDC hits: " << fitted_t0 << " +- " << fitted_t0_error);
      }
    } else {
      B2DEBUG(50,
              "Cannot fit t0 from CDC hits only. Won't set EventT0 for now.");
    }
  } else {
    B2DEBUG(50,
            "Cannot extract background or signal segment because fit failed. Won't set EventT0 for now.");
  }

  if (m_param_storeAllFits) {
    canvas.Draw();
    canvas.SaveAs(debugImageName.c_str());
  }

  gROOT->SetBatch(false);
}
