/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz                                          *
 *                                                                        *
 **************************************************************************/

#ifndef MAKESIMULATIONTREEMODULE_H
#define MAKESIMULATIONTREEMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <generators/dataobjects/MCParticle.h>
#include <TTree.h>
#include <TFile.h>

namespace Belle2 {

  /** The makeSimulationTree module.
   * creates a root tree that contains the information from
   * the MCParticles and the simulated hits.
   * NOTE: so far only the storage of hits from SVD and PXD is implemented.
   */
  class makeSimulationTreeModule : public Module {//start class declaration

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    makeSimulationTreeModule();

    /** Destructor. */
    virtual ~makeSimulationTreeModule() {}

    /** Initializes the module. */
    virtual void initialize();

    /** Method is called for each event. */
    virtual void event();

    /** Initializes the module. */
    virtual void terminate();


  protected:
    std::string m_mcParticleInputColName;  /**< The parameter variable for the name of the input MCParticle collection. */
    std::string m_outputFileName; /**< The filename of the output root file. */
    TTree * m_tree;               /**< The tree that is being filled */
    TFile * m_file;               /**< The file where the tree will be stored in */

    int m_evtNum;                  /**< The current event number */
    int m_runNum;                  /**< The current run number  */
    int m_expNum;                  /**< The current experiment number */


    //member variable for the particle informations:
    /** 1-based index of the particle, will be set automatically after deserialisation if needed. */
    int m_MCP_index; //! transient 1-based index of particle

    unsigned int m_MCP_status;      /**< status code */
    int m_MCP_pdg;                  /**< PDG-Code of the particle */
    float m_MCP_mass;               /**< mass of the particle */
    float m_MCP_energy;             /**< energy of the particle */
    float m_MCP_px;                 /**< momentum of particle, x component */
    float m_MCP_py;                 /**< momentum of particle, y component */
    float m_MCP_pz;                 /**< momentum of particle, z component */

    bool m_MCP_validVertex;         /**< indication wether vertex and time information is useful or just default */

    float m_MCP_productionTime;     /**< production time */
    float m_MCP_productionVertex_x; /**< production vertex of particle, x component */
    float m_MCP_productionVertex_y; /**< production vertex of particle, y component */
    float m_MCP_productionVertex_z; /**< production vertex of particle, z component */

    float m_MCP_decayTime;          /**< decay time */
    float m_MCP_decayVertex_x;      /**< decay vertex of particle, x component */
    float m_MCP_decayVertex_y;      /**< decay vertex of particle, y component */
    float m_MCP_decayVertex_z;      /**< decay vertex of particle, z component */

    int m_MCP_mother;               /**< 1-based index of the mother particle */
    int m_MCP_firstdaughter;        /**< 1-based index of first daughter particle in collection, 0 if no daughters */
    int m_MCP_lastdaughter;         /**< 1-based index of last daughter particle in collection, 0 if no daughters */


    //member variables for the PXD hit information:we allow for 10 hits per particle
    int m_PXD_hcount;                /**< number of PXhits for the track */
    int m_PXD_layerID[10];           /**< Layer number. */
    int m_PXD_ladderID[10];          /**< Ladder number. */
    int m_PXD_sensorID[10];          /**< Sensor number. */

    float m_PXD_posInX[10];      /**< LRF position at entry  X. */
    float m_PXD_posInY[10];      /**< LRF position at entry  Y. */
    float m_PXD_posInZ[10];      /**< LRF position at entry  Z. */
    float m_PXD_posOutX[10];     /**< LRF position at exit  X. */
    float m_PXD_posOutY[10];     /**< LRF position at exit  Y. */
    float m_PXD_posOutZ[10];     /**< LRF position at exit  Z. */
    float m_PXD_theta[10];          /**< Theta angle (wrt global z). */
    float m_PXD_momInX[10];      /**< GRF Momentum at entry  X. */
    float m_PXD_momInY[10];      /**< GRF Momentum at entry  Y. */
    float m_PXD_momInZ[10];      /**< GRF Momentum at entry  Z. */

    int m_PXD_PDGcode[10];         /**< The PDG value of the particle that created the hit. */
    int m_PXD_trackID[10];         /**< The ID of the track that created the hit. */
    float m_PXD_energyDep[10];    /**< Deposited energy. */
    float m_PXD_stepLength[10];   /**< Step length. */
    float m_PXD_globalTime[10];   /**< Global time. */

    //member variables for the SVD hit information:we allow for 20 hits per particle
    int m_SVD_hcount;                /**< number of SVDhits for the track */
    int m_SVD_layerID[20];           /**< Layer number. */
    int m_SVD_ladderID[20];          /**< Ladder number. */
    int m_SVD_sensorID[20];          /**< Sensor number. */

    float m_SVD_posInX[20];      /**< LRF position at entry  X. */
    float m_SVD_posInY[20];      /**< LRF position at entry  Y. */
    float m_SVD_posInZ[20];      /**< LRF position at entry  Z. */
    float m_SVD_posOutX[20];     /**< LRF position at exit  X. */
    float m_SVD_posOutY[20];     /**< LRF position at exit  Y. */
    float m_SVD_posOutZ[20];     /**< LRF position at exit  Z. */
    float m_SVD_theta[20];          /**< Theta angle (wrt global z). */
    float m_SVD_momInX[20];      /**< GRF Momentum at entry  X. */
    float m_SVD_momInY[20];      /**< GRF Momentum at entry  Y. */
    float m_SVD_momInZ[20];      /**< GRF Momentum at entry  Z. */

    int m_SVD_PDGcode[20];         /**< The PDG value of the particle that created the hit. */
    int m_SVD_trackID[20];         /**< The ID of the track that created the hit. */
    float m_SVD_energyDep[20];    /**< Deposited energy. */
    float m_SVD_stepLength[20];   /**< Step length. */
    float m_SVD_globalTime[20];   /**< Global time. */



  };//end class declaration
} // end namespace Belle2

#endif // MAKESIMULATIONTREEMODULE_H
