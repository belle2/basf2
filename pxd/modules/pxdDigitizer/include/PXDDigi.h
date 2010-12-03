/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka, Zbynek Drasal             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDDIGI_H
#define PXDDIGI_H

#include <framework/core/Module.h>

#include <string>

#include "TRandom3.h"
#include <pxd/hitpxd/CIDManager.h>
#include <pxd/modules/pxdDigitizer/PXDcheater.h>

namespace Belle2 {

  /*
   ** Gaussian-smearing PXD digitizer.
   *
   * Now, I only use pre-set simulation-based resolutions provided by the
   * PXDcheater class to randomize hit positions. Error matrices of hit positions
   * are calculated from the same resolutions.
   * The deposited energy is not smeared and no error is given for it.
   * Also, there is now no combination of near hits.
   * A full digitization will be added later.
   */

  class PXDDigi : public Module {

  public:

    /** Constructor.*/
    PXDDigi();

    /** Destructor.*/
    virtual ~PXDDigi();

    /*
     ** Initialize the Module.
     *
     * This method is called at the beginning of data processing.
     * Initialize the cheater, collect geometry information etc.
     */
    virtual void initialize();

    /*
     ** Called when entering a new run.
     *
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /*
     ** Event processor.
     *
     * Convert PXDSimHits of the event to PXDHits.
     */
    virtual void event();

    /*
     ** End-of-run action.
     *
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /*
     ** Termination action.
     *
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  protected:

    /** Print module parameters.*/
    void printModuleParams() const;

    /** Print run statistics.*/
//    void printRunStats() const;

  private:

    /* Steerables. */
    std::string m_inColName;         /**< Input collection name */
    std::string m_outColName;        /**< Output collection name */
//    std::string m_relColNameMC2Digi; /**< Relation collection name (MC hits to Digi hits)*/

    /* Other members.*/
    PXDcheater* m_cheater;  /**< Cheater provides resolutions and pitch data.*/
    CIDManager* m_cid;      /**< Convert between compact ID and layer/ladder/sensor IDs.*/
    TRandom3* m_random;     /** Random number generator.*/

    double m_timeCPU;                /**< CPU time     */
    int    m_nRun;                   /**< Run number   */
    int    m_nEvent;                 /**< Event number */
  };

} // Belle2 namespace

#endif // PXDDigi
