/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <boost/graph/adjacency_list.hpp>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMFPGAFit.h>
#include <eklm/modules/EKLMDigitizer/EKLMDigitizerModule.h>
#include <eklm/simulation/FiberAndElectronics.h>

using namespace Belle2;

REG_MODULE(EKLMDigitizer)

EKLMDigitizerModule::EKLMDigitizerModule() : Module()
{
  setDescription("EKLM digitization module");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("DiscriminatorThreshold", m_DiscriminatorThreshold,
           "ADC amplitude threshold in units of the maximal amplitude of "
           "one photoelectron signal.", double(3.));
  addParam("DigitizationInitialTime", m_DigitizationInitialTime,
           "Initial digitization time (ns).", double(-40.));
  addParam("CreateSim2Hits", m_CreateSim2Hits,
           "Create merged EKLMSim2Hits", false);
  addParam("SaveFPGAFit", m_SaveFPGAFit, "Save FPGA fit data", false);
  addParam("Debug", m_Debug,
           "Debug mode (generates additional output files with histograms).",
           false);
  m_GeoDat = NULL;
  m_Fitter = NULL;
}

EKLMDigitizerModule::~EKLMDigitizerModule()
{
}

void EKLMDigitizerModule::initialize()
{
  m_Digits.registerInDataStore();
  m_Digits.registerRelationTo(m_SimHits);
  if (m_CreateSim2Hits)
    m_Sim2Hits.registerInDataStore();
  if (m_SaveFPGAFit) {
    m_FPGAFits.registerInDataStore();
    m_Digits.registerRelationTo(m_FPGAFits);
  }
  m_GeoDat = &(EKLM::GeometryData::Instance());
  m_Fitter = new EKLM::FPGAFitter(m_DigPar->getNDigitizations());
}

void EKLMDigitizerModule::beginRun()
{
  if (!m_DigPar.isValid())
    B2FATAL("EKLM digitization parameters are not available.");
}

void EKLMDigitizerModule::readAndSortSimHits()
{
  EKLMSimHit* hit;
  int i, strip, maxStrip;
  maxStrip = m_GeoDat->getMaximalStripGlobalNumber();
  m_SimHitVolumeMap.clear();
  for (i = 0; i < m_SimHits.getEntries(); i++) {
    hit = m_SimHits[i];
    strip = hit->getVolumeID();
    if (strip <= 0)
      B2FATAL("Incorrect (non-positive) strip number in EKLM digitizer.");
    /* Background study mode: ignore hits from SiPMs and boards. */
    if (strip > maxStrip)
      continue;
    m_SimHitVolumeMap.insert(std::pair<int, EKLMSimHit*>(strip, hit));
  }
}

void EKLMDigitizerModule::makeSim2Hits()
{
  EKLMSimHit* hit;
  std::multimap<int, EKLMSimHit*>::iterator it, ub, it2;
  std::map<int, EKLMSimHit*>::iterator mapIterator, hitIterator, parentIterator;
  for (it = m_SimHitVolumeMap.begin(); it != m_SimHitVolumeMap.end();
       it = m_SimHitVolumeMap.upper_bound(it->first)) {
    ub = m_SimHitVolumeMap.upper_bound(it->first);
    // we have only tree graphs here, so edge is completely defined by its
    // track ID
    // map to store (edge  <--> hit) == (vertex <--> hit) correspondence
    std::map<int, EKLMSimHit*> hitMap;
    for (it2 = it; it2 != ub; it2++) {
      hit = it2->second;
      // ID key
      int key = hit->getTrackID();
      // here we merge all SimHits produced by the same track.
      //The leading (with smallest time) hit survies.
      //The others are deleted.
      //Integrated energy deposition is prescribed to the leading hit
      //this procedure removes multiple maps entries  with identical keys
      // search if entry already exist
      mapIterator = hitMap.find(key);
      if (mapIterator == hitMap.end()) { // nothing found
        // add an entry
        hitMap.insert(std::pair<int, EKLMSimHit*>(key, hit));
      } else { // entry already exists
        // get  time of the entries
        double oldTime = mapIterator->second->getTime();
        double newTime = hit->getTime();
        if (newTime > oldTime) { // new hit is newer
          //add edep of the new hit to the old one
          mapIterator->second->increaseEDep(hit->getEDep());
        } else { // new hit is older
          // add edep of the old hit to the new one
          hit->increaseEDep(mapIterator->second->getEDep());
          //change second element of the pair to point to the new hit
          mapIterator->second = hit;
        }
      }
    }
    // Now we have map with key of the track ID
    // we are ready to create the graph
    // since vertex numbering always starts from 0,
    // we will use NUMBER OF TRACK IN THE MAP (started from 0)
    // istead of Track ID itself (which could be quite large).
    // this procedure drastically decreases number of graph vertices
    boost::adjacency_list <> G;
    // we here restore the tree of hits using TrackID and ParentTrackID
    // information
    // There is a possibility for two hit trees wich are connected by
    // parentTrackID outside the volume
    // use special ParentTrackID for the tracks coming from abroad remains
    // these trees splittable
    int non_exsisting_rtracks_counter = hitMap.size();
    for (hitIterator = hitMap.begin(); hitIterator != hitMap.end();
         hitIterator++) {
      //search for parent entry in the map
      parentIterator = hitMap.find(hitIterator->second->getParentTrackID());
      if (parentIterator != hitMap.end()) { // nothing found
        // see comments above on the vertex numbering
        add_edge(distance(hitMap.begin(), hitIterator),
                 distance(hitMap.begin(), parentIterator)  , G);
      } else {
        add_edge(distance(hitMap.begin(), hitIterator),
                 non_exsisting_rtracks_counter++  , G);
      }
    }
    // search for connected subgraphs
    std::vector<int> component(num_vertices(G));
    // map for component <-> simhit correspondence
    std::map <int, EKLMSim2Hit*> graphComponentToSimHit;
    // loop over the vertices
    for (hitIterator = hitMap.begin(); hitIterator != hitMap.end();
         hitIterator++) {
      // get EKLMSimHit corresponding to the current vertex
      EKLMSimHit* simHit = hitIterator->second;
      // search for the current component in the map
      std::map <int, EKLMSim2Hit*>::iterator current =
        graphComponentToSimHit.find(component[distance(hitMap.begin(),
                                                       hitIterator)]);
      if (current == graphComponentToSimHit.end()) {
        // no  entry for this component
        // create new EKLMSim2Hit and store all information into it
        EKLMSim2Hit* sim2Hit = m_Sim2Hits.appendNew(simHit);
        // insert hit to the map
        graphComponentToSimHit.insert(
          std::pair<int, EKLMSim2Hit*>(
            component[distance(hitMap.begin(), hitIterator)], sim2Hit));
      } else { // entry already exist
        // compare hittime. The leading one has smallest time
        if (current->second->getTime() < simHit->getTime()) {
          // new hit is successor, add edep of the successor to the ancestor
          current->second->setEDep(current->second->getEDep() +
                                   simHit->getEDep());
        } else {
          // new hit is ancestor,  modify everything
          current->second->setEDep(current->second->getEDep() +
                                   simHit->getEDep());
          current->second->setPosition(simHit->getPositionX(),
                                       simHit->getPositionY(),
                                       simHit->getPositionZ());
          current->second->setLocalPosition(simHit->getLocalPositionX(),
                                            simHit->getLocalPositionY(),
                                            simHit->getLocalPositionZ());
          current->second->setTime(simHit->getTime());
          current->second->setPDG(simHit->getPDG());
          current->second->setMomentum(simHit->getMomentum());
        }
      }
    }
  }
}

/*
 * Light propagation into the fiber, SiPM and electronics effects
 * are simulated in EKLM::FiberAndElectronics class.
 */
void EKLMDigitizerModule::mergeSimHitsToStripHits()
{
  EKLM::FiberAndElectronics fes(&(*m_DigPar), m_Fitter,
                                m_DigitizationInitialTime, m_Debug);
  std::multimap<int, EKLMSimHit*>::iterator it, ub;
  for (it = m_SimHitVolumeMap.begin(); it != m_SimHitVolumeMap.end();
       it = m_SimHitVolumeMap.upper_bound(it->first)) {
    ub = m_SimHitVolumeMap.upper_bound(it->first);
    /* Set hits. */
    fes.setHitRange(it, ub);
    /*
     * Set threshold. Currently, the threshold is 3 maximal amplitudes
     * of 1 photoelectron signal (about 7 photoelectrons of the true signal).
     * TODO: a strip-specific threshold from the database is necessary.
     */
    fes.setThreshold(m_DiscriminatorThreshold);
    /* Simulation for a strip. */
    fes.processEntry();
    if (fes.getGeneratedNPE() == 0)
      continue;
    EKLMSimHit* simHit = it->second;
    EKLMDigit* eklmDigit = m_Digits.appendNew(simHit);
    eklmDigit->setMCTime(simHit->getTime());
    eklmDigit->setSiPMMCTime(fes.getMCTime());
    eklmDigit->setPosition(simHit->getPosition());
    eklmDigit->setGeneratedNPE(fes.getGeneratedNPE());
    eklmDigit->addRelationTo(simHit);
    if (fes.getFitStatus() == EKLM::c_FPGASuccessfulFit) {
      eklmDigit->setTime(fes.getFPGAFit()->getStartTime());
      eklmDigit->setNPE(fes.getNPE());
    } else {
      eklmDigit->setTime(0.);
      eklmDigit->setNPE(0);
    }
    eklmDigit->setFitStatus(fes.getFitStatus());
    if (fes.getFitStatus() == EKLM::c_FPGASuccessfulFit && m_SaveFPGAFit) {
      EKLMFPGAFit* fit = m_FPGAFits.appendNew(*fes.getFPGAFit());
      eklmDigit->addRelationTo(fit);
    }
  }
}

void EKLMDigitizerModule::event()
{
  readAndSortSimHits();
  if (m_CreateSim2Hits)
    makeSim2Hits();
  mergeSimHitsToStripHits();
}

void EKLMDigitizerModule::endRun()
{
}

void EKLMDigitizerModule::terminate()
{
  delete m_Fitter;
}
