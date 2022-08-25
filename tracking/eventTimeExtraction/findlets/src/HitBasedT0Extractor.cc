/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/eventTimeExtraction/findlets/HitBasedT0Extractor.h>
#include <tracking/eventTimeExtraction/findlets/BaseEventTimeExtractor.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/ScopeGuard.h>

#include <TH1D.h>
#include <TF1.h>
#include <TFitResult.h>
#include <TCanvas.h>

#include <boost/lexical_cast.hpp>

#include <memory>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string HitBasedT0Extractor::getDescription()
{
  return "Extracts the T0 time of an event only using CDC Hits";
}

void HitBasedT0Extractor::initialize()
{
  m_eventMetaData.isRequired();
  Super::initialize();
}

void HitBasedT0Extractor::exposeParameters(ModuleParamList* moduleParamList,
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
                                "consider the t0 fit failed which have larger chi2 than this number",
                                m_param_rejectIfChiSquareLargerThan);

  moduleParamList->addParameter(prefixed(prefix, "rejectIfUncertaintyLargerThan"),
                                m_param_rejectIfUncertaintyLargerThan,
                                "consider the t0 fit if the uncertainty on t0 is larger than this value",
                                m_param_rejectIfUncertaintyLargerThan);

  moduleParamList->addParameter(prefixed(prefix, "storeAllFits"),
                                m_param_storeAllFits,
                                "store images for all fits",
                                m_param_storeAllFits);

  moduleParamList->addParameter(prefixed(prefix, "minHitCount"),
                                m_param_minHitCount,
                                "Minimum amount of hits which is required to try the extraction",
                                m_param_minHitCount);

  Super::exposeParameters(moduleParamList, prefix);
}

void HitBasedT0Extractor::apply(std::vector<CDCWireHit>& inputWireHits)
{
  const auto timeHistogramName = "HitBasedT0Extractor_time_hist";
  const std::string debugImageName = "HitBasedT0Extractor_debug_" + boost::lexical_cast<std::string>
                                     (m_eventMetaData->getEvent()) + ".png";

  auto timingHistgram = TH1D(timeHistogramName, timeHistogramName,
                             m_param_binCountTimeHistogram, -m_param_fitWindow,
                             m_param_fitWindow);

  // Enable batch mode - we do not want to show the canvas etc.
  auto batchGuard = ScopeGuard::guardBatchMode();

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

    // Attention: at this stage we use the drift time that was set when the WireHits were created.
    // We will just *assume* that it was zero at this time!
    timingHistgram.Fill(wireHit.getDriftTime());
  }

  if (timingHistgram.GetEntries() < m_param_minHitCount) {
    B2DEBUG(25, "Only " << timingHistgram.GetEntries() << " hits satisfied the requirements for t0 extraction, " << m_param_minHitCount
            << " are required.");
    return;
  }

  // add an overall offset of 1 to not have to care about empty bins in
  // the cumulated plot
  timingHistgram.SetBinContent(1, timingHistgram.GetBinContent(1) + 1);

  B2DEBUG(25,
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
  TF1 fitfuncBkg = TF1("HitBasedT0Extractor_fit_bkg", "[0]*x + [1]",
                       rangeBkg.first, rangeBkg.second);
  TF1 fitfuncSig = TF1("HitBasedT0Extractor_fit_sig", "[0]*x + [1]",
                       rangeSig.first, rangeSig.second);

  auto fitresBkg = cumTimingHistogram->Fit(&fitfuncBkg, "LQS", "",
                                           rangeBkg.first, rangeBkg.second);
  auto fitresSig = cumTimingHistogram->Fit(&fitfuncSig, "LQS", "",
                                           rangeSig.first, rangeSig.second);

  if (m_param_storeAllFits) {
    fitfuncBkg.Draw("SAME");
    fitfuncSig.Draw("SAME");
  }

  TF1 fitfuncSegmented = TF1("HitBasedT0Extractor_fit_seg",
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
      B2DEBUG(25, "First t0 estimate not in proper range" << fitted_t0_first);
    }

    if (m_param_storeAllFits) {
      fitfuncSegmented.Draw("SAME");
    }

    // actually store this result ?
    if (refitSuccess && fitresFull->IsValid()) {
      const double fitted_t0 = -fitresFull->Parameter(3);
      const double fitted_t0_error = fitresFull->Error(3);

      const double norm_chi2 = fitresFull->Chi2() / double(fitresFull->Ndf());

      B2DEBUG(25, "T0 fit with t0 " << fitted_t0 << " +- " << fitted_t0_error << " and normalized chi2 " << norm_chi2 << " and " <<
              timingHistgram.GetEntries() << " hits");

      // check if all the criteria required for a "good fit" have been met
      if (norm_chi2 > m_param_rejectIfChiSquareLargerThan) {
        B2DEBUG(25,
                "T0 fit has too large Chi2 " << fitresFull->Chi2());
      } else if (std::abs(fitted_t0_error) > m_param_rejectIfUncertaintyLargerThan) {
        B2DEBUG(25,
                "T0 fit has too large error " << fitted_t0_error);
      } else {

        // Since drift times are corrected for EventT0 (in RealisticTDCCountTranslator), if any other T0 modules were executed before, add the EventT0 offset back.
        // This leads to "absolute" event T0 determination which should be consistent with other T0 modules.
        double lastEventT0 = m_eventT0->hasEventT0() ? m_eventT0->getEventT0() : 0;
        EventT0::EventT0Component eventT0Component(fitted_t0 + lastEventT0, fitted_t0_error, Const::CDC, "hit based", norm_chi2);
        m_eventT0->addTemporaryEventT0(eventT0Component);
        m_eventT0->setEventT0(eventT0Component);
        m_wasSuccessful = true;
        B2DEBUG(25,
                "Successful t0 extraction with CDC hits: " << fitted_t0 << " +- " << fitted_t0_error);
      }
    } else {
      B2DEBUG(25,
              "Cannot fit t0 from CDC hits only. Won't set EventT0 for now.");
    }
  } else {
    B2DEBUG(25,
            "Cannot extract background or signal segment because fit failed. Won't set EventT0 for now.");
  }

  if (m_param_storeAllFits) {
    canvas.Draw();
    canvas.SaveAs(debugImageName.c_str());
  }
}
