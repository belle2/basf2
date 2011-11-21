
#include <framework/core/ModuleManager.h>
#include <eklm/modules/eklmBackgroundAnalyzer/EKLMBackgroundAnalyzer.h>
#include <eklm/simeklm/EKLMDigitizer.h>
#include <framework/dataobjects/EventMetaData.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/range/irange.hpp>
#include <eklm/dataobjects/EKLMSimHit.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the
//-----------------------------------------------------------------
REG_MODULE(EKLMBackgroundAnalyzer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EKLMBackgroundAnalyzerModule::EKLMBackgroundAnalyzerModule() : Module()
{
  setDescription("EKLM background analyzer module");

}

EKLMBackgroundAnalyzerModule::~EKLMBackgroundAnalyzerModule()
{
}

void EKLMBackgroundAnalyzerModule::initialize()
{
  B2INFO("EKLMBackgroundAnalyzerModule initialized");
}

void EKLMBackgroundAnalyzerModule::beginRun()
{
}

void EKLMBackgroundAnalyzerModule::event()
{
  B2INFO("EKLMBackgroundAnalyzerModule:: event()");

  m_HitVolumeMap.clear();
  readAndSortBkgHits();
  makeGraphs();

}


void EKLMBackgroundAnalyzerModule::readAndSortBkgHits()
{
  B2INFO("EKLMBackgroundAnalyzerModule::readAndSortBkgHits()");

  StoreArray<EKLMBackHit> bkgHitsArray;
  for (int i = 0; i < bkgHitsArray.getEntries(); i++) {

    // search for entries of the same strip
    map<string, vector<EKLMBackHit*> >::iterator
    it = m_HitVolumeMap.find((bkgHitsArray[i])->getName());

    if (it == m_HitVolumeMap.end()) { //  new entry
      vector<EKLMBackHit*> *vectorHits =
        new vector<EKLMBackHit*> (1, (bkgHitsArray[i]));
      m_HitVolumeMap.insert(pair<string, vector<EKLMBackHit*> >
                            ((bkgHitsArray[i])->getName(), *vectorHits));
    } else {
      it->second.push_back(bkgHitsArray[i]);
    }
  }

  B2INFO("EKLMBackgroundAnalyzerModule::readAndSortBkgHits()  completed");
}

void EKLMBackgroundAnalyzerModule::makeGraphs()
{
  StoreArray<EKLMSimHit> simHitsArray;

  using namespace boost;
  B2INFO("EKLMBackgroundAnalyzerModule::makeGraphs()");

  //loop over volumes
  for (map<string, vector<EKLMBackHit*> >::iterator volumeIterator = m_HitVolumeMap.begin();
       volumeIterator != m_HitVolumeMap.end(); volumeIterator++) {



    int volType = 0;
    if (volumeIterator->first.find("SiPM") != string::npos)
      volType = 1;
    if (volumeIterator->first.find("Board") != string::npos)
      volType = 2;





    // we have only tree graphs here, so edge is completely defined by it's track ID
    // map to store (edge  <--> hit) == (vertex <--> hit) correspondence
    map < int , EKLMBackHit*> hitMap;

    for (vector<EKLMBackHit*>::iterator i = volumeIterator->second.begin(); i != volumeIterator->second.end(); i++) {
      // ID key
      int key = (*i)->getTrackID();

      // search if entry already exist
      map < int , EKLMBackHit*>::iterator mapIterator = hitMap.find(key);

      if (mapIterator == hitMap.end()) { // nothing found
        // add an entry
        hitMap.insert(pair< int , EKLMBackHit*>(key, *i));
      } else { // entry already exists
        // get  time of the entries
        double oldTime = mapIterator->second->getTime();
        double newTime = (*i)->getTime();
        if (newTime > oldTime) { // new hit is newer
          //add edep of the new hit to the old one
          mapIterator->second->increaseEDep((*i)->getEDep());
        } else { // new hit is older
          // add edep of the old hit to the new one
          (*i)->increaseEDep(mapIterator->second->getEDep());
          //change second element of the pair to point to the new hit
          mapIterator->second = *i;
        }
      }
    }

    //  Now we have map with key of the track ID
    // we are ready to create the graph

    // since vertex numbering always starts from 0, we will use NUMBER OF TRACK IN THE MAP (started from 0) istead of Track ID itself (which could be quite large).
    // this procedure drastically decreases number of graph vertices

    adjacency_list <> G;

    // used for parent IDs of track coming from abroad to remain graph splittable
    int non_exsisting_rtracks_counter = hitMap.size();


    for (map < int , EKLMBackHit*>::iterator hitIterator = hitMap.begin(); hitIterator != hitMap.end(); hitIterator++) {
      //search for parent entry in the map

      map < int , EKLMBackHit*>::iterator parentIterator =
        hitMap.find(hitIterator->second->getParentTrackID());

      if (parentIterator != hitMap.end()) {
        // see comments ahead on the vertex numbering
        add_edge(distance(hitMap.begin(), hitIterator), distance(hitMap.begin(), parentIterator)  , G);
      } else {
        add_edge(distance(hitMap.begin(), hitIterator), non_exsisting_rtracks_counter++  , G);
      }
    }



    graph_traits < adjacency_list <> >::vertex_iterator it, end;
    property_map < adjacency_list <>, vertex_index_t >::type  index_map = get(vertex_index, G);

    // search for connected subgraphs
    vector<int> component(num_vertices(G));


    // map for component <-> simhit correspondence
    map <int, EKLMSimHit*> graphComponentToSimHit;



    // loop over the vertices
    for (map < int , EKLMBackHit*>::iterator hitIterator = hitMap.begin(); hitIterator != hitMap.end(); hitIterator++) {

      // get EKLMBackHit corresponding to the current vertex
      EKLMBackHit * backhit = hitIterator->second;

      // search for the current component in the map
      map <int, EKLMSimHit*>::iterator current = graphComponentToSimHit.find(component[distance(hitMap.begin(), hitIterator)]);
      if (current == graphComponentToSimHit.end()) {
        // no  entry fot this component
        // create new EKLMSimHit and store all information into it
        EKLMSimHit *simHit = new(simHitsArray->AddrAt(simHitsArray.getEntries()))  EKLMSimHit();

        // insert to the map
        graphComponentToSimHit.insert(pair<int, EKLMSimHit*>(component[distance(hitMap.begin(), hitIterator)], simHit));
        simHit->setGlobalPos(backhit->getPosition());
        simHit->setTime(backhit->getTime());
        simHit->setEDep(backhit->getEDep());
        simHit->setPDGCode(backhit->getPDG());
        simHit->setVolType(volType);
        simHit->setMomentum(backhit->getMomentum());
        simHit->setEnergy(backhit->getEnergy());


        if (volType == 0) { // strip
          simHit->setEndcap(backhit->getEndcap());
          simHit->setSector(backhit->getSector());
          simHit->setLayer(backhit->getLayer());
          simHit->setPlane(backhit->getPlane());
          simHit->setStrip(backhit->getStrip());
        }
      } else { // entry already exist
        // compare hittime. The leading one has smallest time
        if (current->second->getTime() < backhit->getTime()) { // new hit is successor, add edep of the successsor to the ancestor
          current->second->setEDep(current->second->getEDep() + backhit->getEDep());
        } else {
          // new hit is ancestor,  modify everything
          current->second->setEDep(current->second->getEDep() + backhit->getEDep());
          current->second->setGlobalPos(backhit->getPosition());
          current->second->setTime(backhit->getTime());
          current->second->setPDGCode(backhit->getPDG());
          current->second->setMomentum(backhit->getMomentum());
          current->second->setEnergy(backhit->getEnergy());
        }
      }
    }

  }

  // searching for the nearest hit
  //temporal

  // nearest hit
  int min_layer = 100;
  int min_i = -1;

  for (int i = 0; i < simHitsArray.getEntries(); i++) {
    if ((simHitsArray[i])->getEDep() > 10) {
      (simHitsArray[i])->setVolType(100);


      if ((simHitsArray[i])->getLayer() < min_layer) {
        min_layer = (simHitsArray[i])->getLayer();
        min_i = i;
        std::cout << "aa " << min_layer << " " << min_i << std::endl;
      }

    }
    if ((simHitsArray[i])->getEDep() > 0.7)
      for (int j = 0 ;  j < simHitsArray.getEntries(); j++) {
        if (
          (simHitsArray[i])->getLayer() == (simHitsArray[j])->getLayer()
          &&
          (simHitsArray[i])->getSector() == (simHitsArray[j])->getSector()
          &&
          (simHitsArray[i])->getPlane() != (simHitsArray[j])->getPlane()
          &&
          (simHitsArray[j])->getEDep() > 0.7
        ) {
          (simHitsArray[i])->setVolType(101);
          //    std::cout<<(simHitsArray[i])->getEDep()<<" "<<(simHitsArray[j])->getEDep()<<" "<<simHitsArray[i]->getName()<<" "<<simHitsArray[j]->getName()<<std::endl;

          if ((simHitsArray[i])->getLayer() < min_layer) {
            min_layer = (simHitsArray[i])->getLayer();
            min_i = i;
            std::cout << "aa " << min_layer << " " << min_i << std::endl;
          }


        }
      }
  }

  if (min_i != -1)
    (simHitsArray[min_i])->setVolType(200);





  B2INFO("EKLMBackgroundAnalyzerModule::makeGraphs() completed");
}





void EKLMBackgroundAnalyzerModule::endRun()
{
  B2INFO("EKLMBackgroundAnalyzerModule:: endRun()");
}

void EKLMBackgroundAnalyzerModule::terminate()
{
}




