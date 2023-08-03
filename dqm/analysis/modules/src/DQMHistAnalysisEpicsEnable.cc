/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <dqm/analysis/modules/DQMHistAnalysisEpicsEnable.h>

#include <TH1F.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(DQMHistAnalysisEpicsEnable);

DQMHistAnalysisEpicsEnableModule::DQMHistAnalysisEpicsEnableModule()
  : DQMHistAnalysisModule()
{
  // set module description (e.g. insert text)
  setDescription("EPICS output enabler module");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("useEpicsReadOnly", m_useEpicsRO, "use Epics Read Only", false);
  addParam("PVPrefix", m_locPVPrefix, "Set EPICS PV prefix", std::string("TEST:"));

#ifdef _BELLE2_EPICS
  if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
#endif
}

DQMHistAnalysisEpicsEnableModule::~DQMHistAnalysisEpicsEnableModule()
{
#ifdef _BELLE2_EPICS
  if (ca_current_context()) ca_context_destroy();
#endif
}

void DQMHistAnalysisEpicsEnableModule::initialize()
{
#ifdef _BELLE2_EPICS
  setUseEpics(true); // set always true
  setUseEpicsReadOnly(m_useEpicsRO);
  setPVprefix(m_locPVPrefix);
#endif
}

void DQMHistAnalysisEpicsEnableModule::event()
{
  // -> now trigger flush to network
  // it wont harm to do this more often than needed.
  updateEpicsPVs(5.0); // 5 seconds timeout
}
