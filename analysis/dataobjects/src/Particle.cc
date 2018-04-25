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

#include <analysis/ClusterUtility/ClusterUtils.h>

#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/HTML.h>

#include <TClonesArray.h>
#include <TDatabasePDG.h>
#include <TMatrixFSym.h>

#include <iostream>
#include <iomanip>
#include <stdexcept>

using namespace Belle2;

Particle::Particle() :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(nan("")), m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0), m_identifier(-1),
  m_arrayPointer(nullptr)
{
  resetErrorMatrix();
}

Particle::Particle(const TLorentzVector& momentum, const int pdgCode) :
  m_pdgCode(pdgCode), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0), m_identifier(-1),
  m_arrayPointer(nullptr)
{
  setFlavorType();
  set4Vector(momentum);
  resetErrorMatrix();
}

Particle::Particle(const TLorentzVector& momentum,
                   const int pdgCode,
                   EFlavorType flavorType,
                   const EParticleType type,
                   const unsigned mdstIndex) :
  m_pdgCode(pdgCode), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(flavorType), m_particleType(type),
  m_arrayPointer(nullptr)
{
  if (flavorType == c_Unflavored and pdgCode < 0)
    m_pdgCode = -pdgCode;

  setMdstArrayIndex(mdstIndex);
  set4Vector(momentum);
  resetErrorMatrix();
}

Particle::Particle(const TLorentzVector& momentum,
                   const int pdgCode,
                   EFlavorType flavorType,
                   const std::vector<int>& daughterIndices,
                   TClonesArray* arrayPointer) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1),
  m_daughterIndices(daughterIndices),
  m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0), m_identifier(-1),
  m_arrayPointer(arrayPointer)
{
  m_pdgCode = pdgCode;
  m_flavorType = flavorType;
  if (flavorType == c_Unflavored and pdgCode < 0)
    m_pdgCode = -pdgCode;
  set4Vector(momentum);
  resetErrorMatrix();

  if (!daughterIndices.empty()) {
    m_particleType    = c_Composite;
    if (getArrayPointer() == nullptr) {
      B2FATAL("Composite Particle (with daughters) was constructed outside StoreArray without specifying daughter array!");
    }
  }
}


Particle::Particle(const Track* track,
                   const Const::ChargedStable& chargedStable) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0), m_identifier(-1),
  m_arrayPointer(nullptr)
{
  if (!track) return;

  auto closestMassFitResult = track->getTrackFitResultWithClosestMass(chargedStable);
  if (closestMassFitResult == nullptr) return;

  m_pdgCodeUsedForFit = closestMassFitResult->getParticleType().getPDGCode();
  const auto trackFit = closestMassFitResult;

  m_flavorType = c_Flavored; //tracks are charged
  m_particleType = c_Track;

  setMdstArrayIndex(track->getArrayIndex());

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
                   const Const::ChargedStable& chargedStable,
                   const Const::ChargedStable& chargedStableUsedForFit) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0), m_identifier(-1),
  m_arrayPointer(nullptr)
{
  if (!trackFit) return;

  m_flavorType = c_Flavored; //tracks are charged
  m_particleType = c_Track;

  setMdstArrayIndex(trackArrayIndex);

  m_pdgCodeUsedForFit = chargedStableUsedForFit.getPDGCode();
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
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0), m_identifier(-1),
  m_arrayPointer(nullptr)
{
  if (!eclCluster) return;

  // TODO: avoid hard coded values
  m_pdgCode = 22;
  setFlavorType();

  // returns default vertex from clusterutils (from beam parameters if available)
  // leave it like that for the moment to make it transparent
  ClusterUtils C;
  const TVector3 clustervertex = C.GetIPPosition();
  setVertex(clustervertex);

  const TLorentzVector clustermom = C.Get4MomentumFromCluster(eclCluster, clustervertex);
  m_px = clustermom.Px();
  m_py = clustermom.Py();
  m_pz = clustermom.Pz();

  m_particleType = c_ECLCluster;
  setMdstArrayIndex(eclCluster->getArrayIndex());

  // set Chi^2 probability:
  //TODO: gamma quality can be written here
  m_pValue = 1;

  // Get covariance matrix of IP distribution.
  const TMatrixDSym clustervertexcovmat = C.GetIPPositionCovarianceMatrix();

  // Set error matrix.
  TMatrixDSym clustercovmat = C.GetCovarianceMatrix7x7FromCluster(eclCluster, clustervertex, clustervertexcovmat);
  storeErrorMatrix(clustercovmat);
}

Particle::Particle(const KLMCluster* klmCluster) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0), m_identifier(-1),
  m_arrayPointer(nullptr)
{
  if (!klmCluster) return;

  // TODO: avoid hard coded values
  m_pdgCode = 130;
  setFlavorType();

  set4Vector(klmCluster->getMomentum());
  setVertex(klmCluster->getPosition());

  m_particleType = c_KLMCluster;
  setMdstArrayIndex(klmCluster->getArrayIndex());

  // set Chi^2 probability:
  //TODO: KL quality can be written here
  m_pValue = -1;

  // TODO: set error matrix
  resetErrorMatrix();
  //storeErrorMatrix(klmCluster->getErrorMatrix());
}

Particle::Particle(const MCParticle* mcParticle) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleType(c_Undefined), m_mdstIndex(0), m_identifier(-1),
  m_arrayPointer(nullptr)
{
  if (!mcParticle) return;

  m_pdgCode      = mcParticle->getPDG();
  m_particleType = c_MCParticle; // TODO: what about daughters if not FS particle?

  setMdstArrayIndex(mcParticle->getArrayIndex());


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

void Particle::setMdstArrayIndex(const int arrayIndex)
{
  m_mdstIndex = arrayIndex;

  // set the identifier
  if (m_particleType == c_ECLCluster) {
    const ECLCluster* cluster = this->getECLCluster();
    if (cluster) {
      const int crid     = cluster->getConnectedRegionId();
      const int clusterid = cluster->getClusterId();
      m_identifier = 1000 * crid + clusterid;
    } else {
      B2ERROR("Particle is of type = ECLCluster has identifier not set and no relation to ECLCluster.\n"
              "This has happen because old microDST is analysed with newer version of software.");
    }
  } else {
    m_identifier = m_mdstIndex;
  }
}


int Particle::getMdstSource() const
{
  // Is identifier already set
  if (m_identifier > -1)
    return m_identifier + (m_particleType << 24);

  // Identifier is not set.
  int identifier = 0;
  if (m_particleType == c_ECLCluster) {
    const ECLCluster* cluster = this->getECLCluster();
    if (cluster) {
      const int crid     = cluster->getConnectedRegionId();
      const int clusterid = cluster->getClusterId();
      identifier = 1000 * crid + clusterid;
    } else {
      B2ERROR("Particle is of type = ECLCluster has identifier not set and no relation to ECLCluster.\n"
              "This has happen because old microDST is analysed with newer version of software.");
    }
  } else {
    identifier = m_mdstIndex;
  }

  return identifier + (m_particleType << 24);
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
  TMatrixFSym mom;
  const TMatrixFSym& full = getMomentumVertexErrorMatrix();

  // get 4x4 (momentum) submatrix from the full error matrix
  // momentum related elements are in [0,...,3]x[0,...,3] block
  full.GetSub(0, 3, mom, "S");

  return mom;
}

TMatrixFSym Particle::getVertexErrorMatrix() const
{
  TMatrixFSym pos;
  const TMatrixFSym& full = getMomentumVertexErrorMatrix();

  // get 3x3 (position) submatrix from the full error matrix
  // vertex related elements are in [4,5,6]x[4,5,6] block
  full.GetSub(4, 6, pos, "S");

  return pos;
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

void Particle::updateMass(const int pdgCode)
{
  if (TDatabasePDG::Instance()->GetParticle(pdgCode) == NULL)
    B2FATAL("PDG=" << pdgCode << " ***code unknown to TDatabasePDG");
  m_mass = TDatabasePDG::Instance()->GetParticle(pdgCode)->Mass() ;
}

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
  if (i >= getNDaughters()) return NULL;
  return static_cast<Particle*>(getArrayPointer()->At(m_daughterIndices[i]));
}

std::vector<Belle2::Particle*> Particle::getDaughters() const
{
  const unsigned int nDaughters = getNDaughters();
  std::vector<Particle*> daughters(nDaughters);

  const TClonesArray* array = getArrayPointer();
  for (unsigned i = 0; i < nDaughters; i++)
    daughters[i] = static_cast<Particle*>(array->At(m_daughterIndices[i]));

  return daughters;
}

std::vector<const Belle2::Particle*> Particle::getFinalStateDaughters() const
{
  std::vector<const Particle*> fspDaughters;
  fillFSPDaughters(fspDaughters);

  return fspDaughters;
}

std::vector<int> Particle::getMdstArrayIndices(EParticleType type) const
{
  std::vector<int> mdstIndices;
  for (const Particle* fsp : getFinalStateDaughters()) {
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
      if (thisFSPs[tFSP]->getMdstSource() == otherFSPs[oFSP]->getMdstSource())
        return true;

  return false;
}

bool Particle::isCopyOf(const Particle* oParticle) const
{
  // the name of the game is to as quickly as possible determine
  // that the Particles are not copies
  if (this->getPDGCode() != oParticle->getPDGCode())
    return false;

  unsigned nDaughters = this->getNDaughters();
  if (nDaughters != oParticle->getNDaughters())
    return false;

  if (nDaughters) {
    // has daughters: check if the decay chain is the same and it ends with
    // the same FSPs
    std::vector<int> thisDecayChain(nDaughters * 2);
    std::vector<int> othrDecayChain(nDaughters * 2);

    for (unsigned i = 0; i < nDaughters; i++) {
      this->getDaughter(i)->fillDecayChain(thisDecayChain);
      oParticle->getDaughter(i)->fillDecayChain(othrDecayChain);
    }

    for (unsigned i = 0; i < thisDecayChain.size(); i++)
      if (thisDecayChain[i] != othrDecayChain[i])
        return false;

  } else {
    // has no daughters: it's a FSP, compare MDST source and index
    return this->getMdstSource() == oParticle->getMdstSource();
  }

  return true;
}

const Track* Particle::getTrack() const
{
  if (m_particleType == c_Track) {
    StoreArray<Track> tracks;
    return tracks[m_mdstIndex];
  } else
    return nullptr;
}

const PIDLikelihood* Particle::getPIDLikelihood() const
{
  if (m_particleType == c_Track) {
    StoreArray<Track> tracks;
    return tracks[m_mdstIndex]->getRelated<PIDLikelihood>();
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
    return tracks[m_mdstIndex]->getRelated<ECLCluster>();
  } else {
    return nullptr;
  }
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

void Particle::fillDecayChain(std::vector<int>& decayChain) const
{
  decayChain.push_back(m_pdgCode);
  decayChain.push_back(getMdstSource());

  for (unsigned i = 0; i < getNDaughters(); i++)
    getDaughter(i)->fillDecayChain(decayChain);
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

std::string Particle::getName() const
{
  return TDatabasePDG::Instance()->GetParticle(m_pdgCode)->GetName();
}

void Particle::print() const
{
  B2INFO(getInfo());
}

std::string Particle::getInfoHTML() const
{
  std::stringstream stream;
  stream << std::setprecision(4);
  stream << "<b>collection</b>=" << getArrayName();
  stream << "<br>";
  stream << " <b>PDGCode</b>=" << m_pdgCode;
  stream << " <b>Charge</b>=" << getCharge();
  stream << " <b>PDGMass</b>=" << getPDGMass();
  stream << "<br>";
  stream << " <b>flavorType</b>=" << m_flavorType;
  stream << " <b>particleType</b>=" << m_particleType;
  stream << " <b>particleTypeUsedForFit</b>=" << m_pdgCodeUsedForFit;
  stream << "<br>";

  stream << " <b>mdstIndex</b>=" << m_mdstIndex;
  stream << " <b>arrayIndex</b>=" << getArrayIndex();
  stream << " <b>identifier</b>=" << m_identifier;
  stream << " <b>daughterIndices</b>: ";
  for (unsigned i = 0; i < m_daughterIndices.size(); i++) {
    stream << m_daughterIndices[i] << ", ";
  }
  if (m_daughterIndices.empty()) stream << " (none)";
  stream << "<br>";

  if (!m_daughterIndices.empty()) {
    stream << " <b>daughter PDGCodes</b>: ";
    for (unsigned i = 0; i < m_daughterIndices.size(); i++) {
      const Particle* p = getDaughter(i);
      if (p) {stream << p->getPDGCode() << ", ";}
      else {stream << "?, ";}
    }
    stream << "<br>";
  }

  stream << " <b>mass</b>=" << m_mass;
  stream << "<br>";

  stream << " <b>momentum</b>=" << HTML::getString(getMomentum());
  stream << " <b>p</b>=" << getP();
  stream << "<br>";

  stream << " <b>position</b>=" << HTML::getString(getVertex());
  stream << "<br>";

  stream << " <b>p-value of fit</b> (if done): ";
  stream << m_pValue;
  stream << "<br>";

  stream << " <b>error matrix</b> (px, py, pz, E, x, y ,z):<br>";
  stream << HTML::getString(getMomentumVertexErrorMatrix());

  stream << " <b>extra info</b>=( ";
  if (!m_extraInfo.empty()) {
    StoreObjPtr<ParticleExtraInfoMap> extraInfoMap;
    if (!extraInfoMap) {
      B2FATAL("ParticleExtraInfoMap not available, but needed for storing extra info in Particle!");
    }
    const ParticleExtraInfoMap::IndexMap& map = extraInfoMap->getMap(m_extraInfo[0]);
    const unsigned int nVars = m_extraInfo.size();
    for (const auto& pair : map) {
      if (pair.second < nVars) {
        stream << pair.first << "=" << m_extraInfo[pair.second] << " ";
      }
    }

  }
  stream << ") " << "<br>";

  return stream.str();
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
