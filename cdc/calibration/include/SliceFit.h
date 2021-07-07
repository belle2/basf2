/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "TH2D.h"
#include "TH2.h"
#include "TH1D.h"
#include "TF1.h"
#include "TDirectory.h"
#include "TError.h"
#include "TROOT.h"
#include "TString.h"
namespace Belle2 {

  /**
   * Class to do the slice fit.
   */
  class SliceFit {
  public:
    /**
     * Do the slice fit for the 2d histogram.
     */
    static TH1D* doSliceFitY(TH2D* h2, int minHitCut = 0)
    {
      gPrintViaErrorHandler = kTRUE;
      gErrorIgnoreLevel = 3001;
      TString hist_name = h2->GetName();
      double ub = h2->GetYaxis()->GetXmax();
      double lb = h2->GetYaxis()->GetXmin();
      B2DEBUG(199, "Axis: " << lb << "  " << ub);
      if ((h2->GetEntries() / h2->GetNbinsX()) < 30) {
        B2WARNING("Low statictic: " << h2->GetEntries() << " Hits");
        h2->Rebin2D(2, 2, hist_name);
      }

      B2DEBUG(199, "Slice fit for histo " << hist_name);
      B2DEBUG(199, "Number of entries: " << h2->GetEntries());
      TF1* g1 = new TF1("g1", "gaus", lb, ub);
      h2->FitSlicesY(0, 0, -1, minHitCut);


      TString m_name = hist_name + "_1";
      TH1D* hm = (TH1D*)gDirectory->Get(m_name)->Clone("hm");
      if (!hm) return 0;

      B2DEBUG(199, "Number of entries: " << hm->GetEntries());
      TH1D* hlast = (TH1D*)hm->Clone("hlast");
      hlast->Reset();
      hlast->SetName(m_name);
      for (int i = 1; i < h2->GetNbinsX(); ++i) {
        double sum = 0;
        double err = 0;
        double mean = -99;
        TH1D* h1d = h2->ProjectionY("h1d", i, i);
        if (!h1d) continue;
        sum = h1d->GetEntries();
        if (sum < minHitCut) continue; //skip low data slice
        mean = h1d->GetMean();
        double sg = h1d->GetRMS();
        double max = h1d->GetMaximum();
        g1->SetParameters(max, mean, sg);
        h1d->Fit("g1", "QNR", "");
        //  TF1 *f1=h1d->GetFunction("gaus");
        mean = g1->GetParameter(1);
        err = g1->GetParError(1);
        if (sum > 50) {
          double sg2 = g1->GetParameter(2);
          h1d->Fit("g1", "Q0", "", mean - 1.1 * sg2, mean + 1.1 * sg2);
          mean = g1->GetParameter(1);
          //  err=g1->GetParError(1);
        }
        hlast->SetBinContent(i, mean);
        hlast->SetBinError(i, err);
        h1d->Delete();
      }
      return hlast;
    }
  };
}
