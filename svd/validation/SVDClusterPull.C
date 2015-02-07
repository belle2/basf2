/*
<header>
  <input>SVDClusterPullData.root</input>
  <contact>D. Cervenkov, cervenkov@ipnp.troja.mff.cuni.cz</contact>
  <description>This example shows how to read in a data file and produce an output file with a 1D validation plot, a 2D validation plot, and a numerical validation output.
It also demonstrates how a description of the displayed data and the instructions for the check by the shifter can be added.</description>
</header>
*/

static const char* contact = "D. Cervenkov, cervenkov@ipnp.troja.mff.cuni.cz";

void SVDClusterPull()
{
  // open the file with simulated and reconstructed EvtGen particles
  TFile* input = TFile::Open("SVDClusterPullData.root");
  TTree* Utree = (TTree*) input->Get("Utree");
  TTree* Vtree = (TTree*) input->Get("Vtree");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("SVDClusterPull.root", "recreate");

  // an example of a 1D histogram
  TH1F* hUpull = new TH1F("hUpull", "U pull", 100, -5, 5);
  hUpull->GetXaxis()->SetTitle("x");
  hUpull->GetYaxis()->SetTitle("Entries");
  Utree->Draw("pull>>hUpull");
  hUpull->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hUpull->GetListOfFunctions()->Add(new TNamed("Description", "Cluster position pull distribution in U direction; x = (pos_cluster - pos_truehit)/sigma_cluster"));
  hUpull->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hUpull->Write();

  TH1F* hVpull = new TH1F("hVpull", "V pull", 100, -5, 5);
  hVpull->GetXaxis()->SetTitle("x");
  hVpull->GetYaxis()->SetTitle("Entries");
  Vtree->Draw("pull>>hVpull");
  hVpull->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hVpull->GetListOfFunctions()->Add(new TNamed("Description", "Cluster position pull distribution in V direction; x = (pos_cluster - pos_truehit)/sigma_cluster"));
  hVpull->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hVpull->Write();

  delete output;
}
