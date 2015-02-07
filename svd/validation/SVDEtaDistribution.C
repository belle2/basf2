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
  TTree* Layer3Utree = (TTree*) input->Get("Layer3Utree");
  TTree* Layer3Vtree = (TTree*) input->Get("Layer3Vtree");
  TTree* SlantedUtree = (TTree*) input->Get("SlantedUtree");
  TTree* SlantedVtree = (TTree*) input->Get("SlantedVtree");
  TTree* OtherUtree = (TTree*) input->Get("OtherUtree");
  TTree* OtherVtree = (TTree*) input->Get("OtherVtree");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("SVDEtaDistribution.root", "recreate");

  TH1F* hLayer3Ueta = new TH1F("hLayer3Ueta", "Layer3 U eta distribution", 100, 0, 1);
  hLayer3Ueta->GetXaxis()->SetTitle("eta");
  hLayer3Ueta->GetYaxis()->SetTitle("Entries");
  Layer3Utree->Draw("eta>>hLayer3Ueta");
  hLayer3Ueta->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hLayer3Ueta->GetListOfFunctions()->Add(new TNamed("Description", "Normalized interstrip hit position; eta = (cluster_pos modulo pitch) / pitch"));
  hLayer3Ueta->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hLayer3Ueta->Write();

  TH1F* hLayer3Veta = new TH1F("hLayer3Veta", "Layer3 V eta distribution", 100, 0, 1);
  hLayer3Veta->GetXaxis()->SetTitle("eta");
  hLayer3Veta->GetYaxis()->SetTitle("Entries");
  Layer3Vtree->Draw("eta>>hLayer3Veta");
  hLayer3Veta->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hLayer3Veta->GetListOfFunctions()->Add(new TNamed("Description", "Normalized interstrip hit position; eta = (cluster_pos modulo pitch) / pitch"));
  hLayer3Veta->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hLayer3Veta->Write();

  TH1F* hSlantedUeta = new TH1F("hSlantedUeta", "Slanted U eta distribution", 100, 0, 1);
  hSlantedUeta->GetXaxis()->SetTitle("eta");
  hSlantedUeta->GetYaxis()->SetTitle("Entries");
  SlantedUtree->Draw("eta>>hSlantedUeta");
  hSlantedUeta->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hSlantedUeta->GetListOfFunctions()->Add(new TNamed("Description", "Normalized interstrip hit position; eta = (cluster_pos modulo pitch) / pitch"));
  hSlantedUeta->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hSlantedUeta->Write();

  TH1F* hSlantedVeta = new TH1F("hSlantedVeta", "Slanted V eta distribution", 100, 0, 1);
  hSlantedVeta->GetXaxis()->SetTitle("eta");
  hSlantedVeta->GetYaxis()->SetTitle("Entries");
  SlantedVtree->Draw("eta>>hSlantedVeta");
  hSlantedVeta->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hSlantedVeta->GetListOfFunctions()->Add(new TNamed("Description", "Normalized interstrip hit position; eta = (cluster_pos modulo pitch) / pitch"));
  hSlantedVeta->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hSlantedVeta->Write();

  TH1F* hOtherUeta = new TH1F("hOtherUeta", "Other U eta distribution", 100, 0, 1);
  hOtherUeta->GetXaxis()->SetTitle("eta");
  hOtherUeta->GetYaxis()->SetTitle("Entries");
  OtherUtree->Draw("eta>>hOtherUeta");
  hOtherUeta->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hOtherUeta->GetListOfFunctions()->Add(new TNamed("Description", "Normalized interstrip hit position; eta = (cluster_pos modulo pitch) / pitch"));
  hOtherUeta->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hOtherUeta->Write();

  TH1F* hOtherVeta = new TH1F("hOtherVeta", "Other V eta distribution", 100, 0, 1);
  hOtherVeta->GetXaxis()->SetTitle("eta");
  hOtherVeta->GetYaxis()->SetTitle("Entries");
  OtherVtree->Draw("eta>>hOtherVeta");
  hOtherVeta->GetListOfFunctions()->Add(new TNamed("Contact", contact));
  hOtherVeta->GetListOfFunctions()->Add(new TNamed("Description", "Normalized interstrip hit position; eta = (cluster_pos modulo pitch) / pitch"));
  hOtherVeta->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
  hOtherVeta->Write();

  delete output;
}
