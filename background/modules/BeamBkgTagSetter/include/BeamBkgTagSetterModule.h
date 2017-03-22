/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BEAMBKGTAGSETTERMODULE_H
#define BEAMBKGTAGSETTERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <simulation/dataobjects/SimHitBase.h>
#include <simulation/background/BeamBGTypes.h>
#include <background/dataobjects/BackgroundMetaData.h>
#include <string>

namespace Belle2 {

  /**
   * A module that sets m_backgroundTag variable in SimHits (see SimHitBase.h).
   * @return true if at least one SimHit store array has entries.
   */
  class BeamBkgTagSetterModule : public Module {

  public:

    /**
     * Constructor
     */
    BeamBkgTagSetterModule();

    /**
     * Destructor
     */
    virtual ~BeamBkgTagSetterModule();

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

  private:

    std::string m_backgroundType; /**< BG type */
    double m_realTime; /**< real time that corresponds to beam background sample */
    int m_phase; /**< Phase that corresponds to beam background sample */
    std::string m_specialFor; /**< ordinary or special file for ECL, PXD */

    background::BeamBGTypes m_bgTypes;  /**< defined BG types */
    SimHitBase::BG_TAG m_backgroundTag; /**< background tag to set (from BG type) */
    BackgroundMetaData::EFileType m_fileType; /**< file type to set */

    /**
     * functions that set background tag in SimHits
     * @param simHits a reference to DataStore SimHits
     * @return number of tagged SimHit entries
     */
    template<class SIMHIT>
    int setBackgroundTag(StoreArray<SIMHIT>& simHits)
    {
      if (!simHits.isValid()) return 0;

      int numEntries = simHits.getEntries();
      for (int i = 0; i < numEntries; i++) {
        SIMHIT* simHit = simHits[i];
        simHit->setBackgroundTag(m_backgroundTag);
      }
      return numEntries;
    }


  };

} // Belle2 namespace

#endif
