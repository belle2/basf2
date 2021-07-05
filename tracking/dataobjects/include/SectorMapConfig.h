/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once

#include <framework/geometry/B2Vector3.h>

#include <string>
#include <vector>
#include <utility> // std::pair


namespace Belle2 {

  /** simple struct containing all the configuration data needed for the SecMapTrainer. */
  struct SectorMapConfig: public TObject {

  public:

    SectorMapConfig() {}



    /** stores pTCuts for min pT allowed for this . */
    double pTmin = 0.02;
    /** stores pTCuts for min (.first) and max (.second) ptCut. */
    double pTmax = 3.5;

    /** allows smearing of the cuts. Values greater 0 stretch the cuts,
    values smaller 0 narrow them down. */
    double pTSmear = 0.;

    /** stores allowed layers to be used (including virtual IP with layer 0). */
    std::vector<int> allowedLayers = {0, 3, 4, 5, 6};

    /** Defines the sectors boundaries in normalized u coordinates (i.e. in (0,1) ).
    The entries in this vectors are the upper limits of the sector */
    std::vector<double> uSectorDivider = {.15, .5, .85, 1.};

    /** Defines the sectors boundaries in normalized v coordinates (i.e. in (0,1) ).
    The entries in this vectors are the upper limits of the sector */
    std::vector<double> vSectorDivider = {.1, .3, .5, .7, .9, 1.};

    /** Stores all the pdgCodes which are allowed to be used by the SecMap.
    If empty all types are allowed. */
    std::vector<int> pdgCodesAllowed;

    /** Stores a cut for maximum distance of the seed in xy of the given virtual IP.
    WARNING only working if MC-TCs are used,
    VXDTF-TCs use innermost hit as seed != mctc-seed! */
    double seedMaxDist2IPXY = 23.5;

    /** Stores a cut for maximum distance of the seed in z of the given virtual IP.
    WARNING only working if MC-TCs are used,
    VXDTF-TCs use innermost hit as seed != mctc-seed!  */
    double seedMaxDist2IPZ = 23.5;

    /** Stores the minimal number of hits a TC must have to be accepted as TC
    (vIP-Hits are ignored). */
    unsigned nHitsMin = 3;

    /** Stores the position of the assumed position of the interaction point -
    The virtual IP. */
    B2Vector3D vIP = B2Vector3D(0, 0, 0);

    /** Sets the human readable proto-name of the sectorMap. */
    std::string secMapName = "testMap";

    /** Magnetic field value to be set for the filters. */
    double mField = 1.5;

    /** defined 1 == 100%, if relative frequency of sec-combi to the outer-sector is less than threshold, sector-combi will be deleted. */
    double rarenessThreshold = 0.001;

    /** the quantiles to be chosen in the end for determining the cuts first is quantile, second is 1-quantile. */
    std::pair<double, double> quantiles = {0.005, 1. - 0.005};

    //! needed for the root library
    ClassDef(SectorMapConfig, 2);

  };
}

