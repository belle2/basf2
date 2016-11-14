#include <framework/modules/conditions/ConditionsModule.h>

#ifdef __INTEL_COMPILER
#pragma warning (disable:1478)
#else
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <framework/conditions/ConditionsService.h>

#include <iostream>
#include <string>

using namespace Belle2;

REG_MODULE(Conditions)

ConditionsModule::ConditionsModule(): Module()
{
  setDescription("This module is used test conditions API (experimental, should not be used)");
  addParam("globalTag", m_globalTag, "Global Database Conditions Tag", std::string("leps2013_InitialTest_GlobalTag"));
  addParam("experimentName", m_experimentName, "Experiment name", std::string("topcrt-e000002"));
  addParam("runName", m_runName, "Run name", std::string("r000545"));
  addParam("restBaseName", m_restBasename, "Base name for REST services", std::string("http://belle2db.hep.pnnl.gov/b2s/rest/v1/"));
  addParam("fileBaseName", m_fileBasename, "Base name for conditions files", std::string("http://belle2db.hep.pnnl.gov/"));
  addParam("fileBaseLocal", m_fileLocal, "Directory name for local conditions files copies", std::string("/tmp/"));

  /// Parameters for custom payload additions.  See framework/conditions/scripts/add_payload.py example.
  addParam("fileName", m_filename,
           "filename for manual payload addition.  This payload will be added to conditions on intialization.", std::string());
  addParam("package", m_package, "Package for manual payload addition.  This payload will be added to conditions on intialization.",
           std::string());
  addParam("module", m_module, "Module for manual payload addition.  This payload will be added to conditions on intialization.",
           std::string());
  addParam("comment", m_comment, "Comment for manual payload addition.  This payload will be added to conditions on intialization.",
           std::string());

}

void ConditionsModule::initialize()
{
  B2WARNING("The Conditions module is deprecated, please don't use it anymore and switch to DBObjPtr/DBArray");

  //Build actual dbserver tools
  B2INFO("Getting conditions service ... global tag: " << m_globalTag);

  ConditionsService::getInstance()->setRESTbasename(m_restBasename);
  ConditionsService::getInstance()->setFILEbasename(m_fileBasename);
  ConditionsService::getInstance()->setFILEbaselocal(m_fileLocal);

  if (m_filename.length() > 0) {
    B2INFO("custom payload add requested : " << m_filename);
    ConditionsService::getInstance()->writePayloadFile(m_filename, m_package, m_module);
  }

  B2INFO("Initialized: ");

}

void ConditionsModule::beginRun()
{
  ConditionsService::getInstance()->getPayloads(m_globalTag, m_experimentName, m_runName);
}

void ConditionsModule::terminate()
{

}

