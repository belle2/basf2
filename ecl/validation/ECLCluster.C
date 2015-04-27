/* ECLPion.C 
 * ROOT macro for ECL validation plots 
 * Authors: Elisa Manoni, Benjamin Oberhof 
 * 2014
*/

/*
<header>
<input>ECLClusterOutput_500mev.root</input>
<output>ECLCluster.root</output>
<contact>ecl2ml@bpost.kek.jp</contact>
</header>
*/

void ECLCluster()
{
  // open the file with simulated and reconstructed EvtGen particles
  TFile* input_500 = TFile::Open("../ECLClusterOutput_500mev.root");
  TTree* tree = (TTree*) input_500->Get("m_tree");

  TH1F* hMultip_500 = new TH1F("hMultip_500","Reconstructed Cluster Multiplicity", 10, 0., 10.);
  hMultip_500->GetListOfFunctions()->Add(new TNamed("Description", "Multiplicity of reconstructed clusters for 500 MeV/c single photons"));
  hMultip_500->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape.")); 
  hMultip_500->GetXaxis()->SetTitle("Cluster Multiplicity");
  hMultip_500->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp")); 
  tree->Draw("eclClusterMultip>>hMultip_500");

  TH1F* hEnergy_500 = new TH1F("hEnergy_500","Reconstructed Cluster Energy", 100, 0., 0.55);
  hEnergy_500->GetListOfFunctions()->Add(new TNamed("Description", "Deposited energy for 500 MeV/c single photons"));
  hEnergy_500->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape, peak around 0.5 GeV and left-side tail.")); 
  hEnergy_500->GetXaxis()->SetTitle("Cluster Energy (GeV)");
  hEnergy_500->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp"));
  tree->Draw("eclClusterEnergy>>hEnergy_500");

  TH1F* hEnDepSum_500 = new TH1F("hEnDepSum_500","Uncorrected Reconstructed Cluster Energy", 100, 0., 0.55);
  hEnDepSum_500->GetListOfFunctions()->Add(new TNamed("Description", "Uncorrected deposited energy for 500 MeV/c single photons"));
  hEnDepSum_500->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape, peak around 0.5 GeV and left-side tail.")); 
  hEnDepSum_500->GetXaxis()->SetTitle("Cluster Energy (GeV)");
  hEnDepSum_500->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp"));
  tree->Draw("eclClusterEnergyDepSum>>hEnDepSum_500");

  TH1F* hHighestE_500 = new TH1F("hHighestE_500","Highest Energy Deposit", 100, 0., 0.55);
  hHighestE_500->GetListOfFunctions()->Add(new TNamed("Description", "Highest energy deposited in a crystal for 500 MeV/c single photons"));
  hHighestE_500->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape."));
  hHighestE_500->GetXaxis()->SetTitle("Deposited Energy(GeV)");
  hHighestE_500->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp"));
  tree->Draw("eclClusterHighestE>>hHighestE_500");

  TH1F* hE9oE25_500 = new TH1F("hE9oE25_500","E9/E25", 100, 0., 1.05);
  hE9oE25_500->GetListOfFunctions()->Add(new TNamed("Description", "Energy deposited in 3x3 matrix around most energetic deposit over energy deposited in 5x5 matrix around most energetic deposit 500 MeV/c single photons"));
  hE9oE25_500->GetListOfFunctions()->Add(new TNamed("Check", "Consistent shape."));
  hE9oE25_500->GetXaxis()->SetTitle("E9oE25");
  hE9oE25_500->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp"));
  tree->Draw("eclClusterE9oE25>>hE9oE25_500");

  TH1F* hphi_500 = new TH1F("hphi_500", "Reconstructed #phi Angle", 50,-3.155,3.155);
  hphi_500->GetListOfFunctions()->Add(new TNamed("Description", "Reconstructed #phi angle of the cluster for 500 MeV/c single photons"));
  hphi_500->GetListOfFunctions()->Add(new TNamed("Check","Consistent shape, flat distribution"));
  hphi_500->GetXaxis()->SetTitle("#phi (rad)");
  hphi_500->SetMinimum(.0);
  hphi_500->GetListOfFunctions()->Add(new TNamed("Contact","ecl2ml@bpost.kek.jp"));
  tree->Draw("eclClusterPhi>>hphi_500");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("ECLCluster.root", "recreate");
  hMultip_500->Write();
  hEnergy_500->Write();
  hEnDepSum_500->Write();
  hHighestE_500->Write();
  hE9oE25_500->Write();
  hphi_500->Write();
  
  delete output;
}
