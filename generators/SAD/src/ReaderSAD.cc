/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Hiroyuki Nakayama, Igal Jaegle             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/SAD/ReaderSAD.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/GearDir.h>
#include <mdst/dataobjects/MCParticle.h>

//Start my addition
#include <generators/SAD/dataobjects/SADMetaHit.h>
// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// DataStore classes
#include <framework/io/RootIOUtilities.h>
#include <framework/dataobjects/EventMetaData.h>
//End my addition

#include <TRandom3.h>

using namespace std;
using namespace Belle2;


ReaderSAD::ReaderSAD(): m_file(NULL), m_tree(NULL), m_transMatrix(NULL),
  m_sRange(3000.0), m_accRing(ReaderSAD::c_LER), m_pxRes(0.01), m_pyRes(0.01),
  m_SADToRealFactor(5.76e6), m_readoutTime(20.0), m_realPartNum(0),
  m_realPartEntry(0), m_readEntry(0)
{
  m_lostX = 0.0;
  m_lostY = 0.0;
  m_lostS = 0.0;
  m_lostPx = 0.0;
  m_lostPy = 0.0;
  m_lostRate = 0.0;
  m_lostE = 0.0;

  //Start my addition
  m_inputSAD_ssraw =  0;
  m_inputSAD_sraw = 0;
  m_inputSAD_ss = 0;
  m_inputSAD_s = 0;
  m_inputSAD_Lss = 0;
  m_inputSAD_nturn = 0;
  m_inputSAD_x = 0;
  m_inputSAD_y = 0;
  m_inputSAD_px = 0;
  m_inputSAD_py = 0;
  m_inputSAD_xraw = 0;
  m_inputSAD_yraw = 0;
  m_inputSAD_r = 0;
  m_inputSAD_rr = 0;
  m_inputSAD_dp_over_p0 = 0;
  m_inputSAD_E = 0;
  m_inputSAD_rate = 0;
  m_inputSAD_watt = 0;
  //End my addition
}


ReaderSAD::~ReaderSAD()
{
  // comment out the following line for the march 16 build (TF)
  //  if (m_file != NULL) m_file->Close();
}


void ReaderSAD::initialize(TGeoHMatrix* transMatrix, double sRange, ReaderSAD::AcceleratorRings accRing, double readoutTime)
{
  m_transMatrix = transMatrix;
  m_sRange = sRange;
  m_accRing = accRing;
  m_readoutTime = readoutTime;
}


void ReaderSAD::open(const string& filename)
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

  m_tree->SetBranchAddress("ssraw", &m_inputSAD_ssraw);
  m_tree->SetBranchAddress("sraw", &m_inputSAD_sraw);
  m_tree->SetBranchAddress("ss", &m_inputSAD_ss);
  m_tree->SetBranchAddress("Lss", &m_inputSAD_Lss);
  m_tree->SetBranchAddress("nturn", &m_inputSAD_nturn);
  m_tree->SetBranchAddress("xraw", &m_inputSAD_xraw);
  m_tree->SetBranchAddress("yraw", &m_inputSAD_yraw);
  m_tree->SetBranchAddress("r", &m_inputSAD_r);
  m_tree->SetBranchAddress("rr", &m_inputSAD_rr);
  m_tree->SetBranchAddress("dp_over_p0", &m_inputSAD_dp_over_p0);
  m_tree->SetBranchAddress("watt", &m_inputSAD_watt);

  m_readEntry = -1;

  StoreArray<SADMetaHit> hit;
  hit.registerInDataStore();

}


double ReaderSAD::getSADParticle(MCParticleGraph& graph)
{
  if (m_tree == NULL) {
    B2ERROR("The SAD tree doesn't exist !");
    return -1;
  }

  do {
    m_readEntry++;

    //Check for end of file
    if (m_readEntry >= m_tree->GetEntries()) throw SADEndOfFile();

    //Load the SAD particle
    m_tree->GetEntry(m_readEntry);
    convertParamsToSADUnits();
    B2DEBUG(10, "> Read particle " << m_readEntry + 1 << "/" << m_tree->GetEntries() << " with s = " << m_lostS << " cm" <<
            " and rate = " << m_lostRate << " Hz");

    //printf("Read particle %d / %d with s= %f [m]\n", m_readEntry + 1 , (int)m_tree->GetEntries(), m_lostS / 100.);

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
    B2ERROR("The SAD tree doesn't exist !");
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

      B2DEBUG(10, "> Read particle " << m_readEntry + 1 << "/" << m_tree->GetEntries() << " with s = " << m_lostS << " cm" <<
              " and rate = " << m_lostRate << " Hz");
    } while ((fabs(m_lostS) > m_sRange) && (m_readEntry < m_tree->GetEntries()));

    if (fabs(m_lostS) <= m_sRange)
      m_realPartNum = calculateRealParticleNumber(m_lostRate);
  }

  //Create a new real particle from the SAD particle
  if ((fabs(m_lostS) <= m_sRange) && (m_realPartNum > 0)) {
    addParticleToMCParticles(graph);
    B2DEBUG(10, "* Created real particle " << m_realPartEntry + 1 << "/" << m_realPartNum << " for SAD particle " << m_readEntry + 1 <<
            "/" << m_tree->GetEntries());
  }

  m_realPartEntry++;

  return true;
}


void ReaderSAD::addAllSADParticles(MCParticleGraph& graph)
{
  if (m_tree == NULL) {
    B2ERROR("The SAD tree doesn't exist !");
    return ;
  }

  int nPart = m_tree->GetEntries();

  for (int iPart = 0; iPart < nPart; ++iPart) {
    m_tree->GetEntry(iPart);
    convertParamsToSADUnits();
    if (fabs(m_lostS) <= m_sRange) addParticleToMCParticles(graph);

    //std::cout << " in sad " <<  m_inputSAD_sraw << std::endl;

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

  //Start my addition
  m_inputSAD_ssraw =  m_inputSAD_ssraw * Unit::m;
  m_inputSAD_sraw = m_inputSAD_sraw * Unit::m;
  m_inputSAD_ss = m_inputSAD_ss * Unit::m;
  m_inputSAD_s = m_inputSAD_s * Unit::m;
  //m_inputSAD_Lss
  //m_inputSAD_nturn
  m_inputSAD_x = m_inputSAD_x * Unit::m;
  m_inputSAD_y = m_inputSAD_y * Unit::m;
  m_inputSAD_px = m_inputSAD_px * Unit::GeV;
  m_inputSAD_py =  m_inputSAD_py * Unit::GeV;
  m_inputSAD_xraw = m_inputSAD_xraw * Unit::m;
  m_inputSAD_yraw = m_inputSAD_yraw * Unit::m;
  //m_inputSAD_r
  //m_inputSAD_rr
  //m_inputSAD_dp_over_p0
  //m_inputSAD_E
  //m_inputSAD_rate
  //End my addition
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

  if (abs(m_lostS) <= 400.) { //4m
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
    particleMomSAD[0] = gRandom->Gaus(m_lostPx, m_pxRes * m_lostPx); //1% px resolution
    particleMomSAD[1] = -1.0 * gRandom->Gaus(m_lostPy, m_pyRes * m_lostPy);
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

  int ring = 0;

  switch (m_accRing) {
    case c_HER: ring = 1;
      break;
    case c_LER: ring = 2;
      break;
  }
  /*
  int ler_section[12] = {1, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
  int ler_inf_section[12] = {0., 254.74, 498.59, 754.14, 1009.66, 1253.52, 1488.7, 1764.1, 2007.05, 2262.14, 2517.2, 2760.15};
  int her_inf_section[12] = {254.32, 494.31, 750.25, 1009.24, 1249.24, 1488.43, 1763.68, 2002.77, 2258.25, 2516.78, 2755.87, 3011.33};
  int her_section[12] = {1, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
  int ler_sup_section[12] = {0., 254.61, 496.52, 727.23, 1007.7, 1249.3, 1479.82, 1761.31, 2003.88, 2239.89, 2516.31, 2758.89};
  int her_sup_section[12] = {254.3, 495.32, 726.92, 1007.39, 1248.41, 1479.51, 1761, 2002.99, 2239.58, 2516, 2758, 3011.87};

  int section = -1;

  if (ring == 1) {
    for (int i = 0; i < 12; i++) {
      if (her_inf_section[i] <= (m_inputSAD_ssraw + 1500.) && (m_inputSAD_ssraw + 1500.) <= her_sup_section[i])
  section = her_section[i] - 1;
    }
  } else if  (ring == 2) {
    for (int i = 0; i < 12; i++) {
      if (ler_inf_section[i] <= (m_inputSAD_ssraw + 1500.) && (m_inputSAD_ssraw + 1500.) <= ler_sup_section[i])
  section = ler_section[i] - 1;
    }
  }
  */
  //each rings have 12 section of ~250m
  //the 1st section D01, the second section is D12, followed by D11, D10 .... for both rings
  int section_ordering[12] = {1, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
  //int ring_section = section_ordering[(int)((m_inputSAD_ssraw + 1500.) / 12.)];
  double ssraw = 0.;
  if (ring == 1) {
    if (m_inputSAD_ssraw >= 0) ssraw = m_inputSAD_ssraw / 100.;
    else if (m_inputSAD_ssraw < 0) ssraw = 3000. + m_inputSAD_ssraw / 100.;
  } else if (ring == 2) {
    //if (m_inputSAD_ssraw >= 0) ssraw = 3000. - m_inputSAD_ssraw / 100.;
    //else if (m_inputSAD_ssraw < 0) ssraw = -m_inputSAD_ssraw / 100.;
    if (m_inputSAD_ssraw >= 0) ssraw = m_inputSAD_ssraw / 100.;
    else if (m_inputSAD_ssraw < 0) ssraw = 3000. + m_inputSAD_ssraw / 100.;
  }
  int ring_section = section_ordering[(int)((ssraw) / 250.)];

  if (abs(m_lostS) <= 400.) {
    m_transMatrix->LocalToMasterVect(particleMomSAD, particleMomGeant4);
  } else {
    m_transMatrix2->LocalToMasterVect(particleMomSAD, particleMomGeant4);
  }

  //Set missing particle information
  particle.setMomentum(TVector3(particleMomGeant4));
  particle.setProductionVertex(TVector3(particlePosGeant4));
  particle.setProductionTime(0.0);
  particle.setEnergy(sqrt(m_lostE * m_lostE + particle.getMass()*particle.getMass()));
  particle.setValidVertex(true);

  //Start my addition
  StoreArray<SADMetaHit> SADMetaHits;
  SADMetaHits.appendNew(SADMetaHit(m_inputSAD_ssraw, m_inputSAD_sraw, m_inputSAD_ss, m_lostS,
                                   m_inputSAD_Lss, m_inputSAD_nturn,
                                   m_lostX, m_lostY, m_lostPx, m_lostPy, m_inputSAD_xraw, m_inputSAD_yraw,
                                   m_inputSAD_r, m_inputSAD_rr, m_inputSAD_dp_over_p0, m_lostE, m_lostRate,
                                   m_inputSAD_watt, ring, ring_section));
  //End my addition
}


int ReaderSAD::calculateRealParticleNumber(double rate)
{
  double numPart = rate * m_readoutTime / Unit::s; //readoutTime [ns]

  int numPart_int = static_cast<int>(floor(numPart));
  double numPart_dec = numPart  - numPart_int;

  double rnd = gRandom->Uniform(); //returns a random number in the interval ]0, 1]
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

  map<string, straightElement> straights;
  map<string, bendingElement> bendings;
  for (const GearDir& element : content.getNodes("Straight")) {

    string name = element.getString("@name");
    string type = element.getString("@type");

    if (type != "pipe") continue;

    straightElement straight;

    straight.x0 = element.getLength("X0");
    straight.z0 = element.getLength("Z0");
    straight.l = element.getLength("L");
    straight.phi = element.getLength("PHI");

    straights[name] = straight;
  }

  string str_checklist[] = {"LHR1", "LHR2", "LLR1", "LLR2", "LLR3", "LLR4", "LLR5", "LHL1", "LHL2", "LLL1", "LLL2", "LLL3", "LLL4"};
  for (const string& str : str_checklist) {
    if (straights.count(str) == 0)
      B2FATAL("You need FarBeamLine.xml to run SADInput module. Please include FarBeamLine.xml in Belle2.xml. You also need to change 'length' in Belle2.xml to be 40m.");
  }

  for (const GearDir& element : content.getNodes("Bending")) {

    string name = element.getString("@name");
    string type = element.getString("@type");

    if (type != "pipe") continue;

    bendingElement bending;

    bending.rt = element.getLength("RT");
    bending.x0 = element.getLength("X0");
    bending.z0 = element.getLength("Z0");
    bending.sphi = element.getLength("SPHI");
    bending.dphi = element.getLength("DPHI");

    bendings[name] = bending;
  }

  string bend_checklist[] = {"BLC2RE", "BC1RP", "BLCWRP", "BLC1RP", "BLC2RP", "BLC1LE", "BC1LP", "BLC1LP", "BLC2LP"};
  for (const string& bnd : bend_checklist) {
    if (bendings.count(bnd) == 0)
      B2FATAL("You need FarBeamLine.xml to run SADInput module. Please include FarBeamLine.xml in Belle2.xml. You also need to change 'length' in Belle2.xml to be 40m.");
  }

  static double her_breakpoints[6];
  static double ler_breakpoints[16];

  // positive s
  her_breakpoints[0] = straights["LHL1"].l;
  her_breakpoints[1] = her_breakpoints[0] + bendings["BLC1LE"].rt * bendings["BLC1LE"].dphi;
  her_breakpoints[2] = her_breakpoints[1] + straights["LHL2"].l;

  // negative s
  her_breakpoints[3] = -straights["LHR1"].l;
  her_breakpoints[4] = her_breakpoints[3] - bendings["BLC2RE"].rt * bendings["BLC2RE"].dphi;
  her_breakpoints[5] = her_breakpoints[4] - straights["LHR2"].l;

  // positive s
  ler_breakpoints[0] = straights["LLL1"].l;
  ler_breakpoints[1] = ler_breakpoints[0] + bendings["BC1LP"].rt * bendings["BC1LP"].dphi;
  ler_breakpoints[2] = ler_breakpoints[1] + straights["LLL2"].l;
  ler_breakpoints[3] = ler_breakpoints[2] + bendings["BLC1LP"].rt * bendings["BLC1LP"].dphi;
  ler_breakpoints[4] = ler_breakpoints[3] + straights["LLL3"].l;
  ler_breakpoints[5] = ler_breakpoints[4] + bendings["BLC2LP"].rt * bendings["BLC2LP"].dphi;
  ler_breakpoints[6] = ler_breakpoints[5] + straights["LLL4"].l;

  // negative s
  ler_breakpoints[7] = -straights["LLR1"].l;
  ler_breakpoints[8] = ler_breakpoints[7] - bendings["BC1RP"].rt * bendings["BC1RP"].dphi;
  ler_breakpoints[9] = ler_breakpoints[8] - straights["LLR2"].l;
  ler_breakpoints[10] = ler_breakpoints[9] - bendings["BLCWRP"].rt * bendings["BLCWRP"].dphi;
  ler_breakpoints[11] = ler_breakpoints[10] - straights["LLR3"].l;
  ler_breakpoints[12] = ler_breakpoints[11] - bendings["BLC1RP"].rt * bendings["BLC1RP"].dphi;
  ler_breakpoints[13] = ler_breakpoints[12] - straights["LLR4"].l;
  ler_breakpoints[14] = ler_breakpoints[13] - bendings["BLC2RP"].rt * bendings["BLC2RP"].dphi;
  ler_breakpoints[15] = ler_breakpoints[14] - straights["LLR5"].l;

  double dx = 0;
  double dz = 0;
  double phi = 0;
  if (accRing == c_LER) {
    // LER
    // positive s
    if (400.0 * Unit::cm < s) {
      if (s < ler_breakpoints[0]) {
        phi = straights["LLL1"].phi;
        dx = straights["LLL1"].x0 + s * sin(phi);
        dz = straights["LLL1"].z0 + s * cos(phi);
      } else if (s < ler_breakpoints[1]) {
        double sloc = s - ler_breakpoints[0];
        phi = bendings["BC1LP"].sphi + sloc / bendings["BC1LP"].rt;
        // Torus is created in x-y plain.
        // It is then rotated to x-z plain,
        // and its direction changes to reversed,
        // thus phi_real=-phi_xml
        phi = -phi;
        dx = bendings["BC1LP"].x0 + bendings["BC1LP"].rt * cos(-phi);
        dz = bendings["BC1LP"].z0 + bendings["BC1LP"].rt * sin(-phi);
      } else if (s < ler_breakpoints[2]) {
        double sloc = s - ler_breakpoints[1];
        phi = straights["LLL2"].phi;
        dx = straights["LLL2"].x0 + sloc * sin(phi);
        dz = straights["LLL2"].z0 + sloc * cos(phi);
      } else if (s < ler_breakpoints[3]) {
        double sloc = s - ler_breakpoints[2];
        phi = bendings["BLC1LP"].sphi + sloc / bendings["BLC1LP"].rt;
        phi = -phi;
        dx = bendings["BLC1LP"].x0 + bendings["BLC1LP"].rt * cos(-phi);
        dz = bendings["BLC1LP"].z0 + bendings["BLC1LP"].rt * sin(-phi);
      } else if (s < ler_breakpoints[4]) {
        double sloc = s - ler_breakpoints[3];
        phi = straights["LLL3"].phi;
        dx = straights["LLL3"].x0 + sloc * sin(phi);
        dz = straights["LLL3"].z0 + sloc * cos(phi);
      } else if (s < ler_breakpoints[5]) {
        double sloc = s - ler_breakpoints[4];
        // Torus dphi may be only positive,
        // while direction of increasing |s| is sometimes negative,
        // and we need to use -s and not change phi.
        // Since we add pi to phi later,
        // we subtract it now for this element.
        phi = bendings["BLC2LP"].sphi + bendings["BLC2LP"].dphi - sloc / bendings["BLC2LP"].rt;
        phi = -phi;
        dx = bendings["BLC2LP"].x0 + bendings["BLC2LP"].rt * cos(-phi);
        dz = bendings["BLC2LP"].z0 + bendings["BLC2LP"].rt * sin(-phi);
        phi -= M_PI;
      } else if (s < ler_breakpoints[6]) {
        double sloc = s - ler_breakpoints[5];
        phi = straights["LLL4"].phi;
        dx = straights["LLL4"].x0 + sloc * sin(phi);
        dz = straights["LLL4"].z0 + sloc * cos(phi);
      }
      // For this direction rotation angle of elements changes to negative,
      // while SAD coordinates keep orientation.
      // We need to compensate.
      phi += M_PI;
    }
    // negative s
    else if (s < -400.0 * Unit::cm) {
      if (s > ler_breakpoints[7]) {
        double sloc = -s;
        phi = straights["LLR1"].phi;
        dx = straights["LLR1"].x0 + sloc * sin(phi);
        dz = straights["LLR1"].z0 + sloc * cos(phi);
      } else if (s > ler_breakpoints[8]) {
        double sloc = ler_breakpoints[7] - s;
        phi = bendings["BC1RP"].sphi + bendings["BC1RP"].dphi - sloc / bendings["BC1RP"].rt;
        phi = -phi;
        dx = bendings["BC1RP"].x0 + bendings["BC1RP"].rt * cos(-phi);
        dz = bendings["BC1RP"].z0 + bendings["BC1RP"].rt * sin(-phi);
        phi += M_PI;
      } else if (s > ler_breakpoints[9]) {
        double sloc = ler_breakpoints[8] - s;
        phi = straights["LLR2"].phi;
        dx = straights["LLR2"].x0 + sloc * sin(phi);
        dz = straights["LLR2"].z0 + sloc * cos(phi);
      } else if (s > ler_breakpoints[10]) {
        double sloc = ler_breakpoints[9] - s;
        phi = bendings["BLCWRP"].sphi + bendings["BLCWRP"].dphi - sloc / bendings["BLCWRP"].rt;
        phi = -phi;
        dx = bendings["BLCWRP"].x0 + bendings["BLCWRP"].rt * cos(-phi);
        dz = bendings["BLCWRP"].z0 + bendings["BLCWRP"].rt * sin(-phi);
        phi += M_PI;
      } else if (s > ler_breakpoints[11]) {
        double sloc = ler_breakpoints[10] - s;
        phi = straights["LLR3"].phi;
        dx = straights["LLR3"].x0 + sloc * sin(phi);
        dz = straights["LLR3"].z0 + sloc * cos(phi);
      } else if (s > ler_breakpoints[12]) {
        double sloc = ler_breakpoints[11] - s;
        phi = bendings["BLC1RP"].sphi + bendings["BLC1RP"].dphi - sloc / bendings["BLC1RP"].rt;
        phi = -phi;
        dx = bendings["BLC1RP"].x0 + bendings["BLC1RP"].rt * cos(-phi);
        dz = bendings["BLC1RP"].z0 + bendings["BLC1RP"].rt * sin(-phi);
        phi += M_PI;
      } else if (s > ler_breakpoints[13]) {
        double sloc = ler_breakpoints[12] - s;
        phi = straights["LLR4"].phi;
        dx = straights["LLR4"].x0 + sloc * sin(phi);
        dz = straights["LLR4"].z0 + sloc * cos(phi);
      } else if (s > ler_breakpoints[14]) {
        double sloc = ler_breakpoints[13] - s;
        phi = bendings["BLC2RP"].sphi + sloc / bendings["BLC2RP"].rt;
        phi = -phi;
        dx = bendings["BLC2RP"].x0 + bendings["BLC2RP"].rt * cos(-phi);
        dz = bendings["BLC2RP"].z0 + bendings["BLC2RP"].rt * sin(-phi);
      } else if (s > ler_breakpoints[15]) {
        double sloc = ler_breakpoints[14] - s;
        phi = straights["LLR5"].phi;
        dx = straights["LLR5"].x0 + sloc * sin(phi);
        dz = straights["LLR5"].z0 + sloc * cos(phi);
      }
    }
  }
  if (accRing == c_HER) {
    // HER
    // positive s
    if (400.0 * Unit::cm < s) {
      if (s < her_breakpoints[0]) {
        phi = straights["LHL1"].phi;
        dx = straights["LHL1"].x0 + s * sin(phi);
        dz = straights["LHL1"].z0 + s * cos(phi);
      } else if (s < her_breakpoints[1]) {
        double sloc = s - her_breakpoints[0];
        phi = bendings["BLC1LE"].sphi + sloc / bendings["BLC1LE"].rt;
        phi = -phi;
        dx = bendings["BLC1LE"].x0 + bendings["BLC1LE"].rt * cos(-phi);
        dz = bendings["BLC1LE"].z0 + bendings["BLC1LE"].rt * sin(-phi);
      } else if (s < her_breakpoints[2]) {
        double sloc = s - her_breakpoints[1];
        phi = straights["LHL2"].phi;
        dx = straights["LHL2"].x0 + sloc * sin(phi);
        dz = straights["LHL2"].z0 + sloc * cos(phi);
      }
      phi += M_PI;
    }
    // negative s
    else if (s < -400.0 * Unit::cm) {
      if (s > her_breakpoints[3]) {
        double sloc = -s;
        phi = straights["LHR1"].phi;
        dx = straights["LHR1"].x0 + sloc * sin(phi);
        dz = straights["LHR1"].z0 + sloc * cos(phi);
      } else if (s > her_breakpoints[4]) {
        double sloc = her_breakpoints[3] - s;
        phi = bendings["BLC2RE"].sphi + sloc / bendings["BLC2RE"].rt;
        phi = -phi;
        dx = bendings["BLC2RE"].x0 + bendings["BLC2RE"].rt * cos(-phi);
        dz = bendings["BLC2RE"].z0 + bendings["BLC2RE"].rt * sin(-phi);
      } else if (s > her_breakpoints[5]) {
        double sloc = her_breakpoints[4] - s;
        phi = straights["LHR2"].phi;
        dx = straights["LHR2"].x0 + sloc * sin(phi);
        dz = straights["LHR2"].z0 + sloc * cos(phi);
      }
    }
  }

  TGeoHMatrix matrix("SADTrafo");
  matrix.RotateY(phi / Unit::deg);
  matrix.SetDx(dx);
  matrix.SetDz(dz);
  return matrix;
}

