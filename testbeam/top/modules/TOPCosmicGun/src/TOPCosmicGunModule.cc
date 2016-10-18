/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Matthew Barrett                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <testbeam/top/modules/TOPCosmicGun/TOPCosmicGunModule.h>


// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

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
#include <TFile.h>
#include <TF1.h>
#include <TString.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPCosmicGun)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPCosmicGunModule::TOPCosmicGunModule() : Module()

  {
    // set module description
    setDescription("Cosmic ray gun for TOP cosmics tests (no magnetic field!)");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("upperPad", m_upperPad, "Upper rectangular surface (z,x,Dz,Dx,y) in cm",
             m_upperPad);
    addParam("lowerPad", m_lowerPad, "Lower rectangular surface (z,x,Dz,Dx,y) in cm",
             m_lowerPad);
    addParam("alpha", m_alpha, "rotation angle of trigger paddles [deg]", 0.0);
    addParam("swimBackDistance", m_swimBackDistance,
             "swim back a muon by this distance", 0.0);
    addParam("startTime", m_startTime, "Start time in nsec (time at upperPad)", 0.0);
    addParam("momentum", m_momentum,
             "Muon Momentum in GeV/c (for mono-energetic muons).", 3.14159);
    addParam("momentumCutOff", m_momentumCutOff, "Minimum muon momentum in GeV/c", 0.0);
    addParam("momentumDistributionType",  m_momentumDistributionType,
             "Type of momentum distribution to use (monoEnergetic, histogram, polyline)",
             string("monoEnergetic"));
    addParam("momentumHistogramFileName", m_momentumHistogramFileName,
             "Name of root file containing momentum histogram", string(""));
    addParam("momentumHistogramName", m_momentumHistogramName,
             "Name of momentum histogram in file", string(""));
    addParam("momentumPolyline", m_momentumPolyline,
             "momentum distribution given as polyline "
             "(momentum points first, then distribution values)", m_momentumPolyline);
    addParam("angularDistributionType",  m_angularDistributionType,
             "Type of angular distribution to use (None, cosSquared, histogram)",
             string("cosSquared"));
    addParam("angularHistogramFileName", m_angularHistogramFileName,
             "Name of root file containing angular histogram", string(""));
    addParam("angularHistogramName", m_angularHistogramName,
             "Name of angular histogram in file", string(""));

  }

  TOPCosmicGunModule::~TOPCosmicGunModule()
  {
  }

  void TOPCosmicGunModule::initialize()
  {
    // data store objects registration
    StoreArray<MCParticle>::registerPersistent();

    // parameter checks
    if (m_upperPad.size() != 5) {B2FATAL("upperPad not defined, 5 parameters needed.");}
    if (m_lowerPad.size() != 5) {B2FATAL("lowerPad not defined, 5 parameters needed.");}
    if (m_lowerPad[4] >= m_upperPad[4]) {B2INFO("lowerPad is not below UpperPad");}

    if ("monoEnergetic" == m_momentumDistributionType) {
      B2INFO("Generating mono-energetic cosmic ray muons with a momentum of "
             << m_momentum << " GeV/c");

    } else if ("histogram" == m_momentumDistributionType) {
      B2INFO("Generating cosmic muon momentum distribution from a histogram.");
      B2INFO("Using histogram file" << m_momentumHistogramFileName <<
             " to obtain momentum histogram.");
      m_momentumHistogramFile = new TFile(TString(m_momentumHistogramFileName));
      if (m_momentumHistogramFile->IsZombie()) {
        B2ERROR("Could not open ROOT file " << m_momentumHistogramFileName);
        return;
      }

      B2INFO("Reading histogram " << m_momentumHistogramName <<
             " from ROOT file " << m_momentumHistogramFileName << ".");
      m_momentumDistribution = (TH1F*) m_momentumHistogramFile->Get(m_momentumHistogramName.c_str());
      if (!m_momentumDistribution) {
        B2ERROR("Could not read histogram " << m_momentumHistogramName <<
                " from ROOT file " << m_momentumHistogramFileName << ".");
        return;
      }
      double sum = 0;
      unsigned np = m_momentumDistribution->GetNbinsX();
      for (unsigned i = 1; i <= np; i++) {
        if (m_momentumDistribution->GetXaxis()->GetBinCenter(i) > m_momentumCutOff)
          sum += m_momentumDistribution->GetBinContent(i);
      }
      if (sum <= 0) {
        B2ERROR("Distribution is zero above the momentum cutoff");
        return;
      }

    } else if ("polyline" == m_momentumDistributionType) {
      B2INFO("Generating cosmic muon momentum distribution from a polyline.");
      if (m_momentumPolyline.empty()) {
        B2FATAL("polyline is empty - did you forgot to define it?");
      }
      if (m_momentumPolyline.size() % 2 != 0) {
        B2ERROR("polyline size must be an even number");
        return;
      }
      if (m_momentumPolyline.size() < 4) {
        B2ERROR("polyline size must be at least 4");
        return;
      }
      double sum = 0;
      unsigned np = m_momentumPolyline.size() / 2;
      for (unsigned i = 0; i < np; i++) {
        if (m_momentumPolyline[i] > m_momentumCutOff) sum += m_momentumPolyline[i + np];
      }
      if (sum <= 0) {
        B2ERROR("Distribution is zero above the momentum cutoff");
        return;
      }

    } else {
      B2ERROR("Unknown momentum distribution type: " << m_momentumDistributionType);
      return;
    }

    if ("None" == m_angularDistributionType) {
      B2INFO("Generating cosmic ray muons with no angular dependence - "
             "geometrical acceptance only.");

    } else if ("cosSquared" == m_angularDistributionType) {
      B2INFO("Generating cosmic ray muons with a cosine squared angular distribution.");

    } else if ("histogram" == m_angularDistributionType) {
      B2INFO("Generating cosmic muon angular distribution from a histogram.");

      B2INFO("Using histogram file" << m_angularHistogramFileName <<
             " to obtain angular histogram.");
      m_angularHistogramFile = new TFile(TString(m_angularHistogramFileName));
      if (m_angularHistogramFile->IsZombie()) {
        B2ERROR("Could not open ROOT file " << m_angularHistogramFileName);
        return;
      }

      B2INFO("Reading histogram " << m_angularHistogramName <<
             " from ROOT file " << m_angularHistogramFileName << ".");
      m_angularDistribution = (TH1F*) m_angularHistogramFile->Get(m_angularHistogramName.c_str());
      if (!m_angularDistribution) {
        B2ERROR("Could not read histogram " << m_angularHistogramName <<
                " from ROOT file " << m_angularHistogramFileName << ".");
        return;
      }

    } else {
      B2ERROR("Unknown angular distribution type: " << m_angularDistributionType);
      return;
    }

  }

  void TOPCosmicGunModule::beginRun()
  {
  }

  void TOPCosmicGunModule::event()
  {

    // generate points on upper and lower surfaces
    double z1 = m_upperPad[0] + m_upperPad[2] * (gRandom->Rndm() - 0.5);
    double x1 = m_upperPad[1] + m_upperPad[3] * (gRandom->Rndm() - 0.5);
    double y1 = m_upperPad[4];

    double z2 = m_lowerPad[0] + m_lowerPad[2] * (gRandom->Rndm() - 0.5);
    double x2 = m_lowerPad[1] + m_lowerPad[3] * (gRandom->Rndm() - 0.5);
    double y2 = m_lowerPad[4];

    if ("histogram" == m_angularDistributionType || "cosSquared" == m_angularDistributionType) {
      // Note: the theta used in this loop is NOT the theta parameter of the Belle II coordinate system!
      // Theta = 0 here corresponds to vertical cosmic ray muons.
      // The x, y, and z parameters are reordered in this loop to reflect this definition.

      // calculate momentum vector in this coordinate representation:
      TVector3 position1(x1, z1, y1);
      TVector3 position2(x2, z2, y2);

      TVector3 direction = position1 - position2; // Calculate direction muon is coming from.
      double   theta     = direction.Theta();
      double   cosTheta  = direction.CosTheta();

      double fAngle(0);
      if ("histogram"  == m_angularDistributionType) {fAngle = m_angularDistribution->Interpolate(theta);}
      if ("cosSquared" == m_angularDistributionType) {fAngle = cosTheta * cosTheta;}

      while (gRandom->Rndm() > fAngle) {
        //Regenerate muon direction until accepted  by angular distribution.
        //This loop could become inefficient for *very* steep muon selection setups.

        z1 = m_upperPad[0] + m_upperPad[2] * (gRandom->Rndm() - 0.5);
        x1 = m_upperPad[1] + m_upperPad[3] * (gRandom->Rndm() - 0.5);
        y1 = m_upperPad[4];
        z2 = m_lowerPad[0] + m_lowerPad[2] * (gRandom->Rndm() - 0.5);
        x2 = m_lowerPad[1] + m_lowerPad[3] * (gRandom->Rndm() - 0.5);
        y2 = m_lowerPad[4];

        position1 = TVector3(x1, z1, y1);
        position2 = TVector3(x2, z2, y2);
        direction = position1 - position2;
        theta     = direction.Theta();
        cosTheta  = direction.CosTheta();

        if ("histogram"  == m_angularDistributionType) {fAngle = m_angularDistribution->Interpolate(theta);}
        if ("cosSquared" == m_angularDistributionType) {fAngle = cosTheta * cosTheta;}

      }
    }

    // muon hits on paddels
    TVector3 r1(x1, y1, z1);
    TVector3 r2(x2, y2, z2);

    // rotate paddels
    TRotation rot;
    rot.RotateZ(m_alpha * Unit::deg);
    r1.Transform(rot);
    r2.Transform(rot);

    // generate momentum
    double p = m_momentum; //default momentum

    if ("histogram" == m_momentumDistributionType) {
      p = m_momentumDistribution->GetRandom();
      while (p < m_momentumCutOff) {p = m_momentumDistribution->GetRandom();}
    } else if ("polyline" == m_momentumDistributionType) {
      p = randomPolyline(m_momentumPolyline.size() / 2, m_momentumPolyline.data(),
                         m_momentumPolyline.data() + m_momentumPolyline.size() / 2);
      while (p < m_momentumCutOff) {
        p = randomPolyline(m_momentumPolyline.size() / 2, m_momentumPolyline.data(),
                           m_momentumPolyline.data() + m_momentumPolyline.size() / 2);
      }
    }

    // calculate momentum vector
    TVector3 dr = r2 - r1;
    TVector3 dir = dr.Unit();
    TVector3 momentum = p * dir;

    // swim back
    r1 -= m_swimBackDistance * dir;
    double mass = Const::muon.getMass();
    double beta = p / sqrt(p * p + mass * mass);
    double startTime = m_startTime - m_swimBackDistance / beta / Const::speedOfLight;

    // create MCParticles data store
    StoreArray<MCParticle> MCParticles;

    // store generated muon
    MCParticle* part = MCParticles.appendNew();
    part->setPDG(13);
    part->setMassFromPDG();
    part->setStatus(MCParticle::c_PrimaryParticle);
    part->addStatus(MCParticle::c_StableInGenerator);
    part->setProductionVertex(r1);
    part->setProductionTime(startTime);
    part->setMomentum(momentum);
    mass = part->getMass();
    double energy = sqrt(p * p + mass * mass);
    part->setEnergy(energy);

  }


  void TOPCosmicGunModule::endRun()
  {
    if ("histogram" == m_momentumDistributionType) {
      delete m_momentumDistribution;
      m_momentumHistogramFile->Close();
    }
    if ("histogram" == m_angularDistributionType) {
      delete m_angularDistribution;
      m_angularHistogramFile->Close();
    }



  }

  void TOPCosmicGunModule::terminate()
  {
  }


  double TOPCosmicGunModule::randomPolyline(size_t n, const double* x, const double* y)
  {
    std::vector<double> weights(n - 1);
    double sumw(0);
    for (size_t i = 0; i < n - 1; ++i) {
      weights[i] = (x[i + 1] - x[i]) * max(y[i], y[i + 1]);
      sumw += weights[i];
    }
    while (true) {
      double weight = gRandom->Uniform(0, sumw);
      size_t segment(0);
      for (; segment < n - 1; ++segment) {
        weight -= weights[segment];
        if (weight <= 0) break;
      }
      const double x1 = x[segment];
      const double x2 = x[segment + 1];
      const double x = gRandom->Uniform(x1, x2);
      const double y1 = y[segment];
      const double y2 = y[segment + 1];
      const double y = (y2 == y1) ? y1 : y1 + (x - x1) / (x2 - x1) * (y2 - y1);
      const double randY = gRandom->Uniform(0, max(y1, y2));
      if (randY < y) return x;
    }
  }


} // end Belle2 namespace

