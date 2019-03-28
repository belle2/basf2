//THIS MODULE
#include <analysis/modules/ChargedParticleIdentificator/ChargedPidMVAModule.h>

//C++
#include <algorithm>

using namespace Belle2;

REG_MODULE(ChargedPidMVA)

ChargedPidMVAModule::ChargedPidMVAModule() : Module()
{

  setDescription("This module evaluates the response of an MVA trained for charged particle identification between two hypotheses, S and B. It takes a set of Particle objects in a standard charged particle's ParticleList, calculates the MVA score using the appropriate xml weight file for a given input set of (S,B) mass hypotheses, and adds it as ExtraInfo to the Particle objects.");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("sigPdgId",
           m_sig_pdg,
           "The pdgId of the signal mass hypothesis.");
  addParam("bkgPdgId",
           m_bkg_pdg,
           "The pdgId of the background mass hypothesis.");

}


ChargedPidMVAModule::~ChargedPidMVAModule() {}


void ChargedPidMVAModule::initialize()
{
  m_event_metadata.isRequired();

  m_score_varname = "chargedPidBDTScore_" + std::to_string(m_sig_pdg) + "_VS_" + std::to_string(m_bkg_pdg);
}


void ChargedPidMVAModule::beginRun()
{
  m_weightfiles_representation.addCallback([this]() { initializeMVA(); });
  initializeMVA();
}


void ChargedPidMVAModule::event()
{

  auto sigPart = Const::ChargedStable(m_sig_pdg);
  auto bkgPart = Const::ChargedStable(m_bkg_pdg);

  for (const auto& [pdg, name] : m_charged_particle_lists) {

    StoreObjPtr<ParticleList> pList(name);

    // Skip if this particle list does not match any of the input (S, B) hypotheses.
    if (pdg != sigPart.getPDGCode() && pdg != bkgPart.getPDGCode()) {
      continue;
    }

    for (unsigned int ipart(0); ipart < pList->getListSize(); ++ipart) {

      Particle* particle = pList->getParticle(ipart);

      // Retrieve the index for the correct MVA expert and dataset, given (signal hypo, clusterTheta, p)
      auto theta   = particle->getECLCluster()->getTheta();
      auto p       = particle->getP();
      auto index   = m_weightfiles_representation->getMVAWeightIdx(sigPart, theta, p);

      auto nvars = m_variables.at(pdg).at(index).size();
      for (unsigned int ivar(0); ivar < nvars; ++ivar) {
        m_datasets.at(pdg).at(index)->m_input[ivar] = m_variables.at(pdg).at(index).at(ivar)->function(particle);
      }
      // NB: the MVA::Expert used to calculate the score must be the one trained to discriminate THIS signal hypothesis!
      float score = m_experts.at(sigPart.getPDGCode()).at(index)->apply(*m_datasets.at(pdg).at(index))[0];

      // Store the MVA score as a new particle object property.
      particle->writeExtraInfo(m_score_varname, score);

    }

  }
}


void ChargedPidMVAModule::endRun()
{
}


void ChargedPidMVAModule::terminate()
{
}


void ChargedPidMVAModule::initializeMVA()
{

  if (!m_weightfiles_representation) { B2FATAL("No MVA weightfiles representation payload found in database! Abort..."); }

  B2INFO("Load supported MVA interfaces...");

  // The supported methods have to be initialized once (calling it more than once is safe).
  MVA::AbstractInterface::initSupportedInterfaces();
  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();

  // Iterate over standard charged particles.
  for (const auto& [pdg, name] : m_charged_particle_lists) {

    B2INFO("\tLoading weightfiles from the payload class for particle hypothesis: (" << pdg << "," << name << ")");

    auto serialized_weightfiles = m_weightfiles_representation->getMVAWeights(pdg);

    B2INFO("\tConstruct the MVA experts and datasets from the weightfiles...");

    // Initialise list of experts/ds for this pdgId.
    ExpertsList experts(serialized_weightfiles->size());
    DatasetsList datasets(serialized_weightfiles->size());

    // Initialise list of lists of variables/spectators for this pdgId.
    VariablesList variables, spectators;

    for (unsigned int idx(0); idx < serialized_weightfiles->size(); idx++) {

      // De-serialize the string into an MVA::Weightfile object.
      std::stringstream ss(serialized_weightfiles->at(idx));
      auto weightfile = MVA::Weightfile::loadFromStream(ss);

      MVA::GeneralOptions general_options;
      weightfile.getOptions(general_options);

      // Store the list of pointers to the relevant variables for this xml file.
      Variable::Manager& manager = Variable::Manager::Instance();
      variables.push_back(manager.getVariables(general_options.m_variables));
      spectators.push_back(manager.getVariables(general_options.m_spectators));

      // Store an MVA::Expert object.
      experts[idx] = supported_interfaces[general_options.m_method]->getExpert();
      experts.at(idx)->load(weightfile);

      // Store an MVA::SingleDataset object, in which we will save our features later...
      std::vector<float> v(general_options.m_variables.size(), 0.0);
      std::vector<float> s(general_options.m_spectators.size(), 0.0);
      datasets[idx] = std::make_unique<MVA::SingleDataset>(general_options, v, 1.0);

    }

    // Update maps w/ values for this pdgId.
    m_experts.emplace(pdg, std::move(experts)); // Move semantics needed for unique_ptr:
    // https://stackoverflow.com/questions/28921250/c-nested-map-with-unique-ptr/32749132
    // https://stackoverflow.com/questions/6876751/differences-between-unique-ptr-and-shared-ptr
    m_datasets.emplace(pdg, std::move(datasets));
    m_variables.emplace(pdg, variables);
    m_spectators.emplace(pdg, spectators);

  }

}


