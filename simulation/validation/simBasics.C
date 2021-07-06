/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/*
<header>
  <input>EvtGenSimNoBkg.root</input>
  <output>simBasics.root</output>
  <contact>dorisykim@ssu.ac.kr</contact>
  <description> A collection of basic MC information to validate the simulation library. </description>
</header>
*/

// basic particle ID selection requirements

using std::string;

// electrons and posistrons
TCut electrons = "abs(MCParticles.m_pdg) == 11";
// muons 
TCut muons = "abs(MCParticles.m_pdg) == 13";
// photons
TCut photons = "abs(MCParticles.m_pdg) == 22";
// 3 leptons and 3 neutrinos
TCut leptons = "abs(MCParticles.m_pdg) > 10 && abs(MCParticles.m_pdg) < 20";
// pi+, KL, K+
TCut stableMesons = "abs(MCParticles.m_pdg) == 211 || abs(MCParticles.m_pdg) == 130 || abs(MCParticles.m_pdg) == 321";
// p, n
TCut stableBaryons = "abs(MCParticles.m_pdg) == 2212 || abs(MCParticles.m_pdg) == 2112";
// Atoms or their fragments
TCut nuclei = "abs(MCParticles.m_pdg) >= 1000000000"; 

void createVertexHistograms(TTree* tree, const char* particle, const char* info, const TCut& cut);  
void createEnergyHistogram(TTree* tree, const char* particle, const char* info, const TCut& cut, const double& xup);

void numberOfParticles(TTree* tree);
void vertexDistribution(TTree* tree);
void energyDistribution(TTree* tree);

void simBasics()
{
  // open the file with simulated and reconstructed EvtGen particles
  TFile* input = TFile::Open("../EvtGenSimNoBkg.root");
  TTree* tree = (TTree*) input->Get("tree");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("simBasics.root", "recreate");

  numberOfParticles(tree);
  vertexDistribution(tree);
  energyDistribution(tree);

  // close ROOT files
  output->Close();
  input->Close();
}

void numberOfParticles(TTree* tree)
{
  // distribution of leptons 
  const Int_t nbins = 13;  // 12 particles + 1 empty bin in the middle
  const char* leptonNames[nbins] = {"anti-nu_tau", "tau+", "anti-nu_mu", "mu+", "anti_nu_e", "e+", "", "e-", "nu_e", "mu-","nu_mu", "tau-", "nu_tau"};
  const int PDG[nbins] = {-16, -15, -14, -13, -12, -11, 0, 11, 12, 13, 14, 15, 16};

  double xbins[nbins + 1];
  for (int i = 0; i < nbins; i++) xbins[i] = PDG[i] - 0.5;
  xbins[nbins] = PDG[nbins-1] + 0.5;

  TH1F* h =  0;

  h = new TH1F("hLepton", "No. of leptons", nbins, xbins);
  for (int i = 1; i <= nbins; i++) h->GetXaxis()->SetBinLabel(i, leptonNames[i - 1]);
  h->GetXaxis()->SetLabelSize(0.05);
  h->GetXaxis()->SetNdivisions(nbins, kFALSE);
  h->GetXaxis()->SetTitle("Name");
  h->GetYaxis()->SetTitle("Entries/bin");

  tree->Draw("MCParticles.m_pdg >> hLepton", leptons);
  h->SetBins(nbins, xbins[0], xbins[nbins]);

  h->GetListOfFunctions()->Add(new TNamed("Description",
    "No. of leptons: Names a la PDG scheme"));
  h->GetListOfFunctions()->Add(new TNamed("Check",
    "The entry in each bin should be similar to that of the reference"));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "dorisykim@ssu.ac.kr"));

  h->Write();
  delete h;

  // distribution of stable mesons
  const Int_t nbins2 = 7;  // 6 particles + 1 empty bin in the middle
  const char* mesonNames[nbins2] = {"K-", "pi-", "anti-KL", "", "KL", "pi+", "K+"};
  const int PDG2[nbins2] = {-321, -211, -130, 0, 130, 211, 321};

  double xbins2[nbins2 + 1];
  for (int i = 0; i < nbins2; i++) xbins2[i] = PDG2[i] - 0.5;
  xbins2[nbins2] = PDG2[nbins2-1] + 0.5;
 
  h =  new TH1F("hStableMeson", "No. of stable mesons", nbins2, xbins2);
  for (int i = 1; i <= nbins2; i++) h->GetXaxis()->SetBinLabel(i, mesonNames[i - 1]);
  h->GetXaxis()->SetLabelSize(0.05);
  h->GetXaxis()->SetNdivisions(nbins2, kFALSE);
  h->GetXaxis()->SetTitle("Name");
  h->GetYaxis()->SetTitle("Entries/bin");

  tree->Draw("MCParticles.m_pdg >> hStableMeson", stableMesons);
  h->SetBins(nbins2, xbins2[0], xbins2[nbins2]);

  h->GetListOfFunctions()->Add(new TNamed("Description",
    "Number of stable mesons such as charged pions, K Longs, and charged kaons.\n We do not have anti-K Long's."));
  h->GetListOfFunctions()->Add(new TNamed("Check",
    "The entry in each bin should be similar to that of the reference"));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "dorisykim@ssu.ac.kr"));

  h->Write();
  delete h;

  // distribution of stable baryons 
  const Int_t nbins3 = 5;  // 4 particles + 1 empty bin in the middle
  const char* baryonNames[nbins3] = {"p-", "anti-n", "", "n", "p+"};
  const int PDG3[nbins3] = {-2212, -2112, 0, 2112, 2212};

  double xbins3[nbins3 + 1];
  for (int i = 0; i < nbins3; i++) xbins3[i] = PDG3[i] - 0.5;
  xbins3[nbins3] = PDG3[nbins3-1] + 0.5;
 
  h =  new TH1F("hStableBaryon", "No. of stable baryons", nbins3, xbins3);
  for (int i = 1; i <= nbins3; i++) h->GetXaxis()->SetBinLabel(i, baryonNames[i - 1]);
  h->GetXaxis()->SetLabelSize(0.05);
  h->GetXaxis()->SetNdivisions(nbins3, kFALSE);
  h->GetXaxis()->SetTitle("Name");
  h->GetYaxis()->SetTitle("Entries/bin");

  tree->Draw("MCParticles.m_pdg >> hStableBaryon", stableBaryons);
  h->SetBins(nbins3, xbins3[0], xbins3[nbins3]);

  h->GetListOfFunctions()->Add(new TNamed("Description",
    "Number of stable baryons such as protons and neutrons"));
  h->GetListOfFunctions()->Add(new TNamed("Check",
    "The entry in each bin should be similar to that of the reference"));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "dorisykim@ssu.ac.kr"));

  h->Write();
  delete h;
}


void createVertexHistograms(TTree* tree, const char* particle, const char* info, const TCut& cut)
{
  // Distribution

  // x component 
  string name = string("hV") + "X" + particle;
  string particleDetails = string(particle) + " " + info;

  string title = string("MC Vertex [") + "x" + "] of " + particleDetails;

  TH1F* h = 0;

  h = new TH1F(name.c_str(), title.c_str(), 100, -400., 400.);

  title = string("x") + " [cm]";
  h->GetXaxis()->SetTitle(title.c_str());
  h->GetYaxis()->SetTitle("Entries/bin");

  string varexp = string("MCParticles.m_productionVertex_") + "x" + " >> " + name; 
  tree->Draw(varexp.c_str(), cut);

  title = string("Vertex distribution of ") + particleDetails +  " in the " + "x" + "-axis";
  h->GetListOfFunctions()->Add(new TNamed("Description", title.c_str()));
  h->GetListOfFunctions()->Add(new TNamed("Check",
    "The entry in each bin should be similar to that of the reference"));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "dorisykim@ssu.ac.kr"));
  h->Write();
  delete h;

  // y component 
  name = string("hV") + "Y" + particle;
  particleDetails = string(particle) + " " + info;

  title = string("MC Vertex [") + "y" + "] of " + particleDetails;

  h = new TH1F(name.c_str(), title.c_str(), 100, -400., 400.);

  title = string("y") + " [cm]";
  h->GetXaxis()->SetTitle(title.c_str());
  h->GetYaxis()->SetTitle("Entries/bin");

  varexp = string("MCParticles.m_productionVertex_") + "y" + " >> " + name; 
  tree->Draw(varexp.c_str(), cut);

  title = string("Vertex distribution of ") + particleDetails +  " in the " + "y" + "-axis";
  h->GetListOfFunctions()->Add(new TNamed("Description", title.c_str()));
  h->GetListOfFunctions()->Add(new TNamed("Check",
    "The entry in each bin should be similar to that of the reference"));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "dorisykim@ssu.ac.kr"));
  h->Write();
  delete h;

  // z component 
  name = string("hV") + "Z" + particle;
  particleDetails = string(particle) + " " + info;

  title = string("MC Vertex [") + "z" + "] of " + particleDetails;

  h = new TH1F(name.c_str(), title.c_str(), 100, -800., 800.);

  title = string("z") + " [cm]";
  h->GetXaxis()->SetTitle(title.c_str());
  h->GetYaxis()->SetTitle("Entries/bin");

  varexp = string("MCParticles.m_productionVertex_") + "z" + " >> " + name; 
  tree->Draw(varexp.c_str(), cut);

  title = string("Vertex distribution of ") + particleDetails +  " in the " + "z" + "-axis";
  h->GetListOfFunctions()->Add(new TNamed("Description", title.c_str()));
  h->GetListOfFunctions()->Add(new TNamed("Check",
    "The entry in each bin should be similar to that of the reference"));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "dorisykim@ssu.ac.kr"));
  h->Write();
  delete h;

}

void vertexDistribution(TTree* tree)
{
  // Vertex distribution

  // electrons/positrons
  createVertexHistograms(tree, "Electron", "(positrons included)", electrons);

  // photons 
  createVertexHistograms(tree, "Photon", "", photons);

  // nuclei
  createVertexHistograms(tree, "Nucleus", "(atoms or their fragments)", nuclei);

}

void createEnergyHistogram(TTree* tree, const char* particle, const char* info, const TCut& cut, const double& xup)
{

  string name = string("hE") + particle;
  string particleDetails = string(particle) + " " + info;

  string title = string("MC Energy of ") + particleDetails;

  TH1F* h = new TH1F(name.c_str(), title.c_str(), 100, 0., xup);

  title = string("[GeV]");
  h->GetXaxis()->SetTitle(title.c_str());
  h->GetYaxis()->SetTitle("Entries/bin");

  string varexp = string("MCParticles.m_energy") + " >> " + name; 
  tree->Draw(varexp.c_str(), cut);

  title = string("Energy distribution of ") + particleDetails;
  h->GetListOfFunctions()->Add(new TNamed("Description", title.c_str()));
  h->GetListOfFunctions()->Add(new TNamed("Check",
    "The entry in each bin should be similar to that of the reference"));
  h->GetListOfFunctions()->Add(new TNamed("Contact", "dorisykim@ssu.ac.kr"));
  h->Write();
  delete h;

}

void energyDistribution(TTree* tree)
{
  // Vertex distribution

  // electrons/positrons
  createEnergyHistogram(tree, "Electron", "(positrons included)", electrons, 1.0);

  // photons
  createEnergyHistogram(tree, "Photon", "", photons, 2.5);

  // nuclei
  createEnergyHistogram(tree, "Nucleus", "(atoms or their fragments)", nuclei, 5.0);

  // muons 
  createEnergyHistogram(tree, "Muon", "(anti-muons included)", muons, 1.0);

  // stable mesons 
  createEnergyHistogram(tree, "StableMeson", "(charged pions, K Longs, and charged kaons)", stableMesons, 2.5);

  // stable baryons 
  createEnergyHistogram(tree, "StableBaryon", "(protons and neutrons)", stableBaryons, 2.5);

}

