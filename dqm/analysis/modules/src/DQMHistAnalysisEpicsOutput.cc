/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <dqm/analysis/modules/DQMHistAnalysisEpicsOutput.h>

#include <TH1F.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(DQMHistAnalysisEpicsOutput);

DQMHistAnalysisEpicsOutputModule::DQMHistAnalysisEpicsOutputModule()
  : DQMHistAnalysisModule()
{
  // set module description (e.g. insert text)
  setDescription("EPICS output enabler/flusher module");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void DQMHistAnalysisEpicsOutputModule::initialize()
{
  // Crosscheck
  if (!getUseEpics()) {
    B2ERROR("EPICS is not enabled, most likely DQMHistAnalysisEpicsEnableModule is missing in the beginning of the process chain.");
  }
}

void DQMHistAnalysisEpicsOutputModule::event()
{
  // -> now trigger flush to network
  // this is the reason we want to have this modul after all other Analysis modules
  updateEpicsPVs(5.0); // 5 seconds
  // be aware that any "error" arising from a PV in an analysis module, may only show up here (timeout etc)
}
