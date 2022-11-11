/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//THIS MODULE
#include <analysis/modules/ChargedParticleIdentificator/ChargedPidMVAModule.h>

//ANALYSIS
#include <mva/interface/Interface.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

// FRAMEWORK
#include <framework/logging/LogConfig.h>
#include <framework/logging/LogSystem.h>


using namespace Belle2;

REG_MODULE(ChargedPidMVA);

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
           m_decayStrings,
           "The input list of DecayStrings, where each selected (^) daughter should correspond to a standard charged ParticleList, e.g. ['Lambda0:sig -> ^p+ ^pi-', 'J/psi:sig -> ^mu+ ^mu-']. One can also directly pass a list of standard charged ParticleLists, e.g. ['e+:my_electrons', 'pi+:my_pions']. Note that charge-conjugated ParticleLists will automatically be included.",
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

  if (!(*m_weightfiles_representation.get())->isValidPdg(m_sig_pdg)) {
    B2FATAL("PDG: " << m_sig_pdg <<
            " of the signal mass hypothesis is not that of a valid particle in Const::chargedStableSet! Aborting...");
  }
  if (!(*m_weightfiles_representation.get())->isValidPdg(m_bkg_pdg)) {
    B2FATAL("PDG: " << m_bkg_pdg <<
            " of the background mass hypothesis is not that of a valid particle in Const::chargedStableSet! Aborting...");
  }

  /* Initialize MVA if the payload has changed and now. */
  (*m_weightfiles_representation.get()).addCallback([this]() { initializeMVA(); });
  initializeMVA();

  m_score_varname = "pidPairChargedBDTScore_" + std::to_string(m_sig_pdg) + "_VS_" + std::to_string(m_bkg_pdg);

  if (m_ecl_only) {
    m_score_varname += "_" + std::to_string(Const::ECL);
  } else {
    for (const Const::EDetector& det : Const::PIDDetectorSet::set()) {
      m_score_varname += "_" + std::to_string(det);
    }
  }
}


void ChargedPidMVAModule::beginRun()
{
}


void ChargedPidMVAModule::event()
{

  // Debug strings per log level.
  std::map<int, std::string> debugStr = {
    {11, ""},
    {12, ""}
  };

  B2DEBUG(11, "EVENT: " << m_event_metadata->getEvent());

  for (auto decayString : m_decayStrings) {

    DecayDescriptor decayDescriptor;
    decayDescriptor.init(decayString);
    auto pListName = decayDescriptor.getMother()->getFullName();

    unsigned short m_nSelectedDaughters = decayDescriptor.getSelectionNames().size();
    StoreObjPtr<ParticleList> pList(pListName);

    if (!pList) {
      B2FATAL("ParticleList: " << pListName << " could not be found. Aborting...");
    }

    auto pListSize = pList->getListSize();

    B2DEBUG(11, "ParticleList: " << pList->getParticleListName() << " - N = " << pListSize << " particles.");

    const auto nTargetParticles = (m_nSelectedDaughters == 0) ? pListSize : pListSize * m_nSelectedDaughters;

    // Need to get an absolute value in order to check if in Const::ChargedStable.
    std::vector<int> pdgs;
    if (m_nSelectedDaughters == 0) {
      pdgs.push_back(pList->getPDGCode());
    } else {
      pdgs = decayDescriptor.getSelectionPDGCodes();
    }
    for (auto pdg : pdgs) {
      // Check if this ParticleList is made up of legit Const::ChargedStable particles.
      if (!(*m_weightfiles_representation.get())->isValidPdg(abs(pdg))) {
        B2FATAL("PDG: " << pdg << " of ParticleList: " << pListName <<
                " is not that of a valid particle in Const::chargedStableSet! Aborting...");
      }
    }
    std::vector<const Particle*> targetParticles;
    if (m_nSelectedDaughters > 0) {
      for (unsigned int iPart(0); iPart < pListSize; ++iPart) {
        auto* iParticle = pList->getParticle(iPart);
        auto daughters = decayDescriptor.getSelectionParticles(iParticle);
        for (auto* iDaughter : daughters) {
          targetParticles.push_back(iDaughter);
        }
      }
    }

    for (unsigned int ipart(0); ipart < nTargetParticles; ++ipart) {

      const Particle* particle = (m_nSelectedDaughters == 0) ? pList->getParticle(ipart) : targetParticles[ipart];

      if (!(*m_weightfiles_representation.get())->hasImplicitNaNmasking()) {
        // LEGACY TRAININGS: always require a track-cluster match.
        const ECLCluster* eclCluster = particle->getECLCluster();
        if (!eclCluster) {
          if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 11)) {
            B2WARNING("\nParticle [" << ipart << "] has invalid Track-ECLCluster relation, skip MVA application...");
          }
          continue;
        }
      }

      // Retrieve the index for the correct MVA expert and dataset,
      // given the reconstructed (polar angle, p, charge)
      auto thVarName = (*m_weightfiles_representation.get())->getThetaVarName();
      auto theta = std::get<double>(Variable::Manager::Instance().getVariable(thVarName)->function(particle));
      auto p = particle->getP();
      // Set a dummy charge of zero to pick charge-independent payloads, if requested.
      auto charge = (!m_charge_independent) ? particle->getCharge() : 0.0;
      int idx_theta, idx_p, idx_charge;
      auto index = (*m_weightfiles_representation.get())->getMVAWeightIdx(theta, p, charge, idx_theta, idx_p, idx_charge);

      auto* matchVar = Variable::Manager::Instance().getVariable("clusterTrackMatch");
      auto hasMatch = std::isnormal(std::get<double>(matchVar->function(particle)));

      debugStr[11] += "\n";
      debugStr[11] += ("Particle [" + std::to_string(ipart) + "]\n");
      debugStr[11] += ("Has ECL cluster match? " + std::to_string(hasMatch) + "\n");
      debugStr[11] += ("polar angle: " + thVarName + " = " + std::to_string(theta) + " [rad]\n");
      debugStr[11] += ("p = " + std::to_string(p) + " [GeV/c]\n");
      if (!m_charge_independent) {
        debugStr[11] += ("charge = " + std::to_string(charge) + "\n");
      }
      debugStr[11] += ("Is brems corrected ? " + std::to_string(particle->hasExtraInfo("bremsCorrected")) + "\n");
      debugStr[11] += ("Weightfile idx = " + std::to_string(index) + " - (polar angle, p, charge) = (" + std::to_string(
                         idx_theta) + ", " + std::to_string(idx_p) + ", " +
                       std::to_string(idx_charge) + ")\n");
      if (m_cuts.at(index)) {
        debugStr[11] += ("Category cut: " + m_cuts.at(index)->decompile() + "\n");
      }

      B2DEBUG(11, debugStr[11]);
      debugStr[11].clear();

      // Don't even bother if particle does not fulfil the category selection.
      if (m_cuts.at(index)) {

        if (!m_cuts.at(index)->check(particle)) {
          if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 11)) {
            B2WARNING("\nParticle [" << ipart << "] didn't pass MVA category cut, skip MVA application...");
          }
          continue;
        }

      }

      // Fill the MVA::SingleDataset w/ variables and spectators.

      debugStr[11] += "\n";
      debugStr[11] += "MVA variables:\n";

      auto nvars = m_variables.at(index).size();
      for (unsigned int ivar(0); ivar < nvars; ++ivar) {

        auto varobj = m_variables.at(index).at(ivar);

        double var = std::numeric_limits<double>::quiet_NaN();
        auto var_result = varobj->function(particle);
        if (std::holds_alternative<double>(var_result)) {
          var = std::get<double>(var_result);
        } else if (std::holds_alternative<int>(var_result)) {
          var = std::get<int>(var_result);
        } else if (std::holds_alternative<bool>(var_result)) {
          var = std::get<bool>(var_result);
        } else {
          B2ERROR("Variable '" << varobj->name << "' has wrong data type! It must be one of double, integer, or bool.");
        }

        if (!(*m_weightfiles_representation.get())->hasImplicitNaNmasking()) {
          // LEGACY TRAININGS: manual imputation value of -999 for NaN (undefined) variables. Needed by TMVA.
          var = (std::isnan(var)) ? -999.0 : var;
        }

        debugStr[11] += ("\tvar[" + std::to_string(ivar) + "] : " + varobj->name + " = " + std::to_string(var) + "\n");

        m_datasets.at(index)->m_input[ivar] = var;

      }

      B2DEBUG(11, debugStr[11]);
      debugStr[11].clear();

      // Check spectators only when in debug mode.
      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 12)) {

        debugStr[12] += "\n";
        debugStr[12] += "MVA spectators:\n";

        auto nspecs = m_spectators.at(index).size();
        for (unsigned int ispec(0); ispec < nspecs; ++ispec) {

          auto specobj = m_spectators.at(index).at(ispec);

          double spec = std::numeric_limits<double>::quiet_NaN();
          auto spec_result = specobj->function(particle);
          if (std::holds_alternative<double>(spec_result)) {
            spec = std::get<double>(spec_result);
          } else if (std::holds_alternative<int>(spec_result)) {
            spec = std::get<int>(spec_result);
          } else if (std::holds_alternative<bool>(spec_result)) {
            spec = std::get<bool>(spec_result);
          } else {
            B2ERROR("Variable '" << specobj->name << "' has wrong data type! It must be one of double, integer, or bool.");
          }

          debugStr[12] += ("\tspec[" + std::to_string(ispec) + "] : " + specobj->name + " = " + std::to_string(spec) + "\n");

          m_datasets.at(index)->m_spectators[ispec] = spec;

        }

        B2DEBUG(12, debugStr[12]);
        debugStr[12].clear();

      }

      // Compute MVA score.

      debugStr[11] += "\n";
      debugStr[12] += "\n";
      debugStr[11] += "MVA response:\n";

      float score = m_experts.at(index)->apply(*m_datasets.at(index))[0];

      debugStr[11] += ("\tscore = " + std::to_string(score));
      debugStr[12] += ("\textraInfo: " + m_score_varname + "\n");

      // Store the MVA score as a new particle object property.
      m_particles[particle->getArrayIndex()]->writeExtraInfo(m_score_varname, score);

      B2DEBUG(11, debugStr[11]);
      B2DEBUG(12, debugStr[12]);
      debugStr[11].clear();
      debugStr[12].clear();

    }

  }

  // Clear the debug string map before next event.
  debugStr.clear();

}


void ChargedPidMVAModule::registerAliasesLegacy()
{

  std::map<std::string, std::string> aliasesLegacy;

  aliasesLegacy.insert(std::make_pair("__event__", "evtNum"));

  for (Const::DetectorSet::Iterator it = Const::PIDDetectorSet::set().begin();
       it != Const::PIDDetectorSet::set().end(); ++it) {

    auto detName = Const::parseDetectors(*it);

    aliasesLegacy.insert(std::make_pair("missingLogL_" + detName, "pidMissingProbabilityExpert(" + detName + ")"));

    for (auto& [pdgId, info] : m_stdChargedInfo) {

      std::string alias = "deltaLogL_" + std::get<0>(info) + "_" + std::get<1>(info) + "_" + detName;
      std::string var = "pidDeltaLogLikelihoodValueExpert(" + std::to_string(pdgId) + ", " + std::to_string(std::get<2>
                        (info)) + "," + detName + ")";

      aliasesLegacy.insert(std::make_pair(alias, var));

      if (it.getIndex() == 0) {
        alias = "deltaLogL_" + std::get<0>(info) + "_" + std::get<1>(info) + "_ALL";
        var = "pidDeltaLogLikelihoodValueExpert(" + std::to_string(pdgId) + ", " + std::to_string(std::get<2>(info)) + ", ALL)";
        aliasesLegacy.insert(std::make_pair(alias, var));
      }

    }

  }

  B2INFO("Setting hard-coded aliases for the ChargedPidMVA algorithm.");

  std::string debugStr("\n");
  for (const auto& [alias, variable] : aliasesLegacy) {
    debugStr += (alias + " --> " + variable + "\n");
    if (!Variable::Manager::Instance().addAlias(alias, variable)) {
      B2ERROR("Something went wrong with setting alias: " << alias << " for variable: " << variable);
    }
  }
  B2DEBUG(10, debugStr);

}


void ChargedPidMVAModule::registerAliases()
{

  auto aliases = (*m_weightfiles_representation.get())->getAliases();

  if (!aliases->empty()) {

    B2INFO("Setting aliases for the ChargedPidMVA algorithm read from the payload.");

    std::string debugStr("\n");
    for (const auto& [alias, variable] : *aliases) {
      if (alias != variable) {
        debugStr += (alias + " --> " + variable + "\n");
        if (!Variable::Manager::Instance().addAlias(alias, variable)) {
          B2ERROR("Something went wrong with setting alias: " << alias << " for variable: " << variable);
        }
      }
    }
    B2DEBUG(10, debugStr);

    return;

  }

  // Manually set aliases - for bw compatibility
  this->registerAliasesLegacy();

}


void ChargedPidMVAModule::initializeMVA()
{

  B2INFO("Run: " << m_event_metadata->getRun() << ". Load supported MVA interfaces for binary charged particle identification...");

  // Set the necessary variable aliases from the payload.
  this->registerAliases();

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
  m_cuts.resize(nfiles);
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

    // Compile cut for this category.
    const auto cuts = (*m_weightfiles_representation.get())->getCuts(m_sig_pdg);
    const auto cutstr = (!cuts->empty()) ? cuts->at(idx) : "";
    m_cuts[idx] = (!cutstr.empty()) ? Variable::Cut::compile(cutstr) : nullptr;

    B2DEBUG(12, "\t\tcut[" << idx << "] created successfully!");

  }

}
