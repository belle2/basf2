/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sascha Dreyer, Savino Longo                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/GeneratedVertexDisplacerModule.h>
#include <framework/core/ModuleParam.templateDetails.h>



using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(GeneratedVertexDisplacer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeneratedVertexDisplacerModule::GeneratedVertexDisplacerModule() : Module()
{
  // Set module properties
  setDescription(
    R"DOC(""Takes a list of PDG values and lifetime paramters to displaces the vertex of MCParticles with matching PDG value corresponding to the given lifetime parameter. Can be used betwenerator and the detector simulation.)DOC");

  // Parameter definitions
  addParam("lifetimeOption", m_lifetimeOption,
           "Set the lifetime option, either 'fixed', 'flat' or 'exponential'. Option is set globally.", string("fixed"));
  addParam("lifetime", m_lifetime,
           "Set the numerical value of the lifetime c*tau in units of [cm]. Example: (lifetime_1, lifetime_2, ... ,lifetime_N). Lifetime_X is set for PDG_X.",
           std::vector<float> {0});
  addParam("pdgVal", m_pdgVals,
           "PDG values of MCParticles that should be displaced. Subsequent daughters of these will be displaced accordingly. Example: (PDG_1, PDG_2, ... , PDG_N). PDG_X corresponds to lifetime_X.",
           std::vector<int> { -999});
  addParam("maxDecayTime", m_maxDecayTime,
           "Set the maximal allowed decay time in c*tau [cm] for the option 'flat'. The 'exponential' option is not bound. Default 300cm.",
           static_cast<float>(300));
  addParam("ctau", m_ctau,
           "Input unit option. True (default): module expects lifetime as ctau [cm]. False: lifetime as tau [ns].",
           true);

}


void GeneratedVertexDisplacerModule::initialize()
{

  B2DEBUG(0, "Initialize GeneratedVertexDisplacerModule");
  m_mcparticles.isRequired(m_particleList);

  if (m_pdgVals.size() != m_lifetime.size()) {
    B2FATAL("List of PDG values and lifetime parameters have different sizes. Specify a lifetime for each PDG value.");
  }

  if ((m_lifetimeOption.compare("fixed") != 0) && (m_lifetimeOption.compare("flat") != 0)
      && (m_lifetimeOption.compare("exponential") != 0)) {
    B2FATAL("Lifetime option must be 0 (fixed), 1 (flat) or 2 (exponential).");
  }
}


void GeneratedVertexDisplacerModule::event()
{

  if (m_mcparticles.getEntries() < 1) {
    B2WARNING("MC particle container empty. Calling next event.");
    return;
  }

  for (int mcp_index = 0; mcp_index < m_mcparticles.getEntries(); mcp_index++) {

    MCParticle& mcp = *m_mcparticles[mcp_index];

    if (!(mcp.hasStatus(MCParticle::c_PrimaryParticle)) || (mcp.hasStatus(MCParticle::c_Initial))) continue;

    int mcp_pdg = mcp.getPDG();

    for (unsigned int param_index = 0; param_index < m_pdgVals.size(); param_index++) {
      if (m_pdgVals.at(param_index) == mcp_pdg) {
        B2DEBUG(0, "Displacing Vertex of particle with pdg_id: " << mcp_pdg << " with lifetime: " << m_lifetime.at(
                  param_index) << " and option: " << m_lifetimeOption);
        displace(mcp, m_lifetime.at(param_index));
        B2DEBUG(0, "new vertex (x,y,z) at:" << "(" << mcp.getDecayVertex().X() << "," << mcp.getDecayVertex().Y() << "," <<
                mcp.getDecayVertex().Z() << ")" << " with decaylength=" << std::sqrt(std::pow(mcp.getDecayVertex().X(),
                    2) + std::pow(mcp.getDecayVertex().Y(), 2) + std::pow(mcp.getDecayVertex().Z(), 2))) ;
      }
    }
  }
}


void GeneratedVertexDisplacerModule::displace(MCParticle& particle, float lifetime)
{
  TLorentzVector* displacementVector = new TLorentzVector();
  getDisplacement(particle, lifetime, *displacementVector);

  TVector3 newDecayVertex = particle.getProductionVertex();
  newDecayVertex.SetX(newDecayVertex.X() + displacementVector->X());
  newDecayVertex.SetY(newDecayVertex.Y() + displacementVector->Y());
  newDecayVertex.SetZ(newDecayVertex.Z() + displacementVector->Z());

  particle.setDecayVertex(newDecayVertex);
  particle.setDecayTime(particle.getProductionTime() + displacementVector->T());
  particle.setValidVertex(true);

  // displace direct daughters
  if (particle.getNDaughters()) {
    displaceDaughter(*displacementVector, particle.getDaughters());
  }

  delete displacementVector;
}


void GeneratedVertexDisplacerModule::displaceDaughter(TLorentzVector& motherDisplacementVector, std::vector<MCParticle*> daughters)
{
  for (unsigned int daughter_index = 0; daughter_index < daughters.size(); daughter_index++) {

    MCParticle* daughter_mcp = daughters.at(daughter_index);
    int daughter_mcpArrayIndex = daughter_mcp->getArrayIndex();

    // getDaughters returns a copied list, need to change parameters of the original particle in the MCParticle StoreArray.
    MCParticle& mcp = *m_mcparticles[daughter_mcpArrayIndex];
    mcp.setProductionVertex(mcp.getProductionVertex() + motherDisplacementVector.Vect());
    mcp.setProductionTime(mcp.getProductionTime() + motherDisplacementVector.T());
    mcp.setValidVertex(true);

    // Displace subsequent daughters
    if (daughter_mcp->getNDaughters()) {
      displaceDaughter(motherDisplacementVector, daughter_mcp->getDaughters());
    }
  }
}


void GeneratedVertexDisplacerModule::getDisplacement(
  const MCParticle& particle, float lifetime, TLorentzVector& displacement)
{
  TLorentzVector fourVector_mcp = particle.get4Vector();
  float decayLength_mcp = 0;
  if (!m_ctau) lifetime *= Const::speedOfLight;

  if (m_lifetimeOption.compare("fixed") == 0) decayLength_mcp = lifetime;
  else if (m_lifetimeOption.compare("flat") == 0) decayLength_mcp = gRandom->Uniform(0, m_maxDecayTime);
  else {
    decayLength_mcp = -1 * std::log(gRandom->Uniform(0, 1.)) * lifetime * fourVector_mcp.Gamma() * fourVector_mcp.Beta();

    if (!particle.getMass()) {
      B2WARNING("Displacing a particle with zero mass. Forcing Gamma=1 for the decay time.");
      decayLength_mcp = -1 * std::log(gRandom->Uniform(0, 1.)) * lifetime;
    }
  }

  float pMag = fourVector_mcp.P();
  displacement.SetX(decayLength_mcp * fourVector_mcp.X() / pMag);
  displacement.SetY(decayLength_mcp * fourVector_mcp.Y() / pMag);
  displacement.SetZ(decayLength_mcp * fourVector_mcp.Z() / pMag);
  displacement.SetT(decayLength_mcp / (Const::speedOfLight * fourVector_mcp.Beta()));
}


void GeneratedVertexDisplacerModule::terminate()
{

}


