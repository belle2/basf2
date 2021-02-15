/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Qingyuan Liu                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <pxd/utilities/PXDPerformanceStructs.h>
#include <pxd/utilities/PXD2TrackBase.h>


namespace Belle2 {

  using namespace Belle2::PXD;


  /** Class PXD2TrackEvent: Event data container for performance and calibration studies.
   * This class holds all required variables for TTree creation and
   * the analysis, such that no relation lookup is needed.
   *
   * Caveat: this class is not placed in the dataobjects folder
   * to avoid dependency cycles.
   */
  class PXD2TrackEvent : public PXD2TrackBase<Track_t >  {

    ClassDef(PXD2TrackEvent, 1)
  };
} // end namespace Belle2
