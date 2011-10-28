
#include <framework/core/ModuleManager.h>
#include <eklm/modules/eklmBackgroundAnalyzer/EKLMBackgroundAnalyzer.h>
#include <eklm/simeklm/EKLMDigitizer.h>
#include <framework/dataobjects/EventMetaData.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>

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
  cout << bkgHitsArray.getEntries() << " bkgHits read ";
  cout << "in " << m_HitVolumeMap.size() << " volumes" << endl;

  B2INFO("EKLMBackgroundAnalyzerModule::readAndSortBkgHits()  completed");
}

void EKLMBackgroundAnalyzerModule::makeGraphs()
{
  using namespace boost;
  B2INFO("EKLMBackgroundAnalyzerModule::makeGraphs()");

  //loop over volumes
  for (map<string, vector<EKLMBackHit*> >::iterator volumeIterator = m_HitVolumeMap.begin();
       volumeIterator != m_HitVolumeMap.end(); volumeIterator++) {
    // map to store edge <--> hit correspondence
    map < int , EKLMBackHit*> hitMap;
    cout << endl;
    cout << "Name: " << endl << volumeIterator->first << endl;
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
    adjacency_list <> G;



    // used for parent IDs of track coming from abroad to remain graph splittable
    int non_exsisting_rtracks_counter = hitMap.size();



    cerr << "E1" << endl;
    for (map < int , EKLMBackHit*>::iterator hitIterator = hitMap.begin(); hitIterator != hitMap.end(); hitIterator++) {
      //search for parent entry in the map
      cerr << "E2" << endl;
      map < int , EKLMBackHit*>::iterator parentIterator =
        hitMap.find(hitIterator->second->getParentTrackID());
      cerr << "IT1: " << distance(hitMap.begin(), hitIterator) << endl;
      cerr << "IT2: " << distance(hitMap.begin(), parentIterator) << endl;

      if (parentIterator != hitMap.end()) {
        cout << "yes" << endl;
        add_edge(distance(hitMap.begin(), hitIterator), distance(hitMap.begin(), parentIterator)  , G);
      } else {
        cout << "no" << endl;
        add_edge(distance(hitMap.begin(), hitIterator), non_exsisting_rtracks_counter++  , G);
      }
    }


    cerr << "GRAPH OK" << endl;

    graph_traits < adjacency_list <> >::vertex_iterator it, end;
    property_map < adjacency_list <>, vertex_index_t >::type  index_map = get(vertex_index, G);

    cout << "" << endl;
    cout << "vertices:" << endl;

    for (boost::tie(it, end) = vertices(G); it != end; ++it) {
      cout << get(index_map, *it) << endl;
    }
    cout << endl;


    vector<int> component(num_vertices(G));
    int num = connected_components(G, &component[0]);

    vector<int>::size_type i;
    cout << "Total number of components: " << num << endl;
    for (i = 0; i != component.size(); ++i)
      cout << "Vertex " << i << " is in component " << component[i] << endl;
    cout << endl;
































  }
  B2INFO("EKLMBackgroundAnalyzerModule::makeGraphs() completed");
}






void EKLMBackgroundAnalyzerModule::endRun()
{
  B2INFO("EKLMBackgroundAnalyzerModule:: endRun()");
}

void EKLMBackgroundAnalyzerModule::terminate()
{
}




