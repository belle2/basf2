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
#include <generators/dataobjects/MCParticle.h>

using namespace std;
using namespace Belle2;


ReaderSAD::ReaderSAD(): m_file(NULL), m_tree(NULL), m_transMatrix(NULL),
  m_sRange(300.0), m_accRing(ReaderSAD::c_LER), m_pxRes(0.01), m_pyRes(0.01),
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
    //Check for end of file
    if (m_readEntry >= m_tree->GetEntries()) throw SADEndOfFile();

    //Load the SAD particle
    m_tree->GetEntry(m_readEntry);
    convertParamsToSADUnits();

    B2DEBUG(10, "> Read particle " << m_readEntry + 1 << "/" << m_tree->GetEntries() << " with s = " << m_lostS << " cm" << " and rate = " << m_lostRate << " Hz")
    m_readEntry++;
  } while (fabs(m_lostS) > m_sRange);

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

  m_transMatrix->LocalToMaster(particlePosSAD, particlePosGeant4);

  //Convert the momentum of the particle from local SAD space to global geant4 space.
  //Flip the sign for the y and z component to go from the accelerator to the detector coordinate system.
  //Calculate the missing pz by using the energy of the particle at the position where it has been lost.
  double totalMomSqr = (m_lostE * m_lostE - (particle.getMass() * particle.getMass()));

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

  m_transMatrix->LocalToMasterVect(particleMomSAD, particleMomGeant4);

  //Set missing particle information
  particle.setMomentum(TVector3(particleMomGeant4));
  particle.setProductionVertex(TVector3(particlePosGeant4));
  particle.setProductionTime(0.0);
  particle.setEnergy(m_lostE);
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
