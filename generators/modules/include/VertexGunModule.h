/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VERTEXGUNMODULE_H
#define VERTEXGUNMODULE_H

#include <framework/core/Module.h>

#include <generators/dataobjects/MCParticle.h>
#include <generators/dataobjects/MCParticleGraph.h>
#include <generators/particlegun/ParticleGun.h>

#include <string>
#include <vector>

namespace Belle2 {

  /** The VertexGun module.
   * Generates an arbitrary number of vertices each with different arbitrary tracks. The properties of every track of every vertex are used.
   * The output are MCParticle objects put into the data store.
   */
  class VertexGunModule : public Module {
  public:
    /**
     * Constructor.
     * Sets the module parameters.
     */
    VertexGunModule();

    /** Destructor. */
    virtual ~VertexGunModule() {}

    /** Initializes the module. */
    void initialize();

    /** Method is called for each event. */
    void event();

  protected:

    //input parameters
    /** first components of the vertex x position distributions parameter. Every element of the vector is given to one vertex in every event  */
    std::vector<double> m_xParams1;
    /** first components of the vertex y position distributions parameter. Every element of the vector is given to one vertex in every event  */
    std::vector<double> m_yParams1;
    /** first components of the vertex z position distributions parameter. Every element of the vector is given to one vertex in every event  */
    std::vector<double> m_zParams1;
    /** second components of the vertex x position distributions parameter. Every element of the vector is given to one vertex in every event  */
    std::vector<double> m_xParams2;
    /** second components of the vertex y position distributions parameter. Every element of the vector is given to one vertex in every event  */
    std::vector<double> m_yParams2;
    /** second components of the vertex z position distributions parameter. Every element of the vector is given to one vertex in every event  */
    std::vector<double> m_zParams2;
    /** Names of the distributions used for the vertex creation */
    std::vector<std::string> m_vertexDists;
    /** Number of tracks per vertex. Every element of the vector is given to one vertex in every event */
    std::vector<int> m_tracksPerVertex;
    /** pdg codes for every track in very vertex in one event */
    std::vector<int> m_pdgCodes;
    /** first components of the momentum distributions parameter. Every element of the vector is given to the tracks in one vertex in every event  */
    std::vector<double> m_pParams1;
    /** second components of the momentum distributions parameter. Every element of the vector is given to the tracks in one vertex in every event  */
    std::vector<double> m_pParams2;
    /** Names of the distributions used for the momentum creation of the tacks in each vertex */
    std::vector<std::string> m_pDists;
    /** first components of the phi distributions parameter. Every element of the vector is given to the tracks in one vertex in every event  */
    std::vector<double> m_phiParams1;
    /** second components of the phi distributions parameter. Every element of the vector is given to the tracks in one vertex in every event  */
    std::vector<double> m_phiParams2;
    /** Names of the distributions used for the phi creation of the tacks in each vertex */
    std::vector<std::string> m_phiDists;
    /** first components of the theta distributions parameter. Every element of the vector is given to the tracks in one vertex in every event  */
    std::vector<double> m_thetaParams1;
    /** second components of the theta distributions parameter. Every element of the vector is given to the tracks in one vertex in every event  */
    std::vector<double> m_thetaParams2;
    /** Names of the distributions used for the theta creation of the tacks in each vertex */
    std::vector<std::string> m_thetaDists;

    //std::vector<bool> m_varyNumberOfTracks; not used at the moment

    /** function to convert the strings used in the parameter list to the internal encoding */
    ParticleGun::Distribution convertDistribution(std::string name);

    /** Instance of the particle gun */
    ParticleGun m_particleGun;
    /** Parameters of the particle gun for every vertex in one event */
    std::vector<ParticleGun::Parameters> m_parametersForAllVertices;

    /** Particle graph to generate MCParticle list */
    MCParticleGraph m_particleGraph;
  };

} // end namespace Belle2

#endif // VERTEXGUNMODULE_H
