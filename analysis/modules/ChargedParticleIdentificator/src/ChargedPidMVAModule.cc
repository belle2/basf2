//THIS MODULE
#include <analysis/modules/ChargedParticleIdentificator/ChargedPidMVAModule.h>

//ANALYSIS
#include <mva/interface/Interface.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

//MDST
#include <mdst/dataobjects/ECLCluster.h>

using namespace Belle2;

REG_MODULE(ChargedPidMVA)

ChargedPidMVAModule::ChargedPidMVAModule() : Module()
{
  setDescription("This module evaluates the response of an MVA trained for binary charged particle identification between two hypotheses, S and B. For a given input set of (S,B) mass hypotheses, it takes the Particle objects in the appropriate charged stable particle's ParticleLists, calculates the MVA score using the appropriate xml weight file, and adds it as ExtraInfo to the Particle objects.");

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
  addParam("payloadName",
           m_payload_name,
           "The name of the database payload object with the MVA weights.",
           std::string("ChargedPidMVAWeights"));
  addParam("chargeIndependent",
           m_charge_independent,
           "Specify whether to use a charge-independent training of the MVA.",
           bool(false));
  addParam("useECLOnlyTraining",
           m_ecl_only,
           "Specify whether to use an ECL-only training of the MVA.",
           bool(false));
}


ChargedPidMVAModule::~ChargedPidMVAModule() = default;


void ChargedPidMVAModule::initialize()
{

  m_event_metadata.isRequired();

  m_weightfiles_representation = std::make_unique<DBObjPtr<ChargedPidMVAWeights>>(m_payload_name);

}


void ChargedPidMVAModule::beginRun()
{

  // Retrieve the payload from the DB.
  (*m_weightfiles_representation.get()).addCallback([this]() { initializeMVA(); });
  initializeMVA();

  if (!(*m_weightfiles_representation.get())->isValidPdg(m_sig_pdg)) {
    B2FATAL("PDG: " << m_sig_pdg <<
            " of the signal mass hypothesis is not that of a valid particle in Const::chargedStableSet! Aborting...");
  }
  if (!(*m_weightfiles_representation.get())->isValidPdg(m_bkg_pdg)) {
    B2FATAL("PDG: " << m_bkg_pdg <<
            " of the background mass hypothesis is not that of a valid particle in Const::chargedStableSet! Aborting...");
  }

  m_score_varname = "pidPairChargedBDTScore_" + std::to_string(m_sig_pdg) + "_VS_" + std::to_string(m_bkg_pdg);

  if (m_ecl_only) {
    m_score_varname += "_" + std::to_string(Const::ECL);
  } else {
    for (size_t iDet(0); iDet < Const::PIDDetectors::set().size(); ++iDet) {
      m_score_varname += "_" + std::to_string(Const::PIDDetectors::set()[iDet]);
    }
  }
}


void ChargedPidMVAModule::event()
{

  B2DEBUG(11, "EVENT: " << m_event_metadata->getEvent());

  for (const auto& name : m_particle_lists) {

    StoreObjPtr<ParticleList> pList(name);
    if (!pList) { B2FATAL("ParticleList: " << name << " could not be found. Aborting..."); }

    // Need to get an absolute value in order to check if in Const::ChargedStable.
    int pdg = abs(pList->getPDGCode());

    // Check if this ParticleList is made up of legit Const::ChargedStable particles.
    if (!(*m_weightfiles_representation.get())->isValidPdg(pdg)) {
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

      // Check that the particle has a valid relation set between track and ECL cluster.
      // Otherwise, skip to next.
      const ECLCluster* eclCluster = particle->getECLCluster();
      if (!eclCluster) {
        B2WARNING("\tParticle has invalid Track-ECLCluster relation, skip MVA application...");
        continue;
      }

      // Retrieve the index for the correct MVA expert and dataset,
      // given reconstructed (clusterTheta, p, charge)
      auto clusterTheta = eclCluster->getTheta();
      auto p = particle->getP();
      auto charge = (m_charge_independent) ? particle->getCharge() :
                    0.0; // Set a dummy charge of zero to pick charge-independent payloads, if requested.
      int jth, ip, kch;
      auto index = (*m_weightfiles_representation.get())->getMVAWeightIdx(clusterTheta, p, charge, jth, ip, kch);

      // Get the cut defining the MVA category under exam (this reflects the one used in the training).
      const auto cuts   = (*m_weightfiles_representation.get())->getCuts(m_sig_pdg);
      const auto cutstr = (!cuts->empty()) ? cuts->at(index) : "";

      B2DEBUG(11, "\t\tclusterTheta    = " << clusterTheta << " [rad]");
      B2DEBUG(11, "\t\tp               = " << p << " [GeV/c]");
      if (!m_charge_independent) {
        B2DEBUG(11, "\t\tcharge          = " << charge);
      }
      B2DEBUG(11, "\t\tBrems corrected = " << particle->hasExtraInfo("bremsCorrectedPhotonEnergy"));
      B2DEBUG(11, "\t\tWeightfile idx  = " << index << " - (clusterTheta, p, charge) = (" << jth << ", " << ip <<  ", " << kch << ")");
      if (!cutstr.empty()) {
        B2DEBUG(11, "\tCategory cut: " << cutstr);
      }

      // Fill the MVA::SingleDataset w/ variables and spectators.

      B2DEBUG(11, "\tMVA variables:");

      auto nvars = m_variables.at(index).size();
      for (unsigned int ivar(0); ivar < nvars; ++ivar) {

        auto varobj = m_variables.at(index).at(ivar);

        auto var = varobj->function(particle);

        // Manual imputation value of -999 for NaN (undefined) variables.
        var = (std::isnan(var)) ? -999.0 : var;

        B2DEBUG(11, "\t\tvar[" << ivar << "] : " << varobj->name << " = " << var);

        m_datasets.at(index)->m_input[ivar] = var;

      }

      B2DEBUG(12, "\tMVA spectators:");

      auto nspecs = m_spectators.at(index).size();
      for (unsigned int ispec(0); ispec < nspecs; ++ispec) {

        auto specobj = m_spectators.at(index).at(ispec);

        auto spec = specobj->function(particle);

        B2DEBUG(12, "\t\tspec[" << ispec << "] : " << specobj->name << " = " << spec);

        m_datasets.at(index)->m_spectators[ispec] = spec;

      }

      // Compute MVA score only if particle fulfils category selection.
      if (!cutstr.empty()) {

        std::unique_ptr<Variable::Cut> cut = Variable::Cut::compile(cutstr);

        if (!cut->check(particle)) {
          B2WARNING("\tParticle didn't pass MVA category cut, skip MVA application...");
          continue;
        }

      }

      float score = m_experts.at(index)->apply(*m_datasets.at(index))[0];

      B2DEBUG(11, "\tMVA score = " << score);
      B2DEBUG(12, "\tExtraInfo: " << m_score_varname);

      // Store the MVA score as a new particle object property.
      particle->writeExtraInfo(m_score_varname, score);

    }

  }
}


void ChargedPidMVAModule::initializeMVA()
{

  B2INFO("Run: " << m_event_metadata->getRun() << ". Load supported MVA interfaces for binary charged particle identification...");

  // The supported methods have to be initialized once (calling it more than once is safe).
  MVA::AbstractInterface::initSupportedInterfaces();
  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();

  B2INFO("\tLoading weightfiles from the payload class for SIGNAL particle hypothesis: " << m_sig_pdg);

  auto serialized_weightfiles = (*m_weightfiles_representation.get())->getMVAWeights(m_sig_pdg);
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
