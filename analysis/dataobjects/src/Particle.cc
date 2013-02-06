/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/Particle.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

#include <TDatabasePDG.h>

#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLGamma.h>
#include <ecl/dataobjects/ECLPi0.h>
#include <generators/dataobjects/MCParticle.h>
#include <tracking/dataobjects/Track.h>
#include <tracking/dataobjects/TrackFitResult.h>

using namespace Belle2;

// Constructors - START
Particle::Particle() :
  m_plist(0), m_pdgCode(0), m_particleType(c_Undefined), m_mdstIndex(0),
  m_energy(0), m_momentum_x(0), m_momentum_y(0), m_momentum_z(0),
  m_position_x(0), m_position_y(0), m_position_z(0)
{
  resetErrorMatrix();
}

Particle::Particle(const TLorentzVector& momentum, const int pdgCode) :
  m_plist(0), m_pdgCode(pdgCode), m_particleType(c_Undefined), m_mdstIndex(0),
  m_position_x(0), m_position_y(0), m_position_z(0)
{
  set4Vector(momentum);
  resetErrorMatrix();
}

Particle::Particle(const TLorentzVector& momentum, const int pdgCode, const std::vector<int> &daughterIndices) :
  m_plist(0), m_pdgCode(pdgCode), m_mdstIndex(0),
  m_position_x(0), m_position_y(0), m_position_z(0)
{
  set4Vector(momentum);
  resetErrorMatrix();

  if (daughterIndices.size() > 0) {
    m_particleType    = c_Composite;
    m_daughterIndices = daughterIndices;
  } else {
    m_particleType    = c_Undefined;
  }
}

Particle::Particle(const Track* track, const unsigned index, const Const::ChargedStable& chargedStable) :
  m_plist(0), m_particleType(c_Track), m_mdstIndex(index)
{
  const TrackFitResult* trackFit = track->getTrackFitResult(chargedStable);

  int absPDGCode = chargedStable.getPDGCode();
  int signFlip = 1;
  if (absPDGCode < Const::ChargedStable::muon.getPDGCode() + 1)
    signFlip = -1;

  m_pdgCode = chargedStable.getPDGCode() * signFlip * trackFit->getCharge();

  // set position at which the momentum is estimated (= POCA)
  setVertex(trackFit->getPosition());

  // set 4-momentum

  // first get nominal mass from the m_pdgCode
  // TODO: deal with exceptions
  if (TDatabasePDG::Instance()->GetParticle(m_pdgCode) == NULL)
    B2FATAL("Unknown PDG code!");

  float pdgMass = TDatabasePDG::Instance()->GetParticle(m_pdgCode)->Mass() ;

  // construct 4-momentum from measured 3-momentum
  // and assigned mass hypothesis
  TLorentzVector momentum;
  momentum.SetVectM(trackFit->getMomentum(), pdgMass);
  set4Vector(momentum);

  // fill the error matrix
  // m_momentumPositionError  = px, py, pz, E,  x,  y, z
  // p.getErrorMatrix row     = x,  y,  z,  px, py, pz
  // TODO: set error matrix
  /*
  m_momentumPositionError[0]  = trackFit->getErrorMatrix()[3][3]; // px-px
  m_momentumPositionError[1]  = trackFit->getErrorMatrix()[3][4]; // px-py
  m_momentumPositionError[2]  = trackFit->getErrorMatrix()[3][5]; // px-pz
  m_momentumPositionError[4]  = trackFit->getErrorMatrix()[3][0]; // px-x
  m_momentumPositionError[5]  = trackFit->getErrorMatrix()[3][1]; // px-y
  m_momentumPositionError[6]  = trackFit->getErrorMatrix()[3][2]; // px-z
  m_momentumPositionError[7]  = trackFit->getErrorMatrix()[4][4]; // py-py
  m_momentumPositionError[8]  = trackFit->getErrorMatrix()[4][5]; // py-pz

  m_momentumPositionError[10] = trackFit->getErrorMatrix()[4][0]; // py-x
  m_momentumPositionError[11] = trackFit->getErrorMatrix()[4][1]; // py-y
  m_momentumPositionError[12] = trackFit->getErrorMatrix()[4][2]; // py-z
  m_momentumPositionError[13] = trackFit->getErrorMatrix()[5][5]; // pz-pz

  m_momentumPositionError[15] = trackFit->getErrorMatrix()[5][0]; // pz-x
  m_momentumPositionError[16] = trackFit->getErrorMatrix()[5][1]; // pz-y
  m_momentumPositionError[17] = trackFit->getErrorMatrix()[5][2]; // pz-z

  // covariance E-(x,y,z) is set to 0
  m_momentumPositionError[19] = 0.0; // E-x
  m_momentumPositionError[20] = 0.0; // E-y
  m_momentumPositionError[21] = 0.0; // E-z

  m_momentumPositionError[22] = trackFit->getErrorMatrix()[0][0]; // x-x
  m_momentumPositionError[23] = trackFit->getErrorMatrix()[0][1]; // x-y
  m_momentumPositionError[24] = trackFit->getErrorMatrix()[0][2]; // x-z
  m_momentumPositionError[25] = trackFit->getErrorMatrix()[1][1]; // y-y
  m_momentumPositionError[26] = trackFit->getErrorMatrix()[1][2]; // y-z
  m_momentumPositionError[27] = trackFit->getErrorMatrix()[2][2]; // z-z
  */
  // covariance E-(px,py,pz)
  // E = sqrt(px*px+py*py+pz*pz+m*m)
  // therfore covariance E:px
  // is given by
  // cov(px,E) = cov(px,px) * dE/dpx + cov(px,py) * dE/dpy + cov(px,pz) * dE/dpz
  // (similarly for covariance of E and py or pz)
  // and variance of E is given by
  // cov(E,E)  = cov(px,px) * |dE/dpx|^2 + cov(py,py) * |dE/dpy|^2 + cov(pz,pz) * |dE/dpz|^2
  //           + 2 * cov(px,py) * dE/dpx * dE/dpy
  //           + 2 * cov(px,pz) * dE/dpx * dE/dpz
  //           + 2 * cov(py,pz) * dE/dpy * dE/dpz

  double invE = 1.0 / m_energy;

  // cov(px,E)
  m_momentumPositionError[3]  = invE * (m_momentumPositionError[0] * m_momentum_x
                                        + m_momentumPositionError[1] * m_momentum_y
                                        + m_momentumPositionError[2] * m_momentum_z);
  // cov(py,E)
  m_momentumPositionError[9]  = invE * (m_momentumPositionError[1] * m_momentum_x
                                        + m_momentumPositionError[7] * m_momentum_y
                                        + m_momentumPositionError[8] * m_momentum_z);
  // cov(pz,E)
  m_momentumPositionError[14] = invE * (m_momentumPositionError[2] * m_momentum_x
                                        + m_momentumPositionError[8] * m_momentum_y
                                        + m_momentumPositionError[13] * m_momentum_z);
  // cov(E,E)
  m_momentumPositionError[18] = invE * invE * (m_momentumPositionError[0] * m_momentum_x * m_momentum_x
                                               + m_momentumPositionError[7] * m_momentum_y * m_momentum_y
                                               + m_momentumPositionError[13] * m_momentum_z * m_momentum_z
                                               + 2 * m_momentumPositionError[1] * m_momentum_x * m_momentum_y
                                               + 2 * m_momentumPositionError[2] * m_momentum_x * m_momentum_z
                                               + 2 * m_momentumPositionError[8] * m_momentum_y * m_momentum_z);
}

Particle::Particle(const ECLGamma* gamma) :
  m_plist(0), m_pdgCode(Const::photon.getPDGCode()), m_particleType(c_ECLShower)
{
  // position
  // TODO: Obtain the values from ECL group (via xml file or something like this), but hard coded values need to be avoided!
  m_position_x = 0.0;
  m_position_y = 0.0;
  m_position_z = 0.0;

  // 4-momentum
  m_momentum_x = gamma->getPx();
  m_momentum_y = gamma->getPy();
  m_momentum_z = gamma->getPz();
  m_energy     = gamma->getEnergy();

  // set the error matrix
  resetErrorMatrix();

  TMatrixFSym momErrMatrix(C_MOMENTUM_ERROR_MATRIX_DIMENSION);
  // obtain the 4x4 momentum error matrix
  gamma->getErrorMatrix(momErrMatrix);

  TMatrixFSym fullErrMatrix(C_MOMENTUM_POSITION_ERROR_MATRIX_DIMENSION);
  fullErrMatrix.SetSub(0, momErrMatrix);

  // set the (sigma_x)^2, (sigma_y)^2, (sigma_z)^2
  // to arbitrary large values
  // TODO: Obtain the values from ECL group (via xml file or something like this), but hard coded values need to be avoided!
  // TODO: check units!!!!????
  fullErrMatrix(4, 4) = 0.0001;
  fullErrMatrix(5, 5) = 0.0001;
  fullErrMatrix(6, 6) = 0.0001;

  // Note that all other elements are set to 0.0
  fillErrorMatrix(fullErrMatrix);

  // mdst index
  m_mdstIndex = (unsigned)gamma->GetShowerId();
}

Particle::Particle(const ECLPi0* pi0) :
  m_plist(0), m_pdgCode(Const::pi0.getPDGCode()), m_particleType(c_Composite), m_mdstIndex(0)
{
  // position
  // TODO: Obtain the values from ECL group (via xml file or something like this), but hard coded values need to be avoided!
  m_position_x = 0.0;
  m_position_y = 0.0;
  m_position_z = 0.0;

  // 4-momentum
  m_momentum_x = pi0->getPx();
  m_momentum_y = pi0->getPy();
  m_momentum_z = pi0->getPz();
  m_energy     = pi0->getEnergy();

  // set the error matrix
  resetErrorMatrix();

  // TODO: Uncomment once ECLPi0 provides error matrix
  /*
  TMatrixFSym momErrMatrix(C_MOMENTUM_ERROR_MATRIX_DIMENSION);
  // obtain the 4x4 momentum error matrix
  pi0->getErrorMatrix(momErrMatrix);

  TMatrixFSym fullErrMatrix(C_MOMENTUM_POSITION_ERROR_MATRIX_DIMENSION);
  fullErrMatrix.SetSub(0, momErrMatrix);

  // set the (sigma_x)^2, (sigma_y)^2, (sigma_z)^2
  // to arbitrary large values
  // TODO: Obtain the values from ECL group (via xml file or something like this), but hard coded values need to be avoided!
  // TODO: check units!!!!????
  fullErrMatrix(4,4) = 0.0001;
  fullErrMatrix(5,5) = 0.0001;
  fullErrMatrix(6,6) = 0.0001;

  // Note that all other elements are set to 0.0
  fillErrorMatrix(fullErrorMatrix);
  */
}

Particle::Particle(const MCParticle* mcParticle) :
  m_plist(0), m_particleType(c_MCParticle)
{
  m_pdgCode      = mcParticle->getPDG();
  m_mdstIndex    = mcParticle->getIndex();

  // generated 4-momentum
  set4Vector(mcParticle->get4Vector());
  // generated production vertex
  setVertex(mcParticle->getVertex());

  resetErrorMatrix();
}
// Constructors - END

// Setters - START
void Particle::setMomentumVertexErrorMatrix(const TMatrixFSym& m)
{
  // check if provided Error Matrix is of dimension 7x7
  // if not, reset the error matrix and print warning
  if (m.GetNrows() != C_MOMENTUM_POSITION_ERROR_MATRIX_DIMENSION || m.GetNcols() != C_MOMENTUM_POSITION_ERROR_MATRIX_DIMENSION) {
    resetErrorMatrix();
    B2WARNING("Wrong Momentum-Position Error Matrix dimension. It should be 7x7 symetric matrix!");
    return;
  }

  // evrything is OK, so fill the matrix
  fillErrorMatrix(m);
}
// Setters - END

// Getters - START
TMatrixFSym Particle::getMomentumVertexErrorMatrix() const
{
  TMatrixFSym m(C_MOMENTUM_POSITION_ERROR_MATRIX_DIMENSION);

  int element = 0;
  for (int irow = 0; irow < C_MOMENTUM_POSITION_ERROR_MATRIX_DIMENSION; irow++) {
    for (int icol = irow; icol < C_MOMENTUM_POSITION_ERROR_MATRIX_DIMENSION; icol++) {
      m(irow, icol) = m(icol, irow) = m_momentumPositionError[element];
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

float Particle::getPDGMass(void) const
{
  if (TDatabasePDG::Instance()->GetParticle(m_pdgCode) == NULL) {
    // TODO: deal with exceptions
    B2ERROR("Unknown PDG code!");
    return 0.0;
  } else
    return TDatabasePDG::Instance()->GetParticle(m_pdgCode)->Mass();
}

float Particle::getCharge(void) const
{
  if (TDatabasePDG::Instance()->GetParticle(m_pdgCode) == NULL) {
    // TODO: deal with exceptions
    B2ERROR("Unknown PDG code!");
    return 0.0;
  } else
    return TDatabasePDG::Instance()->GetParticle(m_pdgCode)->Charge() / 3.0;
}

const Particle* Particle::getDaughter(unsigned i) const
{
  fixParticleList();

  if (i >= getNDaughters())
    return NULL;

  return static_cast<Particle*>(m_plist->At(m_daughterIndices[i]));
}

const std::vector<Belle2::Particle*> Particle::getDaughters() const
{
  fixParticleList();

  std::vector<Particle*> daughters(getNDaughters());
  for (unsigned i = 0; i < getNDaughters(); i++)
    daughters[i] = static_cast<Particle*>(m_plist->At(m_daughterIndices[i]));

  return daughters;
}

const std::vector<const Belle2::Particle*> Particle::getFinalStateDaughters() const
{
  std::vector<const Particle*> fspDaughters;
  fillFSPDaughters(fspDaughters);

  return fspDaughters;
}

void Particle::fillFSPDaughters(std::vector<const Belle2::Particle*> &fspDaughters) const
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

// Getters - END

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

// Private functions - START
void  Particle::resetErrorMatrix()
{
  for (int i = 0; i < C_MOMENTUM_POSITION_ERROR_MATRIX_DIMENSION_1D_REP; i++)
    m_momentumPositionError[i] = 0.0;
}

void  Particle::fillErrorMatrix(const TMatrixFSym& m)
{
  int element = 0;
  for (int irow = 0; irow < C_MOMENTUM_POSITION_ERROR_MATRIX_DIMENSION; irow++) {
    for (int icol = irow; icol < C_MOMENTUM_POSITION_ERROR_MATRIX_DIMENSION; icol++) {
      m_momentumPositionError[element] = m(irow, icol);
      element++;
    }
  }
}

void Particle::fixParticleList() const
{
  if (m_plist != 0) return;

  TClonesArray* plist(0);

  //Search default location
  StoreArray<Particle> Particles;
  if (Particles && getArrayIndex() >= 0) {
    plist = Particles.getPtr();
  } else {
    //Search all StoreArrays which happen to store Particles
    const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(DataStore::c_Event);
    for (DataStore::StoreObjConstIter iter = map.begin(); iter != map.end(); ++iter) {
      TClonesArray* value = dynamic_cast<TClonesArray*>(iter->second->ptr);
      if (value && value->GetClass() == Class() && value->IndexOf(this) >= 0) {
        plist = value;
        break;
      }
    }
  }
  //Could not find any collection, raise exception
  if (!plist) {
    B2ERROR("Could not determine StoreArray the Particle belongs to !");
    // TODO: deal with exceptions
    //throw NoParticleListSetError();
  }

  //Set plist pointer and index for whole array
  for (int i = 0; i < plist->GetEntriesFast(); i++) {
    Particle& p = *(static_cast<Particle*>(plist->At(i)));
    p.m_plist         = plist;
  }
}

// Private functions - END

ClassImp(Particle);
