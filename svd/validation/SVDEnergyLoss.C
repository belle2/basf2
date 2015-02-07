/*
<header>
  <input>SVDEnergyLossData.root</input>
  <contact>cervenkov@ipnp.troja.mff.cuni.cz</contact>
  <description>This example shows how to read in a data file and produce an output file with a 1D validation plot, a 2D validation plot, and a numerical validation output.
It also demonstrates how a description of the displayed data and the instructions for the check by the shifter can be added.</description>
</header>
*/

static const char* contact = "D. Cervenkov, cervenkov@ipnp.troja.mff.cuni.cz";

void SVDEnergyLoss()
{
  // open the file with simulated and reconstructed EvtGen particles
  TFile* input = TFile::Open("SVDEnergyLossData.root");
  TTree* tree = (TTree*) input->Get("tree");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("SVDEnergyLoss.root", "recreate");

  // an example of a 1D histogram
  TH1F* hdEdx = new TH1F("hdEdx", "dE/dx", 100, 0, 0.02 * 1000);
  hdEdx->GetXaxis()->SetTitle("dE/dx [MeV/cm]");
  hdEdx->GetYaxis()->SetTitle("Entries");
  // conversion to MeV/cm from GeV/cm
  tree->Draw("simhit_dEdx*1000>>hdEdx");
  hdEdx->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hdEdx->GetListOfFunctions()->Add(new TNamed("Description", "Energy loss of the primary particle."));
  hdEdx->GetListOfFunctions()->Add(new TNamed("Check", "Should be landau distributed with MPV around 2.8"));
  hdEdx->Write();

  delete output;
}
