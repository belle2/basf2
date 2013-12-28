/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Hiroyuki Nakayama                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/SAD/ReaderSAD.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/GearDir.h>
#include <mdst/dataobjects/MCParticle.h>

using namespace std;
using namespace Belle2;


ReaderSAD::ReaderSAD(): m_file(NULL), m_tree(NULL), m_transMatrix(NULL),
  m_sRange(3000.0), m_accRing(ReaderSAD::c_LER), m_pxRes(0.01), m_pyRes(0.01),
  m_SADToRealFactor(5.76e6), m_readoutTime(20.0), m_realPartNum(0),
  m_realPartEntry(0), m_readEntry(0)
{
}


ReaderSAD::~ReaderSAD()
{
  if (m_file != NULL) m_file->Close();
}


void ReaderSAD::initialize(TGeoHMatrix* transMatrix, double sRange, ReaderSAD::AcceleratorRings accRing, double readoutTime)
{
  m_transMatrix = transMatrix;
  m_sRange = sRange;
  m_accRing = accRing;
  m_readoutTime = readoutTime;
}


void ReaderSAD::open(const string& filename) throw(SADCouldNotOpenFileError)
{
  if (m_file != NULL) {
    m_file->Close();
    delete m_file;
  }

  m_file = new TFile(filename.c_str(), "READ");
  if (m_file == NULL) throw(SADCouldNotOpenFileError() << filename);

  m_file->cd("");
  m_tree = dynamic_cast<TTree*>(m_file->Get("tp"));
  if (m_tree == NULL) throw(SADCouldNotOpenFileError() << filename);

  m_tree->SetBranchAddress("x", &m_lostX);
  m_tree->SetBranchAddress("y", &m_lostY);
  m_tree->SetBranchAddress("s", &m_lostS);
  m_tree->SetBranchAddress("px", &m_lostPx);
  m_tree->SetBranchAddress("py", &m_lostPy);
  m_tree->SetBranchAddress("rate", &m_lostRate);
  m_tree->SetBranchAddress("E", &m_lostE);

  m_readEntry = -1;
}


double ReaderSAD::getSADParticle(MCParticleGraph& graph)
{
  if (m_tree == NULL) {
    B2ERROR("The SAD tree doesn't exist !")
    return -1;
  }

  do {
    m_readEntry++;

    //Check for end of file
    if (m_readEntry >= m_tree->GetEntries()) throw SADEndOfFile();

    //Load the SAD particle
    m_tree->GetEntry(m_readEntry);
    convertParamsToSADUnits();
    B2DEBUG(10, "> Read particle " << m_readEntry + 1 << "/" << m_tree->GetEntries() << " with s = " << m_lostS << " cm" << " and rate = " << m_lostRate << " Hz")

    printf("Read particle %d / %d with s= %f [m]\n", m_readEntry + 1 , (int)m_tree->GetEntries(), m_lostS / 100.);

    double zMom2 = m_lostE * m_lostE - m_lostPx * m_lostPx - m_lostPy * m_lostPy ;
    if (zMom2 < 0) printf("zMom2= %f is negative. Skipped!\n", zMom2);

  } while ((fabs(m_lostS) > m_sRange) or (m_lostE * m_lostE - m_lostPx * m_lostPx - m_lostPy * m_lostPy < 0));

//  do {
//    //Check for end of file
//    m_readEntry++;
//    if (m_readEntry >= m_tree->GetEntries()) throw SADEndOfFile();
//
//    //Load the SAD particle
//    m_tree->GetEntry(m_readEntry);
//    convertParamsToSADUnits();
//
//    B2DEBUG(10, "> Read particle " << m_readEntry + 1 << "/" << m_tree->GetEntries() << " with s = " << m_lostS << " cm" << " and rate = " << m_lostRate << " Hz" )
//  } while (fabs(m_lostS) > m_sRange);

  addParticleToMCParticles(graph);
  return m_lostRate;
}


bool ReaderSAD::getRealParticle(MCParticleGraph& graph)
{
  if (m_tree == NULL) {
    B2ERROR("The SAD tree doesn't exist !")
    return false;
  }

  //Check for end of file
  if ((m_readEntry >= m_tree->GetEntries()) || (m_tree->GetEntries() == 0)) throw SADEndOfFile();

  //Check if the number of the real particles is reached
  if (m_realPartEntry >= m_realPartNum) {
    m_realPartEntry = 0;
    m_realPartNum   = 0;
  }

  //Check if a new SAD particle has to be read from the file
  if (m_realPartNum == 0) {
    //Read only SAD particles which are inside the chosen sRange
    do {
      m_readEntry++;
      if (m_readEntry >= m_tree->GetEntries()) throw SADEndOfFile();

      m_tree->GetEntry(m_readEntry);
      convertParamsToSADUnits();

      B2DEBUG(10, "> Read particle " << m_readEntry + 1 << "/" << m_tree->GetEntries() << " with s = " << m_lostS << " cm" << " and rate = " << m_lostRate << " Hz")
    } while ((fabs(m_lostS) > m_sRange) && (m_readEntry < m_tree->GetEntries()));

    m_realPartNum = calculateRealParticleNumber(m_lostRate);
  }

  //Create a new real particle from the SAD particle
  if ((fabs(m_lostS) <= m_sRange) && (m_realPartNum > 0)) {
    addParticleToMCParticles(graph);
    B2DEBUG(10, "* Created real particle " << m_realPartEntry + 1 << "/" << m_realPartNum << " for SAD particle " << m_readEntry + 1 << "/" << m_tree->GetEntries())
  }

  m_realPartEntry++;

  return true;
}


void ReaderSAD::addAllSADParticles(MCParticleGraph& graph)
{
  if (m_tree == NULL) {
    B2ERROR("The SAD tree doesn't exist !")
    return ;
  }

  int nPart = m_tree->GetEntries();

  for (int iPart = 0; iPart < nPart; ++iPart) {
    m_tree->GetEntry(iPart);
    convertParamsToSADUnits();
    if (fabs(m_lostS) <= m_sRange) addParticleToMCParticles(graph);
  }
}


//======================================================================
//                         Private methods
//======================================================================

void ReaderSAD::convertParamsToSADUnits()
{
  m_lostX = m_lostX * Unit::m;
  m_lostY = m_lostY * Unit::m;
  m_lostS = m_lostS * Unit::m;
  m_lostPx = m_lostPx * Unit::GeV;
  m_lostPy = m_lostPy * Unit::GeV;
}


void ReaderSAD::addParticleToMCParticles(MCParticleGraph& graph, bool gaussSmearing)
{
  double particlePosSAD[3] = {0.0, 0.0, 0.0};
  double particlePosSADfar[3] = {0.0, 0.0, 0.0};
  double particlePosGeant4[3] = {0.0, 0.0, 0.0};
  double particleMomSAD[3] = {0.0, 0.0, 0.0};
  double particleMomGeant4[3] = {0.0, 0.0, 0.0};

  //Add particle to MCParticle collection
  MCParticleGraph::GraphParticle& particle = graph.addParticle();
  particle.setStatus(MCParticle::c_PrimaryParticle);

  switch (m_accRing) {
    case c_HER: particle.setPDG(11); //electrons
      break;
    case c_LER: particle.setPDG(-11); //positrons
      break;
  }

  particle.setMassFromPDG();
  particle.setChargeFromPDG();

  //Convert the position of the particle from local SAD space to global geant4 space.
  //Flip the sign for the y and z component to go from the accelerator to the detector coordinate system.
  particlePosSAD[0] = m_lostX;
  particlePosSAD[1] = -m_lostY;
  particlePosSAD[2] = -m_lostS;

  particlePosSADfar[0] = m_lostX;
  particlePosSADfar[1] = -m_lostY;
  particlePosSADfar[2] = 0;


  static GearDir content = Gearbox::getInstance().getDetectorComponent("FarBeamLine");
  if (!content)
    B2FATAL("You need FarBeamLine.xml to run SADInput module. Please include FarBeamLine.xml in Belle2.xml. You also need to change 'length' in Belle2.xml to be 40m.");

  TGeoHMatrix* m_transMatrix2 = new TGeoHMatrix(SADtoGeant(m_accRing, m_lostS)); //overwrite m_transMatrix given by initialize()

  if (abs(m_lostS) < 400.) { //4m
    m_transMatrix->LocalToMaster(particlePosSAD, particlePosGeant4);
  } else {
    m_transMatrix2->LocalToMaster(particlePosSADfar, particlePosGeant4);
  }

  //Convert the momentum of the particle from local SAD space to global geant4 space.
  //Flip the sign for the y and z component to go from the accelerator to the detector coordinate system.
  //Calculate the missing pz by using the energy of the particle at the position where it has been lost.
  //double totalMomSqr = (m_lostE * m_lostE - (particle.getMass() * particle.getMass()));
  double totalMomSqr = m_lostE * m_lostE; // SAD output "E" is "P", in fact.

  if (gaussSmearing) {
    particleMomSAD[0] = m_random.Gaus(m_lostPx, m_pxRes * m_lostPx); //1% px resolution
    particleMomSAD[1] = -1.0 * m_random.Gaus(m_lostPy, m_pyRes * m_lostPy);
  } else {
    particleMomSAD[0] = m_lostPx;
    particleMomSAD[1] = -m_lostPy;
  }

  double zMom = sqrt(totalMomSqr - (particleMomSAD[0] *  particleMomSAD[0]) - (particleMomSAD[1] *  particleMomSAD[1]));

  switch (m_accRing) {
    case c_HER: particleMomSAD[2] = zMom;
      break;
    case c_LER: particleMomSAD[2] = -zMom;
      break;
  }

  if (abs(m_lostS) < 400.)
    m_transMatrix->LocalToMasterVect(particleMomSAD, particleMomGeant4);
  else
    m_transMatrix2->LocalToMasterVect(particleMomSAD, particleMomGeant4);

  //Set missing particle information
  particle.setMomentum(TVector3(particleMomGeant4));
  particle.setProductionVertex(TVector3(particlePosGeant4));
  particle.setProductionTime(0.0);
  //particle.setEnergy(m_lostE);
  particle.setEnergy(sqrt(m_lostE * m_lostE + particle.getMass()*particle.getMass()));
  particle.setValidVertex(true);
}


int ReaderSAD::calculateRealParticleNumber(double rate)
{
  double numPart = rate * m_readoutTime / Unit::s; //readoutTime [ns]

  int numPart_int = static_cast<int>(floor(numPart));
  double numPart_dec = numPart  - numPart_int;

  double rnd = m_random.Uniform(); //returns a random number in the interval ]0, 1]
  if (rnd < numPart_dec) numPart_int += 1;

  return numPart_int;
}

TGeoHMatrix ReaderSAD::SADtoGeant(ReaderSAD::AcceleratorRings accRing, double s)
{
  // 0<sraw<3016.3145 m
  // -1500<s<1500 m

  //static double max_s_her = 3016.3145 * Unit::m;
  //static double max_s_ler = 3016.3026 * Unit::m;

  //get parameters from .xml file
  static GearDir content = Gearbox::getInstance().getDetectorComponent("FarBeamLine");

  //--------------
  //-   LHR1

  //  static GearDir content("/Detector/DetectorComponent[@name=\"FarBeamLine\"]/Content");
  static GearDir cLHR1(content, "LHR1/");
  static double LHR1_X0 = cLHR1.getLength("X0");
  static double LHR1_Z0 = cLHR1.getLength("Z0");
  static double LHR1_L = cLHR1.getLength("L");
  static double LHR1_PHI = cLHR1.getLength("PHI");

  //--------------
  //-   BLC2REtube

  //get parameters from .xml file
  static GearDir cBLC2REtube(content, "BLC2REtube/");
  static double BLC2REtube_RT = cBLC2REtube.getLength("RT");
  static double BLC2REtube_X0 = cBLC2REtube.getLength("X0");
  static double BLC2REtube_Z0 = cBLC2REtube.getLength("Z0");
  static double BLC2REtube_SPHI = cBLC2REtube.getLength("SPHI");
  static double BLC2REtube_DPHI = cBLC2REtube.getLength("DPHI");

  //--------------
  //-   LHR2

  //get parameters from .xml file
  static GearDir cLHR2(content, "LHR2/");
  static double LHR2_L = cLHR2.getLength("L");
  static double LHR2_X0 = cLHR2.getLength("X0");
  static double LHR2_Z0 = cLHR2.getLength("Z0");
  static double LHR2_PHI = cLHR2.getLength("PHI");

  //--------------
  //-   LLR1

  //get parameters from .xml file
  static GearDir cLLR1(content, "LLR1/");
  static double LLR1_L = cLLR1.getLength("L");
  static double LLR1_X0 = cLLR1.getLength("X0");
  static double LLR1_Z0 = cLLR1.getLength("Z0");
  static double LLR1_PHI = cLLR1.getLength("PHI");

  //--------------
  //-   BC1RPtube

  //get parameters from .xml file
  static GearDir cBC1RPtube(content, "BC1RPtube/");
  static double BC1RPtube_RT = cBC1RPtube.getLength("RT");
  static double BC1RPtube_X0 = cBC1RPtube.getLength("X0");
  static double BC1RPtube_Z0 = cBC1RPtube.getLength("Z0");
  static double BC1RPtube_SPHI = cBC1RPtube.getLength("SPHI");
  static double BC1RPtube_DPHI = cBC1RPtube.getLength("DPHI");

  //--------------
  //-   LLR2

  //get parameters from .xml file
  static GearDir cLLR2(content, "LLR2/");
  static double LLR2_L = cLLR2.getLength("L");
  static double LLR2_X0 = cLLR2.getLength("X0");
  static double LLR2_Z0 = cLLR2.getLength("Z0");
  static double LLR2_PHI = cLLR2.getLength("PHI");

  //--------------
  //-   BLCWRPtube

  //get parameters from .xml file
  static GearDir cBLCWRPtube(content, "BLCWRPtube/");
  static double BLCWRPtube_RT = cBLCWRPtube.getLength("RT");
  static double BLCWRPtube_X0 = cBLCWRPtube.getLength("X0");
  static double BLCWRPtube_Z0 = cBLCWRPtube.getLength("Z0");
  static double BLCWRPtube_SPHI = cBLCWRPtube.getLength("SPHI");
  static double BLCWRPtube_DPHI = cBLCWRPtube.getLength("DPHI");

  //--------------
  //-   LLR3

  //get parameters from .xml file
  static GearDir cLLR3(content, "LLR3/");
  static double LLR3_L = cLLR3.getLength("L");
  static double LLR3_X0 = cLLR3.getLength("X0");
  static double LLR3_Z0 = cLLR3.getLength("Z0");
  static double LLR3_PHI = cLLR3.getLength("PHI");

  //--------------
  //-   BLC1RPtube

  //get parameters from .xml file
  static GearDir cBLC1RPtube(content, "BLC1RPtube/");
  static double BLC1RPtube_RT = cBLC1RPtube.getLength("RT");
  static double BLC1RPtube_X0 = cBLC1RPtube.getLength("X0");
  static double BLC1RPtube_Z0 = cBLC1RPtube.getLength("Z0");
  static double BLC1RPtube_SPHI = cBLC1RPtube.getLength("SPHI");
  static double BLC1RPtube_DPHI = cBLC1RPtube.getLength("DPHI");

  //--------------
  //-   LLR4

  //get parameters from .xml file
  static GearDir cLLR4(content, "LLR4/");
  static double LLR4_L = cLLR4.getLength("L");
  static double LLR4_X0 = cLLR4.getLength("X0");
  static double LLR4_Z0 = cLLR4.getLength("Z0");
  static double LLR4_PHI = cLLR4.getLength("PHI");

  //--------------
  //-   BLC2RPtube

  //get parameters from .xml file
  static GearDir cBLC2RPtube(content, "BLC2RPtube/");
  static double BLC2RPtube_RT = cBLC2RPtube.getLength("RT");
  static double BLC2RPtube_X0 = cBLC2RPtube.getLength("X0");
  static double BLC2RPtube_Z0 = cBLC2RPtube.getLength("Z0");
  static double BLC2RPtube_SPHI = cBLC2RPtube.getLength("SPHI");
  static double BLC2RPtube_DPHI = cBLC2RPtube.getLength("DPHI");

  //--------------
  //-   LLR5

  //get parameters from .xml file
  static GearDir cLLR5(content, "LLR5/");
  static double LLR5_L = cLLR5.getLength("L");
  static double LLR5_X0 = cLLR5.getLength("X0");
  static double LLR5_Z0 = cLLR5.getLength("Z0");
  static double LLR5_PHI = cLLR5.getLength("PHI");

  //--------------
  //-   LHL1

  //get parameters from .xml file
  static GearDir cLHL1(content, "LHL1/");
  static double LHL1_L = cLHL1.getLength("L");
  static double LHL1_X0 = cLHL1.getLength("X0");
  static double LHL1_Z0 = cLHL1.getLength("Z0");
  static double LHL1_PHI = cLHL1.getLength("PHI");

  //--------------
  //-   BLC1LEtube

  //get parameters from .xml file
  static GearDir cBLC1LEtube(content, "BLC1LEtube/");
  static double BLC1LEtube_RT = cBLC1LEtube.getLength("RT");
  static double BLC1LEtube_X0 = cBLC1LEtube.getLength("X0");
  static double BLC1LEtube_Z0 = cBLC1LEtube.getLength("Z0");
  static double BLC1LEtube_SPHI = cBLC1LEtube.getLength("SPHI");
  static double BLC1LEtube_DPHI = cBLC1LEtube.getLength("DPHI");

  //--------------
  //-   LHL2

  //get parameters from .xml file
  static GearDir cLHL2(content, "LHL2/");
  static double LHL2_L = cLHL2.getLength("L");
  static double LHL2_X0 = cLHL2.getLength("X0");
  static double LHL2_Z0 = cLHL2.getLength("Z0");
  static double LHL2_PHI = cLHL2.getLength("PHI");

  //--------------
  //-   LLL1

  //get parameters from .xml file
  static GearDir cLLL1(content, "LLL1/");
  static double LLL1_L = cLLL1.getLength("L");
  static double LLL1_X0 = cLLL1.getLength("X0");
  static double LLL1_Z0 = cLLL1.getLength("Z0");
  static double LLL1_PHI = cLLL1.getLength("PHI");

  //--------------
  //-   BC1LPtube

  //get parameters from .xml file
  static GearDir cBC1LPtube(content, "BC1LPtube/");
  static double BC1LPtube_RT = cBC1LPtube.getLength("RT");
  static double BC1LPtube_X0 = cBC1LPtube.getLength("X0");
  static double BC1LPtube_Z0 = cBC1LPtube.getLength("Z0");
  static double BC1LPtube_SPHI = cBC1LPtube.getLength("SPHI");
  static double BC1LPtube_DPHI = cBC1LPtube.getLength("DPHI");

  //--------------
  //-   LLL2

  //get parameters from .xml file
  static GearDir cLLL2(content, "LLL2/");
  static double LLL2_L = cLLL2.getLength("L");
  static double LLL2_X0 = cLLL2.getLength("X0");
  static double LLL2_Z0 = cLLL2.getLength("Z0");
  static double LLL2_PHI = cLLL2.getLength("PHI");

  //--------------
  //-   BLC1LPtube

  //get parameters from .xml file
  static GearDir cBLC1LPtube(content, "BLC1LPtube/");
  static double BLC1LPtube_RT = cBLC1LPtube.getLength("RT");
  static double BLC1LPtube_X0 = cBLC1LPtube.getLength("X0");
  static double BLC1LPtube_Z0 = cBLC1LPtube.getLength("Z0");
  static double BLC1LPtube_SPHI = cBLC1LPtube.getLength("SPHI");
  static double BLC1LPtube_DPHI = cBLC1LPtube.getLength("DPHI");

  //--------------
  //-   LLL3

  //get parameters from .xml file
  static GearDir cLLL3(content, "LLL3/");
  static double LLL3_L = cLLL3.getLength("L");
  static double LLL3_X0 = cLLL3.getLength("X0");
  static double LLL3_Z0 = cLLL3.getLength("Z0");
  static double LLL3_PHI = cLLL3.getLength("PHI");

  //--------------
  //-   BLC2LPtube

  //get parameters from .xml file
  static GearDir cBLC2LPtube(content, "BLC2LPtube/");
  static double BLC2LPtube_RT = cBLC2LPtube.getLength("RT");
  static double BLC2LPtube_X0 = cBLC2LPtube.getLength("X0");
  static double BLC2LPtube_Z0 = cBLC2LPtube.getLength("Z0");
  static double BLC2LPtube_SPHI = cBLC2LPtube.getLength("SPHI");
  static double BLC2LPtube_DPHI = cBLC2LPtube.getLength("DPHI");

  //--------------
  //-   LLL4

  //get parameters from .xml file
  static GearDir cLLL4(content, "LLL4/");
  static double LLL4_L = cLLL4.getLength("L");
  static double LLL4_X0 = cLLL4.getLength("X0");
  static double LLL4_Z0 = cLLL4.getLength("Z0");
  static double LLL4_PHI = cLLL4.getLength("PHI");

  static double her_breakpoints[6];
  static double ler_breakpoints[16];
  // positive s
  her_breakpoints[0] = LHL1_L;
  her_breakpoints[1] = her_breakpoints[0] + BLC1LEtube_RT * BLC1LEtube_DPHI;
  her_breakpoints[2] = her_breakpoints[1] + LHL2_L;

  // negative s
  her_breakpoints[3] = -LHR1_L;
  her_breakpoints[4] = her_breakpoints[3] - BLC2REtube_RT * BLC2REtube_DPHI;
  her_breakpoints[5] = her_breakpoints[4] - LHR2_L;

  // positive s
  ler_breakpoints[0] = LLL1_L;
  ler_breakpoints[1] = ler_breakpoints[0] + BC1LPtube_RT * BC1LPtube_DPHI;
  ler_breakpoints[2] = ler_breakpoints[1] + LLL2_L;
  ler_breakpoints[3] = ler_breakpoints[2] + BLC1LPtube_RT * BLC1LPtube_DPHI;
  ler_breakpoints[4] = ler_breakpoints[3] + LLL3_L;
  ler_breakpoints[5] = ler_breakpoints[4] + BLC2LPtube_RT * BLC2LPtube_DPHI;
  ler_breakpoints[6] = ler_breakpoints[5] + LLL4_L;

  // negative s
  ler_breakpoints[7] = -LLR1_L;
  ler_breakpoints[8] = ler_breakpoints[7] - BC1RPtube_RT * BC1RPtube_DPHI;
  ler_breakpoints[9] = ler_breakpoints[8] - LLR2_L;
  ler_breakpoints[10] = ler_breakpoints[9] - BLCWRPtube_RT * BLCWRPtube_DPHI;
  ler_breakpoints[11] = ler_breakpoints[10] - LLR3_L;
  ler_breakpoints[12] = ler_breakpoints[11] - BLC1RPtube_RT * BLC1RPtube_DPHI;
  ler_breakpoints[13] = ler_breakpoints[12] - LLR4_L;
  ler_breakpoints[14] = ler_breakpoints[13] - BLC2RPtube_RT * BLC2RPtube_DPHI;
  ler_breakpoints[15] = ler_breakpoints[14] - LLR5_L;

  double dx = 0;
  double dz = 0;
  double phi = 0;
  if (accRing == c_LER) {
    // LER
    // positive s
    if (400.0 < s) {
      if (s < ler_breakpoints[0]) {
        phi = LLL1_PHI;
        dx = LLL1_X0 + s * sin(phi);
        dz = LLL1_Z0 + s * cos(phi);
      } else if (s < ler_breakpoints[1]) {
        double sloc = s - ler_breakpoints[0];
        phi = BC1LPtube_SPHI + sloc / BC1LPtube_RT;
        // Torus is created in x-y plain.
        // It is then rotated to x-z plain,
        // and its direction changes to reversed,
        // thus phi_real=-phi_xml
        phi = -phi;
        dx = BC1LPtube_X0 + BC1LPtube_RT * cos(-phi);
        dz = BC1LPtube_Z0 + BC1LPtube_RT * sin(-phi);
      } else if (s < ler_breakpoints[2]) {
        double sloc = s - ler_breakpoints[1];
        phi = LLL2_PHI;
        dx = LLL2_X0 + sloc * sin(phi);
        dz = LLL2_Z0 + sloc * cos(phi);
      } else if (s < ler_breakpoints[3]) {
        double sloc = s - ler_breakpoints[2];
        phi = BLC1LPtube_SPHI + sloc / BLC1LPtube_RT;
        phi = -phi;
        dx = BLC1LPtube_X0 + BLC1LPtube_RT * cos(-phi);
        dz = BLC1LPtube_Z0 + BLC1LPtube_RT * sin(-phi);
      } else if (s < ler_breakpoints[4]) {
        double sloc = s - ler_breakpoints[3];
        phi = LLL3_PHI;
        dx = LLL3_X0 + sloc * sin(phi);
        dz = LLL3_Z0 + sloc * cos(phi);
      } else if (s < ler_breakpoints[5]) {
        double sloc = s - ler_breakpoints[4];
        // Torus dphi may be only positive,
        // while direction of increasing |s| is sometimes negative,
        // and we need to use -s and not change phi.
        // Since we add pi to phi later,
        // we subtract it now for this element.
        phi = BLC2LPtube_SPHI + BLC2LPtube_DPHI - sloc / BLC2LPtube_RT;
        phi = -phi;
        dx = BLC2LPtube_X0 + BLC2LPtube_RT * cos(-phi);
        dz = BLC2LPtube_Z0 + BLC2LPtube_RT * sin(-phi);
        phi -= M_PI;
      } else if (s < ler_breakpoints[6]) {
        double sloc = s - ler_breakpoints[5];
        phi = LLL4_PHI;
        dx = LLL4_X0 + sloc * sin(phi);
        dz = LLL4_Z0 + sloc * cos(phi);
      }
      // For this direction rotation angle of elements changes to negative,
      // while SAD coordinates keep orientation.
      // We need to compensate.
      phi += M_PI;
    }
    // negative s
    else if (s < -400.0) {
      if (s > ler_breakpoints[7]) {
        double sloc = -s;
        phi = LLR1_PHI;
        dx = LLR1_X0 + sloc * sin(phi);
        dz = LLR1_Z0 + sloc * cos(phi);
      } else if (s > ler_breakpoints[8]) {
        double sloc = ler_breakpoints[7] - s;
        phi = BC1RPtube_SPHI + BC1RPtube_DPHI - sloc / BC1RPtube_RT;
        phi = -phi;
        dx = BC1RPtube_X0 + BC1RPtube_RT * cos(-phi);
        dz = BC1RPtube_Z0 + BC1RPtube_RT * sin(-phi);
        phi += M_PI;
      } else if (s > ler_breakpoints[9]) {
        double sloc = ler_breakpoints[8] - s;
        phi = LLR2_PHI;
        dx = LLR2_X0 + sloc * sin(phi);
        dz = LLR2_Z0 + sloc * cos(phi);
      } else if (s > ler_breakpoints[10]) {
        double sloc = ler_breakpoints[9] - s;
        phi = BLCWRPtube_SPHI + BLCWRPtube_DPHI - sloc / BLCWRPtube_RT;
        phi = -phi;
        dx = BLCWRPtube_X0 + BLCWRPtube_RT * cos(-phi);
        dz = BLCWRPtube_Z0 + BLCWRPtube_RT * sin(-phi);
        phi += M_PI;
      } else if (s > ler_breakpoints[11]) {
        double sloc = ler_breakpoints[10] - s;
        phi = LLR3_PHI;
        dx = LLR3_X0 + sloc * sin(phi);
        dz = LLR3_Z0 + sloc * cos(phi);
      } else if (s > ler_breakpoints[12]) {
        double sloc = ler_breakpoints[11] - s;
        phi = BLC1RPtube_SPHI + BLC1RPtube_DPHI - sloc / BLC1RPtube_RT;
        phi = -phi;
        dx = BLC1RPtube_X0 + BLC1RPtube_RT * cos(-phi);
        dz = BLC1RPtube_Z0 + BLC1RPtube_RT * sin(-phi);
        phi += M_PI;
      } else if (s > ler_breakpoints[13]) {
        double sloc = ler_breakpoints[12] - s;
        phi = LLR4_PHI;
        dx = LLR4_X0 + sloc * sin(phi);
        dz = LLR4_Z0 + sloc * cos(phi);
      } else if (s > ler_breakpoints[14]) {
        double sloc = ler_breakpoints[13] - s;
        phi = BLC2RPtube_SPHI + sloc / BLC2RPtube_RT;
        phi = -phi;
        dx = BLC2RPtube_X0 + BLC2RPtube_RT * cos(-phi);
        dz = BLC2RPtube_Z0 + BLC2RPtube_RT * sin(-phi);
      } else if (s > ler_breakpoints[15]) {
        double sloc = ler_breakpoints[14] - s;
        phi = LLR5_PHI;
        dx = LLR5_X0 + sloc * sin(phi);
        dz = LLR5_Z0 + sloc * cos(phi);
      }
    }
  }
  if (accRing == c_HER) {
    // HER
    // positive s
    if (400.0 < s) {
      if (s < her_breakpoints[0]) {
        phi = LHL1_PHI;
        dx = LHL1_X0 + s * sin(phi);
        dz = LHL1_Z0 + s * cos(phi);
      } else if (s < her_breakpoints[1]) {
        double sloc = s - her_breakpoints[0];
        phi = BLC1LEtube_SPHI + sloc / BLC1LEtube_RT;
        phi = -phi;
        dx = BLC1LEtube_X0 + BLC1LEtube_RT * cos(-phi);
        dz = BLC1LEtube_Z0 + BLC1LEtube_RT * sin(-phi);
      } else if (s < her_breakpoints[2]) {
        double sloc = s - her_breakpoints[1];
        phi = LHL2_PHI;
        dx = LHL2_X0 + sloc * sin(phi);
        dz = LHL2_Z0 + sloc * cos(phi);
      }
      phi += M_PI;
    }
    // negative s
    else if (s < -400.0) {
      if (s > her_breakpoints[3]) {
        double sloc = -s;
        phi = LHR1_PHI;
        dx = LHR1_X0 + sloc * sin(phi);
        dz = LHR1_Z0 + sloc * cos(phi);
      } else if (s > her_breakpoints[4]) {
        double sloc = her_breakpoints[3] - s;
        phi = BLC2REtube_SPHI + sloc / BLC2REtube_RT;
        phi = -phi;
        dx = BLC2REtube_X0 + BLC2REtube_RT * cos(-phi);
        dz = BLC2REtube_Z0 + BLC2REtube_RT * sin(-phi);
      } else if (s > her_breakpoints[5]) {
        double sloc = her_breakpoints[4] - s;
        phi = LHR2_PHI;
        dx = LHR2_X0 + sloc * sin(phi);
        dz = LHR2_Z0 + sloc * cos(phi);
      }
    }
  }

  TGeoHMatrix matrix("SADTrafo");
  matrix.RotateY(phi / Unit::deg);
  matrix.SetDx(dx);
  matrix.SetDz(dz);
  return matrix;
}

