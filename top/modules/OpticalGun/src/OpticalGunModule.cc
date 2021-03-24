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
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TRandom.h>
#include <TVector3.h>
#include <TRotation.h>
#include <TFormula.h>
#include <TF1.h>

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
    addParam("minAlpha", m_minAlpha, "source minimum emission angle [deg]. ", 0.0);
    addParam("maxAlpha", m_maxAlpha, "source maximum emission angle [deg]. ", 30.);
    addParam("na", m_na, "source numerical aperture. It is used only by the Gaussian distribution", 0.50);
    addParam("angularDistribution", m_angularDistribution,
             "source angular distribution: uniform, Lambertian, an arbitrary TFormula, or  Gaussian (only one to use na and ignore m_minAlpha dn m_maxAlpha). If you are writing a TFormula, assume the angles are measured in degrees. The conversion to rad is done internally.",
             string("Gaussian"));
    addParam("wavelength", m_wavelength, "wavelength of photons [nm]", 405.0);
    addParam("phi", m_phi, "first rotation angle (around z) [deg]", 0.0);
    addParam("theta", m_theta, "second rotation angle (around x) [deg]", 0.0);
    addParam("psi", m_psi, "third rotation angle (around z) [deg]", 0.0);
    addParam("startTime", m_startTime,
             "start time [ns]. If TOPCalPulseGenerator is in path this is relative to the first cal pulse", 0.0);
    addParam("pulseWidth", m_pulseWidth, "pulse duration (Gaussian sigma) [ns]", 0.0);
    addParam("numPhotons", m_numPhotons,
             "average number of photons per pulse, if positive, otherwise exactly one",
             0.0);
    addParam("slotID", m_slotID,
             "TOP slot ID (1-16): if valid, source position and rotation angles assumed to be given in a local bar frame, otherwise Belle II frame assumed",
             0);
    addParam("slitDX", m_slitDX, "slit size in x [cm], if positive, otherwise full open",
             0.0);
    addParam("slitDY", m_slitDY, "slit size in y [cm], if positive, otherwise full open",
             0.0);
    addParam("slitX0", m_slitX0, "slit x-offset in respect to source [cm] ", 0.0);
    addParam("slitY0", m_slitY0, "slit y-offset in respect to source [cm] ", 0.0);
    addParam("slitZ", m_slitZ, "slit distance to source [cm], if > 0.01, otherwise none",
             0.0);
  }

  OpticalGunModule::~OpticalGunModule()
  {
    if (m_customDistribution) delete m_customDistribution;
  }

  void OpticalGunModule::initialize()
  {
    // data store objects registration
    m_MCParticles.registerInDataStore();
    m_simCalPulses.isOptional();

    // parameters check
    if (m_wavelength < 150 or m_wavelength > 1000)
      B2FATAL("Wavelength does not correspond to optical photons.");
    if (m_na < 0 or m_na > 1)
      B2FATAL("Numerical aperture must be between 0 and 1.");
    if (m_minAlpha < 0)
      B2FATAL("Minimum emission angle must be positive");
    if (m_maxAlpha < 0)
      B2FATAL("Maximum emission angle must be positive");
    if (m_minAlpha >= m_maxAlpha)
      B2FATAL("Minimum emission angle msut me smaller than the maximum emission angle");


    if (m_angularDistribution == string("uniform") or
        m_angularDistribution == string("Lambertian") or
        m_angularDistribution == string("Gaussian"))
      B2INFO("Using the  pre-defined angular distribution " << m_angularDistribution);
    else {
      B2INFO(m_angularDistribution << " is not a pre-defined distribution. Checking if it's a valid, positively-defined TFormula.");
      TFormula testFormula("testFormula", m_angularDistribution.c_str());
      int result = testFormula.Compile();
      if (result != 0) {
        B2FATAL(m_angularDistribution << " is not a valid angular distribution keyword, or it's a TFormula that does not compile.");
      }
      double testPoint = m_minAlpha; // let's test if the function is postive defined everywhere
      while (testPoint < m_maxAlpha) {
        double value = testFormula.Eval(testPoint * Unit::deg);
        if (value < 0) {
          B2FATAL("The formula " << m_angularDistribution << " is not positively defined in the test point " << testPoint << " deg (value = "
                  <<
                  value << ")");
        }
        testPoint += (m_maxAlpha - m_minAlpha) / 100.;
      }
      m_customDistribution = new TF1("m_customDistribution", m_angularDistribution.c_str(), m_minAlpha * Unit::deg,
                                     m_maxAlpha * Unit::deg);
    }



    // set other private variables
    m_cosMinAlpha = cos(m_minAlpha * Unit::deg);
    m_cosMaxAlpha = cos(m_maxAlpha * Unit::deg);
    m_energy = TOPGeometryPar::c_hc / m_wavelength * Unit::eV;

    double barY0 = 0;
    double barZ0 = 0;
    double barPhi = 0;
    if (m_slotID != 0) {
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      int Nbars = geo->getNumModules();
      if (Nbars == 0) B2ERROR("TOP bars are not defined");
      if (m_slotID < 0 or m_slotID > Nbars) {
        B2ERROR("barID = " << m_slotID << " : not a valid ID");
        m_slotID = 0;
      } else {
        barY0 = geo->getModule(m_slotID).getRadius();
        barZ0 = geo->getModule(m_slotID).getZc();
        barPhi = geo->getModule(m_slotID).getPhi() - M_PI / 2;
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
      } else { // we already have tested the formula and initialized the TF1 in the initialize() method
        direction = getDirectionCustom();
      }
      TVector3 momentum = m_energy * direction;

      // check if photon flies through the slit
      if (!isInsideSlit(point, direction)) continue;

      // generate polarization vector (unpolarized light source assumed)
      double alphaPol = 2.0 * M_PI * gRandom->Rndm();
      TVector3 polarization(cos(alphaPol), sin(alphaPol), 0);
      polarization.RotateUz(direction);

      // generate emission time
      double emissionTime = gRandom->Gaus(startTime, m_pulseWidth);

      // transform to Belle II frame
      momentum.Transform(m_rotate);
      polarization.Transform(m_rotate);
      point.Transform(m_rotate);
      point += m_translate;
      if (m_slotID > 0) {
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
    double cosTheta = (m_cosMinAlpha - m_cosMaxAlpha) * gRandom->Rndm() + m_cosMaxAlpha;
    double sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    double phi = 2.0 * M_PI * gRandom->Rndm();
    return TVector3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
  }

  TVector3 OpticalGunModule::getDirectionLambertian() const
  {
    double cosTheta = sqrt((m_cosMinAlpha * m_cosMinAlpha - m_cosMaxAlpha * m_cosMaxAlpha) * gRandom->Rndm() +
                           m_cosMaxAlpha * m_cosMaxAlpha);
    double sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    double phi = 2.0 * M_PI * gRandom->Rndm();
    return TVector3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
  }

  TVector3 OpticalGunModule::getDirectionCustom() const
  {
    double alpha = m_customDistribution->GetRandom();
    double phi = 2.0 * M_PI * gRandom->Rndm();
    return TVector3(cos(phi) * sin(alpha), sin(phi) * sin(alpha), cos(alpha));
  }


} // end Belle2 namespace

