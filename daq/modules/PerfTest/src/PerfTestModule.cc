/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <sys/wait.h>
#include <daq/modules/PerfTest/PerfTestModule.h>

#include <framework/core/ModuleManager.h>

using namespace std;
using namespace Belle2;

REG_MODULE(PerfTest)

/* @brief PerfTestModule constructor
 * This initializes member variables from given parameters
*/
PerfTestModule::PerfTestModule() : Module()
{
  setDescription("PerfTest module");
  addParam("overallOutputFileName", m_overallOutputFileName, "Overall performance output", string("performance.overall.txt"));
  addParam("eventsOutputFileName", m_eventsOutputFileName, "Events performance output", string("performance.events.txt"));
}

/// @brief PerfTestModule destructor
PerfTestModule::~PerfTestModule()
{
}

/* @brief Initialize the module
 * This records the start time
*/
void PerfTestModule::initialize()
{
  m_start = clock();
  m_nEvents = 0;
}

/// @brief Begin a run
void PerfTestModule::beginRun()
{
}

/* @brief Process an event
 * If it is the first event, the processing time per event is calculated from the
 * difference between the start time and the current time. Otherwise, it is calculated
 * from an arbitrary time in m_temp which is recorded at the end of an event processing.
*/
void PerfTestModule::event()
{
  clock_t reference;
  clock_t now = clock();

  if (m_nEvents == 0)
    reference = m_start;
  else
    reference = m_temp;

  double dt = timeDifference(now, reference);

  FILE* fp = fopen(m_eventsOutputFileName.c_str(), "a+");
  fprintf(fp, "%d\t%f\n", m_nEvents, dt);
  fclose(fp);

  m_nEvents++;
  m_temp = clock();
}

/// @brief End a run
void PerfTestModule::endRun()
{
  B2INFO("PerfTest module: endRun () ends");
}

/* @brief Terminate the module
 * This calculates the total elapsed time and records it.
*/
void PerfTestModule::terminate()
{
  clock_t point = clock();
  double dt = timeDifference(point, m_start);

  B2INFO("====== Total elapsed time: " << dt << " ms");
  FILE* fp = fopen(m_overallOutputFileName.c_str(), "a+");
  fprintf(fp, "%d\t%f\n", m_nEvents, dt);
  fclose(fp);
  B2INFO("PerfTest module: terminate () called");
}

/* @brief Calculate time different between two points, point - start.
 * The unit of time is milliseconds (ms)
*/
/// @param point Current time
/// @param start Reference time
/// @return Time difference between two points in unit of ms
double PerfTestModule::timeDifference(clock_t point, clock_t start)
{
  double ticks = point - start;
  double times = (ticks * 1000) / CLOCKS_PER_SEC;
  return times;
}
