/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once


#include <EvtGen/EvtGen.hh>
#include <EvtGenBase/EvtParticle.hh>
#include <EvtGenBase/EvtVector4R.hh>
#include <generators/evtgen/EvtGenFwRandEngine.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <framework/dataobjects/MCInitialParticles.h>

#include <framework/utilities/IOIntercept.h>

#include <Math/Vector3D.h>
#include <string>

namespace Belle2 {

  /** Class to interface EvtGen.
   * This class is responsible to handle all interaction with evtgen and its
   * classes when generating events. It sets up the models, calls evtgen to
   * generate the events and converts the evtgen particles to a list of
   * MCParticles
   */
  class EvtGenInterface {

  public:
    /** Create and initialize an EvtGen instance:
     * - Make sure Random engine is setup correctly
     * - Create evt.pdl on the fly from current contents of particle database
     * - Add photos/all models
     * - Use Coherent mixing unless set otherwise
     */
    static EvtGen* createEvtGen(const std::string& decayFileName, bool coherentMixing);

    /**
     * Constructor.
     */
    EvtGenInterface():
      m_parent(0),
      m_Generator(0),
      m_ParentInitialized(false),
      m_logCaptureDebug("EvtGen", LogConfig::c_Debug, LogConfig::c_Warning, 20, 20)
    {}

    /**
     * Destructor.
     */
    ~EvtGenInterface();

    /** Setup evtgen with the given decay files  */
    int setup(const std::string& decayFileName, const std::string& parentParticle,
              const std::string& userFileName = std::string(""), bool coherentMixing = true);

    /** Generate a single event */
    int simulateEvent(MCInitialParticles initial, int inclusiveType, const std::string& inclusiveParticle);

    /** Simulate a particle decay. */
    int simulateDecay(MCParticleGraph& graph,
                      MCParticleGraph::GraphParticle& parent);

  private:
    /** Convert EvtParticle structure to flat MCParticle list */
    int addParticles2Graph(EvtParticle* particle, MCParticleGraph& graph, ROOT::Math::XYZVector pPrimaryVertex,
                           MCParticleGraph::GraphParticle* parent, double timeOffset = 0);

    /** Copy parameters from EvtParticle to MCParticle */
    void updateGraphParticle(EvtParticle* eParticle, MCParticleGraph::GraphParticle* gParticle,
                             ROOT::Math::XYZVector pPrimaryVertex, double timeOffset = 0);

  protected:
    EvtParticle* m_parent;      /**<Variable needed for parent particle.  */
    static EvtGenFwRandEngine m_eng;   /**<Variable needed for random generator. */
    EvtGen* m_Generator;        /**<Variable needed for EvtGen generator. */
    EvtId m_ParentParticle;     /**<Variable needed for parent particle ID. */
    bool m_ParentInitialized;   /**< Whether parent particle is initialized. */
    IOIntercept::OutputToLogMessages m_logCaptureDebug; /**< Capture EvtGen log and transform into basf2 logging as B2DEBUG messages. */
  }; //! end of EvtGen Interface

} //! end of Belle2 namespace

