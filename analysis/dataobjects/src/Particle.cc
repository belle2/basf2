/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Marko Staric, Christian Pulvermacher,       *
 *               Sam Cunliffe, Torben Ferber, Thomas Kuhr,                *
 *               Umberto Tamponi                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>

#include <analysis/ClusterUtility/ClusterUtils.h>

#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/V0.h>
#include <mdst/dbobjects/CollisionBoostVector.h>
#include <mdst/dbobjects/CollisionInvariantMass.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/HTML.h>
#include <framework/utilities/Conversion.h>

#include <TClonesArray.h>
#include <TDatabasePDG.h>
#include <TMatrixFSym.h>

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <queue>

#include <boost/algorithm/string.hpp>

using namespace Belle2;

Particle::Particle() :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(nan("")), m_flavorType(c_Unflavored), m_particleSource(c_Undefined), m_mdstIndex(0), m_properties(0),
  m_arrayPointer(nullptr)
{
  resetErrorMatrix();
}


Particle::Particle(const TLorentzVector& momentum, const int pdgCode) :
  m_pdgCode(pdgCode), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleSource(c_Undefined), m_mdstIndex(0), m_properties(0), m_arrayPointer(nullptr)
{
  setFlavorType();
  set4Vector(momentum);
  resetErrorMatrix();
}


Particle::Particle(const TLorentzVector& momentum,
                   const int pdgCode,
                   EFlavorType flavorType,
                   const EParticleSourceObject source,
                   const unsigned mdstIndex) :
  m_pdgCode(pdgCode), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(flavorType), m_particleSource(source), m_properties(0), m_arrayPointer(nullptr)
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
  m_flavorType(c_Unflavored), m_particleSource(c_Undefined), m_mdstIndex(0),
  m_properties(0), m_arrayPointer(arrayPointer)
{
  m_pdgCode = pdgCode;
  m_flavorType = flavorType;
  if (flavorType == c_Unflavored and pdgCode < 0)
    m_pdgCode = -pdgCode;
  set4Vector(momentum);
  resetErrorMatrix();

  if (!daughterIndices.empty()) {
    m_particleSource    = c_Composite;
    if (getArrayPointer() == nullptr) {
      B2FATAL("Composite Particle (with daughters) was constructed outside StoreArray without specifying daughter array!");
    }
    for (unsigned int i = 0; i < m_daughterIndices.size(); i++) {
      m_daughterProperties.push_back(Particle::PropertyFlags::c_Ordinary);
    }
  }
}

Particle::Particle(const TLorentzVector& momentum,
                   const int pdgCode,
                   EFlavorType flavorType,
                   const std::vector<int>& daughterIndices,
                   const int properties,
                   TClonesArray* arrayPointer) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1),
  m_daughterIndices(daughterIndices),
  m_flavorType(c_Unflavored), m_particleSource(c_Undefined), m_mdstIndex(0),
  m_arrayPointer(arrayPointer)
{
  m_pdgCode = pdgCode;
  m_flavorType = flavorType;
  if (flavorType == c_Unflavored and pdgCode < 0)
    m_pdgCode = -pdgCode;
  set4Vector(momentum);
  resetErrorMatrix();
  m_properties = properties;

  if (!daughterIndices.empty()) {
    m_particleSource    = c_Composite;
    if (getArrayPointer() == nullptr) {
      B2FATAL("Composite Particle (with daughters) was constructed outside StoreArray without specifying daughter array!");
    }
    for (unsigned int i = 0; i < m_daughterIndices.size(); i++) {
      m_daughterProperties.push_back(Particle::PropertyFlags::c_Ordinary);
    }
  }
}


Particle::Particle(const TLorentzVector& momentum,
                   const int pdgCode,
                   EFlavorType flavorType,
                   const std::vector<int>& daughterIndices,
                   const int properties,
                   const std::vector<int>& daughterProperties,
                   TClonesArray* arrayPointer) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1),
  m_daughterIndices(daughterIndices),
  m_flavorType(c_Unflavored), m_particleSource(c_Undefined), m_mdstIndex(0),
  m_daughterProperties(daughterProperties),
  m_arrayPointer(arrayPointer)
{
  m_pdgCode = pdgCode;
  m_flavorType = flavorType;
  if (flavorType == c_Unflavored and pdgCode < 0)
    m_pdgCode = -pdgCode;
  set4Vector(momentum);
  resetErrorMatrix();
  m_properties = properties;

  if (!daughterIndices.empty()) {
    m_particleSource    = c_Composite;
    if (getArrayPointer() == nullptr) {
      B2FATAL("Composite Particle (with daughters) was constructed outside StoreArray without specifying daughter array!");
    }
  }
}


Particle::Particle(const Track* track,
                   const Const::ChargedStable& chargedStable) :
  Particle(track ? track->getArrayIndex() : 0, track ? track->getTrackFitResultWithClosestMass(chargedStable) : nullptr,
           chargedStable)
{
}

Particle::Particle(const int trackArrayIndex,
                   const TrackFitResult* trackFit,
                   const Const::ChargedStable& chargedStable) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleSource(c_Undefined), m_mdstIndex(0), m_properties(0), m_arrayPointer(nullptr)
{
  if (!trackFit) return;

  m_flavorType = c_Flavored; //tracks are charged
  m_particleSource = c_Track;

  setMdstArrayIndex(trackArrayIndex);

  m_pdgCodeUsedForFit = trackFit->getParticleType().getPDGCode();
  m_pdgCode           = generatePDGCodeFromCharge(trackFit->getChargeSign(), chargedStable);

  // set mass
  if (TDatabasePDG::Instance()->GetParticle(m_pdgCode) == nullptr)
    B2FATAL("PDG=" << m_pdgCode << " ***code unknown to TDatabasePDG");
  m_mass = TDatabasePDG::Instance()->GetParticle(m_pdgCode)->Mass() ;

  // set momentum, position and error matrix
  setMomentumPositionErrorMatrix(trackFit);
}

//FIXME: Deprecated, to be removed after release-05
Particle::Particle(const int trackArrayIndex,
                   const TrackFitResult* trackFit,
                   const Const::ChargedStable& chargedStable,
                   const Const::ChargedStable& chargedStableUsedForFit) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleSource(c_Undefined), m_mdstIndex(0), m_properties(0), m_arrayPointer(nullptr)
{
  if (!trackFit) return;

  m_flavorType = c_Flavored; //tracks are charged
  m_particleSource = c_Track;

  setMdstArrayIndex(trackArrayIndex);

  m_pdgCodeUsedForFit = chargedStableUsedForFit.getPDGCode();
  m_pdgCode           = generatePDGCodeFromCharge(trackFit->getChargeSign(), chargedStable);

  // set mass
  if (TDatabasePDG::Instance()->GetParticle(m_pdgCode) == nullptr)
    B2FATAL("PDG=" << m_pdgCode << " ***code unknown to TDatabasePDG");
  m_mass = TDatabasePDG::Instance()->GetParticle(m_pdgCode)->Mass() ;

  // set momentum, position and error matrix
  setMomentumPositionErrorMatrix(trackFit);
}


Particle::Particle(const ECLCluster* eclCluster, const Const::ParticleType& type) :
  m_pdgCode(type.getPDGCode()), m_mass(type.getMass()), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleSource(c_Undefined), m_mdstIndex(0), m_properties(0), m_arrayPointer(nullptr)
{
  if (!eclCluster) return;

  setFlavorType();

  // returns default vertex from clusterutils (from beam parameters if available)
  // leave it like that for the moment to make it transparent
  ClusterUtils C;
  const TVector3 clustervertex = C.GetIPPosition();
  setVertex(clustervertex);

  const TLorentzVector clustermom = C.Get4MomentumFromCluster(eclCluster, clustervertex, getECLClusterEHypothesisBit());
  m_px = clustermom.Px();
  m_py = clustermom.Py();
  m_pz = clustermom.Pz();

  m_particleSource = c_ECLCluster;
  setMdstArrayIndex(eclCluster->getArrayIndex());

  // set Chi^2 probability:
  //TODO: gamma quality can be written here
  m_pValue = 1;

  // get error matrix.
  updateJacobiMatrix();

}


void Particle::updateJacobiMatrix()
{
  ClusterUtils C;

  const ECLCluster* cluster = this->getECLCluster();

  const TVector3 clustervertex = C.GetIPPosition();

  // Get Jacobi matrix.
  TMatrixD jacobi = C.GetJacobiMatrix4x6FromCluster(cluster, clustervertex, getECLClusterEHypothesisBit());
  storeJacobiMatrix(jacobi);

  // Propagate the photon energy scaling to jacobian elements that were calculated using energy.
  TMatrixD scaledJacobi(4, 6);

  int element = 0;

  for (int irow = 0; irow < 4; irow++) {
    for (int icol = 0; icol < 6; icol++) {
      if (icol != 0 && irow != 3) {
        scaledJacobi(irow, icol) = m_jacobiMatrix[element] * m_momentumScale;
      } else {
        scaledJacobi(irow, icol) = m_jacobiMatrix[element];
      }
      element++;
    }
  }

  storeJacobiMatrix(scaledJacobi);

  // Get covariance matrix of IP distribution.
  const TMatrixDSym clustervertexcovmat = C.GetIPPositionCovarianceMatrix();

  // Set error matrix.
  TMatrixDSym clustercovmat = C.GetCovarianceMatrix7x7FromCluster(cluster, clustervertexcovmat, getECLClusterEHypothesisBit(),
                              scaledJacobi);
  storeErrorMatrix(clustercovmat);
}

Particle::Particle(const KLMCluster* klmCluster, const int pdgCode) :
  m_pdgCode(0), m_mass(0), m_px(0), m_py(0), m_pz(0), m_x(0), m_y(0), m_z(0),
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleSource(c_Undefined), m_mdstIndex(0), m_properties(0), m_arrayPointer(nullptr)
{
  if (!klmCluster) return;

  m_pdgCode = pdgCode;
  setFlavorType();

  set4Vector(klmCluster->getMomentum());
  setVertex(klmCluster->getPosition());
  updateMass(m_pdgCode); // KLMCluster internally use Klong mass, overwrite here to allow neutrons

  m_particleSource = c_KLMCluster;
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
  m_pValue(-1), m_flavorType(c_Unflavored), m_particleSource(c_Undefined), m_mdstIndex(0), m_properties(0), m_arrayPointer(nullptr)
{
  if (!mcParticle) return;

  m_pdgCode      = mcParticle->getPDG();
  m_particleSource = c_MCParticle; // TODO: what about daughters if not FS particle?

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


Particle::~Particle() = default;

void Particle::setMdstArrayIndex(const int arrayIndex)
{
  m_mdstIndex = arrayIndex;

  // set the identifier
  if (m_particleSource == c_ECLCluster) {
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
    return m_identifier + (m_particleSource << 24);

  // Identifier is not set.
  int identifier = 0;
  if (m_particleSource == c_ECLCluster) {
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

  return identifier + (m_particleSource << 24);
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


void Particle::setJacobiMatrix(const TMatrixF& m)
{
  // check if provided Jacobi Matrix is of dimension 4x6
  // if not, reset the error matrix and print warning
  if (m.GetNrows() != 4 || m.GetNcols() != 6) {
    resetJacobiMatrix();
    B2WARNING("Jacobi Matrix is not 4x6 ");
    return;
  }
  storeJacobiMatrix(m);
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

float Particle::getCosHelicity(const Particle* mother) const
{
  // boost vector to the rest frame of the particle
  TVector3 boost = -get4Vector().BoostVector();

  // momentum of the mother in the particle's rest frame
  TLorentzVector pMother;
  if (mother) {
    pMother = mother->get4Vector();
  } else {
    static DBObjPtr<CollisionBoostVector> cmsBoost;
    static DBObjPtr<CollisionInvariantMass> cmsMass;
    pMother.SetE(cmsMass->getMass());
    pMother.Boost(cmsBoost->getBoost());
  }
  pMother.Boost(boost);

  // momentum of the daughter (or normal vector) in the particle's rest frame
  TLorentzVector pDaughter;
  if (getNDaughters() == 2) {  // two body decay
    pDaughter = getDaughter(0)->get4Vector();
    pDaughter.Boost(boost);
  } else if (getNDaughters() == 3) {
    if (getPDGCode() == Const::pi0.getPDGCode()) {  // pi0 Dalitz decay
      for (auto& daughter : getDaughters()) {
        if (daughter->getPDGCode() == Const::photon.getPDGCode()) {
          pDaughter = daughter->get4Vector();
        }
      }
      pDaughter.Boost(boost);
    } else {  // three body decay
      TLorentzVector pDaughter0 = getDaughter(0)->get4Vector();
      pDaughter0.Boost(boost);
      TLorentzVector pDaughter1 = getDaughter(1)->get4Vector();
      pDaughter1.Boost(boost);
      pDaughter.SetVect(pDaughter0.Vect().Cross(pDaughter1.Vect()));
    }
  }

  double mag2 = pMother.Vect().Mag2() * pDaughter.Vect().Mag2();
  if (mag2 <= 0) return std::numeric_limits<float>::quiet_NaN();
  return (-pMother.Vect()) * pDaughter.Vect() / sqrt(mag2);
}

float Particle::getCosHelicityDaughter(unsigned iDaughter, unsigned iGrandDaughter) const
{
  // check existence of daughter
  if (getNDaughters() <= iDaughter) {
    B2ERROR("No daughter of particle 'name' with index 'iDaughter' for calculation of helicity angle"
            << LogVar("name", getName()) << LogVar("iDaughter", iDaughter));
    return std::numeric_limits<float>::quiet_NaN();
  }

  // boost vector to the rest frame of the daughter particle
  const Particle* daughter = getDaughter(iDaughter);
  TVector3 boost = -daughter->get4Vector().BoostVector();

  // momentum of the this particle in the daughter's rest frame
  TLorentzVector pMother = get4Vector();
  pMother.Boost(boost);

  // check existence of grand daughter
  if (daughter->getNDaughters() <= iGrandDaughter) {
    B2ERROR("No grand daughter of daugher 'iDaughter' of particle 'name' with index 'iGrandDaughter' for calculation of helicity angle"
            << LogVar("name", getName()) << LogVar("iDaughter", iDaughter) << LogVar("iGrandDaughter", iGrandDaughter));
    return std::numeric_limits<float>::quiet_NaN();
  }

  // momentum of the grand daughter in the daughter's rest frame
  TLorentzVector pGrandDaughter = daughter->getDaughter(iGrandDaughter)->get4Vector();
  pGrandDaughter.Boost(boost);

  double mag2 = pMother.Vect().Mag2() * pGrandDaughter.Vect().Mag2();
  if (mag2 <= 0) return std::numeric_limits<float>::quiet_NaN();
  return (-pMother.Vect()) * pGrandDaughter.Vect() / sqrt(mag2);
}

float Particle::getAcoplanarity() const
{
  // check that we have a decay to two daughters and then two grand daughters each
  if (getNDaughters() != 2) {
    B2ERROR("Cannot calculate acoplanarity of particle 'name' because the number of daughters is not 2"
            << LogVar("name", getName()) << LogVar("# of daughters", getNDaughters()));
    return std::numeric_limits<float>::quiet_NaN();
  }
  const Particle* daughter0 = getDaughter(0);
  const Particle* daughter1 = getDaughter(1);
  if ((daughter0->getNDaughters() != 2) || (daughter1->getNDaughters() != 2)) {
    B2ERROR("Cannot calculate acoplanarity of particle 'name' because the number of grand daughters is not 2"
            << LogVar("name", getName()) << LogVar("# of grand daughters of first daughter", daughter0->getNDaughters())
            << LogVar("# of grand daughters of second daughter", daughter1->getNDaughters()));
    return std::numeric_limits<float>::quiet_NaN();
  }

  // boost vector to the rest frame of the particle
  TVector3 boost = -get4Vector().BoostVector();

  // momenta of the daughters and grand daughters in the particle's rest frame
  TLorentzVector pDaughter0 = daughter0->get4Vector();
  pDaughter0.Boost(boost);
  TLorentzVector pGrandDaughter0 = daughter0->getDaughter(0)->get4Vector();
  pGrandDaughter0.Boost(boost);
  TLorentzVector pDaughter1 = daughter1->get4Vector();
  pDaughter1.Boost(boost);
  TLorentzVector pGrandDaughter1 = daughter1->getDaughter(0)->get4Vector();
  pGrandDaughter1.Boost(boost);

  // calculate angle between normal vectors
  TVector3 normal0 = pDaughter0.Vect().Cross(pGrandDaughter0.Vect());
  TVector3 normal1 = -pDaughter1.Vect().Cross(pGrandDaughter1.Vect());
  double result = normal0.Angle(normal1);
  if (normal0.Cross(normal1) * pDaughter0.Vect() < 0) result = -result;

  return result;
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
  if (TDatabasePDG::Instance()->GetParticle(pdgCode) == nullptr)
    B2FATAL("PDG=" << pdgCode << " ***code unknown to TDatabasePDG");
  m_mass = TDatabasePDG::Instance()->GetParticle(pdgCode)->Mass() ;
}

float Particle::getPDGMass() const
{
  if (TDatabasePDG::Instance()->GetParticle(m_pdgCode) == nullptr) {
    B2ERROR("PDG=" << m_pdgCode << " ***code unknown to TDatabasePDG");
    return 0.0;
  }
  return TDatabasePDG::Instance()->GetParticle(m_pdgCode)->Mass();
}

float Particle::getCharge() const
{
  if (TDatabasePDG::Instance()->GetParticle(m_pdgCode) == nullptr) {
    B2ERROR("PDG=" << m_pdgCode << " ***code unknown to TDatabasePDG");
    return 0.0;
  }
  return TDatabasePDG::Instance()->GetParticle(m_pdgCode)->Charge() / 3.0;
}

const Particle* Particle::getDaughter(unsigned i) const
{
  if (i >= getNDaughters()) return nullptr;
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

std::vector<int> Particle::getMdstArrayIndices(EParticleSourceObject source) const
{
  std::vector<int> mdstIndices;
  for (const Particle* fsp : getFinalStateDaughters()) {
    // is this FSP daughter constructed from given MDST source
    if (fsp->getParticleSource() == source)
      mdstIndices.push_back(fsp->getMdstArrayIndex());
  }
  return mdstIndices;
}


void Particle::appendDaughter(const Particle* daughter, const bool updateType)
{
  if (updateType) {
    // is it a composite particle or fsr corrected?
    m_particleSource = c_Composite;
  }

  // add daughter index
  m_daughterIndices.push_back(daughter->getArrayIndex());
  m_daughterProperties.push_back(Particle::PropertyFlags::c_Ordinary);
}

void Particle::removeDaughter(const Particle* daughter, const bool updateType)
{
  if (getNDaughters() == 0)
    return;

  for (unsigned i = 0; i < getNDaughters(); i++) {
    if (m_daughterIndices[i] == daughter->getArrayIndex()) {
      m_daughterIndices.erase(m_daughterIndices.begin() + i);
      m_daughterProperties.erase(m_daughterProperties.begin() + i);
      i--;
    }
  }

  if (getNDaughters() == 0 and updateType)
    m_particleSource = c_Undefined;
}

bool Particle::overlapsWith(const Particle* oParticle) const
{
  // obtain vectors of daughter final state particles
  std::vector<const Particle*> thisFSPs  = this->getFinalStateDaughters();
  std::vector<const Particle*> otherFSPs = oParticle->getFinalStateDaughters();

  // check if they share any of the FSPs
  for (auto& thisFSP : thisFSPs)
    for (auto& otherFSP : otherFSPs)
      if (thisFSP->getMdstSource() == otherFSP->getMdstSource())
        return true;

  return false;
}

bool Particle::isCopyOf(const Particle* oParticle, bool doDetailedComparison) const
{
  // the name of the game is to as quickly as possible determine
  // that the Particles are not copies
  if (this->getPDGCode() != oParticle->getPDGCode() and !doDetailedComparison)
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

    // Even if the daughters have been provided in a different order in the
    // decay string the particles should be identified as copies / duplicates.
    // Therefore, the decay chain vectors, which contain both the pdg codes and
    // the mdst sources, are sorted here before comparing them.
    sort(thisDecayChain.begin(), thisDecayChain.end());
    sort(othrDecayChain.begin(), othrDecayChain.end());

    for (unsigned i = 0; i < thisDecayChain.size(); i++)
      if (thisDecayChain[i] != othrDecayChain[i])
        return false;

  } else if (this->getMdstSource() != oParticle->getMdstSource() and !doDetailedComparison) {
    // has no daughters: it's a FSP, compare MDST source and index
    return false;
  }
  // Stop here if we do not want a detailed comparison
  if (!doDetailedComparison)
    return true;
  //If we compare here a reconstructed Particle to a generated MCParticle
  //it means that something went horribly wrong and we must stop
  if ((this->getParticleSource() == EParticleSourceObject::c_MCParticle
       and oParticle->getParticleSource() != EParticleSourceObject::c_MCParticle)
      or (this->getParticleSource() != EParticleSourceObject::c_MCParticle
          and oParticle->getParticleSource() == EParticleSourceObject::c_MCParticle)) {
    B2WARNING("Something went wrong: MCParticle is being compared to a non MC Particle. Please check your script!\n"
              "                              If the MCParticle <-> Particle comparison happens in the RestOfEventBuilder,\n"
              "                              the Rest Of Event may contain signal side particles.");
    return false;
  }
  if (this->getParticleSource() == EParticleSourceObject::c_MCParticle
      and oParticle->getParticleSource() == EParticleSourceObject::c_MCParticle) {
    return this->getMCParticle() == oParticle->getMCParticle();
  }
  if (this->getParticleSource() != oParticle->getParticleSource()) {
    return false;
  }
  if (this->getMdstSource() == oParticle->getMdstSource()) {
    return true;
  }
  if (this->getTrack() && oParticle->getTrack() &&
      this->getTrack()->getArrayIndex() != oParticle->getTrack()->getArrayIndex()) {
    return false;
  }
  if (this->getKLMCluster() && oParticle->getKLMCluster()
      && this->getKLMCluster()->getArrayIndex() != oParticle->getKLMCluster()->getArrayIndex()) {
    return false;
  }

  // It can be a bit more complicated for ECLClusters as we might also have to ensure they are connected-region unique
  if (this->getECLCluster() && oParticle->getECLCluster()
      && this->getECLCluster()->getArrayIndex() != oParticle->getECLCluster()->getArrayIndex()) {

    // if either is a track then they must be different
    if (this->getECLCluster()->isTrack() or oParticle->getECLCluster()->isTrack())
      return false;

    // we cannot combine two particles of different hypotheses from the same
    // connected region (as their energies overlap)
    if (this->getECLClusterEHypothesisBit() == oParticle->getECLClusterEHypothesisBit())
      return false;

    // in the rare case that both are neutral and the hypotheses are different,
    // we must also check that they are from different connected regions
    // otherwise they come from the "same" underlying ECLShower
    if (this->getECLCluster()->getConnectedRegionId() != oParticle->getECLCluster()->getConnectedRegionId())
      return false;
  }
  return true;

}

const Track* Particle::getTrack() const
{
  if (m_particleSource == c_Track) {
    StoreArray<Track> tracks;
    return tracks[m_mdstIndex];
  } else
    return nullptr;
}

const TrackFitResult* Particle::getTrackFitResult() const
{
  // if the particle is related to a TrackFitResult then return this
  auto* selfrelated = this->getRelatedTo<TrackFitResult>();
  if (selfrelated)
    return selfrelated;

  // if not get the TFR with closest mass to this particle
  auto* selftrack = this->getTrack();
  if (selftrack)
    return selftrack->getTrackFitResultWithClosestMass(
             Belle2::Const::ChargedStable(std::abs(this->getPDGCode())));

  // otherwise we're probably not a track based particle
  return nullptr;
}

const PIDLikelihood* Particle::getPIDLikelihood() const
{
  if (m_particleSource == c_Track) {
    StoreArray<Track> tracks;
    return tracks[m_mdstIndex]->getRelated<PIDLikelihood>();
  } else
    return nullptr;
}

const V0* Particle::getV0() const
{
  if (m_particleSource == c_V0) {
    StoreArray<V0> v0s;
    return v0s[m_mdstIndex];
  } else {
    return nullptr;
  }
}


const ECLCluster* Particle::getECLCluster() const
{
  if (m_particleSource == c_ECLCluster) {
    StoreArray<ECLCluster> eclClusters;
    return eclClusters[m_mdstIndex];
  } else if (m_particleSource == c_Track) {
    // a track may be matched to several clusters under different hypotheses
    // take the most energetic of the c_nPhotons hypothesis as "the" cluster
    StoreArray<Track> tracks;
    const ECLCluster* bestTrackMatchedCluster = nullptr;
    double highestEnergy = -1.0;
    // loop over all clusters matched to this track
    for (const ECLCluster& cluster : tracks[m_mdstIndex]->getRelationsTo<ECLCluster>()) {
      // ignore everything except the nPhotons hypothesis
      if (!cluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
        continue;
      // check if we're most energetic thus far
      if (cluster.getEnergy(ECLCluster::EHypothesisBit::c_nPhotons) > highestEnergy) {
        highestEnergy = cluster.getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
        bestTrackMatchedCluster = &cluster;
      }
    }
    return bestTrackMatchedCluster;
  } else {
    return nullptr;
  }
}

double Particle::getECLClusterEnergy() const
{
  const ECLCluster* cluster = this->getECLCluster();
  if (!cluster) return 0;
  return cluster->getEnergy(this->getECLClusterEHypothesisBit());
}

const KLMCluster* Particle::getKLMCluster() const
{
  if (m_particleSource == c_KLMCluster) {
    StoreArray<KLMCluster> klmClusters;
    return klmClusters[m_mdstIndex];
  } else if (m_particleSource == c_Track) {
    // If there is an associated KLMCluster, it's the closest one
    StoreArray<Track> tracks;
    const KLMCluster* klmCluster = tracks[m_mdstIndex]->getRelatedTo<KLMCluster>();
    return klmCluster;
  } else {
    return nullptr;
  }
}


const MCParticle* Particle::getMCParticle() const
{
  if (m_particleSource == c_MCParticle) {
    StoreArray<MCParticle> mcParticles;
    return mcParticles[m_mdstIndex];
  } else {
    const MCParticle* related = this->getRelated<MCParticle>();
    if (related)
      return related;
  }
  return nullptr;
}


const Particle* Particle::getParticleFromGeneralizedIndexString(const std::string& generalizedIndex) const
{
  // Split the generalizedIndex string in a vector of strings.
  std::vector<std::string> generalizedIndexes;
  boost::split(generalizedIndexes, generalizedIndex, boost::is_any_of(":"));

  if (generalizedIndexes.empty()) {
    B2WARNING("Generalized index of daughter particle is empty. Skipping.");
    return nullptr;
  }

  // To explore a decay tree of unknown depth, we need a place to store
  // both the root particle and the daughter particle at each iteration
  const Particle* dauPart =
    nullptr; // This will be eventually returned
  const Particle* currentPart = this; // This is the root particle of the next iteration

  // Loop over the generalizedIndexes until you get to the particle you want
  for (auto& indexString : generalizedIndexes) {
    // indexString is a string. First try to convert it into an int
    int dauIndex = 0;
    try {
      dauIndex = Belle2::convertString<int>(indexString);
    } catch (std::invalid_argument&) {
      B2WARNING("Found the string " << indexString << "instead of a daughter index.");
      return nullptr;
    }

    // Check that the daughter index is smaller than the number of daughters of the current root particle
    if (dauIndex >= int(currentPart->getNDaughters()) or dauIndex < 0) {
      B2WARNING("Daughter index " << dauIndex << " out of range");
      B2WARNING("Trying to access non-existing particle.");
      return nullptr;
    } else {
      dauPart = currentPart->getDaughter(dauIndex); // Pick the particle indicated by the generalizedIndex
      currentPart = dauPart;
    }
  }
  return dauPart;
}



//--- private methods --------------------------------------------

void Particle::setMomentumPositionErrorMatrix(const TrackFitResult* trackFit)
{
  // set momentum
  m_px = trackFit->getMomentum().Px();
  m_py = trackFit->getMomentum().Py();
  m_pz = trackFit->getMomentum().Pz();

  // set position at which the momentum is given (= POCA)
  setVertex(trackFit->getPosition());

  // set Chi^2 probability
  m_pValue = trackFit->getPValue();

  // set error matrix
  const auto cov6 = trackFit->getCovariance6();
  constexpr unsigned order[] = {c_X, c_Y, c_Z, c_Px, c_Py, c_Pz};

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

  const float E = getEnergy();
  const float dEdp[] = {m_px / E, m_py / E, m_pz / E};
  constexpr unsigned compMom[] = {c_Px, c_Py, c_Pz};
  constexpr unsigned compPos[] = {c_X,  c_Y,  c_Z};

  // covariances (p,E)
  for (unsigned int i : compMom) {
    float Cov = 0;
    for (int k = 0; k < 3; k++) {
      Cov += errMatrix(i, compMom[k]) * dEdp[k];
    }
    errMatrix(i, c_E) = Cov;
  }

  // covariances (x,E)
  for (unsigned int comp : compPos) {
    float Cov = 0;
    for (int k = 0; k < 3; k++) {
      Cov += errMatrix(comp, compMom[k]) * dEdp[k];
    }
    errMatrix(c_E, comp) = Cov;
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
  for (float& i : m_errMatrix)
    i = 0.0;
}

void Particle::resetJacobiMatrix()
{
  for (float& i : m_jacobiMatrix)
    i = 0.0;
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

void Particle::storeJacobiMatrix(const TMatrixF& m)
{
  int element = 0;
  for (int irow = 0; irow < 4; irow++) {
    for (int icol = 0; icol < 6; icol++) {
      m_jacobiMatrix[element] = m(irow, icol);
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
  if (m_pdgCode == 43) {m_flavorType = c_Unflavored; return;} // Xu0
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

std::vector<std::string> Particle::getExtraInfoNames() const
{
  std::vector<std::string> out;
  if (!m_extraInfo.empty()) {
    StoreObjPtr<ParticleExtraInfoMap> extraInfoMap;
    if (!extraInfoMap)
      B2FATAL("ParticleExtraInfoMap not available, but needed for storing extra info in Particle!");
    const ParticleExtraInfoMap::IndexMap& map = extraInfoMap->getMap(m_extraInfo[0]);
    for (auto const& ee : map) out.push_back(ee.first);
  }
  return out;
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
  stream << " <b>particleSource</b>=" << m_particleSource;
  stream << " <b>particleTypeUsedForFit</b>=" << m_pdgCodeUsedForFit;
  stream << "<br>";

  stream << " <b>mdstIndex</b>=" << m_mdstIndex;
  stream << " <b>arrayIndex</b>=" << getArrayIndex();
  stream << " <b>identifier</b>=" << m_identifier;
  stream << " <b>daughterIndices</b>: ";
  for (int daughterIndex : m_daughterIndices) {
    stream << daughterIndex << ", ";
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

  stream << " <b>momentum scaling factor</b>=" << m_momentumScale;
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
  const auto mapID = (unsigned int)m_extraInfo[0];
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
  const auto mapID = (unsigned int)m_extraInfo[0];
  StoreObjPtr<ParticleExtraInfoMap> extraInfoMap;
  if (!extraInfoMap) {
    B2FATAL("ParticleExtraInfoMap not available, but needed for storing extra info in Particle!");
  }
  unsigned int index = extraInfoMap->getIndex(mapID, name);
  if (index == 0 or index >= m_extraInfo.size()) //actualy indices start at 1
    throw std::runtime_error(std::string("getExtraInfo: Value '") + name + "' not found in Particle!");

  return m_extraInfo[index];

}

void Particle::writeExtraInfo(const std::string& name, const float value)
{
  if (this->hasExtraInfo(name)) {
    this->setExtraInfo(name, value);
  } else {
    this->addExtraInfo(name, value);
  }
}

void Particle::setExtraInfo(const std::string& name, float value)
{
  if (m_extraInfo.empty())
    throw std::runtime_error(std::string("setExtraInfo: Value '") + name + "' not found in Particle!");

  //get index for name
  const auto mapID = (unsigned int)m_extraInfo[0];
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

bool Particle::forEachDaughter(const std::function<bool(const Particle*)>& function,
                               bool recursive, bool includeSelf) const
{
  std::queue<const Particle*> qq;
  // If we include ourselves add only this, otherwise directly all children
  if (includeSelf) {
    qq.push(this);
  } else {
    for (size_t i = 0; i < getNDaughters(); ++i) qq.push(getDaughter(i));
  }
  // Now just repeat until done: take the child, run the functor, remove the
  // child, add all children if needed
  while (!qq.empty()) {
    const Particle* p = qq.front();
    if (function(p)) return true;
    qq.pop();
    // Add children if we go through all children recursively or if we look at
    // the current particle: we always want the direct children.
    if (recursive || p == this)
      for (size_t i = 0; i < p->getNDaughters(); ++i) qq.push(p->getDaughter(i));
  }
  return false;
}

int Particle::generatePDGCodeFromCharge(const int chargeSign, const Const::ChargedStable& chargedStable)
{
  int absPDGCode = chargedStable.getPDGCode();
  int PDGCode = absPDGCode * chargeSign;
  // flip sign of PDG code for leptons: their PDG code is positive if the lepton charge is negative and vice versa
  if (chargedStable == Const::muon || chargedStable == Const::electron) PDGCode = -PDGCode;
  return PDGCode;
}

bool Particle::isMostLikely() const
{
  const PIDLikelihood* likelihood = Particle::getPIDLikelihood();
  if (likelihood) return likelihood->getMostLikely().getPDGCode() == std::abs(m_pdgCode);
  else return false;
}
