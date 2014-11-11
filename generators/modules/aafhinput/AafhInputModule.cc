/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/aafhinput/AafhInputModule.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/GearDir.h>
#include <generators/utilities/cm2LabBoost.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(AafhInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

AafhInputModule::AafhInputModule() : Module()
{
  // Set module properties
  setDescription("AAFH Generator to generate 2-photon events like e+e- -> e+e-e+e-");

  // Parameter definitions
  addParam("mode", m_mode,
           "decay mode to generate."
           " 1: e+e- -> mu+mu-L+L-,"
           " 2: e+e- -> mu+mu-mu+mu-,"
           " 3: e+e- -> e+e-mu+mu-,"
           " 4: e+e- -> e+e-L+L-,"
           " 5: e+e- -> e+e-e+e-. "
           "L is a user defined particle (default: tau)",
           5);
  addParam("rejection", m_rejection,
           "Rejection method."
           " 1: apply once for the final event weight,"
           " 2: apply for the subgenerator weight and for the final weight",
           2);
  addParam("maxTries", m_maxTries,
           "Maximum tries for event generation (ITOT)",
           m_generator.getMaxTries());
  addParam("maxFinalWeight", m_maxFinalWeight,
           "maximum expected final weight for rejection scheme (ESFT)",
           m_generator.getMaxFinalWeight());
  addParam("maxSubgeneratorWeight", m_maxSubgeneratorWeight,
           "maximum expected subgenerator weight for rejection scheme (ESWE)",
           m_generator.getMaxSubGeneratorWeight());
  addParam("subgeneratorWeights", m_subgeneratorWeights,
           "relative weights of the subgenerators: this must be a list of four "
           "or eight values (first four are interpreted as WAP, rest as WBP) "
           "which specify the relativ weights for each of the "
           "four sub generators. The orginial code states that it the program "
           "run most efficient when the maximum weight is equal in all sub "
           "generators and that if one wants to be sure that all peaks in the "
           "differential cross section are accounted the chance to enter each "
           "sub generator should be equal. Values which try to fullfill both "
           "conditions are printed at after generation when the output level "
           "is set to INFO",
           m_generator.getGeneratorWeights());
  addParam("suppressionLimits", m_suppressionLimits,
           "suppression limits for the matrix element calculations. This must "
           "be a list of four values, [FACE, FACM, FACL, PROC]. For FACE, FACM "
           "and FACL these specify the size of the propagator for which the "
           "corresponding spin configurations will be omitted. A value of 1e3 "
           "will calculate the dominat part correctly and a very large value "
           "compared to the mass (i.e. 1e50) will calculate it exactly. PROC "
           "specifies that feynman diagrams which contribute less than 1/PROC "
           "of the biggest contribution are omitted. For exact calculation it "
           "should be very big but 1e9 should be considerably faster without "
           "affecting the result very much",
           m_generator.getSuppressionLimits());
  addParam("minMass", m_minMass,
           "minimum invariant mass of generated fermion pair. Will "
           "automatically be set to at least 2 times the generated particle "
           "mass",
           0.);
  addParam("particle", m_particle,
           "name of the generated particle for mode 1 and 4",
           m_generator.getParticle());
}

void AafhInputModule::initialize()
{
  m_mcparticles.registerInDataStore();

  //FIXME: This is lot of unneeded code. We need general utilities to get the
  //CMS energy and the boost without reading the gearbox
  //FIXME more: This should be in beginRun or we need to check if beam energy
  //changed between runs and find out if aafh can be reinitialized
  GearDir ler("/Detector/SuperKEKB/LER/");
  GearDir her("/Detector/SuperKEKB/HER/");
  const double ELER = ler.getDouble("energy");
  const double EHER = her.getDouble("energy");
  const double crossingAngle = her.getDouble("angle") - ler.getDouble("angle");
  const double angle = her.getDouble("angle");
  m_labBoost = getBoost(EHER, ELER, crossingAngle, angle);
  const double angleLerToB = M_PI - angle;
  const double angleHerToB = crossingAngle - angle;
  TLorentzVector vLer, vHer;
  vLer.SetXYZM(ELER * sin(angleLerToB), 0., ELER * cos(angleLerToB), Const::electronMass);
  vHer.SetXYZM(EHER * sin(angleHerToB), 0., EHER * cos(angleHerToB), Const::electronMass);
  TLorentzVector beamParticle = m_labBoost.Inverse() * (vHer + vLer);
  beamParticle.Print();
  const double beamEnergy = beamParticle.E() / 2.;

  //Set Generator options
  if (m_mode < 1 || m_mode > 5) {
    B2ERROR("AAFH: mode must be a value between 1 and 5");
  }
  if (m_rejection < 1 || m_rejection > 2) {
    B2ERROR("AAFH: rejection must be a value between 1 and 2");
  }
  m_generator.setMaxTries(m_maxTries);
  m_generator.setParticle(m_particle);
  m_generator.setMinimumMass(m_minMass);
  m_generator.setGeneratorWeights(m_subgeneratorWeights);
  m_generator.setSupressionLimits(m_suppressionLimits);
  m_generator.setMaxWeights(m_maxSubgeneratorWeight, m_maxFinalWeight);
  m_generator.initialize(beamEnergy, (AAFHInterface::EMode) m_mode,
                         (AAFHInterface::ERejection)m_rejection);
}

void AafhInputModule::event()
{
  MCParticleGraph mpg;
  //Generate event
  m_generator.generateEvent(mpg);
  //Boost to lab
  for (size_t i = 0; i < mpg.size(); ++i) {
    mpg[i].set4Vector(m_labBoost * mpg[i].get4Vector());
  }
  //Fill MCParticle List
  mpg.generateList(m_mcparticles.getName(), MCParticleGraph::c_setDecayInfo);
}

void AafhInputModule::terminate()
{
  m_generator.finish();
}


