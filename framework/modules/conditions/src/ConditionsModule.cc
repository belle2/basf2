#include <framework/modules/conditions/ConditionsModule.h>
#include <framework/conditions/ConditionsService.h>

#include <iostream>
#include <string>

using namespace Belle2;

REG_MODULE(Conditions)

ConditionsModule::ConditionsModule(): Module()
{
  setDescription("This module is used test conditions API");
  addParam("GlobalTag", m_global_tag, "Global Database Conditions Tag", std::string("leps2013_InitialTest_GlobalTag"));
  addParam("ExperimentName", m_experiment_name, "Experiment name", std::string("topcrt-e000002"));
  addParam("RunName", m_run_name, "Run name", std::string("r000545"));
  addParam("RESTBaseName", m_rest_basename, "Base name for REST services", std::string("http://belle2db.hep.pnnl.gov/b2s/rest/v1/"));
  addParam("FileBaseName", m_file_basename, "Base name for conditions files", std::string("http://belle2db.hep.pnnl.gov/"));

}

void ConditionsModule::initialize()
{
  //Build actual dbserver tools
  B2INFO("Getting conditions service ... global tag: " << m_global_tag);

  ConditionsService::GetInstance()->SetRESTbasename(m_rest_basename);
  ConditionsService::GetInstance()->SetFILEbasename(m_file_basename);

  B2INFO("Initialized: ");
}

void ConditionsModule::beginRun()
{
  ConditionsService::GetInstance()->GetPayloads(m_global_tag, m_experiment_name, m_run_name);
}

void ConditionsModule::terminate()
{

}
