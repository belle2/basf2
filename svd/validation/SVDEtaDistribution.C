/*
<header>
  <input>SVDEtaDistributionData.root</input>
  <contact>cervenkov@ipnp.troja.mff.cuni.cz</contact>
  <description>This example shows how to read in a data file and produce an output file with a 1D validation plot, a 2D validation plot, and a numerical validation output.
It also demonstrates how a description of the displayed data and the instructions for the check by the shifter can be added.</description>
</header>
*/

static const char* contact = "D. Cervenkov, cervenkov@ipnp.troja.mff.cuni.cz";

void SVDEtaDistribution()
{
  // open the file with simulated and reconstructed EvtGen particles
  TFile* input = TFile::Open("SVDEtaDistributionData.root");
  TTree* Utree = (TTree*) input->Get("Utree");
  TTree* Vtree = (TTree*) input->Get("Vtree");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("SVDEtaDistribution.root", "recreate");

  TH1F* hUeta = new TH1F("hUeta", "U eta distribution", 100, 0, 1);
  hUeta->GetXaxis()->SetTitle("eta");
  hUeta->GetYaxis()->SetTitle("Entries");
  Utree->Draw("eta>>hUeta");
  hUeta->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hUeta->GetListOfFunctions()->Add(new TNamed("Description", "Normalized interstrip hit position; eta = (cluster_pos modulo pitch) / pitch"));
  hUeta->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hUeta->Write();

  TH1F* hVeta = new TH1F("hVeta", "V eta distribution", 100, 0, 1);
  hVeta->GetXaxis()->SetTitle("eta");
  hVeta->GetYaxis()->SetTitle("Entries");
  Vtree->Draw("eta>>hVeta");
  hVeta->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hVeta->GetListOfFunctions()->Add(new TNamed("Description", "Normalized interstrip hit position; eta = (cluster_pos modulo pitch) / pitch"));
  hVeta->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hVeta->Write();

  delete output;
}
