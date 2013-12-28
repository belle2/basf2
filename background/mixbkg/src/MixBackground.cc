/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <background/mixbkg/MixBackground.h>
#include <framework/logging/Logger.h>

#include <framework/datastore/DataStore.h>
#include <mdst/dataobjects/MCParticle.h>

#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <top/dataobjects/TOPSimHit.h>
#include <arich/dataobjects/ARICHSimHit.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLHit.h>
#include <eklm/dataobjects/EKLMSimHit.h>
#include <bklm/dataobjects/BKLMSimHit.h>

#include <TFile.h>
#include <TChain.h>
#include <TChainElement.h>

using namespace std;
using namespace Belle2;
using namespace background;


MixBackground::~MixBackground()
{
  for (map<int, DetectorBackgroundBase*>::iterator mapIter = m_detectorBackgrounds.begin();
       mapIter != m_detectorBackgrounds.end(); ++mapIter) {
    delete mapIter->second;
  }
  m_detectorBackgrounds.clear();
}


void MixBackground::addFile(const std::string& filename)
{
  //Load the content information of the file(s) given by filename
  int subdetector = 0;
  int mcParticleWriteMode = 0;
  string* bkgComponent = new string("");
  string* bkgGenerator = new string("");
  string* bkgSimHitCollection = new string("");
  string* bkgSimHitRelation = new string("");
  TChain chain("ContentTree");
  chain.SetBranchAddress("Subdetector", &subdetector);
  chain.SetBranchAddress("Component", &bkgComponent);
  chain.SetBranchAddress("Generator", &bkgGenerator);
  chain.SetBranchAddress("SimHitCollection", &bkgSimHitCollection);
  chain.SetBranchAddress("SimHitRelation", &bkgSimHitRelation);
  chain.SetBranchAddress("MCParticleWriteMode", &mcParticleWriteMode);
  chain.Add(filename.c_str());

  //Loop over the files and add them into the correct section
  int iTree = -1;
  for (int iEntry = 0; iEntry < chain.GetEntries(); ++iEntry) {
    chain.GetEntry(iEntry);
    if (chain.GetTreeNumber() == iTree) continue;
    // New file, and this is its first entry
    iTree = chain.GetTreeNumber();
    string currentFileName(chain.GetCurrentFile()->GetName());

    //Check if all files have the same MCParticleWriteMode to ensure consistency.
    if (m_mcParticleWriteMode < 0) {
      m_mcParticleWriteMode = mcParticleWriteMode;
    } else {
      if (m_mcParticleWriteMode != mcParticleWriteMode) {
        B2FATAL("All background ROF files have to be produced with the same MCParticle write mode !")
      }
    }

    //Check if the subdetector background has already been created. If not create it.
    map<int, DetectorBackgroundBase*>::iterator mapIter = m_detectorBackgrounds.find(subdetector);
    if (mapIter == m_detectorBackgrounds.end()) {
      switch (subdetector) {
        case 1 : m_detectorBackgrounds.insert(make_pair(subdetector, new DetectorBackground<PXDSimHit>()));
          break;
        case 2 : m_detectorBackgrounds.insert(make_pair(subdetector, new DetectorBackground<SVDSimHit>()));
          break;
        case 3 : m_detectorBackgrounds.insert(make_pair(subdetector, new DetectorBackground<CDCSimHit>()));
          break;
        case 4 : m_detectorBackgrounds.insert(make_pair(subdetector, new DetectorBackground<TOPSimHit>()));
          break;
        case 5 : m_detectorBackgrounds.insert(make_pair(subdetector, new DetectorBackground<ARICHSimHit>()));
          break;
        case 6 : m_detectorBackgrounds.insert(make_pair(subdetector, new DetectorBackground<ECLHit>()));
          break;
        case 7 : m_detectorBackgrounds.insert(make_pair(subdetector, new DetectorBackground<EKLMSimHit>()));
          break;
        case 8 : m_detectorBackgrounds.insert(make_pair(subdetector, new DetectorBackground<BKLMSimHit>()));
          break;
        case 9 : m_detectorBackgrounds.insert(make_pair(subdetector, new DetectorBackground<ECLSimHit>()));
          break;
      }
      mapIter = m_detectorBackgrounds.find(subdetector);
      B2INFO(">> Created new subdetector background for '" << mapIter->second->getSimHitClassname() << "'.")
    }

    //Add the background ROF Root file to the subdetector
    mapIter->second->addFile(*bkgComponent, *bkgGenerator, currentFileName, *bkgSimHitCollection, *bkgSimHitRelation);

  }

  delete bkgComponent;
  delete bkgGenerator;
  delete bkgSimHitCollection;
  delete bkgSimHitRelation;
}


DetectorBackgroundBase* MixBackground::getFirstSubdetector()
{
  m_FirstNextIter = m_detectorBackgrounds.begin();
  if (m_FirstNextIter == m_detectorBackgrounds.end()) return NULL;
  return m_FirstNextIter->second;
}


DetectorBackgroundBase* MixBackground::getNextSubdetector()
{
  ++m_FirstNextIter;
  if (m_FirstNextIter == m_detectorBackgrounds.end()) return NULL;
  return m_FirstNextIter->second;
}


void MixBackground::fillDataStore()
{
  for (map<int, DetectorBackgroundBase*>::iterator mapIter = m_detectorBackgrounds.begin();
       mapIter != m_detectorBackgrounds.end(); ++mapIter) {
    mapIter->second->fillDataStore(m_analysisMode);
  }

  //Reset the indices for the standard MCParticle collection
  StoreArray<MCParticle> mcPartCollection;
  if (mcPartCollection.getEntries() > 0) dynamic_cast<MCParticle*>(mcPartCollection[0])->fixParticleList();
}
