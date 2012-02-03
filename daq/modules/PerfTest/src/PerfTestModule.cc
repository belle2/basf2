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
  addParam("outputFileName", m_outputFileName, "output", string("testResult"));
  //setPropertyFlags(c_Input);
}

PerfTestModule::~PerfTestModule()
{
}

void PerfTestModule::initialize()
{
  m_start = clock();
}

void PerfTestModule::beginRun()
{
}

void PerfTestModule::event()
{
  FILE* fp = fopen(m_outputFileName.c_str(), "a+");
  clock_t point = clock();
  double dt = timeDifference(point, m_start);

  fprintf(fp, "%f\n", dt);
  B2INFO("======= Elapsed time: " << dt << " ms");

  fclose(fp);
}

void PerfTestModule::endRun()
{
  B2INFO("PerfTest module: endRun () ends");
}

void PerfTestModule::terminate()
{
  B2INFO("PerfTest module: terminate () called");
}

double PerfTestModule::timeDifference(clock_t point, clock_t start)
{
  double ticks = point - start;
  double times = (ticks * 1000) / CLOCKS_PER_SEC;
  return times;
}
