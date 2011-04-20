/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDSPACEPOINTMAKERMODULE_H
#define VXDSPACEPOINTMAKERMODULE_H

#include <framework/core/Module.h>
#include <pxd/geopxd/CIDManager.h>
#include <pxd/geopxd/SiGeoCache.h>

// Mulitiindices
#include <pxd/dataobjects/HitSorter.h>
#include <pxd/dataobjects/RelationHolder.h>

#include <string>


namespace Belle2 {

  /**
   * VXD (PXD+SVD spacepoint maker.
   *
   * This module takes data from PXD/SVD SimHits and transforms them to global
   * coordinates.
   */

  class VXDSpacePointMakerModule : public Module {

  public:

    /** Constructor.*/
    VXDSpacePointMakerModule();

    /** Destructor.*/
    virtual ~VXDSpacePointMakerModule();

    /**
     * Initialize the Module.
     *
     * This method is called at the beginning of data processing.
     * Initialize geometry etc.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     *
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     *
     * Convert PXD/SVDSimHits of the event to VXDSpacePoints.
     */
    virtual void event();

    /**
     * End-of-run action.
     *
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     *
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  protected:

    /** Print module parameters.*/
    void printModuleParams() const;

//    /** Print run statistics.*/
//    void printRunStats() const;

  private:

    std::string m_mcColName;          /**< Collecton of MCParticles. */
    std::string m_pxdColName;         /**< Input collection of PXDSimHits. */
    std::string m_pxdRelName;         /**< Input collection of MCPart-PXDSimHit relations. */
    std::string m_svdColName;         /**< Input collection of SVDSimHits. */
    std::string m_svdRelName;         /**< Input collection of MCPart-SVDSimHit relations. */
    std::string m_outColName;        /**< Output collection name */
    std::string m_outRelName;       /**< Output relations MCParticles to VXDSpacePoints.*/

    int m_nPointsSaved;         /**< Number of saved points per event. */
    int m_nRelationsSaved;      /**< Number of saved relations per event. */
    TwoSidedRelationSet relMCPXD; /**< A structure to handle MCPart->PXDSimHit relations.*/
    TwoSidedRelationSet relMCSVD; /**< A structure to handle MCPart->SVDSimHit relations.*/

    /* State variables. */
    // Current sensor parameters from geometry
    short int m_currentLayerID;       /**< Current layer ID. */
    short int m_currentLadderID;      /**< Current ladder ID. */
    short int m_currentSensorID;      /**< Current sensor ID. */
    short int m_currentSensorUniID;     /**< Current compact ID (layer/ladder/sensor). */

    /* Other members.*/
    SiGeoCache* m_geometry;           /**< Pointer to SiGeoCache instance. */

    double m_timeCPU;                /**< CPU time.     */
    int    m_nRun;                   /**< Run number.   */
    int    m_nEvent;                 /**< Event number. */
  };

} // Belle2 namespace

#endif // VXDSPACEPOINTMAKERMODULE_H
