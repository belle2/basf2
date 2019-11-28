/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sascha Dreyer                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/GeneratedVertexDisplacerModule.h>

// needed for std::vector as input
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

    if (!(mcp.hasStatus(MCParticle::c_PrimaryParticle)) || (mcp.hasStatus(MCParticle::c_Initial))) return;
    // what about virtual? will the S-particle have the virtual-flag?
    if (mcp.hasStatus(MCParticle::c_IsVirtual)) return;
    int mcp_pdg = mcp.getPDG();

    for (unsigned int param_index = 0; param_index < m_pdgVals.size(); param_index++) {
      if (m_pdgVals.at(param_index) == mcp_pdg) {
        //skip if particle already displaced -- could happen if it is (subsequent) daughter of a previously displaced mother
        if (std::find(displaced_particles.begin(), displaced_particles.end(), mcp.getIndex()) != displaced_particles.end()) return;
        B2DEBUG(0, "Displacing Vertex of particle with pdg_id: " << mcp_pdg << " with lifetime: " << m_lifetime.at(
                  param_index) << " and option: " << m_lifetimeOption);
        displace(mcp, m_lifetime.at(param_index));
        B2DEBUG(0, "new vertex (x,y,z) at:" << "(" << mcp.getDecayVertex().X() << "," << mcp.getDecayVertex().Y() << "," <<
                mcp.getDecayVertex().Z() << ")" << " with decaylength=" << std::sqrt(std::pow(mcp.getDecayVertex().X(),
                    2) + std::pow(mcp.getDecayVertex().Y(), 2) + std::pow(mcp.getDecayVertex().Z(), 2))) ;
      }
    }
  }
  displaced_particles.clear();
}


void GeneratedVertexDisplacerModule::displace(MCParticle& particle, float lifetime)
{
  // misusing TLV class to pass X,Y,Z and time of the vertex
  TLorentzVector* displaceVertex = new TLorentzVector();
  getDisplacement(particle, lifetime, *displaceVertex);

  TVector3 mother_decVtx = particle.getDecayVertex();

  mother_decVtx.SetX(mother_decVtx.X() + displaceVertex->X());
  mother_decVtx.SetY(mother_decVtx.Y() + displaceVertex->Y());
  mother_decVtx.SetZ(mother_decVtx.Z() + displaceVertex->Z());

  particle.setDecayVertex(mother_decVtx);
  particle.setDecayTime(displaceVertex->T());
  particle.setValidVertex(true);

  displaced_particles.push_back(particle.getIndex());

  // displace first daughters
  if (particle.getNDaughters()) {
    displaceDaughter(particle, particle.getDaughters());
  }
}


void GeneratedVertexDisplacerModule::displaceDaughter(MCParticle& particle, std::vector<MCParticle*> daughters)
{
  for (unsigned int daughter_index = 0; daughter_index < daughters.size(); daughter_index++) {

    MCParticle* daughter_mcp = daughters.at(daughter_index);
    int daughter_mcpIndex = daughter_mcp->getIndex();
    TVector3 daughter_prodVtx = daughter_mcp->getProductionVertex();
    TVector3 mother_decVtx = particle.getDecayVertex();

    daughter_prodVtx.SetX(daughter_prodVtx.X() + mother_decVtx.X());
    daughter_prodVtx.SetY(daughter_prodVtx.Y() + mother_decVtx.Y());
    daughter_prodVtx.SetZ(daughter_prodVtx.Z() + mother_decVtx.Z());

    // getDaughters returns a copied list, need to change parameters of the original particle in the MCParticle StoreArray.
    MCParticle& mcp = *m_mcparticles[daughter_mcpIndex];
    mcp.setProductionVertex(particle.getDecayVertex());
    mcp.setProductionTime(particle.getDecayTime());
    mcp.setValidVertex(true);

    displaced_particles.push_back(daughter_mcpIndex);

    // Displace subsequent daughters
    if (daughter_mcp->getNDaughters()) {
      displaceDaughter(particle, daughter_mcp->getDaughters());
    }
  }
}


void GeneratedVertexDisplacerModule::getDisplacement(MCParticle& particle, float lifetime, TLorentzVector& displacement)
{
  TLorentzVector fourVector_mcp = particle.get4Vector();
  float lifetime_mcp = 0;

  if (m_lifetimeOption.compare("fixed") == 0)
    lifetime_mcp = lifetime;
  else if (m_lifetimeOption.compare("flat") == 0) {
    TF1 flat_lifetime("flat", "1", 0, 1000000);
    // ad-hoc cut at 5*lifetime
    flat_lifetime.SetRange(0, 5 * lifetime);
    lifetime_mcp = flat_lifetime.GetRandom();
  } else  {
    TF1 exp_lifetime("exp(x)", "exp(-x/[0])", 0, 1000000);
    exp_lifetime.SetParameter(0, fourVector_mcp.Gamma()*lifetime);
    lifetime_mcp = exp_lifetime.GetRandom();
  }

  // calculate the magnitude of the displacement from the lifetime
  float decayLength = fourVector_mcp.Beta() * lifetime_mcp;
  float pMag = fourVector_mcp.P();

  displacement.SetX(decayLength * fourVector_mcp.X() / pMag);
  displacement.SetY(decayLength * fourVector_mcp.Y() / pMag);
  displacement.SetZ(decayLength * fourVector_mcp.Z() / pMag);
  displacement.SetT(lifetime_mcp);
}


void GeneratedVertexDisplacerModule::terminate()
{
  B2DEBUG(0, "Terminate GeneratedVertexDisplacerModule");
}


