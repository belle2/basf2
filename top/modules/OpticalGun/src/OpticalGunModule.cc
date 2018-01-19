/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *               Stefano Lacaprara                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/OpticalGun/OpticalGunModule.h>
#include <top/geometry/TOPGeometryPar.h>


// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TRandom.h>
#include <TVector3.h>
#include <TRotation.h>

using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(OpticalGun)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  OpticalGunModule::OpticalGunModule() : Module()
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
    addParam("na", m_na, "source numerical aperture", 0.50);
    addParam("wavelength", m_wavelength, "wavelength of photons [nm]", 405.0);
    addParam("phi", m_phi, "first rotation angle (around z) [deg]", 0.0);
    addParam("theta", m_theta, "second rotation angle (around x) [deg]", 0.0);
    addParam("psi", m_psi, "third rotation angle (around z) [deg]", 0.0);
    addParam("startTime", m_startTime, "start time [ns]", 0.0);
    addParam("pulseWidth", m_pulseWidth, "pulse duration (Gaussian sigma) [ns]", 0.0);
    addParam("numPhotons", m_numPhotons,
             "average number of photons per pulse, if positive, otherwise exactly one",
             0.0);
    addParam("barID", m_barID,
             "TOP bar ID: if valid, source position and rotation angles assumed to be given in a local bar frame, otherwise Belle II frame assumed",
             0);
    addParam("slitDX", m_slitDX, "slit size in x [cm], if positive, otherwise full open",
             0.0);
    addParam("slitDY", m_slitDY, "slit size in y [cm], if positive, otherwise full open",
             0.0);
    addParam("slitX0", m_slitX0, "slit x-offset in respect to source [cm] ", 0.0);
    addParam("slitY0", m_slitY0, "slit y-offset in respect to source [cm] ", 0.0);
    addParam("slitZ", m_slitZ, "slit distance to source [cm], if > 0.01, otherwise none",
             0.0);
    addParam("angularDistribution", m_angularDistribution,
             "source angular distribution: uniform, Lambertian, Gaussian (uses na)", string("Gaussian"));
  }

  OpticalGunModule::~OpticalGunModule()
  {
  }

  void OpticalGunModule::initialize()
  {
    // data store objects registration
    m_MCParticles.registerInDataStore();
    m_simCalPulses.isOptional();

    // parameters check
    if (m_wavelength < 150 or m_wavelength > 1000)
      B2FATAL("Wavelength does not correspond to optical photons");
    if (m_na < 0 or m_na > 1)
      B2FATAL("Numerical aperture must be between 0 and 1");

    // set other private variables
    m_cosAlpha = cos(m_alpha * Unit::deg);
    m_energy = 1240.0 / m_wavelength * Unit::eV;

    double barY0 = 0;
    double barZ0 = 0;
    double barPhi = 0;
    if (m_barID != 0) {
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      int Nbars = geo->getNumModules();
      if (Nbars == 0) B2ERROR("TOP bars are not defined");
      if (m_barID < 0 || m_barID > Nbars) {
        B2ERROR("barID = " << m_barID << " : not a valid ID");
        m_barID = 0;
      } else {
        barY0 = geo->getModule(m_barID).getRadius();
        barZ0 = geo->getModule(m_barID).getZc();
        barPhi = geo->getModule(m_barID).getPhi() - M_PI / 2;
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

    // generate number of photons
    int numPhotons = 1;
    if (m_numPhotons > 0) numPhotons = gRandom->Poisson(m_numPhotons);

    // start time
    double startTime = m_startTime;
    if (m_simCalPulses.getEntries() > 0) { // TOPCalPulseGenerator in the path
      startTime += m_simCalPulses[0]->getTime(); // set start time w.r.t cal pulse
    }

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
      TVector3 direction;
      if (m_angularDistribution == string("uniform")) {
        direction = getDirectionUniform();
      } else if (m_angularDistribution == string("Lambertian")) {
        direction = getDirectionLambertian();
      } else if (m_angularDistribution == string("Gaussian")) {
        direction = getDirectionGaussian();
      } else {
        B2FATAL("Wrong source angular distribution. Available ones are: uniform, Lambertian, Gaussian(default)");
      }
      TVector3 momentum = m_energy * direction;

      // check if photon flies through the slit
      if (!isInsideSlit(point, direction)) continue;

      // generate polarization vector (unpolarized light source assumed)
      double alpha = 2.0 * M_PI * gRandom->Rndm();
      TVector3 polarization(cos(alpha), sin(alpha), 0);
      polarization.RotateUz(direction);

      // generate emission time
      double emissionTime = gRandom->Gaus(startTime, m_pulseWidth);

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
      auto* part = m_MCParticles.appendNew();
      part->setPDG(0); // optical photon
      part->setMass(0);
      part->setStatus(MCParticle::c_PrimaryParticle);
      part->addStatus(MCParticle::c_StableInGenerator);
      part->setProductionVertex(point);
      part->setProductionTime(emissionTime);
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

  TVector3 OpticalGunModule::getDirectionGaussian() const
  {
    // NA is defined as the aperture where the amplitude is 5% of that of the
    // peak, which translates into 2.45 sigma for a gaussian distribution
    double x = 0;
    double y = 0;
    double z = 0;
    do {
      x = gRandom->Gaus(0., asin(m_na) / 2.45);
      y = gRandom->Gaus(0., asin(m_na) / 2.45);
      z = 1. - x * x - y * y;
    } while (z < 0);
    return TVector3(x, y, sqrt(z));
  }

  TVector3 OpticalGunModule::getDirectionUniform() const
  {
    double cosTheta = (1.0 - m_cosAlpha) * gRandom->Rndm() + m_cosAlpha;
    double sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    double phi = 2.0 * M_PI * gRandom->Rndm();
    return TVector3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
  }

  TVector3 OpticalGunModule::getDirectionLambertian() const
  {
    double cosTheta = sqrt((1.0 - m_cosAlpha * m_cosAlpha) * gRandom->Rndm() +
                           m_cosAlpha * m_cosAlpha);
    double sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    double phi = 2.0 * M_PI * gRandom->Rndm();
    return TVector3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
  }


} // end Belle2 namespace

