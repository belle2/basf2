/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/OpticalGun/OpticalGunModule.h>

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <mdst/dataobjects/MCParticle.h>

// ROOT
#include <TRandom3.h>
#include <TVector3.h>
#include <TRotation.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(OpticalGun)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  OpticalGunModule::OpticalGunModule() : Module(),
    m_cosAlpha(0), m_energy(0), m_topgp(TOP::TOPGeometryPar::Instance())
  {
    // set module description
    setDescription("Source of optical photons");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("x", m_x, "position in x [cm]", 0.0);
    addParam("y", m_y, "position in y [cm]", 0.0);
    addParam("z", m_z, "position in z [cm]", 0.0);
    addParam("diameter", m_diameter, "source diameter [cm]", 0.0);
    addParam("alpha", m_alpha, "source maximal emission angle [deg]", 0.0);
    addParam("wavelength", m_wavelength, "wavelength of photons [nm]", 405.0);
    addParam("phi", m_phi, "first rotation angle (around z) [deg]", 0.0);
    addParam("theta", m_theta, "second rotation angle (around x) [deg]", 0.0);
    addParam("psi", m_psi, "third rotation angle (around z) [deg]", 0.0);
    addParam("startTime", m_startTime, "start time [ns]", 0.0);
    addParam("pulseWidth", m_pulseWidth, "pulse duration (Gaussian sigma) [ns]", 0.0);
    addParam("numPhotons", m_numPhotons,
             "average number of photons per pulse, if positive, otherwise exactly one",
             0.0);
    addParam("barID", m_barID, "TOP bar ID: if valid, source position and rotation angles assumed to be given in a local bar frame, otherwise Belle II frame assumed", 0);
    addParam("slitDX", m_slitDX, "slit size in x [cm], if positive, otherwise full open",
             0.0);
    addParam("slitDY", m_slitDY, "slit size in y [cm], if positive, otherwise full open",
             0.0);
    addParam("slitX0", m_slitX0, "slit x-offset in respect to source [cm] ", 0.0);
    addParam("slitY0", m_slitY0, "slit y-offset in respect to source [cm] ", 0.0);
    addParam("slitZ", m_slitZ, "slit distance to source [cm], if > 0.01, otherwise none",
             0.0);
    addParam("angularDistribution", m_angularDistribution,
             "source angular distribution: uniform, Lambertian", string("uniform"));
  }

  OpticalGunModule::~OpticalGunModule()
  {
  }

  void OpticalGunModule::initialize()
  {
    // data store objects registration
    StoreArray<MCParticle>::registerPersistent();

    // parameters check
    if (m_wavelength < 150 || m_wavelength > 1000)
      B2FATAL("Wavelength does not correspond to optical photons");

    // set other private variables
    m_cosAlpha = cos(m_alpha * Unit::deg);
    m_energy = 1240.0 / m_wavelength * Unit::eV;

    double barY0 = 0;
    double barZ0 = 0;
    double barPhi = 0;
    if (m_barID != 0) {
      int Nbars = m_topgp->getNbars();
      if (Nbars == 0) B2ERROR("TOP bars are not defined");
      if (m_barID < 0 || m_barID > Nbars) {
        B2ERROR("barID = " << m_barID << " : not a valid ID");
        m_barID = 0;
      } else {
        m_topgp->setBasfUnits();
        barY0 = m_topgp->getRadius() + m_topgp->getQthickness() / 2.0;
        barZ0 = (m_topgp->getZ1() + m_topgp->getZ2()) / 2.0;
        barPhi = m_topgp->getPhi0() - 0.5 * M_PI + 2.0 * M_PI / Nbars * (m_barID - 1);
      }
    }
    m_translate.SetXYZ(m_x, m_y + barY0, m_z + barZ0);
    m_rotate.RotateXEulerAngles(m_phi * Unit::deg,
                                m_theta * Unit::deg,
                                m_psi * Unit::deg);
    m_rotateBar.RotateZ(barPhi);

  }

  void OpticalGunModule::beginRun()
  {
  }

  void OpticalGunModule::event()
  {

    // if not already existed, create MCParticles data store
    StoreArray<MCParticle> MCParticles;
    if (!MCParticles.isValid()) MCParticles.create();

    // generate number of photons
    int numPhotons = 1;
    if (m_numPhotons > 0) numPhotons = gRandom->Poisson(m_numPhotons);

    // generate photons and store them to MCParticles
    for (int i = 0; i < numPhotons; i++) {

      // generate emission point
      double x, y;
      do {
        x = m_diameter * (gRandom->Rndm() - 0.5);
        y = m_diameter * (gRandom->Rndm() - 0.5);
      } while (x * x + y * y > m_diameter * m_diameter / 4.0);
      TVector3 point(x, y, 0);

      // generate direction
      double cosTheta = 1.0;
      if (m_angularDistribution == string("uniform")) {
        cosTheta = (1.0 - m_cosAlpha) * gRandom->Rndm() + m_cosAlpha;
      } else if (m_angularDistribution == string("Lambertian")) {
        cosTheta = sqrt((1.0 - m_cosAlpha * m_cosAlpha) * gRandom->Rndm() +
                        m_cosAlpha * m_cosAlpha);
      } else { // default: uniform
        cosTheta = (1.0 - m_cosAlpha) * gRandom->Rndm() + m_cosAlpha;
        B2WARNING("Angular distribution '" << m_angularDistribution <<
                  "' not available, uniform generated instead");
      }
      double sinTheta = sqrt(1.0 - cosTheta * cosTheta);
      double phi = 2.0 * M_PI * gRandom->Rndm();
      TVector3 direction(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
      TVector3 momentum = m_energy * direction;

      // check if photon flies through the slit
      if (!isInsideSlit(point, direction)) continue;

      // generate polarization vector (unpolarized light source assumed)
      double alpha = 2.0 * M_PI * gRandom->Rndm();
      TVector3 polarization(cos(alpha), sin(alpha), 0);
      polarization.RotateUz(direction);

      // generate emission time
      double startTime = gRandom->Gaus(m_startTime, m_pulseWidth);

      // transform to Belle II frame
      momentum.Transform(m_rotate);
      polarization.Transform(m_rotate);
      point.Transform(m_rotate);
      point += m_translate;
      if (m_barID > 0) {
        momentum.Transform(m_rotateBar);
        polarization.Transform(m_rotateBar);
        point.Transform(m_rotateBar);
      }

      // store generated photon
      MCParticle* part = MCParticles.appendNew();
      part->setPDG(0); // optical photon
      part->setMass(0);
      part->setStatus(MCParticle::c_PrimaryParticle);
      part->addStatus(MCParticle::c_StableInGenerator);
      part->setProductionVertex(point);
      part->setProductionTime(startTime);
      part->setMomentum(momentum);
      part->setEnergy(m_energy);
      part->setDecayVertex(polarization); // use this location temporary
    }

  }


  void OpticalGunModule::endRun()
  {
  }

  void OpticalGunModule::terminate()
  {
  }

  void OpticalGunModule::printModuleParams() const
  {
  }

  bool OpticalGunModule::isInsideSlit(const TVector3& point,
                                      const TVector3& direction) const
  {
    if (m_slitZ < 0.01) return true; // no screen with a slit is put infront of a source
    if (direction.Z() < 1.0e-6) return false; // must fly toward the slit

    double pathLength = (m_slitZ - point.Z()) / direction.Z();
    if (m_slitDX > 0) {
      double x = point.X() + pathLength * direction.X();
      if (abs(x - m_slitX0) > m_slitDX / 2.0) return false;
    }
    if (m_slitDY > 0) {
      double y = point.Y() + pathLength * direction.Y();
      if (abs(y - m_slitY0) > m_slitDY / 2.0) return false;
    }

    return true;
  }


} // end Belle2 namespace

