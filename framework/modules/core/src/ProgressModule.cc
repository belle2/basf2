/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz                                          *
 *                                                                        *
 **************************************************************************/

#include <framework/modules/core/ProgressModule.h>
#include <framework/logging/Logger.h>
#include <cmath>

using namespace std;
using namespace Belle2;

REG_MODULE(Progress)

ProgressModule::ProgressModule() : Module(), m_maxOrder(3), m_evtNr(0), m_runNr(0),
  m_output("Processed: %3d runs, %6d events")
{
  setDescription("Periodically writes the number of processed events/runs to the"
                 " logging system to give a progress indication.\n"
                 "The output is logarithmic, meaning it will output the first 10 events, "
                 "then every tenth event up to 100, then every hundreth event up to 1000, etc. "
                 "Output cannot be suppressed using set_log_level. "
                 "If you don't want messages, you don't want this module");
  addParam("maxN", m_maxOrder,
           "At most, 10^N events will lie between outputs", m_maxOrder);
}

void ProgressModule::initialize()
{
  //Force module logging level to be info
  setLogLevel(LogConfig::c_Info);
  m_runNr = m_evtNr = 0;
}

void ProgressModule::beginRun()
{
  ++m_runNr;
  B2INFO("Begin of new run");
}

void ProgressModule::event()
{
  ++m_evtNr;
  //Calculate the order of magnitude
  int order = (m_evtNr == 0) ? 1 : (int)(min(log10(m_evtNr), (double)m_maxOrder));
  int interval = (int)pow(10., order);
  if (m_evtNr % interval == 0) B2INFO(m_output % m_runNr % m_evtNr);
}
