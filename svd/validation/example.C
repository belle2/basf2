/*
<header>
  <input>exampleInput.root</input>
  <contact>cervenkov@ipnp.troja.mff.cuni.cz</contact>
  <description>This example shows how to read in a data file and produce an output file with a 1D validation plot, a 2D validation plot, and a numerical validation output.
It also demonstrates how a description of the displayed data and the instructions for the check by the shifter can be added.</description>
</header>
*/

void example()
{
    // open the file with simulated and reconstructed EvtGen particles
    TFile* input = TFile::Open("exampleInput.root");
    TTree* tree = (TTree*) input->Get("tree");

    // open the output file for the validation histograms
    TFile* output = TFile::Open("example.root", "recreate");

    // an example of a 1D histogram
    TH1F* hTest = new TH1F("hTest", "Test histo", 100, 0, 100000000);
    hTest->GetXaxis()->SetTitle("x title");
    hTest->GetYaxis()->SetTitle("y title");
    tree->Draw("SVDSimHits.m_electronProfile>>hTest");
    hTest->GetListOfFunctions()->Add(new TNamed("Contact", "cervenkov@ipnp.troja.mff.cuni.cz"));
    hTest->GetListOfFunctions()->Add(new TNamed("Description", "This is an example of a validation plot."));
    hTest->GetListOfFunctions()->Add(new TNamed("Check", "- There is a colored line for each revision/release.<br>- There is a histogram with black line and grey fill color as reference plot."));
    hTest->Write();

    delete output;
}
