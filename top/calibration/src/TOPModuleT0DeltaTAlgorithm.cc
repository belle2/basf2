/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/calibration/TOPModuleT0DeltaTAlgorithm.h>
#include <top/dbobjects/TOPCalModuleT0.h>
#include <math.h>
#include <TF1.h>
#include <TH2F.h>
#include <TFile.h>
#include <string>

namespace Belle2 {
  namespace TOP {

    TOPModuleT0DeltaTAlgorithm::TOPModuleT0DeltaTAlgorithm():
      CalibrationAlgorithm("TOPModuleT0DeltaTCollector")
    {
      setDescription("Calibration algorithm for method DeltaT");
    }

    CalibrationAlgorithm::EResult TOPModuleT0DeltaTAlgorithm::calibrate()
    {

      return c_OK;
    }


    TFitResultPtr TOPModuleT0DeltaTAlgorithm::fitSingleGaus(std::shared_ptr<TH1F> h)
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

      return h->Fit(func, "LERSQ");
    }


    TFitResultPtr TOPModuleT0DeltaTAlgorithm::fitDoubleGaus(std::shared_ptr<TH1F> h)
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

      return h->Fit(func, "LERSQ");
    }

  } // end namespace TOP
} // end namespace Belle2
