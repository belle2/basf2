/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/*
<header>
  <input>EvtGenSimRec.root</input>
  <contact>Kilian.Lieret@lmu.de</contact>

  <description>
    This example shows how to read in a data file and produce an
    output file with a 1D validation plot, a 2D validation plot, and a
    numerical validation output. It also demonstrates how a description of the
    displayed data and the instructions for the check by the shifter can be
    added.
  </description>

</header>
*/

void example()
{
  // open the file with simulated and reconstructed EvtGen particles
  TFile* input = TFile::Open("../EvtGenSimRec.root");
  TTree* tree = (TTree*) input->Get("tree");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("example.root", "recreate");

  // an example of a 1D histogram
  TH1F* hTopLHs = new TH1F("hTopLHs", "TOP Likelihoods", 100, -200, 200);
  hTopLHs->GetXaxis()->SetTitle("TOP LL(#pi)-LL(K)");
  hTopLHs->GetYaxis()->SetTitle("tracks");
  tree->Draw("TOPLikelihoods.getLogL_pi()-TOPLikelihoods.getLogL_K()>>hTopLHs");
  hTopLHs->GetListOfFunctions()->Add(new TNamed("Description", "This is an example of a validation plot."));
  hTopLHs->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hTopLHs->GetListOfFunctions()->Add(new TNamed("MetaOptions", "nocompare"));
  hTopLHs->Write();

  // an example of a 2D histogram
  TH2F* hMCVertex = new TH2F("hMCVertex", "Vertex of MC Particles", 100, -400, 400, 100, -400, 400);
  hMCVertex->GetXaxis()->SetTitle("x [cm]");
  hMCVertex->GetYaxis()->SetTitle("y [cm]");
  tree->Draw("MCParticles.m_productionVertex_y:MCParticles.m_productionVertex_x>>hMCVertex");
  hMCVertex->GetListOfFunctions()->Add(new TNamed("Description", "This is an example of a 2D validation plot."));
  hMCVertex->GetListOfFunctions()->Add(new TNamed("Check", "There is a 2D plot for the reference and for each revision/release."));
  hMCVertex->GetListOfFunctions()->Add(new TNamed("MetaOptions", "nocompare"));
  hMCVertex->Write();

  // an example of a numerical value
  TH1F* hNTrackCands = new TH1F("hNTrackCands", "Number of Track Candidates", 100, -0.5, 99.5);
  tree->Draw("@Tracks.size()>>hNTrackCands");
  TNtuple* nTrackCands = new TNtuple("nTrackCands", "Average Number of Track Candidates", "number:rms");
  nTrackCands->Fill(hNTrackCands->GetMean(), hNTrackCands->GetRMS());
  nTrackCands->SetAlias("Description", "This is an example of a table with validation values.");
  nTrackCands->SetAlias("Check", "There is a row for the reference and for each revision/release.");
  delete hNTrackCands;
  nTrackCands->Write();

  delete output;
}
