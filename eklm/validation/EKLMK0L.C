
/*
<header>
<input>EKLMK0LOutput.root</input>
<output>EKLMK0L.root</output>
<contact>Timofey Uglov, uglov@itep.ru</contact>
</header>
*/


void EKLMK0L()
{
  TString dataobj = "$BELLE2_LOCAL_DIR/lib/$BELLE2_SUBDIR/libdataobjects.so";  
  gROOT->LoadMacro(gSystem->ExpandPathName(dataobj.Data()));  
  TFile* fin = new TFile("../EKLMK0LOutput.root");
  TTree* tree = (TTree*) fin->Get("tree");
  TFile* fout = new TFile("EKLMK0L.root", "recreate");
  TH1F* xres = new TH1F("k0l_xres", "EKLM K0L decay vertex X resolution",
    100, -50, 50);
  TH1F* yres = new TH1F("k0l_yres", "EKLM K0L decay vertex Y resolution",
    100, -50, 50);
  TH1F* zres = new TH1F("k0l_zres", "EKLM K0L decay vertex Z resolution",
    100, -50, 50);
  TH1F* tres = new TH1F("k0l_tres", "EKLM K0L decay time resolution",
    100, -10., 10.);
  TH1F* pres = new TH1F("k0l_pres", "EKLM K0L momentum resolution",
    100, -1., 1.);
  TH1F* ptres = new TH1F("k0l_ptres", "EKLM K0L momentum theta resolution",
    100, -0.2, 0.2);
  TH1F* ppres = new TH1F("k0l_ppres", "EKLM K0L momentum phi resolution",
    100, -0.2, 0.2);
  xres->SetXTitle("cm");
  xres->SetYTitle("Events");
  yres->SetXTitle("cm");
  yres->SetYTitle("Events");
  zres->SetXTitle("cm");
  zres->SetYTitle("Events");
  tres->SetXTitle("ns");
  tres->SetYTitle("Events");
  pres->SetXTitle("GeV");
  pres->SetYTitle("Events");
  ptres->SetXTitle("rad");
  ptres->SetYTitle("Events");
  ppres->SetXTitle("rad");
  ppres->SetYTitle("Events");
  int n = tree->GetEntries();
  TClonesArray *k0lArray;
  TClonesArray *mcParticleArray;
  tree->SetBranchAddress("EKLMK0Ls", &k0lArray);
  tree->SetBranchAddress("MCParticles", &mcParticleArray);
  for (int i = 0; i < n; i++) {
    tree->GetEntry(i);
    int nmc = mcParticleArray->GetEntries();
    Belle2::MCParticle *mcp;
    /* Find primary K0L */
    bool k0lFound = false;
    for (int j = 0; j < nmc; j++) {
      mcp = (Belle2::MCParticle*)mcParticleArray->AddrAt(j);
      if (mcp->getPDG() != 130)
        continue;
      if (mcp->getProductionTime() > 0)
        continue;
      k0lFound = true;
      break;
    }
    if (!k0lFound)
      continue;
    /* Check if K0L decay point is in EKLM */
    TVector3 v = mcp->getDecayVertex();
    TVector3 vk;
    double x = v.x();
    double y = v.y();
    double z = v.z();
    double r = sqrt(x * x + y * y);
    if (r < 132.5 || r > 329.0)
      continue;
    if (abs(x) < 8.2 || abs(y) < 8.2)
      continue;
    if (!((z > -315.1 && z < -183.0) || (z > 277.0 && z < 409.1)))
      continue;
    int nkl = k0lArray->GetEntries();
    Belle2::EKLMK0L *k0l;
    int jmax = -1;
    int mindist = 10000.;
    for (int j = 0; j < nkl; j++) {
      k0l = (Belle2::EKLMK0L*)k0lArray->AddrAt(j);
      vk = k0l->getPosition();
      double d = (v - vk).Mag();
      if (d < mindist) {
        jmax = j;
        mindist = d;
      }
    }
    if (jmax < 0)
      continue;
    k0l = (Belle2::EKLMK0L*)k0lArray->AddrAt(jmax);
    vk = k0l->getPosition() - v;
    TVector3 p = k0l->getMomentumRoot().Vect();
    TVector3 pmc = mcp->getMomentum();
    xres->Fill(vk.x());
    yres->Fill(vk.y());
    zres->Fill(vk.z());
    tres->Fill(k0l->getTime() - mcp->getDecayTime());
    pres->Fill(p.Mag() - pmc.Mag());
    ptres->Fill(p.Theta() - pmc.Theta());
    ppres->Fill(p.Phi() - pmc.Phi());
  }
  xres->GetListOfFunctions()->Add(new TNamed("Description",
    "X resolution")); 
  xres->GetListOfFunctions()->Add(new TNamed("Check",
    "No bias, resolution ~ 10 cm.")); 
  yres->GetListOfFunctions()->Add(new TNamed("Description",
    "Y resolution")); 
  yres->GetListOfFunctions()->Add(new TNamed("Check",
    "No bias, resolution ~ 10 cm.")); 
  zres->GetListOfFunctions()->Add(new TNamed("Description",
    "Z resolution")); 
  zres->GetListOfFunctions()->Add(new TNamed("Check",
    "No bias, resolution ~ 10 cm.")); 
  tres->GetListOfFunctions()->Add(new TNamed("Description",
    "Time resolution")); 
  tres->GetListOfFunctions()->Add(new TNamed("Check",
    "No bias. See bug #997.")); 
  pres->GetListOfFunctions()->Add(new TNamed("Description",
    "Momentum resolution")); 
  pres->GetListOfFunctions()->Add(new TNamed("Check",
    "No bias. See bug #997.")); 
  ptres->GetListOfFunctions()->Add(new TNamed("Description",
    "Momentum theta resolution")); 
  ptres->GetListOfFunctions()->Add(new TNamed("Check",
    "No bias, resolution ~ 0.03")); 
  ppres->GetListOfFunctions()->Add(new TNamed("Description",
    "Momentum phi resolution")); 
  ppres->GetListOfFunctions()->Add(new TNamed("Check",
    "No bias, resolution ~ 0.05")); 
  xres->Write();
  yres->Write();
  zres->Write();
  tres->Write();
  pres->Write();
  ptres->Write();
  ppres->Write();
  delete xres;
  delete yres;
  delete zres;
  delete tres;
  delete pres;
  delete ptres;
  delete ppres;
  delete fin;
  delete fout;
}

