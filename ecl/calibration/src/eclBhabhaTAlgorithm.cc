
#include <ecl/calibration/eclBhabhaTAlgorithm.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

#include "TH2F.h"
#include "TFile.h"
#include "TMath.h"
#include "TF1.h"
#include "TMinuit.h"
#include "TROOT.h"

using namespace std;
using namespace Belle2;
using namespace ECL;

eclBhabhaTAlgorithm::eclBhabhaTAlgorithm():
  // Parameters
  CalibrationAlgorithm("ECLBhabhaTCollector"),
  cellIDLo(0),
  cellIDHi(8735),
  maxIterations(15),
  debugOutput(true),
  debugFilename("eclBhabhaTAlgorithm.root"),
  // Private members
  m_run_count(0)
{
  setDescription(
    "Calculate time offsets from bhabha events by fitting gaussian function to the (t - T0) difference."
  );
}

CalibrationAlgorithm::EResult eclBhabhaTAlgorithm::calibrate()
{
  /** Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();

  /* Histogram with the data collected by eclBhabhaTCollectorModule */
  auto TimevsCrys = getObjectPtr<TH2F>("TimevsCrys");
  if (!TimevsCrys) return c_Failure;

  /**-----------------------------------------------------------------------------------------------*/

  TFile* histfile = 0;
  TTree* tree = 0;
  int tree_cid;
  double chi2;

  // Vector of time offsets to be saved in the database.
  std::vector<float> t_offsets;
  // Vector of time offset uncertainties to be saved in the database.
  std::vector<float> t_offsets_unc;

  // Function parameters
  double norm, peak, sigma;
  double peak_unc;

  // Number of current iteration.
  int iter;

  if (debugOutput) {
    histfile = new TFile("eclBhabhaTAlgorithm.root", "recreate");
    tree     = new TTree("tree", "Debug data from bhabha time calibration algorithm");

    TimevsCrys->Write();

    tree->Branch("cid", &tree_cid)->SetTitle("Cell ID, 1..8736");
    tree->Branch("coef", &peak)->SetTitle("Time offset, ns");
    tree->Branch("err", &peak_unc)->SetTitle("Error of time offset, ns.");
    tree->Branch("sigma", &sigma)->SetTitle("Sigma");
    tree->Branch("chi2", &chi2)->SetTitle("Chi2");
    tree->SetAutoSave(10);
  }

  double hist_xmin = TimevsCrys->GetXaxis()->GetXmin();
  double hist_xmax = TimevsCrys->GetXaxis()->GetXmax();

  // TODO: It's better to use ECL integer time for this algorithm.
  //       This way, there will be no problems with binning.
  // Conversion coefficient from ADC ticks to nanoseconds
  const double TICKS_TO_NS = 0.4931; // ns/clock

  for (int crys_id = cellIDLo; crys_id <= cellIDHi; crys_id++) {
    TH1D* h_time = TimevsCrys->ProjectionY("h_time", crys_id + 1, crys_id + 1);

    TF1* gaus = new TF1("func", "gaus(0)", hist_xmin, hist_xmax);
    gaus->SetParNames("normalization", "peak", "sigma");
    gaus->ReleaseParameter(0);
    gaus->ReleaseParameter(1);
    gaus->ReleaseParameter(2);

    double peak_min, peak_max;
    double sig_min(0.5), sig_max(15);

    double hist_max = h_time->GetMaximum();

    //=== Estimate initial value of sigma as std dev.
    double stddev = h_time->GetStdDev();
    sigma = stddev;

    //=== Estimate initial peak position as median of maximum bin positions.
    std::vector<double> maxima;
    for (int i = 0; i < h_time->GetNbinsX(); i++) {
      if (h_time->GetBinContent(i) == hist_max) {
        maxima.push_back(h_time->GetBinCenter(i));
      }
    }
    peak = maxima[maxima.size() / 2];

    //=== Setting parameters for initial iteration
    gaus->FixParameter(0, hist_max);
    gaus->FixParameter(1, peak);
    gaus->SetParameter(2, sigma);
    gaus->SetParLimits(2, sig_min, sig_max);
    // L -- Use log likelihood method
    // I -- Use integral of function in bin instead of value at bin center
    // R -- Use the range specified in the function range
    // B -- Fix one or more parameters with predefined function
    // Q -- Quiet mode
    h_time->Fit(gaus, "LIRBQ", "", peak - stddev, peak + stddev);

    //=== Perform several additional iterations
    double left, right;

    gaus->ReleaseParameter(0);
    double norm_min(0.85 * hist_max), norm_max(1.35 * hist_max);
    gaus->SetParLimits(0, norm_min, norm_max);
    gaus->ReleaseParameter(1);

    peak  = gaus->GetParameter(1);
    sigma = gaus->GetParameter(2);

    double peak_prev[2];
    peak_prev[0] = peak_prev[1] = 0;

    for (iter = 0; iter < maxIterations; iter++) {
      if (abs(peak - peak_prev[1]) < 1e-3) {
        // "Stuck between two iterations, setting peak to average between the two\n"
        peak = 0.5 * (peak_prev[1] + peak_prev[0]);
        gaus->SetParameter(1, peak);
      }

      // Setting ranges to interval where f(x) >= norm * 0.2
      left  = std::max(peak - sigma * 1.27, -80.);
      right = std::min(peak + sigma * 1.27,  80.);

      // Peak should stay within central 40% of fit area.
      peak_min = left  + 0.3 * (right - left);
      peak_max = right - 0.3 * (right - left);
      gaus->SetParLimits(1, peak_min, peak_max);

      h_time->Fit(gaus, "LIRBQ", "", left, right);

      norm  = gaus->GetParameter(0);
      peak  = gaus->GetParameter(1);
      sigma = gaus->GetParameter(2);
      // Do not stop if parameter is at limit.
      if (abs(peak - peak_min) < 1e-3 || abs(peak - peak_max) < 1e-3) continue;
      // Do not stop if parameter is at limit.
      if (abs(norm - norm_min) < 0.1 || abs(norm - norm_max) < 0.1) continue;
      // Do not exit if parameter is at limit.
      if (abs(sigma - sig_min) < 1e-3 || abs(sigma - sig_max) < 1e-3) continue;
      // Do not stop if sigma is much larger than interval of the fit.
      if (sigma > 2 * (right - left)) continue;
      // TODO: Move from hardcoded value to parameter
      if (gaus->GetParError(1) < 0.5) break;

      peak_prev[1] = peak_prev[0];
      peak_prev[0] = peak;
    }

    peak     = gaus->GetParameter(1);
    peak_unc = gaus->GetParError(1);
    sigma    = gaus->GetParameter(2);
    chi2     = gaus->GetChisquare();

    // For the database, convert back from ns to ADC ticks.
    t_offsets.push_back(peak / TICKS_TO_NS);
    t_offsets_unc.push_back(peak_unc / TICKS_TO_NS);

    histfile->WriteTObject(h_time, (std::string("h_time_") + std::to_string(crys_id + 1)).c_str());

    tree_cid  = crys_id + 1;
    tree->Fill();
  }

  // Set time offset for crystal that weren't fitted to zero.
  // TODO: Instead, use data from previous calibration.
  for (int i = 0; i < cellIDLo; i++) {
    t_offsets.insert(t_offsets.begin(), 0);
    t_offsets_unc.insert(t_offsets_unc.begin(), -1);
  }
  for (int i = cellIDHi + 1; i < 8736; i++) {
    t_offsets.push_back(0);
    t_offsets_unc.push_back(-1);
  }

  ECLCrystalCalib* BhabhaTCalib = new ECLCrystalCalib();
  BhabhaTCalib->setCalibVector(t_offsets, t_offsets_unc);
  saveCalibration(BhabhaTCalib, "ECLCrystalTimeOffset");

  tree->Write();

  // TODO: Return failure at least in some conditions.
  //return c_Failure;
  return c_OK;
}

