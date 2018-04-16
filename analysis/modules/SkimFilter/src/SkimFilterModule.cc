/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Phillip Urquijo                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/SkimFilter/SkimFilterModule.h>
#include <analysis/dataobjects/ParticleList.h>
#include <framework/core/Environment.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <cmath>
#include <algorithm>
#include <TParameter.h>
using namespace std;
using namespace Belle2;
using namespace boost::algorithm;

// Register module in the framework
REG_MODULE(SkimFilter)

SkimFilterModule::SkimFilterModule() : Module()
{
  //Set module properties
  setDescription("Filter based on ParticleLists, by setting return value to true if at least one of the given lists is not empty.");
  setPropertyFlags(c_ParallelProcessingCertified);
  //Parameter definition
  addParam("particleLists", m_strParticleLists, "List of ParticleLists", vector<string>());

  // initializing the rest of private memebers
  m_nEvents = 0;
  m_nPass   = 0;
}

void SkimFilterModule::initialize()
{

  int nParticleLists = m_strParticleLists.size();
  B2INFO("Number of ParticleLists studied " << nParticleLists << " ");

  m_nEvents = 0;
  m_nPass = 0;
}

void SkimFilterModule::event()
{

  setReturnValue(0);

  // number of ParticleLists
  int nParticleLists = m_strParticleLists.size();
  bool pass = false;
  for (int iList = 0; iList < nParticleLists; ++iList) {

    StoreObjPtr<ParticleList> particlelist(m_strParticleLists[iList]);
    if (!particlelist) {
      B2INFO("ParticleList " << m_strParticleLists[iList] << " not found");
      continue;
    } else {
      if (particlelist->getListSize() == 0)continue;
      pass = true;
    }

  }

  m_nEvents++;
  if (pass) m_nPass++;
  //module condition
  setReturnValue(pass);

}

void SkimFilterModule::terminate()
{
  B2INFO("SkimFilter Summary: \n");
  std::ostringstream stream;
  stream <<  "\n=======================================================\n";
  stream <<  "Total Retention: " << Form("%6.4f\n", (float)m_nPass / (float)Environment::Instance().getNumberOfEvents());
  stream <<  "\n=======================================================\n";
  B2INFO(stream.str());
}

