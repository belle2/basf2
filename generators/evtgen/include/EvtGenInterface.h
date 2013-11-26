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
#include <generators/dataobjects/MCParticleGraph.h>

#include <evtgen/EvtGen/EvtGen.hh>
#include <evtgen/EvtGenBase/EvtCPUtil.hh>
#include <evtgen/EvtGenBase/EvtParticle.hh>
#include <evtgen/EvtGenBase/EvtParticleFactory.hh>
#include <evtgen/EvtGenBase/EvtPatches.hh>
#include <evtgen/EvtGenBase/EvtPDL.hh>
#include <evtgen/EvtGenBase/EvtRandom.hh>
#include <evtgen/EvtGenBase/EvtReport.hh>
#include <evtgen/EvtGenBase/EvtStdHep.hh>
#include <evtgen/EvtGenBase/EvtStdlibRandomEngine.hh>
#include <evtgen/EvtGenBase/EvtVector4R.hh>

#include <generators/evtgen/EvtGenFwRandEngine.h>

#include <string>
#include <fstream>

#include <TLorentzRotation.h>

namespace Belle2 {

  //! Module for using EvtGen generator
  class EvtGenInterface {

  public:

    //Define exceptions
    /**
     * Constructor.
     */
    EvtGenInterface(): m_parent(0), m_Generator(0), m_pinit(0, 0, 0, 0) {}

    /**
     * Destructor.
     */
    ~EvtGenInterface() {
      if (m_Generator) delete m_Generator;
    }

    int setup(const std::string& decayFileName, const std::string& pdlFileName, const std::string& parentParticle, const std::string& userFileName = std::string("")); /**< Member setup for user decay  */
    int simulateEvent(MCParticleGraph& graph, TLorentzVector pParentParticle, int inclusiveType, const std::string& inclusiveParticle); /**< MC simulation function */
    TLorentzRotation m_labboost;     /**< Boost&rotation vector for boost from CM to LAB. */

  private:
    int addParticles2Graph(EvtParticle* particle, MCParticleGraph& graph); /**< Function to add particle decays */
    void updateGraphParticle(EvtParticle* eParticle, MCParticleGraph::GraphParticle* gParticle); /**< Function to update particle decays */

  protected:
    EvtParticle* m_parent;      /**<Variable needed for parent particle.  */
    EvtStdHep   m_evtstdhep;    /**<Variable needed for STDHEP format.    */
    EvtGenFwRandEngine m_eng;   /**<Variable needed for random generator. */
    EvtGen* m_Generator;        /**<Variable needed for EvtGen generator. */
    EvtVector4R m_pinit;        /**<Variable needed for initial momentum. */
    EvtId m_ParentParticle;     /**<Variable needed for parent particle ID. */
  }; //! end of EvtGen Interface

} //! end of Belle2 namespace

#endif //EVTGENINTERFACE_H
