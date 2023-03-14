/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <top/modules/OpticalGun/OpticalGunModule.h>

// TOP headers.
#include <top/geometry/TOPGeometryPar.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TRandom.h>
#include <Math/EulerAngles.h>
#include <TFormula.h>
#include <TF1.h>
#include <top/reconstruction_cpp/func.h>

using namespace std;
using namespace ROOT::Math;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  ///                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(OpticalGun);

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
             "source angular distribution: uniform, Lambertian, an arbitrary TFormula, or  Gaussian "
             "(numerical aperture instead of minAlpha and maxAlpha). If you are writing a TFormula, "
             "assume the angles are measured in degrees. The conversion to radians is done internally.",
             string("Gaussian"));
    addParam("wavelength", m_wavelength, "wavelength of photons [nm]", 405.0);
    addParam("phi", m_phi, "first rotation angle (around z) [deg]", 0.0);
    addParam("theta", m_theta, "second rotation angle (around x) [deg]", 0.0);
    addParam("psi", m_psi, "third rotation angle (around z) [deg]", 0.0);
    addParam("startTime", m_startTime,
             "start time [ns]. If TOPCalPulseGenerator is in path this is relative to the first cal pulse", 0.0);
    addParam("pulseWidth", m_pulseWidth, "pulse duration (Gaussian sigma) [ns]", 0.0);
    addParam("numPhotons", m_numPhotons,
             "average number of photons per pulse, if positive, otherwise exactly one", 0.0);
    addParam("slotID", m_slotID,
             "TOP slot ID (1-16): if valid, source position and rotation angles assumed to be given in a local bar frame, "
             "otherwise Belle II frame is assumed", 0);
    addParam("slitDX", m_slitDX, "slit size in x [cm], if positive, otherwise full open", 0.0);
    addParam("slitDY", m_slitDY, "slit size in y [cm], if positive, otherwise full open", 0.0);
    addParam("slitX0", m_slitX0, "slit x-offset in respect to source [cm] ", 0.0);
    addParam("slitY0", m_slitY0, "slit y-offset in respect to source [cm] ", 0.0);
    addParam("slitZ", m_slitZ, "slit distance to source [cm], if > 0.01, otherwise slit full open", 0.0);
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
      B2FATAL("Minimum emission angle must me smaller than the maximum emission angle");

    if (m_angularDistribution == string("uniform") or
        m_angularDistribution == string("Lambertian") or
        m_angularDistribution == string("Gaussian"))
      B2INFO("Using the  pre-defined angular distribution " << m_angularDistribution);
    else {
      B2INFO(m_angularDistribution << " is not a pre-defined distribution. "
             << "Checking if it's a valid, positively-defined TFormula.");
      TFormula testFormula("testFormula", m_angularDistribution.c_str());
      int result = testFormula.Compile();
      if (result != 0) {
        B2FATAL(m_angularDistribution << " TFormula does not compile.");
      }
      double testPoint = m_minAlpha; // let's test if the function is postive defined everywhere
      while (testPoint < m_maxAlpha) {
        double value = testFormula.Eval(testPoint * Unit::deg);
        if (value < 0) {
          B2FATAL("The formula " << m_angularDistribution << " is not positively defined at the test point "
                  << testPoint << " deg (value = " << value << ")");
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

    EulerAngles ea(-m_phi * Unit::deg, -m_theta * Unit::deg, -m_psi * Unit::deg); // rotation of an object as in TRotation
    m_transform = Transform3D(Rotation3D(ea), Translation3D(m_x, m_y, m_z)); // source positioning and elevation
    if (m_slotID != 0) {
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      if (not geo->isModuleIDValid(m_slotID)) B2FATAL("Slot ID is not valid");
      const auto& T = geo->getModule(m_slotID).getTransformation(); // slot to BelleII
      m_transform = T * m_transform;
    }

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
      XYZPoint point(x, y, 0);

      // generate direction
      XYZVector direction;
      if (m_angularDistribution == string("uniform")) {
        direction = getDirectionUniform();
      } else if (m_angularDistribution == string("Lambertian")) {
        direction = getDirectionLambertian();
      } else if (m_angularDistribution == string("Gaussian")) {
        direction = getDirectionGaussian();
      } else { // we already have tested the formula and initialized the TF1 in the initialize() method
        direction = getDirectionCustom();
      }
      XYZVector momentum = m_energy * direction;

      // check if photon passes the slit
      if (not isInsideSlit(point, direction)) continue;

      // generate polarization vector (unpolarized light source assumed)
      double alphaPol = 2.0 * M_PI * gRandom->Rndm();
      XYZVector polarization(cos(alphaPol), sin(alphaPol), 0);
      func::rotateUz(polarization, direction);

      // generate emission time
      double emissionTime = gRandom->Gaus(startTime, m_pulseWidth);

      // transform to Belle II frame
      point = m_transform * point;
      momentum = m_transform * momentum;
      polarization = m_transform * polarization;

      // store generated photon
      auto* part = m_MCParticles.appendNew();
      part->setPDG(0); // optical photon
      part->setMass(0);
      part->setStatus(MCParticle::c_PrimaryParticle);
      part->addStatus(MCParticle::c_StableInGenerator);
      part->setProductionVertex(static_cast<XYZVector>(point));
      part->setProductionTime(emissionTime);
      part->setMomentum(momentum);
      part->setEnergy(m_energy);
      part->setDecayVertex(polarization); // use this location temporary to pass photon polarization to FullSim
    }

  }


  bool OpticalGunModule::isInsideSlit(const XYZPoint& point,
                                      const XYZVector& direction) const
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

  XYZVector OpticalGunModule::getDirectionGaussian() const
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
    return XYZVector(x, y, sqrt(z));
  }

  XYZVector OpticalGunModule::getDirectionUniform() const
  {
    double cosTheta = (m_cosMinAlpha - m_cosMaxAlpha) * gRandom->Rndm() + m_cosMaxAlpha;
    double sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    double phi = 2.0 * M_PI * gRandom->Rndm();
    return XYZVector(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
  }

  XYZVector OpticalGunModule::getDirectionLambertian() const
  {
    double cosTheta = sqrt((m_cosMinAlpha * m_cosMinAlpha - m_cosMaxAlpha * m_cosMaxAlpha) * gRandom->Rndm() +
                           m_cosMaxAlpha * m_cosMaxAlpha);
    double sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    double phi = 2.0 * M_PI * gRandom->Rndm();
    return XYZVector(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
  }

  XYZVector OpticalGunModule::getDirectionCustom() const
  {
    double alpha = m_customDistribution->GetRandom();
    double phi = 2.0 * M_PI * gRandom->Rndm();
    return XYZVector(cos(phi) * sin(alpha), sin(phi) * sin(alpha), cos(alpha));
  }


} // end Belle2 namespace

