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

Particle::Particle() :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_flavorType(0), m_particleType(c_Undefined), m_mdstIndex(0), m_plist(0)
{
  resetErrorMatrix();
}

Particle::Particle(const TLorentzVector& momentum, const int pdgCode) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_flavorType(0), m_particleType(c_Undefined), m_mdstIndex(0), m_plist(0)
{
  m_pdgCode = pdgCode;
  setFlavorType();
  set4Vector(momentum);
  resetErrorMatrix();
}

Particle::Particle(const TLorentzVector& momentum,
                   const int pdgCode,
                   const unsigned flavorType,
                   const unsigned index,
                   const EParticleType type) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_flavorType(0), m_particleType(c_Undefined), m_mdstIndex(0), m_plist(0)
{
  m_pdgCode = pdgCode;
  m_flavorType = flavorType;
  m_mdstIndex = index;
  m_particleType = type;
  set4Vector(momentum);
  resetErrorMatrix();
}

Particle::Particle(const TLorentzVector& momentum,
                   const int pdgCode,
                   const unsigned flavorType,
                   const std::vector<int> &daughterIndices) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_flavorType(0), m_particleType(c_Undefined), m_mdstIndex(0), m_plist(0)
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


Particle::Particle(const Track* track, const unsigned index,
                   const Const::ChargedStable& chargedStable) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_flavorType(0), m_particleType(c_Undefined), m_mdstIndex(0), m_plist(0)
{
  if (!track) return;
  const TrackFitResult* trackFit = track->getTrackFitResult(chargedStable);
  if (!trackFit) return;

  m_flavorType = 1;
  m_particleType = c_Track;
  m_mdstIndex = index;

  // set PDG code
  int absPDGCode = chargedStable.getPDGCode();
  int signFlip = 1;
  if (absPDGCode < Const::ChargedStable::muon.getPDGCode() + 1) signFlip = -1;
  m_pdgCode = chargedStable.getPDGCode() * signFlip * trackFit->getCharge();

  // set mass
  if (TDatabasePDG::Instance()->GetParticle(m_pdgCode) == NULL)
    B2FATAL("PDG=" << m_pdgCode << " ***code unknown to TDatabasePDG");
  m_mass = TDatabasePDG::Instance()->GetParticle(m_pdgCode)->Mass() ;

  // set momentum
  m_px = trackFit->getMomentum().Px();
  m_py = trackFit->getMomentum().Py();
  m_pz = trackFit->getMomentum().Pz();

  // set position at which the momentum is given (= POCA)
  setVertex(trackFit->getPosition());

  // set error matrix: TODO when available (getCovariance6() in what order is it given?)
  resetErrorMatrix();

  // m_errMatrix  = px, py, pz, E,  x,  y, z
  // p.getErrorMatrix row     = x,  y,  z,  px, py, pz
  // TODO: set error matrix
  /*
  m_errMatrix[0]  = trackFit->getErrorMatrix()[3][3]; // px-px
  m_errMatrix[1]  = trackFit->getErrorMatrix()[3][4]; // px-py
  m_errMatrix[2]  = trackFit->getErrorMatrix()[3][5]; // px-pz
  m_errMatrix[4]  = trackFit->getErrorMatrix()[3][0]; // px-x
  m_errMatrix[5]  = trackFit->getErrorMatrix()[3][1]; // px-y
  m_errMatrix[6]  = trackFit->getErrorMatrix()[3][2]; // px-z
  m_errMatrix[7]  = trackFit->getErrorMatrix()[4][4]; // py-py
  m_errMatrix[8]  = trackFit->getErrorMatrix()[4][5]; // py-pz

  m_errMatrix[10] = trackFit->getErrorMatrix()[4][0]; // py-x
  m_errMatrix[11] = trackFit->getErrorMatrix()[4][1]; // py-y
  m_errMatrix[12] = trackFit->getErrorMatrix()[4][2]; // py-z
  m_errMatrix[13] = trackFit->getErrorMatrix()[5][5]; // pz-pz

  m_errMatrix[15] = trackFit->getErrorMatrix()[5][0]; // pz-x
  m_errMatrix[16] = trackFit->getErrorMatrix()[5][1]; // pz-y
  m_errMatrix[17] = trackFit->getErrorMatrix()[5][2]; // pz-z

  // covariance E-(x,y,z) is set to 0
  m_errMatrix[19] = 0.0; // E-x
  m_errMatrix[20] = 0.0; // E-y
  m_errMatrix[21] = 0.0; // E-z

  m_errMatrix[22] = trackFit->getErrorMatrix()[0][0]; // x-x
  m_errMatrix[23] = trackFit->getErrorMatrix()[0][1]; // x-y
  m_errMatrix[24] = trackFit->getErrorMatrix()[0][2]; // x-z
  m_errMatrix[25] = trackFit->getErrorMatrix()[1][1]; // y-y
  m_errMatrix[26] = trackFit->getErrorMatrix()[1][2]; // y-z
  m_errMatrix[27] = trackFit->getErrorMatrix()[2][2]; // z-z
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

  double invE = 1.0 / getEnergy();

  // cov(px,E)
  m_errMatrix[3]  = invE * (m_errMatrix[0] * m_px
                                        + m_errMatrix[1] * m_py
                                        + m_errMatrix[2] * m_pz);
  // cov(py,E)
  m_errMatrix[9]  = invE * (m_errMatrix[1] * m_px
                                        + m_errMatrix[7] * m_py
                                        + m_errMatrix[8] * m_pz);
  // cov(pz,E)
  m_errMatrix[14] = invE * (m_errMatrix[2] * m_px
                                        + m_errMatrix[8] * m_py
                                        + m_errMatrix[13] * m_pz);
  // cov(E,E)
  m_errMatrix[18] = invE * invE * (m_errMatrix[0] * m_px * m_px
                                               + m_errMatrix[7] * m_py * m_py
                                               + m_errMatrix[13] * m_pz * m_pz
                                               + 2 * m_errMatrix[1] * m_px * m_py
                                               + 2 * m_errMatrix[2] * m_px * m_pz
                                               + 2 * m_errMatrix[8] * m_py * m_pz);
  */

}


Particle::Particle(const ECLGamma* gamma, const unsigned index) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_flavorType(0), m_particleType(c_Undefined), m_mdstIndex(0), m_plist(0)
{
  m_pdgCode = 22;
  m_px = gamma->getPx();
  m_py = gamma->getPy();
  m_pz = gamma->getPz();
  // position: TODO obtain the values that are used in the momentum construction

  m_particleType = c_ECLShower;
  m_mdstIndex = index;

  resetErrorMatrix();

  // set error matrix: TODO set x,y,z part correctly (with correlations)!

  TMatrixFSym momErrMatrix(c_DimMomentum);
  gamma->getErrorMatrix(momErrMatrix);

  TMatrixFSym fullErrMatrix(c_DimMatrix);
  fullErrMatrix.SetSub(0, momErrMatrix);

  // set diagonals for x, y, z uncertainties to some large values
  // Note that all other elements are set to 0.0
  fullErrMatrix(4, 4) = 1000;
  fullErrMatrix(5, 5) = 1000;
  fullErrMatrix(6, 6) = 1000;

  fillErrorMatrix(fullErrMatrix);
}


Particle::Particle(const ECLPi0* pi0) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_flavorType(0), m_particleType(c_Undefined), m_mdstIndex(0), m_plist(0)
{
  m_pdgCode = 111;
  m_mass = pi0->getMassFit();
  m_px = pi0->getPx();
  m_py = pi0->getPy();
  m_pz = pi0->getPz();
  // position: TODO obtain the values that are used for gamma momentum construction

  m_particleType = c_Composite; // but what about daughter photons? TODO

  resetErrorMatrix();

  // set the error matrix

  TMatrixFSym momErrMatrix = pi0->getErrorMatrix();
  TMatrixFSym fullErrMatrix(c_DimMatrix);
  fullErrMatrix.SetSub(0, momErrMatrix);

  // set diagonals for x, y, z uncertainties to some large values
  // Note that all other elements are set to 0.0
  fullErrMatrix(4, 4) = 1000;
  fullErrMatrix(5, 5) = 1000;
  fullErrMatrix(6, 6) = 1000;

  fillErrorMatrix(fullErrMatrix);
}


Particle::Particle(const MCParticle* mcParticle) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_flavorType(0), m_particleType(c_Undefined), m_mdstIndex(0), m_plist(0)
{
  m_pdgCode      = mcParticle->getPDG();
  m_particleType = c_MCParticle; // what about daughters if not FS particle?
  m_mdstIndex    = mcParticle->getIndex();
  setFlavorType();

  // generated 4-momentum
  set4Vector(mcParticle->get4Vector());
  // generated production vertex
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
  fillErrorMatrix(m);
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

float Particle::getPDGMass(void) const
{
  if (TDatabasePDG::Instance()->GetParticle(m_pdgCode) == NULL) {
    B2ERROR("Unknown PDG code!");
    return 0.0;
  }
  return TDatabasePDG::Instance()->GetParticle(m_pdgCode)->Mass();
}

float Particle::getCharge(void) const
{
  if (TDatabasePDG::Instance()->GetParticle(m_pdgCode) == NULL) {
    B2ERROR("Unknown PDG code!");
    return 0.0;
  }
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


void  Particle::resetErrorMatrix()
{
  for (int i = 0; i < c_SizeMatrix; i++)
    m_errMatrix[i] = 0.0;
}

void  Particle::fillErrorMatrix(const TMatrixFSym& m)
{
  int element = 0;
  for (int irow = 0; irow < c_DimMatrix; irow++) {
    for (int icol = irow; icol < c_DimMatrix; icol++) {
      m_errMatrix[element] = m(irow, icol);
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


void Particle::setFlavorType()
{
  m_flavorType = 1; // flavored particle
  if (m_pdgCode < 0) return;
  if (m_pdgCode == 22) {m_flavorType = 0; return;} // gamma
  if (m_pdgCode == 310) {m_flavorType = 0; return;} // K_s
  if (m_pdgCode == 130) {m_flavorType = 0; return;} // K_L
  int nnn = m_pdgCode / 10;
  int q3 = nnn % 10; nnn /= 10;
  int q2 = nnn % 10; nnn /= 10;
  int q1 = nnn % 10;
  if (q1 == 0 && q2 == q3) m_flavorType = 0; // unflavored meson
}



ClassImp(Particle);
