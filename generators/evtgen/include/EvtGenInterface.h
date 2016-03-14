/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Susanne Koblitz                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVTGENINTERFACE_H
#define EVTGENINTERFACE_H

#include <framework/core/FrameworkExceptions.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <EvtGen/EvtGen.hh>
#include <EvtGenBase/EvtCPUtil.hh>
#include <EvtGenBase/EvtParticle.hh>
#include <EvtGenBase/EvtParticleFactory.hh>
#include <EvtGenBase/EvtPDL.hh>
#include <EvtGenBase/EvtRandom.hh>
#include <EvtGenBase/EvtVector4R.hh>
#include <generators/evtgen/EvtGenFwRandEngine.h>

#include <string>
#include <fstream>

namespace Belle2 {

  /** Class to interface EvtGen.
   * This class is responsible to handle all interaction with evtgen and its
   * classes when generating events. It sets up the models, calls evtgen to
   * generate the events and converts the evtgen particles to a list of
   * MCParticles
   */
  class EvtGenInterface {

  public:
    /**
     * Constructor.
     */
    EvtGenInterface(): m_parent(0), m_Generator(0), m_pinit(0, 0, 0, 0) {}

    /**
     * Destructor.
     */
    ~EvtGenInterface();

    /** Setup evtgen with the given decay and pdl files  */
    int setup(const std::string& decayFileName, const std::string& pdlFileName,
              const std::string& parentParticle,
              const std::string& userFileName = std::string(""));

    /** Generate a single event */
    int simulateEvent(MCParticleGraph& graph, TLorentzVector pParentParticle,
                      TVector3 pPrimaryVertex, int inclusiveType, const std::string& inclusiveParticle);

  private:
    /** Convert EvtParticle structure to flat MCParticle list */
    int addParticles2Graph(EvtParticle* particle, MCParticleGraph& graph, TVector3 pPrimaryVertex);

    /** Copy parameters from EvtParticle to MCParticle */
    void updateGraphParticle(EvtParticle* eParticle,
                             MCParticleGraph::GraphParticle* gParticle, TVector3 pPrimaryVertex);

  protected:
    EvtParticle* m_parent;      /**<Variable needed for parent particle.  */
    EvtGenFwRandEngine m_eng;   /**<Variable needed for random generator. */
    EvtGen* m_Generator;        /**<Variable needed for EvtGen generator. */
    EvtVector4R m_pinit;        /**<Variable needed for initial momentum. */
    EvtId m_ParentParticle;     /**<Variable needed for parent particle ID. */
  }; //! end of EvtGen Interface

} //! end of Belle2 namespace

#endif //EVTGENINTERFACE_H
