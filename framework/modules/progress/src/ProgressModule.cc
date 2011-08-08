/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz                                          *
 *                                                                        *
 **************************************************************************/

#include <framework/modules/progress/ProgressModule.h>
#include <framework/logging/Logger.h>
#include <cmath>

using namespace std;
using namespace Belle2;

REG_MODULE(Progress)

ProgressModule::ProgressModule() : Module(), m_maxOrder(3), m_evtNr(0), m_runNr(0),
    m_output("Processed: %3d runs, %6d events")
{
  setDescription("Output number of processed events. "
                 "Output cannot be suppressed using set_log_level. "
                 "If you don't want messages, you don't want this module");
  addParam("maxN", m_maxOrder,
           "maximum 10^N events between output", m_maxOrder);
}

void ProgressModule::initialize()
{
  //Force module logging level to be info
  setLogLevel(LogConfig::c_Info);
}

void ProgressModule::beginRun()
{
  ++m_runNr;
  B2INFO("Begin of new run")
}

void ProgressModule::event()
{
  ++m_evtNr;
  //Calculate the order of magnitude
  int order = (m_evtNr == 0) ? 1 : static_cast<int>(min(log10(m_evtNr), (double)m_maxOrder));
  int interval = static_cast<int>(pow(10., order));
  if (m_evtNr % interval == 0) B2INFO(m_output % m_runNr % m_evtNr);
}
