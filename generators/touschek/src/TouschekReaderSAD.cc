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
#include <framework/datastore/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

using namespace std;
using namespace Belle2;


TouschekReaderSAD::TouschekReaderSAD(TGeoHMatrix* transMatrix)
{
  m_file = NULL;
  m_tree = NULL;
  m_transMatrix = transMatrix;
  m_readEntry = 0;
}


TouschekReaderSAD::~TouschekReaderSAD()
{
  if (m_file != NULL) m_file->Close();
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


int TouschekReaderSAD::getParticles(int number, double sRange, double beamEnergy, int pdg, MCParticleGraph &graph)
{
  if (m_tree == NULL) return 0;

  double particlePosTouschek[3] = {0.0, 0.0, 0.0};
  double particlePosGeant4[3] = {0.0, 0.0, 0.0};
  double particleMomTouschek[3] = {0.0, 0.0, 0.0};
  double particleMomGeant4[3] = {0.0, 0.0, 0.0};

  int iEntry = 0;
  while (((iEntry < number) || (number < 0)) && (m_readEntry < m_tree->GetEntries())) {
    m_tree->GetEntry(m_readEntry);

    double lostX = m_lostX * Unit::m;
    double lostY = m_lostY * Unit::m;
    double lostS = m_lostS * Unit::m;

    //Check s range
    if (fabs(lostS) > sRange) {
      m_readEntry++;
      continue;
    }

    //Add particles to MCParticle collection
    MCParticleGraph::GraphParticle &particle = graph.addParticle();
    particle.setStatus(MCParticle::c_PrimaryParticle);
    particle.setPDG(pdg);
    particle.setMassFromPDG();

    //Convert the position of the particle from local Touschek space to global geant4 space.
    //Flip the sign for the y and z component to go from the accelerator to the detector coordinate system
    particlePosTouschek[0] = lostX;
    particlePosTouschek[1] = -lostY;
    particlePosTouschek[2] = -lostS;

    m_transMatrix->LocalToMaster(particlePosTouschek, particlePosGeant4);

    //Convert the momentum of the particle from local Touschek space to global geant4 space.
    //Flip the sign for the y and z component to go from the accelerator to the detector coordinate system
    //Calculate the missing pz by using the nominal beam energy
    double totalMomSqr = (beamEnergy * beamEnergy) - (particle.getMass() * particle.getMass());
    particleMomTouschek[0] = m_lostPx * Unit::GeV;
    particleMomTouschek[1] = -m_lostPy * Unit::GeV;
    particleMomTouschek[2] = -sqrt(totalMomSqr - (m_lostPx * m_lostPx) - (m_lostPy * m_lostPy));
    m_transMatrix->LocalToMasterVect(particleMomTouschek, particleMomGeant4);

    //Set missing particle information
    particle.setMomentum(TVector3(particleMomGeant4));
    particle.setProductionVertex(TVector3(particlePosGeant4));
    particle.setProductionTime(0.0);
    particle.setEnergy(beamEnergy);
    particle.setValidVertex(true);

    //setting weight of track as event weight, if only one track is in event:
    if (number == 1) {
      StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
      eventMetaDataPtr->setGeneratedWeight(m_lostW);
    }

    iEntry++;
    m_readEntry++;
  }

  return iEntry;
}
