/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <framework/modules/core/ProgressModule.h>

/* Framework headers. */
#include <framework/core/Environment.h>
#include <framework/logging/Logger.h>

/* C++ headers. */
#include <cmath>

using namespace Belle2;

REG_MODULE(Progress)

ProgressModule::ProgressModule() : Module()
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
  // Force module logging level to be c_Info
  setLogLevel(LogConfig::c_Info);
  m_totalEvtNr = Environment::Instance().getNumberOfEvents();
}

void ProgressModule::beginRun()
{
  ++m_runNr;
  B2INFO("Begin of new run.");
}

void ProgressModule::event()
{
  ++m_evtNr;
  // Calculate the order of magnitude
  uint32_t order = (m_evtNr == 0) ? 1 : (uint32_t)(std::min(std::log10(m_evtNr), (double)m_maxOrder));
  uint32_t interval = (uint32_t)std::pow(10., order);
  if (m_evtNr % interval == 0)
    B2INFO(m_output % m_runNr % m_evtNr % m_totalEvtNr);
}
