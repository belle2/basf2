/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDRECOHITMAKERMODULE_H
#define SVDRECOHITMAKERMODULE_H

#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * SVDRecoHit maker module.
   *
   * This simple module takes a DataStore array of SVDHits as input and converts
   * it to an array of SVDRecoHits - enriched hits containing also detector plane
   * information.
   */

  class SVDRecoHitMakerModule : public Module {

  public:

    /** Constructor.*/
    SVDRecoHitMakerModule();

    /** Destructor.*/
    virtual ~SVDRecoHitMakerModule();

    /**
     * Initialize the Module.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     *
     * Refresh SiGeoCache.
     */
    virtual void beginRun();

    /**
     * Event processor.
     *
     * Convert SVDHits of the event to SVDRecoHits.
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


  private:

    std::string m_mcColName;         /**< Name of MCParticles collection. */
    std::string m_inColName;         /**< SVDHits collection name */
    std::string m_outColName;        /**< SVDRecoHits collection name */
    std::string m_relHitName;        /**< Relation collection name (MC hits to SVD Hits)*/
    std::string m_relRecName;        /**< Relation collection name (MC hits to Reco hits)*/

    double m_timeCPU;                /**< CPU time.     */
    int    m_nRun;                   /**< Run number.   */
    int    m_nEvent;                 /**< Event number. */
  };

} // Belle2 namespace

#endif // SVDDIGIMODULE_H
