//THIS MODULE
#include <analysis/modules/ChargedParticleIdentificator/ChargedPidMVAModule.h>

//MDST
#include <mdst/dataobjects/ECLCluster.h>

//C++
#include <algorithm>

using namespace Belle2;

REG_MODULE(ChargedPidMVA)

ChargedPidMVAModule::ChargedPidMVAModule() : Module()
{
  setDescription("This module evaluates the response of an MVA trained for charged particle identification between two hypotheses, S and B. Currently, it uses only ECL-based inputs. For a given input set of (S,B) mass hypotheses, it takes the Particle objects in the appropriate charged stable particle's ParticleLists, calculates the MVA score using the appropriate xml weight file, and adds it as ExtraInfo to the Particle objects.");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("sigHypoPDGCode",
           m_sig_pdg,
           "The input signal mass hypothesis' pdgId.",
           int(0));
  addParam("bkgHypoPDGCode",
           m_bkg_pdg,
           "The input background mass hypothesis' pdgId.",
           int(0));
  addParam("particleLists",
           m_particle_lists,
           "The input list of ParticleList names.",
           std::vector<std::string>());
}


ChargedPidMVAModule::~ChargedPidMVAModule() {}


void ChargedPidMVAModule::initialize()
{
  m_event_metadata.isRequired();
}


void ChargedPidMVAModule::beginRun()
{
  // Retrieve the payload from the DB.
  m_weightfiles_representation.addCallback([this]() { initializeMVA(); });
  initializeMVA();

  if (!m_weightfiles_representation->isValidPdg(m_sig_pdg)) {
    B2FATAL("PDG: " << m_sig_pdg <<
            " of the signal mass hypothesis is not that of a valid particle in Const::chargedStableSet! Aborting...");
  }
  if (!m_weightfiles_representation->isValidPdg(m_bkg_pdg)) {
    B2FATAL("PDG: " << m_bkg_pdg <<
            " of the background mass hypothesis is not that of a valid particle in Const::chargedStableSet! Aborting...");
  }

  m_score_varname = "pidPairChargedBDTScore_" + std::to_string(m_sig_pdg) + "_VS_" + std::to_string(m_bkg_pdg);

}


void ChargedPidMVAModule::event()
{

  auto sigPart = Const::ChargedStable(m_sig_pdg);

  B2DEBUG(11, "EVENT: " << m_event_metadata->getEvent());

  for (const auto& name : m_particle_lists) {

    StoreObjPtr<ParticleList> pList(name);
    if (!pList) { B2FATAL("ParticleList: " << name << " could not be found. Aborting..."); }

    // Need to get an absolute value in order to check if in Const::ChargedStable.
    int pdg = abs(pList->getPDGCode());

    // Check if this ParticleList is made up of legit Const::ChargedStable particles.
    if (!m_weightfiles_representation->isValidPdg(pdg)) {
      B2FATAL("PDG: " << pList->getPDGCode() << " of ParticleList: " << pList->getParticleListName() <<
              " is not that of a valid particle in Const::chargedStableSet! Aborting...");
    }

    // Skip if this ParticleList does not match any of the input (S, B) hypotheses.
    if (pdg != m_sig_pdg && pdg != m_bkg_pdg) {
      continue;
    }

    B2DEBUG(11, "ParticleList: " << pList->getParticleListName() << " - N = " << pList->getListSize() << " particles.");

    for (unsigned int ipart(0); ipart < pList->getListSize(); ++ipart) {

      Particle* particle = pList->getParticle(ipart);

      B2DEBUG(11, "\tParticle [" << ipart << "]");

      // If the particle list was created by the FSRCorrection module,
      // the info in the mdst objects associated to the particles cannot be retrieved directly from the particle.
      // Instead, one has to get the associated daughters, that hold relations with the mdst objects:
      // -) the 0-th daughter, i.e. the original particle before brem correction,
      // -) the 1-st daughter i.e. the brems photon particle (if found), needed to correct E/p.
      auto nDaughters = particle->getNDaughters();
      const Particle* daughterLep = (nDaughters) ? particle->getDaughter(0) : nullptr;
      const Particle* daughterPh  = (nDaughters > 0) ? particle->getDaughter(1) : nullptr;

      // Check that the particle (or the 'daughterLep') has a valid relation set between track and ECL cluster.
      // Otherwise, skip to next.
      const ECLCluster* eclCluster = (!nDaughters) ? particle->getECLCluster() : daughterLep->getECLCluster();
      if (!eclCluster) {
        B2DEBUG(11, "\t --> Invalid track-cluster relation, skip...");
        continue;
      }

      // Retrieve the index for the correct MVA expert and dataset, given (signal hypo, clusterTheta, p)
      auto theta   = eclCluster->getTheta();
      auto p       = particle->getP(); // This is the momentum from the 4-vec after any possible correction,
      // hence it must be the one of the actual particle.
      int jth, ip;
      auto index   = m_weightfiles_representation->getMVAWeightIdx(sigPart, theta, p, jth, ip);

      B2DEBUG(11, "\t\tclusterTheta = " << theta << " [rad]");
      B2DEBUG(11, "\t\tp = " << p << " [GeV/c]");
      B2DEBUG(11, "\t\tFSRCorrected? " << static_cast<bool>(nDaughters));
      B2DEBUG(11, "\t\tweightfile idx in payload = " << index << " - (clusterTheta, p) = (" << jth << ", " << ip << ")");

      // Fill the MVA::SingleDataset w/ variables and spectators.
      auto nvars  = m_variables.at(index).size();
      for (unsigned int ivar(0); ivar < nvars; ++ivar) {
        auto varobj =  m_variables.at(index).at(ivar);
        // Set the variables from the daughter particle, if found.
        // In that case, also ensure E/p is calculated *after* the 4-vec correction,
        // i.e. using the actual particle's momentum, and the cluster energy includes the brems photon contribution.
        auto var(-999.0);
        if (varobj->name == "clusterEoP") {
          if (nDaughters) {
            auto energyLep = eclCluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
            auto energyPh = (daughterPh) ? daughterPh->getECLCluster()->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons) : 0.0;
            var = (energyLep + energyPh) / p;
          } else {
            var = varobj->function(particle);
          }
        } else {
          var = varobj->function((!nDaughters) ? particle : daughterLep);
        }
        B2DEBUG(11, "\t\t\tvar[" << ivar << "] : " << varobj->name << " = " << var);
        m_datasets.at(index)->m_input[ivar] = var;
      }
      auto nspecs  = m_spectators.at(index).size();
      for (unsigned int ispec(0); ispec < nspecs; ++ispec) {
        auto specobj =  m_spectators.at(index).at(ispec);
        auto spec = specobj->function((!nDaughters) ? particle : daughterLep);
        B2DEBUG(11, "\t\t\tspec[" << ispec << "] : " << specobj->name << " = " << spec);
        m_datasets.at(index)->m_spectators[ispec] = spec;
      }

      float score = m_experts.at(index)->apply(*m_datasets.at(index))[0];

      B2DEBUG(11, "\t\tscore = " << score);

      // Store the MVA score as a new particle object property.
      particle->writeExtraInfo(m_score_varname, score);

    }

  }
}


void ChargedPidMVAModule::initializeMVA()
{

  B2INFO("Load supported MVA interfaces for charged particle identification...");

  // The supported methods have to be initialized once (calling it more than once is safe).
  MVA::AbstractInterface::initSupportedInterfaces();
  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();

  B2INFO("\tLoading weightfiles from the payload class for SIGNAL particle hypothesis: " << m_sig_pdg);

  auto serialized_weightfiles = m_weightfiles_representation->getMVAWeights(m_sig_pdg);
  auto nfiles = serialized_weightfiles->size();

  B2INFO("\tConstruct the MVA experts and datasets from N = " << nfiles << " weightfiles...");

  // The size of the vectors must correspond
  // to the number of available weightfiles for this pdgId.
  m_experts.resize(nfiles);
  m_datasets.resize(nfiles);
  m_variables.resize(nfiles);
  m_spectators.resize(nfiles);

  for (unsigned int idx(0); idx < nfiles; idx++) {

    B2DEBUG(12, "\t\tweightfile[" << idx << "]");

    // De-serialize the string into an MVA::Weightfile object.
    std::stringstream ss(serialized_weightfiles->at(idx));
    auto weightfile = MVA::Weightfile::loadFromStream(ss);

    MVA::GeneralOptions general_options;
    weightfile.getOptions(general_options);

    // Store the list of pointers to the relevant variables for this xml file.
    Variable::Manager& manager = Variable::Manager::Instance();
    m_variables[idx] = manager.getVariables(general_options.m_variables);
    m_spectators[idx] = manager.getVariables(general_options.m_spectators);

    B2DEBUG(12, "\t\tRetrieved N = " << general_options.m_variables.size()
            << " variables, N = " << general_options.m_spectators.size()
            << " spectators");

    // Store an MVA::Expert object.
    m_experts[idx] = supported_interfaces[general_options.m_method]->getExpert();
    m_experts.at(idx)->load(weightfile);

    B2DEBUG(12, "\t\tweightfile loaded successfully into expert[" << idx << "]!");

    // Store an MVA::SingleDataset object, in which we will save our features later...
    std::vector<float> v(general_options.m_variables.size(), 0.0);
    std::vector<float> s(general_options.m_spectators.size(), 0.0);
    m_datasets[idx] = std::make_unique<MVA::SingleDataset>(general_options, v, 1.0, s);

    B2DEBUG(12, "\t\tdataset[" << idx << "] created successfully!");

  }

}


