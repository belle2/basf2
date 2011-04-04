/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/touschek/TouschekReaderSAD.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <generators/dataobjects/MCParticle.h>

using namespace std;
using namespace Belle2;


TouschekReaderSAD::TouschekReaderSAD(): m_file(NULL), m_tree(NULL), m_transMatrix(NULL),
    m_sRange(300.0), m_pdg(-11), m_beamenergy(4.0), m_pxRes(0.01), m_pyRes(0.01),
    m_touschekToRealFactor(5.76e6), m_realPartNum(0),
    m_realPartEntry(0), m_readEntry(0)
{

}


TouschekReaderSAD::~TouschekReaderSAD()
{
  if (m_file != NULL) m_file->Close();
}


void TouschekReaderSAD::initialize(TGeoHMatrix* transMatrix, double sRange, int pdg, double beamEnergy, double current, double lifetime, double readoutTime)
{
  m_transMatrix = transMatrix;
  m_sRange = sRange;
  m_pdg = pdg;
  m_beamenergy = beamEnergy;

  //Calculate the Touschek to real particle number factor (assumes the accelerator to be 2.3 km long)
  double totalCurrentLoss   = current / lifetime;
  double timePartRound      = (2.3 * Unit::km) / (Unit::speed_of_light);
  double totalLossPerRound  = totalCurrentLoss * timePartRound;
  double lossPerRound       = (totalLossPerRound / Unit::s) / 1.6e-19; //The unit is required because [A] = [C]/[s]
  m_touschekToRealFactor    = lossPerRound * readoutTime;

  B2DEBUG(10, "The Touschek particle to real particle weight factor: " << m_touschekToRealFactor)
}


void TouschekReaderSAD::open(const string& filename) throw(TouschekCouldNotOpenFileError)
{
  if (m_file != NULL) {
    m_file->Close();
    delete m_file;
  }

  m_file = new TFile(filename.c_str(), "READ");
  if (m_file == NULL) throw(TouschekCouldNotOpenFileError() << filename);

  m_file->cd("");
  m_tree = dynamic_cast<TTree*>(m_file->Get("tp"));
  if (m_tree == NULL) throw(TouschekCouldNotOpenFileError() << filename);

  m_tree->SetBranchAddress("x", &m_lostX);
  m_tree->SetBranchAddress("y", &m_lostY);
  m_tree->SetBranchAddress("s", &m_lostS);
  m_tree->SetBranchAddress("px", &m_lostPx);
  m_tree->SetBranchAddress("py", &m_lostPy);
  m_tree->SetBranchAddress("w", &m_lostW);
}


double TouschekReaderSAD::getParticle(MCParticleGraph& graph)
{
  if (m_tree == NULL) {
    B2ERROR("The SAD tree doesn't exist !")
    return -1;
  }

  //Check for end of file
  if (m_readEntry >= m_tree->GetEntries()) throw TouschekEndOfFile();

  //Check if the number of the real particles is reached
  if (m_realPartEntry >= m_realPartNum) {
    m_realPartEntry = 0;
    m_realPartNum   = 0;
  }

  //Check if a new SAD particle has to be read from the file
  if (m_realPartNum == 0) {
    //Read only SAD particles which are inside the chosen sRange
    do {
      m_tree->GetEntry(m_readEntry);
      convertParamsToSADUnits();

      //Do a Gaussian smearing of the px and py value
      m_lostPx = m_random.Gaus(m_lostPx, m_pxRes * m_lostPx); //1% px resolution
      m_lostPy = m_random.Gaus(m_lostPy, m_pyRes * m_lostPy);

      B2DEBUG(10, "> Read particle " << m_readEntry + 1 << "/" << m_tree->GetEntries() << " with s = " << m_lostS << " cm")
      m_readEntry++;
    } while ((fabs(m_lostS) > m_sRange) && (m_readEntry <= m_tree->GetEntries()));

    m_realPartNum = calculateRealParticleNumber(m_lostW);
  }

  //Create a new real particle from the SAD particle
  if ((fabs(m_lostS) <= m_sRange) && (m_realPartNum > 0)) {
    addParticleToMCParticles(graph);
    B2DEBUG(10, "* Created real particle " << m_realPartEntry + 1 << "/" << m_realPartNum << " for SAD particle " << m_readEntry << "/" << m_tree->GetEntries())
  }

  m_realPartEntry++;
  return m_lostW;
}


void TouschekReaderSAD::addAllSADParticles(MCParticleGraph& graph)
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

void TouschekReaderSAD::convertParamsToSADUnits()
{
  m_lostX = m_lostX * Unit::m;
  m_lostY = m_lostY * Unit::m;
  m_lostS = m_lostS * Unit::m;
  m_lostPx = m_lostPx * Unit::GeV;
  m_lostPy = m_lostPy * Unit::GeV;
}


void TouschekReaderSAD::addParticleToMCParticles(MCParticleGraph& graph)
{
  double particlePosTouschek[3] = {0.0, 0.0, 0.0};
  double particlePosGeant4[3] = {0.0, 0.0, 0.0};
  double particleMomTouschek[3] = {0.0, 0.0, 0.0};
  double particleMomGeant4[3] = {0.0, 0.0, 0.0};

  //Add particle to MCParticle collection
  MCParticleGraph::GraphParticle &particle = graph.addParticle();
  particle.setStatus(MCParticle::c_PrimaryParticle);
  particle.setPDG(m_pdg);
  particle.setMassFromPDG();
  particle.setChargeFromPDG();

  //Convert the position of the particle from local Touschek space to global geant4 space.
  //Flip the sign for the y and z component to go from the accelerator to the detector coordinate system
  particlePosTouschek[0] = m_lostX;
  particlePosTouschek[1] = -m_lostY;
  particlePosTouschek[2] = -m_lostS;

  m_transMatrix->LocalToMaster(particlePosTouschek, particlePosGeant4);

  //Convert the momentum of the particle from local Touschek space to global geant4 space.
  //Flip the sign for the y and z component to go from the accelerator to the detector coordinate system
  //Calculate the missing pz by using the nominal beam energy
  double totalMomSqr = (m_beamenergy * m_beamenergy) - (particle.getMass() * particle.getMass());
  particleMomTouschek[0] = m_lostPx * Unit::GeV;
  particleMomTouschek[1] = -m_lostPy * Unit::GeV;
  particleMomTouschek[2] = -sqrt(totalMomSqr - (m_lostPx * m_lostPx) - (m_lostPy * m_lostPy));
  m_transMatrix->LocalToMasterVect(particleMomTouschek, particleMomGeant4);

  //Set missing particle information
  particle.setMomentum(TVector3(particleMomGeant4));
  particle.setProductionVertex(TVector3(particlePosGeant4));
  particle.setProductionTime(0.0);
  particle.setEnergy(m_beamenergy);
  particle.setValidVertex(true);
}


int TouschekReaderSAD::calculateRealParticleNumber(double weight)
{
  double numPart = weight * m_touschekToRealFactor;

  //For a value smaller than one do a random choice if the particle should be kept
  if (numPart < 1.0) {
    double rnd = m_random.Uniform(); //returns a random number in the interval ]0, 1]
    B2DEBUG(10, "  Real particle number is smaller than 1. Random generator output is" << rnd << " real number is " << numPart)

    if (rnd < numPart) {
      numPart = 1.0;
      B2DEBUG(10, "  => Particle is kept.")
    } else {
      numPart = 0.0;
      B2DEBUG(10, "  => Particle is discarded.")
    }
  }
  return static_cast<int>(floor(numPart + 0.5));
}
