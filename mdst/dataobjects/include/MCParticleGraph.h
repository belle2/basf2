/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCPARTICLEGRAPH_H
#define MCPARTICLEGRAPH_H

#include <framework/core/FrameworkExceptions.h>
#include <framework/core/MemoryPool.h>
#include <mdst/dataobjects/MCParticle.h>

#include <set>
#include <string>


namespace Belle2 {

  /**
   * Class to build, validate and sort a particle decay chain.
   *
   * Several particles can be added with addParticle(). Decay information between these
   * particles can be added using addDecay(), MCParticleGraph::Particle::decaysInto() and
   * MCParticleGraph::Particle::comesFrom().
   *
   * Once all all particles and decays are added and their parameters set, call generateList() to fill
   * the specified StoreArray with the resulting MCParticle list. The decay chain will be checked for
   * cyclic references and a runtime_error will be thrown in that case. The particle will be sorted
   * breadth first: First all primary particles, then all particles first generation and so on.
   */
  class MCParticleGraph {

  public:

    //Exception definition

    /** The exception is thrown if a cyclic reference in the graph was detected. */
    BELLE2_DEFINE_EXCEPTION(CyclicReferenceError, "Cyclic decay, cannot continue")
    /** The exception is thrown if two particles do not belong to the same graph. */
    BELLE2_DEFINE_EXCEPTION(NotSameGraphError, "Particles not from same graph")
    /** The exception is thrown if a non-physical decay was detected in the graph. */
    BELLE2_DEFINE_EXCEPTION(NonContinousDaughtersError, "Can not represent decay graph, non continuous indices for daughters")
    /** The exception is thrown if a daughter already has a mother assigned to it. */
    BELLE2_DEFINE_EXCEPTION(DaughterHasMotherError,
                            "A daughter particle was already assigned to a mother. A particle can't have two mothers!")
    /** The exception is thrown if the specified index is out of range. */
    BELLE2_DEFINE_EXCEPTION(OutOfRangeError, "Index out of range")

    /** Type representing a decay in the graph. */
    typedef std::pair<unsigned int, unsigned int> DecayLine;

    /** Possible options for generating the MCParticle list. */
    enum GraphOptions {
      c_setNothing      = 0, /**< Do nothing special */
      c_setDecayVertex = 1, /**< Set the decay vertex to the production vertex of the last daughter (ordered by production time) */
      c_setDecayTime   = 2, /**< Set decay time to the largest production time of the daughters */
      c_setDecayInfo   = c_setDecayVertex | c_setDecayTime, /**< Set decay time and vertex */
      c_checkCyclic     = 4,  /**< Check for cyclic dependencies */
      c_clearParticles = 8 /**< Clear the particle list before adding the graph */
    };

    /**
     * Class to represent Particle data in graph.
     *
     * This is an extended version of the MCParticle class, allowing to set index and
     * daughter indices as well as adding decay information. Use MCParticleGraph::addParticle()
     * to add a new particle to the graph.
     */
    class GraphParticle: public MCParticle {

    public:
      /**
       * Assign the values of an existing MCParticle to this GraphParticle.
       * This assignment will leave all bookkeeping members of the
       * GraphParticle untouched (decays, ignore status, track id) but will
       * assign all values/flags from the MCParticle.
       * @param particle Particle which values (momentum, energy, mass, vertex, etc.) shall be assigned
       */
      GraphParticle& operator=(const MCParticle& particle)
      {
        MCParticle::operator=(particle);
        //The pointer to the TClonesArray the MCParticle is stored in makes no
        //sense inside the Graph so we set it to some invalid value to avoid
        //the MCParticle::fixParticleList() to complain
        m_plist = (TClonesArray*) - 1;
        return *this;
      }

      /**
       * Tells the graph that this particle decays into daughter.
       * @param daughter The daughter particle in which this particle decays.
       */
      void decaysInto(GraphParticle& daughter) { m_graph->addDecay(*this, daughter); }

      /**
       * Tells the graph that this particle is a decay product of mother.
       * @param mother The mother particle of this particle.
       */
      void comesFrom(GraphParticle& mother) { m_graph->addDecay(mother, *this); }

      /**
       * Set the 1-based index of the first daughter, 0 means no daughters.
       *
       * Will be reset when sorting the particle graph and will be set according
       * to the information given with decaysInto() and comesFrom().
       * @param daughter The index of the first daughter particle. Set to 0 for no daughters.
       */
      void setFirstDaughter(int daughter) { m_firstDaughter = daughter; }

      /**
       * Set the 1-based index of the last daughter, 0 means no daughters.
       *
       * @see setFirstDaughter()
       * @param daughter The index of the last daughter particle. Set to 0 for no daughters.
       */
      void setLastDaughter(int daughter) { m_lastDaughter = daughter; }

      /**
       * Set or remove the ignore flag.
       *
       * If ignore is set, this particle will not be written to the MCParticle list
       * and all its daughters without ignore flag will be attributed to all parents
       * of this particle.
       * @param ignore Set to true to ignore this particle when creating the MCParticle list.
       */
      void setIgnore(bool ignore = true) { m_ignore = ignore; }

      /**
       * Get the ignore flag.
       *
       * @return whether the ignore flag is set
       */
      bool getIgnore() const { return m_ignore; }

      /**
       * Set the track ID for the particle.
       *
       * The track ID is used to build the relationship between MCParticles and created Hits.
       * @param trackID The ID of the Geant4 track which created this MCParticle.
       */
      void setTrackID(int trackID) { m_trackID = trackID; }

      /**
       * Returns the track ID assigned to this MCParticle.
       *
       * @return The ID of the Geant4 track which created this particle.
       */
      int getTrackID() const { return m_trackID; }


    private:

      /**
       * Private constructor. This class gets instantiated by MCParticleGraph::addParticle()
       */
      GraphParticle() {}

      /**
       * No copying allowed.
       * @param copy Reference to the particle which should be copied.
       */
      GraphParticle(const GraphParticle& copy): MCParticle(copy) {}

      /**
       * Hide MCParticle "almost copy" constructor.
       */
      GraphParticle(TClonesArray*, const MCParticle&) {}

      /**
       * Internally used constructor. Create a new Particle with given index and a pointer
       * to the containing graph.
       * @param graph Pointer to the graph the particle is part of.
       * @param index The vertex id of the particle in the graph.
       */
      GraphParticle(MCParticleGraph* graph, unsigned int vertexId): MCParticle(),
        m_graph(graph), m_vertexId(vertexId), m_ignore(false), m_primary(true), m_trackID(0)
      {
        //The pointer to the TClonesArray the MCParticle is stored in makes no
        //sense inside the Graph so we set it to some invalid value to avoid
        //the MCParticle::fixParticleList() to complain
        m_plist = (TClonesArray*) - 1;
      }

      /**
       * Set the 1-based index of the particle.
       * @param index The index of the particle.
       */
      void setIndex(int index) { m_index = index; }

      MCParticleGraph* m_graph; /**< internal pointer to the graph this particle belongs to */
      unsigned int m_vertexId;  /**< vertex id in the graph */
      bool m_ignore;            /**< ignore particle when writing MCParticle list ? */
      bool m_primary;           /**< Is this a primary particle ? */
      int m_trackID;            /**< The track ID from geant4 that created this particle. */

      friend class MCParticleGraph;
      friend class ParticleSorter;
    };

    /**
     * Constructor.
     */
    MCParticleGraph() {}

    /**
     * Destructor.
     * Frees the allocated spaces
     */
    virtual ~MCParticleGraph() { clear(); }

    /**
     * Add new particle to the graph.
     * Set the values of the returned reference to determine the particle parameters.
     * @return A reference to the added particle.
     */
    GraphParticle& addParticle();

    /**
     * Add decay information between two particles.
     * If the daughter has already a mother attached, an exception of type DaughterHasMotherError is thrown.
     *
     * @param mother The mother particle which decays.
     * @param daughter The daughter particle in which the mother particle decays.
     */
    void addDecay(GraphParticle& mother, GraphParticle& daughter);

    /**
     * Return reference to added particle with range check.
     * @return A reference to the particle given by its index.
     */
    GraphParticle& operator[](size_t i) { if (i >= m_particles.size()) throw OutOfRangeError(); return *m_particles[i]; }

    /**
     * Return the number of particles in the graph.
     * @return The number of particles in the graph.
     */
    size_t size() const { return m_particles.size(); }

    /**
     * Generates the MCParticle list and stores it in the StoreArray with the given name.
     *
     * The graph will be checked for validity. If a cyclic reference is detected, a runtime_error is thrown.
     * Similar, if the decay chain cannot be represented with continuous daughter indices, a runtime_error
     * is raised. This can happen for artificial kinds of decays (eg: a->b,c,d; b->c,e;) but should not
     * happen in physical cases.
     *
     * @param name Name of the StoreArray for the MCParticle list
     * @param options Additional options which steer the creation of the StoreArray.
     * @see class MCParticle
     */
    void generateList(const std::string& name = "", int options = c_setNothing);


    /**
     * Load the MCParticle list given by name into the Graph
     * @param name Name of the StoreArray for the MCParticle list
     * @see class MCParticle
     */
    void loadList(const std::string& name = "");

    /**
     * Reset particles and decay information to make the class reusable.
     * New particles can be added again after a call to clear(),
     * e.g. in the next event
     */
    void clear();


  protected:

    /**
     * Class to sort particles in graph, not used outside MCParticleGraph.
     * only forward declared since it is a has to be a friend of MCParticleGraph::GraphParticle
     */
    class ParticleSorter;

    MemoryPool<GraphParticle> m_particles; /**< internal list of particles */
    std::set<DecayLine> m_decays;          /**< internal set of decay lines */
  };


  inline void MCParticleGraph::clear()
  {
    m_particles.clear();
    m_decays.clear();
  }


  inline MCParticleGraph::GraphParticle& MCParticleGraph::addParticle()
  {
    unsigned int index = m_particles.size() + 1;
    MCParticleGraph::GraphParticle* p = new(m_particles.add()) MCParticleGraph::GraphParticle(this, index);
    return *p;
  }


  inline void MCParticleGraph::addDecay(MCParticleGraph::GraphParticle& mother, MCParticleGraph::GraphParticle& daughter)
  {
    if (this != mother.m_graph || this != daughter.m_graph) throw NotSameGraphError();
    //if (daughter.getMother() != NULL) throw DaughterHasMotherError();
    m_decays.insert(DecayLine(mother.m_vertexId, daughter.m_vertexId));
    daughter.m_primary = false;
  }

} // end namespace Belle2

#endif //MCPARTICLEGRAPH_H
