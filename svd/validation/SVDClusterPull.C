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
  TTree* Layer3Utree = (TTree*) input->Get("Layer3Utree");
  TTree* Layer3Vtree = (TTree*) input->Get("Layer3Vtree");
  TTree* SlantedUtree = (TTree*) input->Get("SlantedUtree");
  TTree* SlantedVtree = (TTree*) input->Get("SlantedVtree");
  TTree* OtherUtree = (TTree*) input->Get("OtherUtree");
  TTree* OtherVtree = (TTree*) input->Get("OtherVtree");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("SVDClusterPull.root", "recreate");

  TH1F* hLayer3Upull = new TH1F("hLayer3Upull", "Layer3 U pull", 100, -5, 5);
  hLayer3Upull->GetXaxis()->SetTitle("x");
  hLayer3Upull->GetYaxis()->SetTitle("Entries");
  Layer3Utree->Draw("pull>>hLayer3Upull");
  hLayer3Upull->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hLayer3Upull->GetListOfFunctions()->Add(new TNamed("Description", "Cluster position pull distribution in Layer3U direction; x = (pos_cluster - pos_truehit)/sigma_cluster"));
  hLayer3Upull->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hLayer3Upull->Write();

  TH1F* hLayer3Vpull = new TH1F("hLayer3Vpull", "Layer3 V pull", 100, -5, 5);
  hLayer3Vpull->GetXaxis()->SetTitle("x");
  hLayer3Vpull->GetYaxis()->SetTitle("Entries");
  Layer3Vtree->Draw("pull>>hLayer3Vpull");
  hLayer3Vpull->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hLayer3Vpull->GetListOfFunctions()->Add(new TNamed("Description", "Cluster position pull distribution in Layer3V direction; x = (pos_cluster - pos_truehit)/sigma_cluster"));
  hLayer3Vpull->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hLayer3Vpull->Write();

  TH1F* hSlantedUpull = new TH1F("hSlantedUpull", "Slanted U pull", 100, -5, 5);
  hSlantedUpull->GetXaxis()->SetTitle("x");
  hSlantedUpull->GetYaxis()->SetTitle("Entries");
  SlantedUtree->Draw("pull>>hSlantedUpull");
  hSlantedUpull->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hSlantedUpull->GetListOfFunctions()->Add(new TNamed("Description", "Cluster position pull distribution in SlantedU direction; x = (pos_cluster - pos_truehit)/sigma_cluster"));
  hSlantedUpull->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hSlantedUpull->Write();

  TH1F* hSlantedVpull = new TH1F("hSlantedVpull", "Slanted V pull", 100, -5, 5);
  hSlantedVpull->GetXaxis()->SetTitle("x");
  hSlantedVpull->GetYaxis()->SetTitle("Entries");
  SlantedVtree->Draw("pull>>hSlantedVpull");
  hSlantedVpull->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hSlantedVpull->GetListOfFunctions()->Add(new TNamed("Description", "Cluster position pull distribution in SlantedV direction; x = (pos_cluster - pos_truehit)/sigma_cluster"));
  hSlantedVpull->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hSlantedVpull->Write();

  TH1F* hOtherUpull = new TH1F("hOtherUpull", "Other U pull", 100, -5, 5);
  hOtherUpull->GetXaxis()->SetTitle("x");
  hOtherUpull->GetYaxis()->SetTitle("Entries");
  OtherUtree->Draw("pull>>hOtherUpull");
  hOtherUpull->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hOtherUpull->GetListOfFunctions()->Add(new TNamed("Description", "Cluster position pull distribution in OtherU direction; x = (pos_cluster - pos_truehit)/sigma_cluster"));
  hOtherUpull->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hOtherUpull->Write();

  TH1F* hOtherVpull = new TH1F("hOtherVpull", "Other V pull", 100, -5, 5);
  hOtherVpull->GetXaxis()->SetTitle("x");
  hOtherVpull->GetYaxis()->SetTitle("Entries");
  OtherVtree->Draw("pull>>hOtherVpull");
  hOtherVpull->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hOtherVpull->GetListOfFunctions()->Add(new TNamed("Description", "Cluster position pull distribution in OtherV direction; x = (pos_cluster - pos_truehit)/sigma_cluster"));
  hOtherVpull->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hOtherVpull->Write();

  delete output;
}
