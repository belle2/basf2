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
#include <simulation/background/BeamBGTypes.h>
#include <background/dataobjects/BackgroundMetaData.h>
#include <string>

#include "TFile.h"
#include "TChain.h"
#include "TClonesArray.h"


namespace Belle2 {

  /**
   * New beam background mixer; this one doesn't need ROF files
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

  private:

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

    /**
     * structure to hold samples of a particular background type
     */
    struct BkgFiles {
      SimHitBase::BG_TAG tag;  /**< background tag */
      std::string type;        /**< background type */
      double realTime;         /**< real time of BG samlpe */
      double scaleFactor;      /**< scale factor for the rate */
      std::vector<std::string> fileNames;     /**< file names */
      BackgroundMetaData::EFileType fileType; /**< file type */
      TChain* tree;            /**< tree pointer */
      unsigned numFiles;       /**< number of files connected to TChain */
      unsigned numEvents;      /**< number of events (tree entries) in the sample */
      unsigned eventCount;     /**< current event (tree entry) */
      double rate;             /**< background rate of the sample */
      BkgHits simHits;         /**< input event buffer */

      /**
       * default constructor
       */
      BkgFiles(): tag(SimHitBase::bg_none), realTime(0.0), scaleFactor(1.0),
        fileType(BackgroundMetaData::c_Usual),
        tree(0), numFiles(0), numEvents(0), eventCount(0), rate(0.0)
      {}
      /**
       * constructor with tag, type, file name, and real time
       * @param bkgTag background tag
       * @param bkgType background type
       * @param fileName file name
       * @param realTime real time that corresponds to background sample
       * @param fileTyp file type
       */
      BkgFiles(SimHitBase::BG_TAG bkgTag,
               const std::string& bkgType,
               const std::string& fileName,
               double time,
               BackgroundMetaData::EFileType fileTyp = BackgroundMetaData::c_Usual):
        tag(bkgTag), type(bkgType), realTime(time), scaleFactor(1.0),
        fileType(fileTyp),
        tree(0), numFiles(0), numEvents(0), eventCount(0), rate(0.0) {
        fileNames.push_back(fileName);
      }
    };


    /**
     * functions that add background SimHits to those in the DataStore
     * @param simHits a reference to DataStore SimHits
     * @param cloneArray a pointer to background SimHits read from a file
     * @param timeShift time shift to be applied to background SimHits
     * @param minTime time window left edge
     * @param maxTime time window right edge
     */
    template<class SIMHIT>
    void addSimHits(StoreArray<SIMHIT>& simHits,
                    TClonesArray* cloneArray,
                    double timeShift,
                    double minTime,
                    double maxTime) {
      if (!cloneArray) return;
      if (!simHits.isValid()) return;

      int numEntries = cloneArray->GetEntriesFast();
      for (int i = 0; i < numEntries; i++) {
        SIMHIT* bkgSimHit = static_cast<SIMHIT*>(cloneArray->AddrAt(i));
        SIMHIT* simHit = simHits.appendNew();
        (*simHit) = (*bkgSimHit);
        simHit->shiftInTime(timeShift);
        if (simHit->getBackgroundTag() == 0) // should be properly set at bkg simulation
          simHit->setBackgroundTag(SimHitBase::bg_other);
        if (m_wrapAround) {
          double time = simHit->getGlobalTime();
          if (time > maxTime) {
            double windowSize = maxTime - minTime;
            double shift = int((time - minTime) / windowSize) * windowSize;
            simHit->shiftInTime(-shift);
          }
        }
      }

    }


    /**
     * Returns true if a component is found in components or the list is empty.
     * If found a component is erased from components.
     * @param components list of components to be included
     * @param component the name of detector component to check
     * @return true if component found in components
     */
    bool isComponentIncluded(std::vector<std::string>& components,
                             const std::string& component);

    /**
     * appends background sample to m_backgrounds
     * @param bkgTag background tag
     * @param bkgType background type
     * @param fileName file name
     * @param realTime real time that corresponds to background sample
     * @param fileTyp file type
     */
    void appendSample(SimHitBase::BG_TAG bkgTag,
                      const std::string& bkgType,
                      const std::string& fileName,
                      double realTime,
                      BackgroundMetaData::EFileType fileTyp);


    std::vector<std::string> m_backgroundFiles; /**< names of beam background files */
    std::vector<std::tuple<std::string, double> > m_scaleFactors; /**< scale factors */
    double m_minTime;  /**< minimal time shift of background event */
    double m_maxTime;  /**< maximal time shift of background event */
    std::vector<std::string> m_components; /**< detector components */
    bool m_wrapAround; /**< if true wrap around events in the tail after maxTime */
    double m_minTimeECL;  /**< minimal time shift of background event for ECL */
    double m_maxTimeECL;  /**< maximal time shift of background event for ECL */

    std::vector<BkgFiles> m_backgrounds;  /**< container for background samples */

    bool m_PXD; /**< true if found in m_components */
    bool m_SVD; /**< true if found in m_components */
    bool m_CDC; /**< true if found in m_components */
    bool m_TOP; /**< true if found in m_components */
    bool m_ARICH; /**< true if found in m_components */
    bool m_ECL; /**< true if found in m_components */
    bool m_BKLM; /**< true if found in m_components */
    bool m_EKLM; /**< true if found in m_components */

    background::BeamBGTypes m_bgTypes;  /**< defined BG types */

  };

} // Belle2 namespace

#endif
