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

    int setup(const std::string& decayFileName, const std::string& pdlFileName, const std::string& parentParticle, const std::string& userFileName = std::string(""));
    int simulateEvent(MCParticleGraph& graph);

    TLorentzRotation m_labboost;     /**< Boost&rotation vector for boost from CM to LAB. */

  private:
    int addParticles2Graph(EvtParticle* particle, MCParticleGraph& graph);
    void updateGraphParticle(EvtParticle* eParticle, MCParticleGraph::GraphParticle* gParticle);


  protected:
    EvtParticle* m_parent;
    EvtStdHep   m_evtstdhep;
    EvtGenFwRandEngine m_eng;
    EvtGen* m_Generator;
    EvtVector4R m_pinit;
    EvtId m_ParentParticle;
  };

}

#endif //EVTGENINTERFACE_H
