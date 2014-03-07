/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BEAMBKGMIXERMODULE_H
#define BEAMBKGMIXERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <simulation/dataobjects/SimHitBase.h>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"


namespace Belle2 {

  /**
   * An alternative to the existing beam background mixer; this one doesn't need ROF files
   */
  class BeamBkgMixerModule : public Module {

  public:

    /**
     * Constructor
     */
    BeamBkgMixerModule();

    /**
     * Destructor
     */
    virtual ~BeamBkgMixerModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

    /**
     * Prints module parameters.
     */
    void printModuleParams() const;

  private:

    std::vector<std::string> m_backgroundFiles; /**< names of beam background files */
    std::vector<double> m_scaleFactors; /**< rate scale factors */
    double m_realTime; /**< real time that corresponds to beam background simulation */
    double m_minTime;  /**< minimal time shift of background event */
    double m_maxTime;  /**< maximal time shift of background event */

    /**
     * An input event buffer definition for background SimHits
     */
    struct BkgHits {
      TClonesArray* PXD; /**< PXD SimHits from collision file */
      TClonesArray* SVD; /**< SVD SimHits from collision file */
      TClonesArray* CDC; /**< CDC SimHits from collision file */
      TClonesArray* TOP; /**< TOP SimHits from collision file */
      TClonesArray* ARICH; /**< ARICH SimHits from collision file */
      TClonesArray* ECL; /**< ECL SimHits from collision file */
      TClonesArray* BKLM; /**< BKLM SimHits from collision file */
      TClonesArray* EKLM; /**< EKLM SimHits from collision file */

      /**
       * default constructor
       */
      BkgHits():
        PXD(0), SVD(0), CDC(0), TOP(0), ARICH(0), ECL(0), BKLM(0), EKLM(0)
      {}
    };

    std::vector<TFile*> m_files;  /**< vector of file pointers */
    std::vector<TTree*> m_trees;  /**< vector of tree pointers */
    std::vector<unsigned> m_numEvents;  /**< number of events in each file */
    std::vector<unsigned> m_eventCount; /**< current event number in each file */
    std::vector<double> m_bkgRates;  /**< background rate of each file */
    std::vector<BkgHits> m_bkgSimHits;  /**< input event buffer */


    /**
     * functions that add background SimHits to those in the DataStore
     * @param simHits a reference to DataStore SimHits
     * @param cloneArray a pointer to background SimHits read from a file
     * @param timeShift time shift to be applied to background SimHits
     */
    template<class SIMHIT>
    void addSimHits(StoreArray<SIMHIT>& simHits,
                    TClonesArray* cloneArray,
                    double timeShift) {
      if (!cloneArray) return;
      if (!simHits.isValid()) return;

      int numEntries = cloneArray->GetEntriesFast();
      for (int i = 0; i < numEntries; i++) {
        SIMHIT* bkgSimHit = (SIMHIT*)cloneArray->AddrAt(i);
        SIMHIT* simHit = simHits.appendNew();
        (*simHit) = (*bkgSimHit);
        simHit->shiftInTime(timeShift);
        if (simHit->getBackgroundTag() == 0) // should be properly set at bkg simulation
          simHit->setBackgroundTag(SimHitBase::bg_other);
      }

    }


  };

} // Belle2 namespace

#endif
