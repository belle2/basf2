/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett, Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Const.h>
#include <framework/io/RootIOUtilities.h>
#include <reconstruction/dataobjects/DedxConstants.h>

#include <TFile.h>
#include <TMath.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TApplication.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TSystem.h>

#include <iostream>

using namespace Belle2;

/** Number of bins on dE/dx axis. */
const int num_dedx_bins = 100;


/** Save 2d PDFs of dE/dx vs. momentum for use with the CDCDedxPID and VXDDedxPID modules.
 *
 *  PDFs for individual dE/dx measurements as well as their truncated means are
 *  produced.
 */
int main(int argc, char* argv[])
{
  const int num_pdg_codes = Const::ChargedStable::c_SetSize;

  if (argc < 2 or argc > 3) {
    std::cerr << "Usage: " << argv[0] << " INPUT_FILE [OUTPUT_FILE]\n\n";
    std::cerr <<
              "Generates PDFs for the DedxPID module, requires an input .root file created by running DedxPID with enableDebug=True.\n";
    std::cerr << "If OUTPUT_FILE is not given, histograms will be drawn instead of being saved.\n";
    return 1;
  }
  RootIOUtilities::loadDictionaries();

  const TString input_filename(argv[1]);
  TString output_filename;
  if (argc == 3)
    output_filename = TString(argv[2]);

  //TApplication eats command line arguments, don't use argc/argv after that
  TApplication app("noname", &argc, argv);
  gROOT->SetStyle("Plain");
  gStyle->SetPalette(1);

  TFile* file = new TFile(input_filename, "READ");
  TTree* tree = dynamic_cast<TTree*>(file->Get("tree"));
  if (!tree) {
    std::cerr << "'tree' not found, aborting.\n";
    return 1;
  }

  TCanvas* c1 = 0;


  const bool save_graphs = !output_filename.IsNull();
  TFile* pdffile = 0;
  if (save_graphs) {
    pdffile = new TFile(output_filename, "RECREATE");
  } else {
    c1 = new TCanvas("c1", input_filename, 600, 900);
    c1->Divide(num_pdg_codes, Dedx::c_num_detectors);
  }

  //create momentum binning (larger bins for high momentum)
  const int num_p_bins = 69;
  double pbins[num_p_bins + 1];
  pbins[0] = 0.0; pbins[1] = 0.05;
  std::cout << ": " << pbins[0] << "\n";
  std::cout << ": " << pbins[1] << "\n";
  for (int bin = 2; bin <= num_p_bins; bin++) {
    if (bin <= 19)
      pbins[bin] = 0.025 + 0.025 * bin;
    else if (bin <= 59)
      pbins[bin] = pbins[19] + 0.05 * (bin - 19);
    else
      pbins[bin] = pbins[59] + 0.3 * (bin - 59);
    std::cout << ": " << pbins[bin] << "\n";
  }

  for (int i = 0; i < 2; i++) { //normal/truncated
    const bool use_truncated_mean = (i == 1);
    const char* suffix = use_truncated_mean ? "_trunc" : "";

    for (int iPart = 0; iPart < num_pdg_codes; iPart++) {
      TH2F* hists[Dedx::c_num_detectors]; //one for each Detector
      const int pdg_code = Const::chargedStableSet.at(iPart).getPDGCode();
      //const int num_dedx_bins = use_truncated_mean ? (num_dedx_bins_hits) : num_dedx_bins_hits;

      // first prepare the histograms for the VXD
      for (int detector = 0; detector <= Dedx::c_SVD; detector++) {
        std::cout << "i " << i << ", d" << detector << "\n";
        const char* flayer_var = "VXDDedxTracks.dedxLayer";
        TString flayer_selection;
        double dedx_cutoff = 0;
        switch (Dedx::Detector(detector)) {
          case Dedx::c_PXD:
            flayer_selection = TString::Format("%s < 0 && %s >= -2", flayer_var, flayer_var);
            dedx_cutoff = 10e3;
            break;
          case Dedx::c_SVD:
            flayer_selection = TString::Format("%s < -2", flayer_var);
            dedx_cutoff = 2e4;
            break;
          case Dedx::c_CDC:
            break;
        }

        const TString histname = TString::Format("hist_d%i_%i%s", detector, pdg_code, suffix);
        const char* varname = use_truncated_mean ? "VXDDedxTracks.m_dedx_avg_truncated" : "VXDDedxTracks.dedx";

        hists[detector] = new TH2F(histname.Data(), histname.Data(),
                                   num_p_bins, pbins,
                                   num_dedx_bins, 0, dedx_cutoff);
        hists[detector]->Sumw2(); //save weights (important for fitting)
        if (use_truncated_mean)
          tree->Project(histname.Data(),
                        TString::Format("%s[][%i]:VXDDedxTracks.m_p", varname, detector),
                        TString::Format("%s < %g && abs(VXDDedxTracks.m_pdg) == %i && %s", varname, dedx_cutoff, pdg_code, flayer_selection.Data()));
        else
          tree->Project(histname.Data(),
                        TString::Format("%s:VXDDedxTracks.m_p", varname),
                        TString::Format("%s < %g && abs(VXDDedxTracks.m_pdg) == %i && %s", varname, dedx_cutoff, pdg_code, flayer_selection.Data()));
      } //detector type

      // now add the CDC histograms
      int detector = (int)Dedx::c_CDC;
      std::cout << "i " << i << ", d" << detector << "\n";
      double dedx_cutoff = 0;
      dedx_cutoff = 3.0;

      const TString histname = TString::Format("hist_d%i_%i%s", detector, pdg_code, suffix);
      const char* varname = use_truncated_mean ? "CDCDedxTracks.m_dedx_avg_truncated" : "CDCDedxTracks.l_dedx";

      hists[detector] = new TH2F(histname.Data(), histname.Data(),
                                 num_p_bins, pbins,
                                 num_dedx_bins, 0, dedx_cutoff);
      hists[detector]->Sumw2(); //save weights (important for fitting)
      if (use_truncated_mean)
        tree->Project(histname.Data(),
                      TString::Format("%s[][%i]:CDCDedxTracks.m_p_cdc", varname, detector),
                      TString::Format("CDCDedxTracks.l_nHitsUsed > 15 && %s < %g && abs(CDCDedxTracks.m_pdg) == %i", varname, dedx_cutoff, pdg_code));
      else
        tree->Project(histname.Data(),
                      TString::Format("%s:CDCDedxTracks.m_p_cdc", varname),
                      TString::Format("CDCDedxTracks.l_nHitsUsed > 15 && %s < %g && abs(CDCDedxTracks.m_pdg) == %i", varname, dedx_cutoff, pdg_code));

      //{{{ for each momentum bin, normalize pdf (disable if you want to keep the orginals, e.g. for fitting)
      if (true) {
        for (int d = 0; d < Dedx::c_num_detectors; d++) {
          for (int pbin = 0; pbin <= num_p_bins + 1; pbin++) {
            // get number of entries in this pbin
            double integral = 0;
            for (int dedxbin = 0; dedxbin <= num_dedx_bins + 1; dedxbin++) {
              integral += hists[d]->GetBinContent(pbin, dedxbin);
            }

            if (integral == 0)
              continue; //nothing to do

            // normalize this pbin to 1
            const double normal_width = -(hists[d]->GetYaxis()->GetBinLowEdge(1) - hists[d]->GetYaxis()->GetBinUpEdge(
                                            num_dedx_bins)) / num_dedx_bins;
            for (int dedxbin = 0; dedxbin <= num_dedx_bins + 1; dedxbin++) {
              hists[d]->SetBinContent(pbin, dedxbin, hists[d]->GetBinContent(pbin,
                                      dedxbin) / integral * hists[d]->GetYaxis()->GetBinWidth(dedxbin) / normal_width);
              //std::cout << d << ", " << dedxbin << ": " <<  hists[d]->GetYaxis()->GetBinWidth(dedxbin)/normal_width << "\n";
            }
          }
        }
      }
      //}}}

      for (int d = 0; d < Dedx::c_num_detectors; d++) {
        if (save_graphs) {
          hists[d]->Write();
        } else if (i == 0) {
          TPad* pad = static_cast<TPad*>(c1->cd(num_pdg_codes * d + iPart + 1));
          pad->SetLogz();
          hists[d]->Draw("colz");
          c1->Update();
        }
      }
    } //particle type
  } //normal / trunc

  if (save_graphs) {
    pdffile->Close();
    delete pdffile;

    std::cout << "Saved histograms into '" << output_filename.Data() << "'.\n";

    //no output, so we'll just exit when we're done
    gSystem->Exit(0);
  }

  app.Run(true);
  return 0;
}
