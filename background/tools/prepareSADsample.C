/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
  
  int numEntries = chain.GetEntries();
  if(numEntries <= 0) {
    cout << "tree 'tp' is empty";
    return;
  }

  TFile* file = TFile::Open(outputFile.c_str(), "recreate");
  TTree* tree = new TTree("sad", "selected SAD particles: abs(s) < 4.0 m");
  tree->Branch("s", &m_sad.s, "s/D");
  tree->Branch("x", &m_sad.x, "x/D");
  tree->Branch("px", &m_sad.px, "px/D");
  tree->Branch("y", &m_sad.y, "y/D");
  tree->Branch("py", &m_sad.py, "py/D");
  tree->Branch("E", &m_sad.E, "E/D");
  tree->Branch("rate", &m_sad.rate, "rate/D");

  double rate = 0;
  for(int i = 0; i < numEntries; i++) {
    chain.GetEntry(i);
    m_sad.rate /= nFiles;
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
