/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Marko Staric                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <TClonesArray.h>
#include <TDatabasePDG.h>

#include <iostream>
#include <iomanip>
#include <stdexcept>

using namespace Belle2;

Particle::Particle() :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0),
  m_arrayPointer(0)
{
  resetErrorMatrix();
}

Particle::Particle(const TLorentzVector& momentum, const int pdgCode) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0),
  m_arrayPointer(0)
{
  m_pdgCode = pdgCode;
  setFlavorType();
  set4Vector(momentum);
  resetErrorMatrix();
}

Particle::Particle(const TLorentzVector& momentum,
                   const int pdgCode,
                   EFlavorType flavorType,
                   const EParticleType type,
                   const unsigned mdstIndex) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0),
  m_arrayPointer(0)
{
  m_pdgCode = pdgCode;
  m_flavorType = flavorType;
  m_mdstIndex = mdstIndex;
  m_particleType = type;
  set4Vector(momentum);
  resetErrorMatrix();
}

Particle::Particle(const TLorentzVector& momentum,
                   const int pdgCode,
                   EFlavorType flavorType,
                   const std::vector<int>& daughterIndices,
                   TClonesArray* arrayPointer) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0),
  m_arrayPointer(arrayPointer)
{
  m_pdgCode = pdgCode;
  m_flavorType = flavorType;
  set4Vector(momentum);
  resetErrorMatrix();

  if (!daughterIndices.empty()) {
    m_particleType    = c_Composite;
    m_daughterIndices = daughterIndices;
  }
}


Particle::Particle(const Track* track,
                   const Const::ChargedStable& chargedStable) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0),
  m_arrayPointer(0)
{
  if (!track) return;
  const TrackFitResult* trackFit = track->getTrackFitResult(chargedStable);
  if (!trackFit) return;

  m_flavorType = c_Flavored; //tracks are charged
  m_particleType = c_Track;

  m_mdstIndex = track->getArrayIndex();

  // set PDG code TODO: ask Anze why this procedure is needed?
  int absPDGCode = chargedStable.getPDGCode();
  int signFlip = 1;
  if (absPDGCode < Const::muon.getPDGCode() + 1) signFlip = -1;
  m_pdgCode = chargedStable.getPDGCode() * signFlip * trackFit->getChargeSign();

  // set mass
  if (TDatabasePDG::Instance()->GetParticle(m_pdgCode) == NULL)
    B2FATAL("PDG=" << m_pdgCode << " ***code unknown to TDatabasePDG");
  m_mass = TDatabasePDG::Instance()->GetParticle(m_pdgCode)->Mass() ;

  // set momentum, position and error matrix
  setMomentumPositionErrorMatrix(trackFit);
}

Particle::Particle(const int trackArrayIndex,
                   const TrackFitResult* trackFit,
                   const Const::ChargedStable& chargedStable) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0),
  m_arrayPointer(0)
{
  if (!trackFit) return;

  m_flavorType = c_Flavored; //tracks are charged
  m_particleType = c_Track;

  m_mdstIndex = trackArrayIndex;

  int absPDGCode = chargedStable.getPDGCode();
  int signFlip = 1;
  if (absPDGCode < Const::muon.getPDGCode() + 1) signFlip = -1;
  m_pdgCode = chargedStable.getPDGCode() * signFlip * trackFit->getChargeSign();

  // set mass
  if (TDatabasePDG::Instance()->GetParticle(m_pdgCode) == NULL)
    B2FATAL("PDG=" << m_pdgCode << " ***code unknown to TDatabasePDG");
  m_mass = TDatabasePDG::Instance()->GetParticle(m_pdgCode)->Mass() ;

  // set momentum, position and error matrix
  setMomentumPositionErrorMatrix(trackFit);
}

Particle::Particle(const ECLCluster* eclCluster) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0),
  m_arrayPointer(0)
{
  if (!eclCluster) return;

  // TODO: avoid hard coded values
  m_pdgCode = 22;

  m_px = eclCluster->getPx();
  m_py = eclCluster->getPy();
  m_pz = eclCluster->getPz();

  setVertex(eclCluster->getPosition());

  m_particleType = c_ECLCluster;
  m_mdstIndex = eclCluster->getArrayIndex();

  // set Chi^2 probability:
  //TODO: gamma quality can be written here
  m_pValue = 1;

  // set error matrix
  storeErrorMatrix(eclCluster->getError7x7());
}

Particle::Particle(const KLMCluster* klmCluster) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0),
  m_arrayPointer(0)
{
  if (!klmCluster) return;

  // TODO: avoid hard coded values
  m_pdgCode = 130;

  set4Vector(klmCluster->getMomentum());
  setVertex(klmCluster->getPosition());

  m_particleType = c_KLMCluster;
  m_mdstIndex = klmCluster->getArrayIndex();

  // set Chi^2 probability:
  //TODO: gamma quality can be written here
  m_pValue = -1;

  // TODO: set error matrix
  resetErrorMatrix();
  //storeErrorMatrix(klmCluster->???);
}

Particle::Particle(const MCParticle* mcParticle) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0),
  m_arrayPointer(0)
{
  if (!mcParticle) return;

  m_pdgCode      = mcParticle->getPDG();
  m_particleType = c_MCParticle; // TODO: what about daughters if not FS particle?

  m_mdstIndex = mcParticle->getArrayIndex();

  setFlavorType();

  // mass and momentum
  m_mass = mcParticle->getMass();
  m_px = mcParticle->getMomentum().Px();
  m_py = mcParticle->getMomentum().Py();
  m_pz = mcParticle->getMomentum().Pz();
  // production vertex
  // TODO: good only for FS particles, for composite we must use decay vertex
  setVertex(mcParticle->getVertex());

  resetErrorMatrix();
}


Particle::~Particle()
{
}


void Particle::setMomentumVertexErrorMatrix(const TMatrixFSym& m)
{
  // check if provided Error Matrix is of dimension 7x7
  // if not, reset the error matrix and print warning
  if (m.GetNrows() != c_DimMatrix || m.GetNcols() != c_DimMatrix) {
    resetErrorMatrix();
    B2WARNING("Error Matrix is not 7x7 ");
    return;
  }
  storeErrorMatrix(m);
}


TMatrixFSym Particle::getMomentumVertexErrorMatrix() const
{
  TMatrixFSym m(c_DimMatrix);

  int element = 0;
  for (int irow = 0; irow < c_DimMatrix; irow++) {
    for (int icol = irow; icol < c_DimMatrix; icol++) {
      m(irow, icol) = m(icol, irow) = m_errMatrix[element];
      element++;
    }
  }
  return m;
}


TMatrixFSym Particle::getMomentumErrorMatrix() const
{
  TMatrixFSym m_mom;
  TMatrixFSym m_full = getMomentumVertexErrorMatrix();

  // get 4x4 (momentum) submatrix from the full error matrix
  // momentum related elements are in [0,...,3]x[0,...,3] block
  m_full.GetSub(0, 3, m_mom, "S");

  return m_mom;
}

TMatrixFSym Particle::getVertexErrorMatrix() const
{
  TMatrixFSym m_pos;
  TMatrixFSym m_full = getMomentumVertexErrorMatrix();

  // get 3x3 (position) submatrix from the full error matrix
  // vertex related elements are in [4,5,6]x[4,5,6] block
  m_full.GetSub(4, 6, m_pos, "S");

  return m_pos;
}

/*
float Particle::getMassError(void) const
{
  float result = 0.0;

  if(m_pValue<0)
    return result;

  float invMass = getMass();

  TMatrixFSym covarianceMatrix = getMomentumErrorMatrix();
  TVectorF    jacobian(c_DimMomentum);
  jacobian[0] = -1.0*getPx()/invMass;
  jacobian[1] = -1.0*getPy()/invMass;
  jacobian[2] = -1.0*getPz()/invMass;
  jacobian[3] =  1.0*getEnergy()/invMass;

  result = jacobian * (covarianceMatrix * jacobian);

  covarianceMatrix.Print();

  if(result<0.0)
    result = 0.0;

  return TMath::Sqrt(result);
}
*/

float Particle::getPDGMass(void) const
{
  if (TDatabasePDG::Instance()->GetParticle(m_pdgCode) == NULL) {
    B2ERROR("PDG=" << m_pdgCode << " ***code unknown to TDatabasePDG");
    return 0.0;
  }
  return TDatabasePDG::Instance()->GetParticle(m_pdgCode)->Mass();
}

float Particle::getCharge(void) const
{
  if (TDatabasePDG::Instance()->GetParticle(m_pdgCode) == NULL) {
    B2ERROR("PDG=" << m_pdgCode << " ***code unknown to TDatabasePDG");
    return 0.0;
  }
  return TDatabasePDG::Instance()->GetParticle(m_pdgCode)->Charge() / 3.0;
}

const Particle* Particle::getDaughter(unsigned i) const
{
  if (!m_arrayPointer) fixArrayPointer();
  if (!m_arrayPointer) return NULL; // fixing failed

  if (i >= getNDaughters()) return NULL;
  return static_cast<Particle*>(m_arrayPointer->At(m_daughterIndices[i]));
}

const std::vector<Belle2::Particle*> Particle::getDaughters() const
{
  std::vector<Particle*> daughters(getNDaughters());

  if (!m_arrayPointer) fixArrayPointer();
  if (!m_arrayPointer) return daughters; // fixing failed

  for (unsigned i = 0; i < getNDaughters(); i++)
    daughters[i] = static_cast<Particle*>(m_arrayPointer->At(m_daughterIndices[i]));

  return daughters;
}

const std::vector<const Belle2::Particle*> Particle::getFinalStateDaughters() const
{
  std::vector<const Particle*> fspDaughters;
  fillFSPDaughters(fspDaughters);

  return fspDaughters;
}

std::vector<int> Particle::getMdstArrayIndices(EParticleType type) const
{
  std::vector<int> mdstIndices;
  std::vector<const Particle*> fspDaughters = this->getFinalStateDaughters();

  for (unsigned i = 0; i < fspDaughters.size(); i++) {
    const Particle* fsp = fspDaughters[i];

    // is this FSP daughter constructed from given MDST type
    if (fsp->getParticleType() == type)
      mdstIndices.push_back(fsp->getMdstArrayIndex());
  }
  return mdstIndices;
}


void Particle::appendDaughter(const Particle* daughter)
{
  // it's a composite particle
  m_particleType = c_Composite;

  // add daughter index
  m_daughterIndices.push_back(daughter->getArrayIndex());
}

void Particle::removeDaughter(const Particle* daughter)
{
  if (getNDaughters() == 0)
    return;

  for (unsigned i = 0; i < getNDaughters(); i++) {
    if (m_daughterIndices[i] == daughter->getArrayIndex()) {
      m_daughterIndices.erase(m_daughterIndices.begin() + i);
      i--;
    }
  }

  if (getNDaughters() == 0)
    m_particleType = c_Undefined;
}

bool Particle::overlapsWith(const Particle* oParticle) const
{
  // obtain vectors of daughter final state particles
  std::vector<const Particle*> thisFSPs  = this->getFinalStateDaughters();
  std::vector<const Particle*> otherFSPs = oParticle->getFinalStateDaughters();

  // check if they share any of the FSPs
  for (unsigned tFSP = 0; tFSP < thisFSPs.size(); tFSP++)
    for (unsigned oFSP = 0; oFSP < otherFSPs.size(); oFSP++)
      if (thisFSPs[tFSP]->getParticleType() == otherFSPs[oFSP]->getParticleType()
          && thisFSPs[tFSP]->getMdstArrayIndex() == otherFSPs[oFSP]->getMdstArrayIndex())
        return true;

  return false;
}

const Track* Particle::getTrack() const
{
  if (m_particleType == c_Track) {
    StoreArray<Track> tracks;
    return tracks[m_mdstIndex];
  } else
    return nullptr;
}

const ECLCluster* Particle::getECLCluster() const
{
  if (m_particleType == c_ECLCluster) {
    StoreArray<ECLCluster> eclClusters;
    return eclClusters[m_mdstIndex];
  } else if (m_particleType == c_Track) {
    StoreArray<Track> tracks;
    return tracks[m_mdstIndex]->getRelatedTo<ECLCluster>();
  } else
    return nullptr;
}

const KLMCluster* Particle::getKLMCluster() const
{
  if (m_particleType == c_KLMCluster) {
    StoreArray<KLMCluster> klmClusters;
    return klmClusters[m_mdstIndex];
  } else
    return nullptr;
}

const MCParticle* Particle::getMCParticle() const
{
  if (m_particleType == c_MCParticle) {
    StoreArray<MCParticle> mcParticles;
    return mcParticles[m_mdstIndex];
  } else
    return nullptr;
}

//--- private methods --------------------------------------------

void Particle::setMomentumPositionErrorMatrix(const TrackFitResult* trackFit)
{
  // set momenum
  m_px = trackFit->getMomentum().Px();
  m_py = trackFit->getMomentum().Py();
  m_pz = trackFit->getMomentum().Pz();

  // set position at which the momentum is given (= POCA)
  setVertex(trackFit->getPosition());

  // set Chi^2 probability
  m_pValue = trackFit->getPValue();

  // set error matrix
  TMatrixF cov6(trackFit->getCovariance6());
  unsigned order[] = {c_X, c_Y, c_Z, c_Px, c_Py, c_Pz};

  TMatrixFSym errMatrix(c_DimMatrix);
  for (int i = 0; i < 6; i++) {
    for (int j = i; j < 6; j++) {
      // although it seems to make no sense to fill all elements of the
      // symetric matrix, it has to be (do not touch this code)
      errMatrix(order[j], order[i]) = errMatrix(order[i], order[j]) = cov6(i, j);
    }
  }

  /*
     E = sqrt(px^2 + py^2 + pz^2 + m^2) thus:
     cov(x,E)  = cov(px,x) *dE/dpx + cov(py,x) *dE/dpy + cov(pz,x) *dE/dpz
     cov(y,E)  = cov(px,y) *dE/dpx + cov(py,y) *dE/dpy + cov(pz,y) *dE/dpz
     cov(z,E)  = cov(px,z) *dE/dpx + cov(py,z) *dE/dpy + cov(pz,z) *dE/dpz
     cov(px,E) = cov(px,px)*dE/dpx + cov(px,py)*dE/dpy + cov(px,pz)*dE/dpz
     cov(py,E) = cov(py,px)*dE/dpx + cov(py,py)*dE/dpy + cov(py,pz)*dE/dpz
     cov(pz,E) = cov(pz,px)*dE/dpx + cov(pz,py)*dE/dpy + cov(pz,pz)*dE/dpz
     cov(E,E)  = cov(px,px)*(dE/dpx)^2 + cov(py,py)*(dE/dpy)^2 + cov(pz,pz)*(dE/dpz)^2
               + 2*cov(px,py)*dE/dpx*dE/dpy
               + 2*cov(py,pz)*dE/dpy*dE/dpz
               + 2*cov(pz,px)*dE/dpz*dE/dpx
     dE/dpx = px/E etc.
  */

  float E = getEnergy();
  float dEdp[] = {m_px / E, m_py / E, m_pz / E};
  unsigned compMom[] = {c_Px, c_Py, c_Pz};
  unsigned compPos[] = {c_X,  c_Y,  c_Z};

  // covariances (p,E)
  for (int i = 0; i < 3; i++) {
    float Cov = 0;
    for (int k = 0; k < 3; k++) {
      Cov += errMatrix(compMom[i], compMom[k]) * dEdp[k];
    }
    errMatrix(compMom[i], c_E) = Cov;
  }

  // covariances (x,E)
  for (int i = 0; i < 3; i++) {
    float Cov = 0;
    for (int k = 0; k < 3; k++) {
      Cov += errMatrix(compPos[i], compMom[k]) * dEdp[k];
    }
    errMatrix(c_E, compPos[i]) = Cov;
  }

  // variance (E,E)
  float Cov = 0;
  for (int i = 0; i < 3; i++) {
    Cov += errMatrix(compMom[i], compMom[i]) * dEdp[i] * dEdp[i];
  }
  for (int i = 0; i < 3; i++) {
    int k = (i + 1) % 3;
    Cov += 2 * errMatrix(compMom[i], compMom[k]) * dEdp[i] * dEdp[k];
  }
  errMatrix(c_E, c_E) = Cov;

  storeErrorMatrix(errMatrix);
}

void Particle::resetErrorMatrix()
{
  for (int i = 0; i < c_SizeMatrix; i++)
    m_errMatrix[i] = 0.0;
}

void Particle::storeErrorMatrix(const TMatrixFSym& m)
{
  int element = 0;
  for (int irow = 0; irow < c_DimMatrix; irow++) {
    for (int icol = irow; icol < c_DimMatrix; icol++) {
      m_errMatrix[element] = m(irow, icol);
      element++;
    }
  }
}

void Particle::fixArrayPointer() const
{

  TClonesArray* arrayPointer(0);

  std::string arrayName = getArrayName();
  if (arrayName.empty())
    B2ERROR("Particle::fixArrayPointer particle does not belong to a StoreArray");

  StoreArray<Particle> Particles(arrayName);
  arrayPointer = Particles.getPtr();

  //Set the StoreArray pointer for all elements
  for (int i = 0; i < arrayPointer->GetEntriesFast(); i++) {
    Particle& p = *(static_cast<Particle*>(arrayPointer->At(i)));
    p.m_arrayPointer = arrayPointer;
  }
}


void Particle::fillFSPDaughters(std::vector<const Belle2::Particle*>& fspDaughters) const
{
  // this is FSP
  if (getNDaughters() == 0) {
    fspDaughters.push_back(this);
    return;
  }

  // this is not FSP (go one level down)
  for (unsigned i = 0; i < getNDaughters(); i++)
    getDaughter(i)->fillFSPDaughters(fspDaughters);
}


void Particle::setFlavorType()
{
  m_flavorType = c_Flavored;
  if (m_pdgCode < 0) return;
  if (m_pdgCode == 22) {m_flavorType = c_Unflavored; return;} // gamma
  if (m_pdgCode == 310) {m_flavorType = c_Unflavored; return;} // K_s
  if (m_pdgCode == 130) {m_flavorType = c_Unflavored; return;} // K_L
  int nnn = m_pdgCode / 10;
  int q3 = nnn % 10; nnn /= 10;
  int q2 = nnn % 10; nnn /= 10;
  int q1 = nnn % 10;
  if (q1 == 0 && q2 == q3) m_flavorType = c_Unflavored; // unflavored meson
}


void Particle::print() const
{
  std::cout << "Particle: collection=";
  std::cout << getArrayName();
  std::cout << " PDGCode=" << m_pdgCode;
  std::cout << " Charge=" << getCharge();
  std::cout << " PDGMass=" << getPDGMass();
  std::cout << " flavorType=" << m_flavorType;
  std::cout << " particleType=" << m_particleType;
  std::cout << std::endl;

  std::cout << " mdstIndex=" << m_mdstIndex;
  std::cout << " arrayIndex=" << getArrayIndex();
  std::cout << " daughterIndices: ";
  for (unsigned i = 0; i < m_daughterIndices.size(); i++) {
    std::cout << m_daughterIndices[i] << ", ";
  }
  if (m_daughterIndices.empty()) std::cout << " (none)";
  std::cout << std::endl;

  if (!m_daughterIndices.empty()) {
    std::cout << " daughter PDGCodes: ";
    for (unsigned i = 0; i < m_daughterIndices.size(); i++) {
      const Particle* p = getDaughter(i);
      if (p) {std::cout << p->getPDGCode() << ", ";}
      else {std::cout << "?, ";}
    }
    std::cout << std::endl;
  }

  std::cout << " mass=" << m_mass;
  std::cout << std::endl;

  std::cout << " momentum=(";
  std::cout << m_px << "," << m_py << "," << m_pz << ")";
  std::cout << " p=" << getP();
  std::cout << std::endl;

  std::cout << " position=(";
  std::cout << m_x << "," << m_y << "," << m_z << ")";
  std::cout << std::endl;

  std::cout << " p-value of fit (if done): ";
  std::cout << m_pValue;
  std::cout << std::endl;

  std::cout << " error matrix:";
  std::cout << std::endl;

  TMatrixFSym errMatrix = getMomentumVertexErrorMatrix();
  int prec = std::cout.precision();
  std::cout << std::setprecision(4);
  for (int i = 0; i < errMatrix.GetNrows(); i++) {
    for (int k = 0; k < errMatrix.GetNcols(); k++) {
      std::cout << std::setw(11) << errMatrix(i, k);
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
  std::cout << std::setprecision(prec);

  std::cout << " extra info=( ";
  if (!m_extraInfo.empty()) {
    StoreObjPtr<ParticleExtraInfoMap> extraInfoMap;
    if (!extraInfoMap) {
      B2FATAL("ParticleExtraInfoMap not available, but needed for storing extra info in Particle!");
    }
    const ParticleExtraInfoMap::IndexMap& map = extraInfoMap->getMap(m_extraInfo[0]);
    const unsigned int nVars = m_extraInfo.size();
    for (const auto & pair : map) {
      if (pair.second < nVars) {
        std::cout << pair.first << "=" << m_extraInfo[pair.second] << " ";
      }
    }

  }
  std::cout << ") " << std::endl;

}

bool Particle::hasExtraInfo(const std::string& name) const
{
  if (m_extraInfo.empty())
    return false;

  //get index for name
  const unsigned int mapID = (unsigned int)m_extraInfo[0];
  StoreObjPtr<ParticleExtraInfoMap> extraInfoMap;
  if (!extraInfoMap) {
    B2FATAL("ParticleExtraInfoMap not available, but needed for storing extra info in Particle!");
  }
  unsigned int index = extraInfoMap->getIndex(mapID, name);
  if (index == 0 or index >= m_extraInfo.size()) //actualy indices start at 1
    return false;

  return true;
}

void Particle::removeExtraInfo()
{
  m_extraInfo.clear();
}

float Particle::getExtraInfo(const std::string& name) const
{
  if (m_extraInfo.empty())
    throw std::runtime_error(std::string("getExtraInfo: Value '") + name + "' not found in Particle!");

  //get index for name
  const unsigned int mapID = (unsigned int)m_extraInfo[0];
  StoreObjPtr<ParticleExtraInfoMap> extraInfoMap;
  if (!extraInfoMap) {
    B2FATAL("ParticleExtraInfoMap not available, but needed for storing extra info in Particle!");
  }
  unsigned int index = extraInfoMap->getIndex(mapID, name);
  if (index == 0 or index >= m_extraInfo.size()) //actualy indices start at 1
    throw std::runtime_error(std::string("getExtraInfo: Value '") + name + "' not found in Particle!");

  return m_extraInfo[index];

}


void Particle::setExtraInfo(const std::string& name, float value)
{
  if (m_extraInfo.empty())
    throw std::runtime_error(std::string("setExtraInfo: Value '") + name + "' not found in Particle!");

  //get index for name
  const unsigned int mapID = (unsigned int)m_extraInfo[0];
  StoreObjPtr<ParticleExtraInfoMap> extraInfoMap;
  if (!extraInfoMap) {
    B2FATAL("ParticleExtraInfoMap not available, but needed for storing extra info in Particle!");
  }
  unsigned int index = extraInfoMap->getIndex(mapID, name);
  if (index == 0 or index >= m_extraInfo.size()) //actualy indices start at 1
    throw std::runtime_error(std::string("setExtraInfo: Value '") + name + "' not found in Particle!");

  m_extraInfo[index] = value;

}

void Particle::addExtraInfo(const std::string& name, float value)
{
  if (hasExtraInfo(name))
    throw std::runtime_error(std::string("addExtraInfo: Value '") + name + "' already set!");

  StoreObjPtr<ParticleExtraInfoMap> extraInfoMap;
  if (!extraInfoMap)
    extraInfoMap.create();
  if (m_extraInfo.empty()) {
    unsigned int mapID = extraInfoMap->getMapForNewVar(name);
    m_extraInfo.push_back(mapID);
    m_extraInfo.push_back(value);
  } else {
    unsigned int oldMapID = m_extraInfo[0];
    unsigned int insertIndex = m_extraInfo.size();
    unsigned int mapID = extraInfoMap->getMapForNewVar(name, oldMapID, insertIndex);

    m_extraInfo[0] = mapID; //update map
    m_extraInfo.push_back(value); //add value
  }
}


ClassImp(Particle);
