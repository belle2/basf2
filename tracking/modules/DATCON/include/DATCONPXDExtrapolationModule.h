/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/dataobjects/DATCONTrack.h>
#include <tracking/dataobjects/DATCONMostProbableHit.h>

#include <tracking/dataobjects/PXDIntercept.h>

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <root/TVector2.h>

#include <vxd/dataobjects/VxdID.h>


namespace Belle2 {

  /** The DATCONPXDExtrapolationModule does an extrapolation to the
    * PXD and creates "Most Probable Hits" (MPH) on the PXD.
    * The MPH are very similar to the PXDIntercept, but contain
    * more information needed for the ROI calculation by DATCON
    * lateron. However, for framework compatibility, the MPH are
    * also stored as PXDIntercepts so that the framework functions
    * for the analysis of the PXDIntercept can be used for performance
    * evaluation.
    */
  class DATCONPXDExtrapolationModule : public Module {

  public:
    /** Constructor.  */
    DATCONPXDExtrapolationModule();

    /** Standard Deconstructor */
    virtual ~DATCONPXDExtrapolationModule() = default;

    /** Initialize the module and check module parameters */
    virtual void initialize();
    /** Run tracking */
    virtual void event();

  protected:

    /** Members holding module parameters: */

    /** 1. Collections */
    /** Name of the collection for the SVD Hough tracks */
    std::string m_storeDATCONTracksName;
    /** Name of the PXDIntercepts StoreArray */
    std::string m_storeDATCONPXDInterceptsName;
    /** Name of the DATCONMPH StoreArray */
    std::string m_storeDATCONMPHName;

    /** StoreArray of the tracks found by DATCON */
    StoreArray<DATCONTrack>           storeDATCONTracks;
    /** StoreArray of the PXDIntercepts calculated by DATCON */
    StoreArray<PXDIntercept>          storeDATCONPXDIntercepts;
    /** StoreArray for the DATCON-specific "Most Probable Hits" */
    StoreArray<DATCONMostProbableHit> storeDATCONMPHs;

  };//end class declaration
} // end namespace Belle2
