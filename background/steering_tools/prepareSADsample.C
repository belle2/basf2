/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/**
 * Structure of the TTree in the SAD file
 */
struct SADTree {
  double s = 0;  /**< lost position measured from IP along the ring [m] */
  double x = 0;  /**< x at lost position [m] */
  double px = 0; /**< px at lost position [GeV] */
  double y = 0;  /**< y at lost position [m] */
  double py = 0; /**< py at lost position [GeV] */
  double E = 0;  /**< E at lost position [GeV] (in fact momentum magnitude!) */
  double rate = 0; /**< lost rate [Hz] */
  double ss = 0; /**< scattered position (|s|<Ltot/2) [m] */
  double ssraw = 0; /**< scattered position [m] */
  int nturn = 0; /**< number of turns from scattered to lost */
  double sraw = 0; /**< s at lost position [m] before matching G4 beam pipe inner surface */
  double xraw = 0; /**< x at lost position [m] before matching G4 beam pipe inner surface */
  double yraw = 0; /**< y at lost position [m] before matching G4 beam pipe inner surface */
  double r = 0; /**< sqrt(x*x+y*y) [m] */
  double rr = 0; /**< sqrt(xraw*xraw+yraw*yraw) [m] */
  double dp_over_p0 = 0; /**< momentum deviation of the lost particle */
  double watt = 0; /**< loss wattage [W] */
};

SADTree m_sad; /**< TTree entry data */

/**
 * Root macro to prepare SAD sample for BeamBkgGenerator from SAD files converted to root
 * SAD files (*.tab) converted to root files with read.C (Nakayama-san private code)
 * output to TTree 'sad': particles within IR region (abs(s) < 4 m)
 *
 * @param inputFile name of the SAD root file(s) - wild cards can be used
 * @param outputFile name of the output root file
 */
void prepareSADsample(std::string inputFile, std::string outputFile)
{

  TChain chain("tp");
  int nFiles = chain.Add(inputFile.c_str());
  if(nFiles == 0) {
    cout << inputFile << ": no such file(s)" << endl;
    return;
  }
  else {
    cout << "number of files: " << nFiles << endl;
  }

  chain.SetBranchAddress("s", &m_sad.s);
  chain.SetBranchAddress("x", &m_sad.x);
  chain.SetBranchAddress("px", &m_sad.px);
  chain.SetBranchAddress("y", &m_sad.y);
  chain.SetBranchAddress("py", &m_sad.py);
  chain.SetBranchAddress("E", &m_sad.E);
  chain.SetBranchAddress("rate", &m_sad.rate);
  chain.SetBranchAddress("ss", &m_sad.ss);
  chain.SetBranchAddress("ssraw", &m_sad.ssraw);
  chain.SetBranchAddress("sraw", &m_sad.sraw);
  chain.SetBranchAddress("nturn", &m_sad.nturn);
  chain.SetBranchAddress("xraw", &m_sad.xraw);
  chain.SetBranchAddress("yraw", &m_sad.yraw);
  chain.SetBranchAddress("r", &m_sad.r);
  chain.SetBranchAddress("rr", &m_sad.rr);
  chain.SetBranchAddress("dp_over_p0", &m_sad.dp_over_p0);
  chain.SetBranchAddress("watt", &m_sad.watt);

  int numEntries = chain.GetEntries();
  if(numEntries <= 0) {
    cout << "tree 'tp' is empty";
    return;
  }

  TFile* file = TFile::Open(outputFile.c_str(), "recreate");
  TTree* tree = new TTree("sad", "selected SAD particles: abs(s) < 4.0 m");
  tree->Branch("sraw", &m_sad.sraw, "sraw/D");
  tree->Branch("s", &m_sad.s, "s/D");
  tree->Branch("ss", &m_sad.ss, "ss/D");
  tree->Branch("ssraw", &m_sad.ssraw, "ssraw/D");
  tree->Branch("x", &m_sad.x, "x/D");
  tree->Branch("px", &m_sad.px, "px/D");
  tree->Branch("y", &m_sad.y, "y/D");
  tree->Branch("py", &m_sad.py, "py/D");
  tree->Branch("E", &m_sad.E, "E/D");
  tree->Branch("rate", &m_sad.rate, "rate/D");
  tree->Branch("nturn", &m_sad.nturn, "nturn/I");
  tree->Branch("xraw", &m_sad.xraw, "xraw/D");
  tree->Branch("yraw", &m_sad.yraw, "yraw/D");
  tree->Branch("r", &m_sad.r, "r/D");
  tree->Branch("rr", &m_sad.rr, "rr/D");
  tree->Branch("dp_over_p0", &m_sad.dp_over_p0, "dp_over_p0/D");
  tree->Branch("watt", &m_sad.watt, "watt/D");

  double rate = 0;
  for(int i = 0; i < numEntries; i++) {
    chain.GetEntry(i);
    m_sad.rate /= nFiles;
    // IR
    if(abs(m_sad.s) < 4.0) {
      tree->Fill();
      rate += m_sad.rate;
    }
  }
  cout << "entries written to 'sad' tree: " << tree->GetEntries() << endl;
  cout << "total IR rate: " << rate/1e6 << " MHz" << endl;
  tree->Write();
  file->Close();
  
}
