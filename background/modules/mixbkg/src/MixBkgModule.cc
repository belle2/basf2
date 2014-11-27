/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <background/modules/mixbkg/MixBkgModule.h>

#include <boost/format.hpp>

using namespace std;
using namespace Belle2;
using namespace background;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MixBkg)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MixBkgModule::MixBkgModule() : Module()
{
  //Set module properties
  setDescription("Mixes background into the existing SimHits collections.");

  //Parameters
  addParam("BackgroundFiles", m_fileNames, "The ROF Root files containing the background SimHits. Wildcards can be used !");
  addParam("AnalysisMode", m_analysisMode, "Activates the analysis mode which stores all background MCParticles and background information into separate collections.", false);
}


void MixBkgModule::initialize()
{
  B2INFO("============================================================")
  B2INFO("                         MixBkg                             ")
  B2INFO("------------------------------------------------------------")

  // Create an empty MCParticles collection if there is none.
  StoreArray<MCParticle> storeMCParticles("MCParticles");
  storeMCParticles.registerInDataStore();
  m_mixBackground.setAnalysisMode(m_analysisMode);
  if (m_analysisMode) {
    B2INFO(">> Analysis mode: ON");
    // Register analysis-mode specific background collections
    StoreArray<BackgroundInfo> storeBackgroundInfos("BackgroundInfos");
    storeBackgroundInfos.registerInDataStore();
    StoreArray<MCParticle> storeBackgroundParticles("BackgroundMCParticles");
    storeBackgroundParticles.registerInDataStore();

  } else {
    B2INFO(">> Analysis mode: OFF")
  }

  //Add the background ROF Root files to the background mixing library
  for (vector<string>::iterator listIter = m_fileNames.begin(); listIter != m_fileNames.end(); ++listIter)
    m_mixBackground.addFile(*listIter);

  B2INFO("============================================================")
  B2INFO("       SimHit Class       |    #ROF max    |    #ROF min    ")
  B2INFO("------------------------------------------------------------")
  DetectorBackgroundBase* currDet = m_mixBackground.getFirstSubdetector();
  while (currDet != NULL) {
    B2INFO(boost::format("%1% %|26t||") % currDet->getSimHitClassname() << boost::format(" %1% %|16t|| %2%") % currDet->getMaxNumberReadoutFrames() % currDet->getMinNumberReadoutFrames());
    currDet = m_mixBackground.getNextSubdetector();
  }
  B2INFO("============================================================")
}


void MixBkgModule::event()
{
  m_mixBackground.fillDataStore();
}


void MixBkgModule::terminate()
{

}
