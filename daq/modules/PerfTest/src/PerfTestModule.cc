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

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PerfTest)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PerfTestModule::PerfTestModule() : Module()
{
  setDescription("PerfTest module");
  addParam("overallOutputFileName", m_overallOutputFileName, "Overall performance output", string("performance.overall.txt"));
  addParam("eventsOutputFileName", m_eventsOutputFileName, "Events performance output", string("performance.events.txt"));
  //setPropertyFlags(c_Input);
}

PerfTestModule::~PerfTestModule()
{
}

void PerfTestModule::initialize()
{
  m_start = clock();
  m_nEvents = 0;
}

void PerfTestModule::beginRun()
{
}

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
}

void PerfTestModule::endRun()
{
  B2INFO("PerfTest module: endRun () ends");
}

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

double PerfTestModule::timeDifference(clock_t point, clock_t start)
{
  double ticks = point - start;
  double times = (ticks * 1000) / CLOCKS_PER_SEC;
  return times;
}
