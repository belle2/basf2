/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <TOPBackSplashTimingModule.h>

/* --------------- WARNING ---------------------------------------------- *
If you have more complex parameter types in your class then simple int,
double or std::vector of those you might need to uncomment the following
include directive to avoid an undefined reference on compilation.
* ---------------------------------------------------------------------- */
// #include <framework/core/ModuleParam.templateDetails.h>



using namespace Belle2;

REG_MODULE(TOPBackSplashTiming);

TOPBackSplashTimingModule::TOPBackSplashTimingModule() : Module(), m_topClusterTime("TOPClusterTime")
{
  // Set module properties
  setDescription(R"DOC(Extract timing of an ECL cluster from TOP signal)DOC");

  // Parameter definitions
  addParam("minClusterE", m_minClusterE, "Minimum energy of the cluster to be used", 0.1);
  addParam("getPDFfromDB", m_getPDFfromDB, "Get the PDF parameters from DB, otherwise floating", True);

}

TOPBackSplashTimingModule::~TOPBackSplashTimingModule()
{
}

void TOPBackSplashTimingModule::initialize()
{
  m_eclCluster.isRequired();
  m_topClusterTime.registerInDataStore();
}

void TOPBackSplashTimingModule::beginRun()
{
}

void TOPBackSplashTimingModule::event()
{
}

void TOPBackSplashTimingModule::endRun()
{
}

void TOPBackSplashTimingModule::terminate()
{
}


