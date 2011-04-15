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
    std::string m_outputFileName; /**< The filename of the output root file. */
    TTree * m_tree;               /**< The tree that is being filled */
    TFile * m_file;               /**< The file where the tree will be stored in */

    int m_evtNum;                  /**< The current event number */
    int m_runNum;                  /**< The current run number  */
    int m_expNum;                  /**< The current experiment number */


    //member variable for the particle informations:
    /** 1-based index of the particle, will be set automatically after deserialisation if needed. */
    int m_MCPindex; //! transient 1-based index of particle

    unsigned int m_MCPstatus;      /**< status code */
    int m_MCPpdg;                  /**< PDG-Code of the particle */
    float m_MCPmass;               /**< mass of the particle */
    float m_MCPcharge;             /**< charge of the particle */
    float m_MCPenergy;             /**< energy of the particle */
    float m_MCPpX;                 /**< momentum of particle, x component */
    float m_MCPpY;                 /**< momentum of particle, y component */
    float m_MCPpZ;                 /**< momentum of particle, z component */
    float m_MCPpTot;               /**< total momentum of particle */
    float m_MCPpT;                 /**< transverse momentum of particle */
    float m_MCPphi;                 /**< phi angle of particle */
    float m_MCPtheta;                 /**< theta angle of particle */

    bool m_MCPvalidVertex;         /**< indication wether vertex and time information is useful or just default */

    float m_MCPproductionTime;     /**< production time */
    float m_MCPproductionVertexX; /**< production vertex of particle, x component */
    float m_MCPproductionVertexY; /**< production vertex of particle, y component */
    float m_MCPproductionVertexZ; /**< production vertex of particle, z component */

    float m_MCPdecayTime;          /**< decay time */
    float m_MCPdecayVertexX;      /**< decay vertex of particle, x component */
    float m_MCPdecayVertexY;      /**< decay vertex of particle, y component */
    float m_MCPdecayVertexZ;      /**< decay vertex of particle, z component */

    int m_MCPmother;               /**< 1-based index of the mother particle */
    int m_MCPfirstDaughter;        /**< 1-based index of first daughter particle in collection, 0 if no daughters */
    int m_MCPlastDaughter;         /**< 1-based index of last daughter particle in collection, 0 if no daughters */

    static const int cMaxSizeHitArray = 100;    /**< Maximum size for the hit arrays */

    //member variables for the PXD hit information:we allow for 10 hits per particle
    int m_PXDhcount;                /**< number of PXhits for the track */
    int m_PXDlayerID[cMaxSizeHitArray];           /**< Layer number. */
    int m_PXDladderID[cMaxSizeHitArray];          /**< Ladder number. */
    int m_PXDsensorID[cMaxSizeHitArray];          /**< Sensor number. */

    float m_PXDposInX[cMaxSizeHitArray];      /**< LRF position at entry  X. */
    float m_PXDposInY[cMaxSizeHitArray];      /**< LRF position at entry  Y. */
    float m_PXDposInZ[cMaxSizeHitArray];      /**< LRF position at entry  Z. */
    float m_PXDposOutX[cMaxSizeHitArray];     /**< LRF position at exit  X. */
    float m_PXDposOutY[cMaxSizeHitArray];     /**< LRF position at exit  Y. */
    float m_PXDposOutZ[cMaxSizeHitArray];     /**< LRF position at exit  Z. */
    float m_PXDtheta[cMaxSizeHitArray];          /**< Theta angle (wrt global z). */
    float m_PXDmomInX[cMaxSizeHitArray];      /**< GRF Momentum at entry  X. */
    float m_PXDmomInY[cMaxSizeHitArray];      /**< GRF Momentum at entry  Y. */
    float m_PXDmomInZ[cMaxSizeHitArray];      /**< GRF Momentum at entry  Z. */

    int m_PXDPDGcode[cMaxSizeHitArray];         /**< The PDG value of the particle that created the hit. */
    int m_PXDtrackID[cMaxSizeHitArray];         /**< The ID of the track that created the hit. */
    float m_PXDenergyDep[cMaxSizeHitArray];    /**< Deposited energy. */
    float m_PXDstepLength[cMaxSizeHitArray];   /**< Step length. */
    float m_PXDglobalTime[cMaxSizeHitArray];   /**< Global time. */

    //member variables for the SVD hit information:we allow for 40 hits per particle
    int m_SVDhcount;                /**< number of SVDhits for the track */
    int m_SVDlayerID[cMaxSizeHitArray];           /**< Layer number. */
    int m_SVDladderID[cMaxSizeHitArray];          /**< Ladder number. */
    int m_SVDsensorID[cMaxSizeHitArray];          /**< Sensor number. */

    float m_SVDposInX[cMaxSizeHitArray];      /**< LRF position at entry  X. */
    float m_SVDposInY[cMaxSizeHitArray];      /**< LRF position at entry  Y. */
    float m_SVDposInZ[cMaxSizeHitArray];      /**< LRF position at entry  Z. */
    float m_SVDposOutX[cMaxSizeHitArray];     /**< LRF position at exit  X. */
    float m_SVDposOutY[cMaxSizeHitArray];     /**< LRF position at exit  Y. */
    float m_SVDposOutZ[cMaxSizeHitArray];     /**< LRF position at exit  Z. */
    float m_SVDtheta[cMaxSizeHitArray];          /**< Theta angle (wrt global z). */
    float m_SVDmomInX[cMaxSizeHitArray];      /**< GRF Momentum at entry  X. */
    float m_SVDmomInY[cMaxSizeHitArray];      /**< GRF Momentum at entry  Y. */
    float m_SVDmomInZ[cMaxSizeHitArray];      /**< GRF Momentum at entry  Z. */

    int m_SVDPDGcode[cMaxSizeHitArray];         /**< The PDG value of the particle that created the hit. */
    int m_SVDtrackID[cMaxSizeHitArray];         /**< The ID of the track that created the hit. */
    float m_SVDenergyDep[cMaxSizeHitArray];    /**< Deposited energy. */
    float m_SVDstepLength[cMaxSizeHitArray];   /**< Step length. */
    float m_SVDglobalTime[cMaxSizeHitArray];   /**< Global time. */



  };//end class declaration
} // end namespace Belle2

#endif // MAKESIMULATIONTREEMODULE_H
