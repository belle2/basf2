void EKLMK0L()
{
  TString dataobj = "$BELLE2_LOCAL_DIR/lib/$BELLE2_SUBDIR/libdataobjects.so";  
  gROOT->LoadMacro(gSystem->ExpandPathName(dataobj.Data()));  
  TFile* fin = new TFile("../EKLMK0LOutput.root");
  TTree* tree = (TTree*) fin->Get("tree");
  TFile* fout = new TFile("EKLMK0L.root", "recreate");
  TH1F* xres = new TH1F("k0l_xres", "EKLM K0L decay vertex X resolution",
    200, -100, 100);
  TH1F* yres = new TH1F("k0l_yres", "EKLM K0L decay vertex Y resolution",
    200, -100, 100);
  TH1F* zres = new TH1F("k0l_zres", "EKLM K0L decay vertex Z resolution",
    200, -100, 100);
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
    xres->Fill(vk.x());
    yres->Fill(vk.y());
    zres->Fill(vk.z());
  } 
  xres->Write();
  yres->Write();
  zres->Write();
  delete xres;
  delete yres;
  delete zres;
  delete fin;
  delete fout;
}

