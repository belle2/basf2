/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
