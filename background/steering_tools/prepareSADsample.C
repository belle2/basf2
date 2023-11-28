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
  double Lss = 0; /**< length of element in which scattered [m]*/
  int bunchN = 0; /**< number of bunches */
  double beamE = 0; /**< beam energy [GeV]*/
  double bunchI = 0; /**< bunch current [A]*/
  double Ltot = 0; /**< ring length */
};

SADTree m_sad; /**< TTree entry data */

/**
 * Root macro to prepare SAD sample for BeamBkgGenerator from SAD files converted to root
 * SAD files (*.tab) converted to root files with read.C (Nakayama-san private code)
 * output to TTree 'sad': particles within the region (abs(s) < rangeS m)
 *
 * @param inputFile name of the SAD root file(s) - wild cards can be used
 * @param outputFile name of the output root file
 * @param rangeS beam loss region in meters, collect losses within (—rangeS; +rangeS) around the IP, typically 4 (IR) or 29 [m] (IR+FarBeamLine)
 * @param ring SuperKEKB ring name: LER or HER
 * @param d1 the nearest to the IP collimator aperture (inner jaw) in meters: D02H4IN in the LER or D01H5IN in the HER 
 * @param d2 the nearest to the IP collimator aperture (outer jaw) in meters: D02H4OUT in the LER or D01H5OUT in the HER 
 */
void prepareSADsample(std::string inputFile = "", std::string outputFile = "", double rangeS = 4, const std::string& ring = "LER", double d1 = -8.62e-3, double d2 = 9.35e-3)
{

  TChain chain("tp");
  int nFiles = chain.Add(inputFile.c_str());
  if(nFiles == 0) {
    cout << inputFile << ": no such file(s)" << endl;
    return;
  }
  else {
    cout << "number of files: " << nFiles << endl;
    // print list of files
    TObjArray *fileElements = chain.GetListOfFiles();
    TIter next(fileElements);
    TChainElement *chEl = 0;
    while (( chEl=(TChainElement*)next() )) {
        fprintf(stdout, "[%s]\tListOfFiles\t'%s'\n", __FUNCTION__, chEl->GetTitle() );
    }
    if(rangeS <= 0 || rangeS > 29) {
	cout<<endl<<"warning:: rangeS <= 0 [m] or rangeS > 29 [m]. rangeS = 4 [m] is used as default."<<endl<<endl;
	rangeS = 4.0; // [m]
    }
    else {
	cout<<endl<<"rangeS = "<<rangeS<<" [m]"<<endl<<endl;
    }
    cout<<"ring = "<<ring<<endl;
    cout<<"d1 = "<<d1<<" [m] | d2 = "<<d2<<" [m]"<<endl;
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
  chain.SetBranchAddress("Lss", &m_sad.Lss);
  chain.SetBranchAddress("bunchN", &m_sad.bunchN);
  chain.SetBranchAddress("beamE", &m_sad.beamE);
  chain.SetBranchAddress("bunchI", &m_sad.bunchI);
  chain.SetBranchAddress("Ltot", &m_sad.Ltot);              

  int numEntries = chain.GetEntries();
  if(numEntries <= 0) {
    cout << "tree 'tp' is empty";
    return;
  }

  TFile* file = TFile::Open(outputFile.c_str(), "recreate");
  TTree* tree = new TTree("sad", Form("selected SAD particles: abs(s) < %.1f [m]",rangeS));
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
  tree->Branch("Lss", &m_sad.Lss, "Lss/D");
  tree->Branch("bunchN", &m_sad.bunchN, "bunchN/I");
  tree->Branch("beamE", &m_sad.beamE, "beamE/D");
  tree->Branch("bunchI", &m_sad.bunchI, "bunchI/D");
  tree->Branch("Ltot", &m_sad.Ltot, "Ltot/D"); 

  double collPosS = (ring == "LER") ? -15.88 : 16.83; // longitudinal position of the LER : HER collimator in [m], from the SAD lattice file 
  double delta = 0.01; // region of beam losses around the collimator center in [m]
  double jawWidth = 6.0e-3; // collimator jaw width in [m]
  double tipLength = 10.0e-3; // collimator tip/head length in [mm] 
  double tipAngle = 12.0*TMath::Pi()/180.0; // collimator tip/head angle in [rad] = 12 [deg]
  double pipeRx = 0.040; // default beam pipe aperture in X-axis
  double pipeRy = 0.040; // default beam pipe aperture in Y-axis
  double rate = 0;
  for(int i = 0; i < numEntries; i++) {
	chain.GetEntry(i);
	m_sad.rate /= nFiles;
	// Select Range
	if(abs(m_sad.s) < rangeS) {
		// put lost particles at the nearest collimator onto its surface
		if(collPosS-delta < m_sad.s && m_sad.s < collPosS+delta) { // longitudinal cut
			if(abs(m_sad.y) <= jawWidth) { // vertical cut within the jaw width
				// 1. shift by the half length of the collimator tip
				m_sad.s = m_sad.s - tipLength/2;
				// 2. move onto the surface of the jaw
				if(m_sad.x < 0) { // inner jaw
					m_sad.s = m_sad.s - abs(m_sad.x - d1)/TMath::Tan(tipAngle);
				}
				else { // outer jaw
					m_sad.s = m_sad.s - abs(m_sad.x - d2)/TMath::Tan(tipAngle);
				}
			}
			else {
				// manually set the particle outside the jaw edge onto the surface of the beam-pipe
				Double_t xraw = m_sad.x;
				Double_t yraw = m_sad.y;
				Double_t ellip = TMath::Sqrt((xraw/pipeRx)*(xraw/pipeRx)+(yraw/pipeRy)*(yraw/pipeRy));
				m_sad.x = xraw/ellip;
				m_sad.y = yraw/ellip;
			}
		}
		tree->Fill();
		rate += m_sad.rate;
	}
  }
  cout << "entries written to 'sad' tree: " << tree->GetEntries() << endl;
  cout << "total IR rate: " << rate/1e6 << " MHz" << endl;
  tree->Write();
  file->Close();
  
}
