/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/modules/aafhinput/AafhInputModule.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(AafhInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

AafhInputModule::AafhInputModule() : Module(), m_initial(BeamParameters::c_smearVertex)
{
  // Set module properties
  setDescription("AAFH Generator to generate non-radiative two-photon events like e+e- -> e+e-e+e-");

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

  //Beam Parameters, initial particle - AAFH cannot handle beam energy spread
  m_initial.initialize();
}

void AafhInputModule::event()
{

  // Check if the BeamParameters have changed (if they do, abort the job! otherwise cross section calculation will be a nightmare.)
  if (m_beamParams.hasChanged()) {
    if (!m_initialized) {
      initializeGenerator();
    } else {
      B2FATAL("AafhInputModule::event(): BeamParameters have changed within a job, this is not supported for AAFH!");
    }
  }

  // Initial particle from beam parameters (for random vertex)
  const MCInitialParticles& initial = m_initial.generate();

  // True boost.
  TLorentzRotation boost = initial.getCMSToLab();

  // vertex.
  TVector3 vertex = initial.getVertex();

  MCParticleGraph mpg;

  //Generate event.
  m_generator.generateEvent(mpg);

  //Boost to lab and set vertex.
  for (size_t i = 0; i < mpg.size(); ++i) {
    mpg[i].set4Vector(boost * mpg[i].get4Vector());

    TVector3 v3 = mpg[i].getProductionVertex();
    v3 = v3 + vertex;
    mpg[i].setProductionVertex(v3);
    mpg[i].setValidVertex(true);
  }

  //Fill MCParticle List
  mpg.generateList(m_mcparticles.getName(), MCParticleGraph::c_setDecayInfo);
}

void AafhInputModule::terminate()
{
  m_generator.finish();
}

void AafhInputModule::initializeGenerator()
{
  const BeamParameters& nominal = m_initial.getBeamParameters();
  const double beamEnergy = nominal.getMass() / 2.;

  //Set Generator options
  if (m_mode < 1 || m_mode > 5) {
    B2ERROR("AafhInputModule::initializeGenerator: 'mode' must be a value between 1 and 5");
  }
  if (m_rejection < 1 || m_rejection > 2) {
    B2ERROR("AafhInputModule::initializeGenerator: 'rejection' must be a value between 1 and 2");
  }
  m_generator.setMaxTries(m_maxTries);
  m_generator.setParticle(m_particle);
  m_generator.setMinimumMass(m_minMass);
  m_generator.setGeneratorWeights(m_subgeneratorWeights);
  m_generator.setSupressionLimits(m_suppressionLimits);
  m_generator.setMaxWeights(m_maxSubgeneratorWeight, m_maxFinalWeight);
  m_generator.initialize(beamEnergy, (AAFHInterface::EMode) m_mode,
                         (AAFHInterface::ERejection)m_rejection);

  m_initialized = true;

}


