/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHDIGIMODULE_H
#define ARICHDIGIMODULE_H

#include <framework/core/Module.h>

#include <string>

#include <TRandom3.h>

namespace Belle2 {

  /**
   * ARICH digitizer module.
   *
   * Takes the hits form G4 simulation (ARICHSimHit), applies q.e. of HAPDs,
   * calculates and saves hit channel numbers (ARICHHit).
   * If channel has multiple hits, only one is saved.
   */

  class ARICHDigiModule : public Module {

  public:


    /** Constructor.*/
    ARICHDigiModule();

    /** Destructor.*/
    virtual ~ARICHDigiModule();

    /**
     * Initialize the Module.
     *
     * This method is called at the beginning of data processing.
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
     * Convert ARICHSimHits of the event to ARICHHits.
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

    void printModuleParams() const;

  private:

    std::string m_inColName;         /**< Input collection name */
    std::string m_outColName;        /**< Output collection name */

    /* Other members.*/
    TRandom3* m_random;              /** Random number generator.*/
    double m_timeCPU;                /**< CPU time.     */
    int    m_nRun;                   /**< Run number.   */
    int    m_nEvent;                 /**< Event number. */

    //! Returns q.e. of detector at given photon energy
    double QESuperBialkali(double energy);

    //! Apply q.e., returns 1 if photon is detected and 0 if not.
    int DetectorQE(double energy);

  };

} // Belle2 namespace

#endif // ARICHDIGIMODULE_H
