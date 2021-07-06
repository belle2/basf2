/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>

#include "tracking/vxdCaTracking/ExporterHitInfo.h"
#include "tracking/vxdCaTracking/ExporterTcInfo.h"


namespace Belle2 {

  /**
   * Bundles information for a single event to be stored by NonRootDataExportModule.
   */
  class ExporterEventInfo {
  public:
    /** Empty constructor. */
    explicit ExporterEventInfo(int evtNum):
      m_eventNumber(evtNum) {}


    /** Destructor. */
    ~ExporterEventInfo() {}


    /** adds Hit to internal storage.  */
    void addHit(ExporterHitInfo aHit);


    /** adds Tc to internal storage.  */
    void addTc(ExporterTcInfo aTc);


    /** returns a pointer to internal vector of hits */
    std::vector<ExporterHitInfo>* getHits();


    /** returns number of current event */
    int getEventNumber() { return m_eventNumber; }


    /** returns a pointer to internal vector of tcs */
    std::vector<ExporterTcInfo>* getTcs();


    /** returns number of stored pxdTruehits */
    int getNPXDTrueHits();


    /** returns number of stored svdTruehits */
    int getNSVDTrueHits();


    /** returns total number of stored mcHits stored in all Tcs of this event */
    int getNMCHits();


  protected:
    /** used by getXXXHits to determine number of hits of certain type found in current event */
    int countHitsOfType(int type);

    int m_eventNumber; /**< knows number of current event */
    std::vector<ExporterHitInfo> m_hits; /**< stores all hits of event */
    std::vector<ExporterTcInfo> m_tcs; /**< stores all tcs of event */
  };
}
