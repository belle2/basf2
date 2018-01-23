/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <simulation/dataobjects/SimHitBase.h>
#include <simulation/background/BeamBGTypes.h>
#include <background/dataobjects/BackgroundMetaData.h>
#include <string>
#include <map>

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
      TClonesArray* BeamBackHits; /**< BeamBackHits from collision file */

      /**
       * default constructor
       */
      BkgHits():
        PXD(0), SVD(0), CDC(0), TOP(0), ARICH(0), ECL(0), BKLM(0), EKLM(0),
        BeamBackHits(0)
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
      unsigned index;          /**< index of this element in the std::vector */

      /**
       * default constructor
       */
      BkgFiles(): tag(SimHitBase::bg_none), realTime(0.0), scaleFactor(1.0),
        fileType(BackgroundMetaData::c_Usual),
        tree(0), numFiles(0), numEvents(0), eventCount(0), rate(0.0), index(0)
      {}
      /**
       * usefull constructor
       * @param bkgTag background tag
       * @param bkgType background type
       * @param fileName file name
       * @param time real time that corresponds to background sample
       * @param scaleFac scale factor for the rate
       * @param fileTyp file type
       * @param indx index of this element in the std::vector
       */
      BkgFiles(SimHitBase::BG_TAG bkgTag,
               const std::string& bkgType,
               const std::string& fileName,
               double time,
               double scaleFac,
               BackgroundMetaData::EFileType fileTyp = BackgroundMetaData::c_Usual,
               unsigned indx = 0):
        tag(bkgTag), type(bkgType), realTime(time), scaleFactor(scaleFac),
        fileType(fileTyp),
        tree(0), numFiles(0), numEvents(0), eventCount(0), rate(0.0), index(indx)
      {
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
                    double maxTime)
    {
      if (!cloneArray) return;
      if (!simHits.isValid()) return;

      int numEntries = cloneArray->GetEntriesFast();
      for (int i = 0; i < numEntries; i++) {
        SIMHIT* bkgSimHit = static_cast<SIMHIT*>(cloneArray->AddrAt(i));
        SIMHIT* simHit = simHits.appendNew(*bkgSimHit);
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
     * functions that add BeamBackHits to those in the DataStore
     * @param hits a reference to DataStore BeamBackHits
     * @param cloneArray a pointer to BeamBackHits read from a file
     * @param timeShift time shift to be applied to BeamBackHits
     * @param minTime time window left edge
     * @param maxTime time window right edge
     */
    template<class HIT>
    void addBeamBackHits(StoreArray<HIT>& hits, TClonesArray* cloneArray,
                         double timeShift, double minTime, double maxTime)
    {
      //Match SubDet id from BeamBackHits to whether we keep it or not
      bool keep[] = {false, m_PXD, m_SVD, m_CDC, m_ARICH, m_TOP, m_ECL, m_EKLM, m_BKLM};
      if (!cloneArray) return;
      if (!hits.isValid()) return;
      // this is basically a copy of addSimHits but we only add the
      // BeamBackHits from the specified sub detectors so we have to check
      // each if it is from one of the enabled subdetectors
      int numEntries = cloneArray->GetEntriesFast();
      for (int i = 0; i < numEntries; i++) {
        HIT* bkgHit =  static_cast<HIT*>(cloneArray->AddrAt(i));
        //Only keep selected
        if (!keep[bkgHit->getSubDet()]) continue;
        HIT* hit = hits.appendNew(*bkgHit);
        hit->shiftInTime(timeShift);
        //TODO: BeamBackHits does not have a setBackgroundTag so we do not
        //check or set it
        if (m_wrapAround) {
          double time = hit->getTime();
          if (time > maxTime) {
            double windowSize = maxTime - minTime;
            double shift = int((time - minTime) / windowSize) * windowSize;
            hit->shiftInTime(-shift);
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

    /**
     * Checks for deposited energy of ECLHits and returns true if Edep < m_maxEdepECL
     * @param cloneArrayECL pointer to TClonesArray of ECLHits
     * @return true if all ECLHits have deposited energy below m_maxEdepECL
     */
    bool acceptEvent(TClonesArray* cloneArrayECL);


    std::vector<std::string> m_backgroundFiles; /**< names of beam background files */
    double m_overallScaleFactor; /**< overall scale factor */
    std::vector<std::tuple<std::string, double> > m_scaleFactors; /**< scale factors */
    double m_minTime;  /**< minimal time shift of background event */
    double m_maxTime;  /**< maximal time shift of background event */
    std::vector<std::string> m_components; /**< detector components */
    bool m_wrapAround; /**< if true wrap around events in the tail after maxTime */
    double m_minTimeECL;  /**< minimal time shift of background event for ECL */
    double m_maxTimeECL;  /**< maximal time shift of background event for ECL */
    double m_minTimePXD;  /**< minimal time shift of background event for PXD */
    double m_maxTimePXD;  /**< maximal time shift of background event for PXD */
    double m_maxEdepECL;  /**< maximal allowed deposited energy in ECL */
    int m_cacheSize;  /**< file cache size in Mbytes */

    std::vector<BkgFiles> m_backgrounds;  /**< container for background samples */
    BkgHits m_simHits;         /**< input event buffer */

    bool m_PXD = false; /**< true if found in m_components */
    bool m_SVD = false; /**< true if found in m_components */
    bool m_CDC = false; /**< true if found in m_components */
    bool m_TOP = false; /**< true if found in m_components */
    bool m_ARICH = false; /**< true if found in m_components */
    bool m_ECL = false; /**< true if found in m_components */
    bool m_BKLM = false; /**< true if found in m_components */
    bool m_EKLM = false; /**< true if found in m_components */
    bool m_BeamBackHits = false; /**<  if true add also background hits */

    background::BeamBGTypes m_bgTypes;  /**< defined BG types */

    std::map<std::string, int> m_rejected; /**< messages: rejected events */
    std::map<std::string, int> m_reused;  /**< messages: rejused events */
    int m_rejectedCount = 0;  /**< counter for suppresing "rejected event" messages */

  };

} // Belle2 namespace

