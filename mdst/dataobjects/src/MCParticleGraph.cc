/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mdst/dataobjects/MCParticleGraph.h>

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>

#include <limits>
#include <vector>
#include <queue>


using namespace std;
using namespace boost;
using namespace Belle2;

/** Simple struct to check boost graph for cyclic references. */
struct cycle_detector : public dfs_visitor<> {
  /**
   * This method is invoked on back edges in the graph.
   * If a back edge is found, it means a cyclic reference was found and therefore
   * an exception of type MCParticleGraph::CyclicReferenceError() is thrown.
   * @tparam Edge The edge of the graph.
   * @tparam Graph Reference to the graph itself.
   */
  template <class Edge, class Graph> void back_edge(Edge, Graph&) { throw MCParticleGraph::CyclicReferenceError(); }
};


/**
 * Class to go over all the particles in the Graph an sort them in a sensible way.
 *
 * All particles will be assigned an index, starting at 1. If an particle has its
 * ignore flag set, then it will get assigned the index of its last, unignored parent.
 * The particles will be sorted by depth: First all primary particles, then all direct
 * daughters of the first primary particle, all direct daughters of the second primary
 * particle and so forth.
 */
class Belle2::MCParticleGraph::ParticleSorter {

public:

  /**
   * ParticleSorter constructor.
   * @param particles Reference to the list of particles which should be sorted.
   * @param plist The final array of sorted particles which is stored in the DataStore.
   * @param setVertex Set to true if the vertex information should be saved.
   * @param setTime Set to true if the production time information should be saved.
   */
  ParticleSorter(MemoryPool<MCParticleGraph::GraphParticle>& particles, TClonesArray* plist, bool setVertex, bool setTime):
    m_index(0), m_particles(particles), m_plist(plist), m_setVertex(setVertex), m_setTime(setTime) {}

  /**
   * Set the starting index for the particle graph. Normally this is 0 so the first real particle has an id of 1
   * but this can be used if there are already particles present so the first particle will have id N+1
   * @param index number of particles already present
   */
  void setStartIndex(int index) { m_index = index; }

  /**
   * Sort the particles and generate MCParticle list.
   * @param g The graph which should be sorted and transformed to a list of particles.
   */
  template <class Graph> void sort(Graph& g)
  {
    //Set seen flag for all vertices to false
    m_seen.clear();
    m_seen.resize(num_vertices(g), false);

    //Create a dummy GraphParticle, needed only to find all primary particles
    MCParticleGraph::GraphParticle dummy(0, 0);

    //Add all direct children of the 0th vertex to the queue.
    //This are the primary particles
    find_daughters(0, g, dummy);

    //Go through the queue and write out each particle.
    //Daughters of particles will be added to the queue by find_daughters
    while (!m_vqueue.empty()) {
      unsigned int cur = m_vqueue.front();
      m_vqueue.pop();
      finish_vertex(cur, g);
    }
  }


  /**
   * Go through the daughters of the vertex. Add the found particles to the final particle list.
   * @param v The vertex whose daughters are investigated.
   * @param g The graph in which the vertex lives.
   */
  template <class Vertex, class Graph> void finish_vertex(Vertex v, Graph& g)
  {
    MCParticleGraph::GraphParticle& p = *m_particles[v - 1];

    //Reset daughter information, will be filled by find_daughters
    p.setFirstDaughter(0);
    p.setLastDaughter(0);
    //Find all direct daughters
    find_daughters(v, g, p);

    //If stable particle, set decaytime to infinity
    if (out_degree(v, g) == 0 && m_setTime) {
      p.setDecayTime(numeric_limits<double>::infinity());
    }
    //If given a pointer to a TClonesArray, create MCParticle at the appropriate index position
    if (m_plist) {
      new(m_plist->AddrAt(p.getIndex() - 1)) MCParticle(m_plist, p);
    }
  }


  /**
   * Find the daughters of the given vertex.
   * @param v The vertex whose daughters are investigated.
   * @param g The graph in which the vertex lives.
   * @param mother Reference to the mother particle.
   */
  template <class Vertex, class Graph> void find_daughters(Vertex v, Graph& g, MCParticleGraph::GraphParticle& mother)
  {
    //References to the daughter information of the mother for easier access
    int& d1 = mother.m_firstDaughter;
    int& d2 = mother.m_lastDaughter;

    typename graph_traits<Graph>::out_edge_iterator j, j_end;
    for (tie(j, j_end) = out_edges(v, g); j != j_end; ++j) {
      //Get daughter particle from list
      Vertex nv = target(*j, g);
      MCParticleGraph::GraphParticle& daughter = *m_particles[nv - 1];

      if (daughter.m_ignore) {
        //daughter ignored, search its children and treat them as direct children of mother
        //if we haven't seen this particle yet, set its index to that of its last unignored parent
        if (!m_seen[nv]) daughter.setIndex(mother.getIndex());
        find_daughters(nv, g, mother);
      } else {
        //If we didn't see this particle already, set its index and add it to the queue for writing out
        if (!m_seen[nv]) {
          daughter.setIndex(++m_index);
          m_vqueue.push(nv);
        }
        //Set daughter information of mother. If 0, no daughters yet so just take current daughter as only
        //daughter. Otherwise allow extension of daughter information in both directions.
        if (d1 == 0) {
          d1 = daughter.getIndex();
          d2 = d1;
        } else if ((d2 + 1) == daughter.getIndex()) {
          ++d2;
        } else if ((d1 - 1) == daughter.getIndex()) {
          --d1;
        } else {
          //Daughter indices are not continous, cannot continue
          throw MCParticleGraph::NonContinousDaughtersError();
        }
        //Set Vertex and time information if requested
        setVertexTime(mother, daughter);
        daughter.m_mother = mother.getIndex();
      }
      //Mark particle as seen
      m_seen[nv] = true;
    }
  }


  /**
   * Set the vertex and time information of the mother particle.
   * @param m Reference to the mother particle whose vertex and time information should be set.
   * @param d Reference to the daughter particle.
   */
  void setVertexTime(MCParticleGraph::GraphParticle& m, const MCParticleGraph::GraphParticle& d)
  {
    //Only set vertex information if both particles have a valid vertex set
    m.setValidVertex(m.hasValidVertex() &&  d.hasValidVertex());
    if (m.hasValidVertex() && d.getProductionTime() >= m.getDecayTime()) {
      if (m_setVertex) {
        m.setDecayVertex(B2Vector3D(d.getProductionVertex()));
      }
      if (m_setTime) {
        m.setDecayTime(d.getProductionTime());
      }
    }
  }


protected:

  int m_index;                                             /**< The latest index given to a particle. */
  MemoryPool<MCParticleGraph::GraphParticle>& m_particles; /**< Reference to the list of particles which should be sorted. */
  TClonesArray*
  m_plist;                                   /**< The final array of sorted particles which is stored in the DataStore. */
  bool m_setVertex;                                        /**< True if the vertex information should be saved. */
  bool m_setTime;                                          /**< True if the production time information should be saved. */
  vector<bool>
  m_seen;                                     /**< Vector of the particles that were already seen while sorting the graph. */
  std::queue<unsigned int> m_vqueue;                       /**< The list of the vertices that will be visited. */
};


void MCParticleGraph::generateList(const string& name, int options)
{
  StoreArray<MCParticle> MCParticles(name);

  //Make Graph and connect all primary vertices (particles without mother)
  //to an artificial 0ths vertex to be able to find them easily
  typedef adjacency_list<vecS, vecS, directedS> Graph;
  int num_particles(0);
  //Determine number of not ignored particles and add an edge from 0ths vertex to any primary
  //particle
  for (unsigned int i = 0; i < m_particles.size(); ++i) {
    if (!m_particles[i]->m_ignore) ++num_particles;
    if (m_particles[i]->m_primary) m_decays.insert(DecayLine(0, i + 1));
  }
  Graph g(m_decays.begin(), m_decays.end(), m_particles.size() + 1);

  //Check for cyclic dependency
  if (options & c_checkCyclic) {
    cycle_detector vis;
    depth_first_search(g, visitor(vis));
  }

  //Fill TClonesArray in correct order
  if (options & c_clearParticles) MCParticles.getPtr()->Clear();
  MCParticles.getPtr()->Expand(num_particles + MCParticles.getEntries());
  MCParticleGraph::ParticleSorter psorter(m_particles, MCParticles.getPtr(), options & c_setDecayVertex, options & c_setDecayTime);
  psorter.setStartIndex(MCParticles.getEntries());
  psorter.sort(g);
}

void MCParticleGraph::loadList(const string& name)
{
  StoreArray<MCParticle> MCParticles(name);
  if (!MCParticles) {
    B2ERROR("MCParticle Collection is not valid, cannot load into Graph");
    return;
  }

  unsigned numParticles = MCParticles.getEntries();
  unsigned particleOffset = size();
  //Here we assume that the MCParticle collection is somehow ordered: All
  //particles which are product of a decay come in the list after the mother,
  //thus having a higher index. This is true for all lists generated by the
  //MCParticleGraph and also for the standard lists produced by Evtgen and
  //similiar generators.
  for (unsigned i = 0; i < numParticles; ++i) {
    GraphParticle& newParticle = addParticle();
    const MCParticle& oldParticle = *MCParticles[i];
    //Copy all values
    newParticle = oldParticle;
    //If this particle has a mother we just add the decay to this mother
    const MCParticle* oldMother = oldParticle.getMother();
    if (oldMother != nullptr) {
      unsigned motherIndex  = oldMother->getArrayIndex() + particleOffset;
      if (motherIndex >= size())
        B2FATAL("MCParticle collection \"" << name << "\" not sorted correctly: mother index larger than daughter. Cannot load into Graph");
      newParticle.comesFrom((*this)[oldMother->getArrayIndex() + particleOffset]);
    }
  }
}
