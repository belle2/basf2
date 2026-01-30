/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <skim/modules/MdstRounderModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>

#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>

#include <Math/Vector3D.h>

#include <cmath>

using namespace Belle2;

REG_MODULE(MdstRounder);

class Module;

MdstRounderModule::MdstRounderModule() : Module()
{
  setDescription("This module loads all ECLClusters and TrackFitResults present in each event and "
                 "rounds their Double32_t member variables to the precision they"
                 "would have if they were read from an mdst file. Also updates all particles that were "
                 "already created. Useful if you want "
                 "to perform analysis/skimming in the same steering file that also runs"
                 "event generation/simulation/reconstruction.");
}

void MdstRounderModule::initialize() { }

void MdstRounderModule::event()
{
  roundECLClusters();
  roundTrackFitResults();
  updateParticles();
}

void MdstRounderModule::roundECLClusters() const
{
  B2DEBUG(20, "Rounding all Double32_t ECLCluster members to mdst precision");

  const double pi = 3.141592653589793238462643383279502884;

  for (int i = 0; i < m_eclclusters.getEntries(); i++) {
    ECLCluster* cluster = m_eclclusters[i];

    TMatrixDSym covMat = cluster->getCovarianceMatrix3x3();
    double matRounded[6] = {
      covMat(0, 0),
      roundToPrecision(covMat(1, 0), 0.0, 10.0, 12),
      covMat(1, 1),
      roundToPrecision(covMat(2, 0), 0.0, 10.0, 12),
      roundToPrecision(covMat(2, 1), 0.0, 10.0, 12),
      covMat(2, 2)
    };
    cluster->setCovarianceMatrix(matRounded);
    // Set diagonal entries seperately to avoid sqrt after rounding
    cluster->setUncertaintyEnergy(roundToPrecision(cluster->getUncertaintyEnergy(), 0.0, 0.3, 10));
    cluster->setUncertaintyPhi(roundToPrecision(cluster->getUncertaintyPhi(), 0.0, 0.05, 8));
    cluster->setUncertaintyTheta(roundToPrecision(cluster->getUncertaintyTheta(), 0.0, 0.05, 8));

    cluster->setdeltaL(roundToPrecision(cluster->getDeltaL(), -250.0, 250.0, 10));
    cluster->setMinTrkDistance(roundToPrecision(cluster->getMinTrkDistance(), 0.0, 250.0, 10));
    cluster->setAbsZernike40(roundToPrecision(cluster->getAbsZernike40(), 0.0, 1.7, 10));
    cluster->setAbsZernike51(roundToPrecision(cluster->getAbsZernike51(), 0.0, 1.2, 10));
    cluster->setZernikeMVA(roundToPrecision(cluster->getZernikeMVA(), 0.0, 1.0, 10));
    cluster->setE1oE9(roundToPrecision(cluster->getE1oE9(), 0.0, 1.0, 10));
    cluster->setE9oE21(roundToPrecision(cluster->getE9oE21(), 0.0, 1.0, 10));
    cluster->setSecondMoment(roundToPrecision(cluster->getSecondMoment(), 0.0, 40.0, 10));
    cluster->setLAT(roundToPrecision(cluster->getLAT(), 0.0, 1.0, 10));
    cluster->setNumberOfCrystals(roundToPrecision(cluster->getNumberOfCrystals(), 0.0, 200.0, 10));
    cluster->setTime(roundToPrecision(cluster->getTime(), -1000.0, 1000.0, 12));
    cluster->setDeltaTime99(roundToPrecision(cluster->getDeltaTime99(), 0.0, 1000.0, 12));
    cluster->setTheta(roundToPrecision(cluster->getTheta(), 0.0, pi, 16));
    cluster->setPhi(roundToPrecision(cluster->getPhi(), -pi, pi, 16));
    cluster->setR(roundToPrecision(cluster->getR(), 75.0, 300.0, 16));
    cluster->setPulseShapeDiscriminationMVA(roundToPrecision(cluster->getPulseShapeDiscriminationMVA(), 0.0, 1.0, 18));
    cluster->setNumberOfHadronDigits(roundToPrecision(cluster->getNumberOfHadronDigits(), 0.0, 255.0, 18));

    // Behavior of getEnergy depends on cluster hypothesis
    if (!cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {
      cluster->addHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
      cluster->setLogEnergy(roundToPrecision(log(cluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons)), -5.0, 3.0, 18));
      cluster->removeHypothesis(ECLCluster::EHypothesisBit::c_nPhotons);
    } else {
      cluster->setLogEnergy(roundToPrecision(log(cluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons)), -5.0, 3.0, 18));
    }
    cluster->setLogEnergyRaw(roundToPrecision(log(cluster->getEnergyRaw()), -5.0, 3.0, 18));
    cluster->setLogEnergyHighestCrystal(roundToPrecision(log(cluster->getEnergyHighestCrystal()), -5.0, 3.0, 18));
  }
}

void MdstRounderModule::roundTrackFitResults() const
{
  B2DEBUG(20, "Rounding all Double32_t TrackFitResult members to mdst precision");

  for (int i = 0; i < m_trackfitresults.getEntries(); i++) {
    // Since the TrackFitResult constructor expects floats, this copying is enough to get the appropriate rounding
    TrackFitResult* tfr = m_trackfitresults[i];
    const TrackFitResult tfr_copy(tfr->getTau(), tfr->getCov(), tfr->getParticleType(), tfr->getPValue(),
                                  tfr->getHitPatternCDC().getInteger(), tfr->getHitPatternVXD().getInteger(), tfr->getNDF());
    tfr->updateTrackFitResult(tfr_copy);
  }
}

void MdstRounderModule::updateParticles()
{
  B2DEBUG(20, "Updating momenta of all final state particles previously created");

  // Update cluster-, track- and kink-based particles first
  const unsigned int n = m_particles.getEntries();
  for (unsigned i = 0; i < n; i++) {
    Particle* particle = m_particles[i];
    if (particle->getParticleSource() == Particle::EParticleSourceObject::c_ECLCluster) {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        // For cluster based particles, just recalculate 4momentum and update jacobi matrix
        const ROOT::Math::PxPyPzEVector clusterP = m_C.Get4MomentumFromCluster(cluster, particle->getECLClusterEHypothesisBit());
        particle->set4Vector(clusterP);
        particle->updateJacobiMatrix();
      } else {
        B2WARNING("Mdst Rounder module encountered a cluster based particle that has no relation to "
                  "an ECLCluster object.");
      }
    } else if (particle->getParticleSource() == Particle::EParticleSourceObject::c_Track ||
               particle->getParticleSource() == Particle::EParticleSourceObject::c_Kink) {
      const TrackFitResult* tfr = particle->getTrackFitResult();
      if (tfr) {
        // For track based particles, recalculate 4momentum and errormatrix, and update vertex
        const ROOT::Math::XYZVector mom = tfr->getMomentum();
        const double E = sqrt(mom.X() * mom.X() + mom.Y() * mom.Y() + mom.Z() * mom.Z() + particle->getMass() * particle->getMass());
        const ROOT::Math::PxPyPzEVector fourmom(mom.X(), mom.Y(), mom.Z(), E);
        const ROOT::Math::XYZVector vert = tfr->getPosition();
        const double pval = tfr->getPValue();
        const auto cov6 = tfr->getCovariance6();
        const TMatrixFSym errMatrix = getErrMatrixFromCov(cov6, fourmom);
        particle->updateMomentum(fourmom, vert, errMatrix, pval);
      } else {
        B2WARNING("Mdst Rounder module encountered a track based particle that has no relation to "
                  "a TrackFitResult object.");
      }
    }
  }

  // Then, update V0-based particles as they need their daughters to be updated first
  for (unsigned i = 0; i < n; i++) {
    Particle* particle = m_particles[i];
    if (particle->getParticleSource() == Particle::EParticleSourceObject::c_V0) {
      ROOT::Math::PxPyPzEVector mom(0.0, 0.0, 0.0, 0.0);
      std::vector<Particle*> daughters = particle->getDaughters();
      if (daughters.size() == 2) {
        // For V0 based particles, resum the daughter momenta and reset the errormatrix and vertex
        for (Particle* daug : particle->getDaughters()) {
          mom += daug->get4Vector();
        }
        TMatrixFSym errMatrix(Particle::c_DimMatrix);
        for (int j = 0; j < Particle::c_DimMatrix; j++) {
          for (int k = j; k < Particle::c_DimMatrix; k++) {
            errMatrix(j, k) = errMatrix(k, j) = 0.0;
          }
        }
        particle->updateMomentum(mom, ROOT::Math::XYZVector(0.0, 0.0, 0.0), errMatrix, -1);
      } else {
        B2WARNING("Mdst Rounder module encountered a V0 based particle that does not have exactly two daughters.");
      }
    }
  }
}

TMatrixFSym MdstRounderModule::getErrMatrixFromCov(const TMatrixDSym& cov6, const ROOT::Math::PxPyPzEVector& vec) const
{
  // Note: all of the following code is copied almost 1-to-1 from Particle::setMomentumPositionErrorMatrix

  constexpr unsigned order[] = {Particle::c_X, Particle::c_Y, Particle::c_Z, Particle::c_Px, Particle::c_Py, Particle::c_Pz};

  TMatrixFSym errMatrix(Particle::c_DimMatrix);
  for (int i = 0; i < 6; i++) {
    for (int j = i; j < 6; j++) {
      // although it seems to make no sense to fill all elements of the
      // symmetric matrix, it has to be (do not touch this code)
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

  const double dEdp[] = {vec.Px() / vec.E(), vec.Py() / vec.E(), vec.Pz() / vec.E()};
  constexpr unsigned compMom[] = {Particle::c_Px, Particle::c_Py, Particle::c_Pz};
  constexpr unsigned compPos[] = {Particle::c_X,  Particle::c_Y,  Particle::c_Z};

  // covariances (p,E)
  for (unsigned int i : compMom) {
    double Cov = 0;
    for (int k = 0; k < 3; k++) {
      Cov += errMatrix(i, compMom[k]) * dEdp[k];
    }
    errMatrix(i, Particle::c_E) = Cov;
  }

  // covariances (x,E)
  for (unsigned int comp : compPos) {
    double Cov = 0;
    for (int k = 0; k < 3; k++) {
      Cov += errMatrix(comp, compMom[k]) * dEdp[k];
    }
    errMatrix(Particle::c_E, comp) = Cov;
  }

  // variance (E,E)
  double Cov = 0;
  for (int i = 0; i < 3; i++) {
    Cov += errMatrix(compMom[i], compMom[i]) * dEdp[i] * dEdp[i];
  }
  for (int i = 0; i < 3; i++) {
    int k = (i + 1) % 3;
    Cov += 2 * errMatrix(compMom[i], compMom[k]) * dEdp[i] * dEdp[k];
  }
  errMatrix(Particle::c_E, Particle::c_E) = Cov;

  return errMatrix;
}

double MdstRounderModule::roundToPrecision(Double32_t value, double min, double max, int nBits) const
{
  const double range = max - min;
  const double scale = (1 << nBits) / range;
  return min + std::lround((std::clamp(value, min, max) - min) * scale) / scale;
}