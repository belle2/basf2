/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/calibration/TOPModuleT0DeltaTAlgorithm.h>
#include <top/dbobjects/TOPCalModuleT0.h>
#include <math.h>
#include <TROOT.h>
#include <TFile.h>
#include <TF1.h>
#include <TH2F.h>
#include <TMatrixDSym.h>
#include <TDecompChol.h>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

namespace Belle2 {
  namespace TOP {

    TOPModuleT0DeltaTAlgorithm::TOPModuleT0DeltaTAlgorithm():
      CalibrationAlgorithm("TOPModuleT0DeltaTCollector")
    {
      setDescription("Calibration algorithm for method DeltaT");
    }

    CalibrationAlgorithm::EResult TOPModuleT0DeltaTAlgorithm::calibrate()
    {
      gROOT->SetBatch();

      // get basic histogram

      auto slotPairs = getObjectPtr<TH2F>("slots");
      if (not slotPairs) {
        B2ERROR("TOPModuleT0DeltaTAlgorithm: histogram 'slots' not found");
        return c_Failure;
      }

      // construct file name and open output root file

      const auto& expRun = getRunList();
      string expNo = to_string(expRun[0].first);
      while (expNo.length() < 4) expNo.insert(0, "0");
      string runNo = to_string(expRun[0].second);
      while (runNo.length() < 5) runNo.insert(0, "0");
      string outputFileName = "moduleT0_rough-e" + expNo + "-r" + runNo + ".root";
      auto* file = TFile::Open(outputFileName.c_str(), "recreate");

      slotPairs->Write();

      // create vectors and a histogram to store the results of fits

      std::vector<double> deltaT0;
      std::vector<double> sigma;
      std::vector<std::vector<double> > A;
      auto* chi2Fits = new TH2F("chi2_of_fits", "normalized chi2 of succesfull fits",
                                16, 0.5, 16.5, 16, 0.5, 16.5);
      chi2Fits->SetXTitle("first slot number");
      chi2Fits->SetYTitle("second slot number");

      // fit histograms of time differences and store the results

      for (int i = 0; i < slotPairs->GetNbinsX(); i++) {
        for (int k = 0; k < slotPairs->GetNbinsY(); k++) {
          if (slotPairs->GetBinContent(i + 1, k + 1) < m_minEntries) continue;
          int slot1 = slotPairs->GetXaxis()->GetBinCenter(i + 1);
          int slot2 = slotPairs->GetYaxis()->GetBinCenter(k + 1);
          string name = "deltaT0_" + to_string(slot1) + "-" + to_string(slot2);
          auto h = getObjectPtr<TH1F>(name);
          if (not h) continue;
          int status = fitHistogram(h);
          if (status != 0) continue;
          h->Write();
          deltaT0.push_back(m_delT0);
          sigma.push_back(m_error);
          std::vector<double> a(16, 0);
          a[slot1 - 1] = 1;
          a[slot2 - 1] = -1;
          A.push_back(a);
          chi2Fits->SetBinContent(i + 1, k + 1, m_chi2 / m_ndf);
        }
      }

      // append the bound (sum of all calibration constants equals to 0)

      A.push_back(std::vector<double>(16, 1));
      deltaT0.push_back(0);
      sigma.push_back(0.001); // arbitrary but small compared to calibration precision

      // check degrees of freedom and return if not positive

      int m = deltaT0.size();
      int ndf = m - 16;
      if (ndf <= 0) {
        file->Write();
        file->Close();
        B2INFO("TOPModuleT0DeltaTAlgorithm: NDF < 0");
        return c_NotEnoughData;
      }

      // construct the matrix of a linear system of equations

      TMatrixDSym B(16);
      for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
          for (int k = 0; k < m; k++) {
            B[i][j] += A[k][i] * A[k][j] / (sigma[k] * sigma[k]);
          }
        }
      }

      // invert the matrix and return if not positive definite

      TDecompChol C(B); // Choletski decomposition to check also for positive definitness
      bool ok = C.Invert(B);
      if (not ok) {
        file->Write();
        file->Close();
        B2INFO("TOPModuleT0DeltaTAlgorithm: matrix is not positive definite");
        return c_NotEnoughData;
      }

      // construct the right side of a linear system of equations

      std::vector<double> b(16, 0);
      for (int i = 0; i < 16; i++) {
        for (int k = 0; k < m; k++) {
          b[i] += A[k][i] * deltaT0[k] / (sigma[k] * sigma[k]);
        }
      }

      // solve for unknown module T0's

      std::vector<double> x(16, 0); // module T0's
      std::vector<double> e(16, 0); // uncertainties on module T0
      for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
          x[i] += B[i][j] * b[j];
        }
        e[i] = sqrt(B[i][i]);
      }

      // calculate chi^2

      double chi2 = 0;
      for (int k = 0; k < m; k++) {
        double s = 0;
        for (int i = 0; i < 16; i++) {
          s += A[k][i] * x[i];
        }
        chi2 += pow((s - deltaT0[k]) / sigma[k], 2);
      }

      // store the results in a histogram

      stringstream ss;
      ss.precision(3);
      ss << "Module T0, chi^2/NDF = " << chi2 << "/" << ndf;
      string title = ss.str();

      auto* h_moduleT0 = new TH1F("moduleT0", title.c_str(), 16, 0.5, 16.5);
      h_moduleT0->SetXTitle("slot number");
      h_moduleT0->SetYTitle("module T0 [ns]");
      for (int i = 0; i < 16; i++) {
        h_moduleT0->SetBinContent(i + 1, x[i]);
        h_moduleT0->SetBinError(i + 1, e[i]);
      }

      // write the results and close the file

      file->Write();
      file->Close();

      // check the results and return if not good enough

      for (auto err : e) {
        if (err > m_minError) return c_NotEnoughData;
      }

      // otherwise create and import payload to DB

      auto* moduleT0 = new TOPCalModuleT0();
      for (int i = 0; i < 16; i++) {
        moduleT0->setT0(i + 1, x[i], e[i]);
      }
      moduleT0->suppressAverage();
      saveCalibration(moduleT0);

      return c_OK;
    }


    int TOPModuleT0DeltaTAlgorithm::fitHistogram(std::shared_ptr<TH1F> h)
    {
      int numEntries = h->GetSumOfWeights();
      int status = 0;
      if (numEntries > m_cutoffEntries and m_tailFractInit > 0) {
        status = fitDoubleGaus(h);
        if (status != 0) status = fitSingleGaus(h);
      } else {
        status = fitSingleGaus(h);
      }
      return status;
    }


    int TOPModuleT0DeltaTAlgorithm::fitSingleGaus(std::shared_ptr<TH1F> h)
    {
      double sum = h->GetSumOfWeights();
      if (sum < 5) return 5;
      double maxPosition = h->GetBinCenter(h->GetMaximumBin());
      double binWidth = h->GetBinWidth(1);
      double xmin = h->GetXaxis()->GetXmin();
      double xmax = h->GetXaxis()->GetXmax();

      auto* func = new TF1("func1g",
                           "[0] + [1]/sqrt(2*pi)/[3]*exp(-0.5*((x-[2])/[3])**2)",
                           xmin, xmax);
      func->SetParameter(0, 0);
      func->SetParameter(1, sum * binWidth);
      func->SetParameter(2, maxPosition);
      func->SetParameter(3, m_sigmaCoreInit);

      int status = h->Fit(func, "LERSQ");

      m_delT0 = func->GetParameter(2);
      m_error = func->GetParError(2);
      m_chi2 = func->GetChisquare();
      m_ndf = func->GetNDF();

      return status;
    }


    int TOPModuleT0DeltaTAlgorithm::fitDoubleGaus(std::shared_ptr<TH1F> h)
    {
      double sum = h->GetSumOfWeights();
      if (sum < 7) return 7;
      double maxPosition = h->GetBinCenter(h->GetMaximumBin());
      double binWidth = h->GetBinWidth(1);
      double xmin = h->GetXaxis()->GetXmin();
      double xmax = h->GetXaxis()->GetXmax();

      auto* func = new TF1("func2g",
                           "[0] + [1]*((1-[4])/sqrt(2*pi)/[3]*exp(-0.5*((x-[2])/[3])**2)"
                           "+ [4]/sqrt(2*pi)/[5]*exp(-0.5*((x-[2])/[5])**2))",
                           xmin, xmax);
      func->SetParameter(0, 0);
      func->SetParameter(1, sum * binWidth);
      func->SetParameter(2, maxPosition);
      func->SetParameter(3, m_sigmaCoreInit);
      func->SetParameter(4, m_tailFractInit);
      func->SetParameter(5, m_sigmaTailInit);

      int status = h->Fit(func, "LERSQ");

      m_delT0 = func->GetParameter(2);
      m_error = func->GetParError(2);
      m_chi2 = func->GetChisquare();
      m_ndf = func->GetNDF();

      return status;
    }

  } // end namespace TOP
} // end namespace Belle2
