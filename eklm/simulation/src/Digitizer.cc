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
#include <G4Box.hh>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

/* Belle2 headers. */
#include <eklm/simulation/Digitizer.h>
#include <eklm/simulation/FiberAndElectronics.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

static const char MemErr[] = "Memory allocation error.";

void EKLM::setDefDigitizationParams(struct DigitizationParams* digPar)
{
  GearDir dig("/Detector/DetectorComponent[@name=\"EKLM\"]/"
              "Content/DigitizationParams");
  digPar->ADCRange = dig.getDouble("ADCRange");
  digPar->ADCSamplingTime = dig.getDouble("ADCSamplingTime");
  digPar->nDigitizations = dig.getDouble("nDigitizations");
  digPar->ADCPedestal = dig.getDouble("ADCPedestal");
  digPar->ADCPEAmplitude = dig.getDouble("ADCPEAmplitude");
  digPar->ADCSaturation = dig.getDouble("ADCSaturation");
  digPar->nPEperMeV = dig.getDouble("nPEperMeV");
  digPar->minCosTheta = cos(dig.getDouble("MaxTotalIRAngle") / 180.0 * M_PI);
  digPar->mirrorReflectiveIndex = dig.getDouble("MirrorReflectiveIndex");
  digPar->scintillatorDeExcitationTime = dig.getDouble("ScintDeExTime");
  digPar->fiberDeExcitationTime = dig.getDouble("FiberDeExTime");
  digPar->fiberLightSpeed = dig.getDouble("FiberLightSpeed");
  digPar->attenuationLength = dig.getDouble("AttenuationLength");
  digPar->PEAttenuationFreq = dig.getDouble("PEAttenuationFreq");
  digPar->meanSiPMNoise = dig.getDouble("MeanSiPMNoise");
  digPar->enableConstBkg = dig.getDouble("EnableConstBkg") > 0;
  digPar->timeResolution = dig.getDouble("TimeResolution");
}

EKLM::Digitizer::Digitizer(struct EKLM::DigitizationParams* digPar) :
  m_fitter(digPar->nDigitizations)
{
  m_GeoDat = &(EKLM::GeometryData::Instance());
  m_digPar = digPar;
}

EKLM::Digitizer::~Digitizer()
{
}

void EKLM::Digitizer::readAndSortSimHits()
{
  EKLMSimHit* hit;
  StoreArray<EKLMSimHit> simHitsArray;
  int i, strip, maxStrip;
  maxStrip = m_GeoDat->getMaximalStripNumber();
  for (i = 0; i < simHitsArray.getEntries(); i++) {
    hit = simHitsArray[i];
    strip = hit->getVolumeID();
    if (strip <= 0)
      B2FATAL("Incorrect (non-positive) strip number in EKLM digitizer.");
    /* Background study mode: ignore hits from SiPMs and boards. */
    if (strip > maxStrip)
      continue;
    m_simHitVolumeMap.insert(std::pair<int, EKLMSimHit*>(strip, hit));
  }
}


void EKLM::Digitizer::makeSim2Hits()
{
  EKLMSimHit* hit;
  std::multimap<int, EKLMSimHit*>::iterator it, ub, it2;
  std::map<int, EKLMSimHit*>::iterator mapIterator, hitIterator, parentIterator;
  for (it = m_simHitVolumeMap.begin(); it != m_simHitVolumeMap.end();
       it = m_simHitVolumeMap.upper_bound(it->first)) {
    ub = m_simHitVolumeMap.upper_bound(it->first);
    // we have only tree graphs here, so edge is completely defined by it's track ID
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

    //  Now we have map with key of the track ID
    // we are ready to create the graph

    // since vertex numbering always starts from 0,
    // we will use NUMBER OF TRACK IN THE MAP (started from 0)
    // istead of Track ID itself (which could be quite large).
    // this procedure drastically decreases number of graph vertices

    boost::adjacency_list <> G;

    // we here restore the tree of hits using TrackID and ParentTrackID information
    // There is a possibility for two hit trees wich are connected by parentTrackID outside the volume
    // use special ParentTrackID for the tracks coming from abroad  remains these trees splittable
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
        graphComponentToSimHit.find(component[distance(hitMap.begin(), hitIterator)]);
      if (current == graphComponentToSimHit.end()) {    // no  entry for this component

        // create new EKLMSim2Hit and store all information into it
        EKLMSim2Hit* sim2Hit =
          m_simHitsArray.appendNew(simHit);
        // insert hit to the map
        graphComponentToSimHit.insert(std::pair<int, EKLMSim2Hit*>(component[distance(hitMap.begin(), hitIterator)], sim2Hit));

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
void EKLM::Digitizer::mergeSimHitsToStripHits(double threshold)
{
  EKLM::FiberAndElectronics fes(m_digPar, &m_fitter);
  std::multimap<int, EKLMSimHit*>::iterator it, ub;
  for (it = m_simHitVolumeMap.begin(); it != m_simHitVolumeMap.end();
       it = m_simHitVolumeMap.upper_bound(it->first)) {
    ub = m_simHitVolumeMap.upper_bound(it->first);
    fes.setHitRange(it, ub);
    fes.processEntry();
    EKLMSimHit* simHit = it->second;
    EKLMDigit* stripHit = m_stripHitsArray.appendNew(simHit);
    stripHit->setMCTime(simHit->getTime());
    stripHit->setSiPMMCTime(fes.getMCTime());
    stripHit->setPosition(simHit->getPosition());
    stripHit->setGeneratedNPE(fes.getGeneratedNPE());
    stripHit->addRelationTo(simHit);
    if (!fes.getFitStatus()) {
      stripHit->setTime(fes.getFitResults()->startTime);
      stripHit->setNPE(fes.getNPE());
    } else {
      stripHit->setTime(0.);
      stripHit->setNPE(0);
    }
    stripHit->setFitStatus(fes.getFitStatus());
    if (stripHit->getNPE() > threshold)
      stripHit->isGood(true);
    else
      stripHit->isGood(false);
    /* cppcheck-suppress memleak */
  }
}

