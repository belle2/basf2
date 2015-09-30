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

#include <tracking/vectorTools/B2Vector3.h>

#include <string>
#include <vector>
#include <utility> // std::pair


namespace Belle2 {

  /** simple struct containing all the configuration data needed for the SecMapTrainer. */
  struct TrainerConfigData {
    /** stores pTCuts for min (.first) and max (.second) ptCut. */
    std::pair<double, double> pTCuts;

    /** allows smearing of the cuts. Values greater 0 stretch the cuts, values smaller 0 narrow them down. */
    double pTSmear;

    /** stores allowed layers to be used, with min (.first) and max (.second) layer to be named. Numbers below 0 assume no cut on that side. */
    std::pair<int, int> minMaxLayer;

    /** stores the cuts in u-direction in local relative coordinates of a sensor (0-1), to be used for determining the sectorID.
     * First one has to be 0, last one has to be 1. */
    std::vector<double> uDirectionCuts;

    /** stores the cuts in v-direction in local relative coordinates of a sensor (0-1), to be used for determining the sectorID.
     * First one has to be 0, last one has to be 1. */
    std::vector<double> vDirectionCuts;

    /** stores all the pdgCodes which are allowed to be used by the SecMap. if empty all types are allowed. */
    std::vector<int> pdgCodesAllowed;

    /** stores a cut for maximum distance of the seed in xy of the given virtual IP. */
    double seedMaxDist2IPXY;

    /** stores a cut for maximum distance of the seed in z of the given virtual IP. */
    double seedMaxDist2IPZ;

    /** stores the minimal number of hits a TC must have to be accepted as TC (vIP-Hits are ignored). */
    int nHitsMin;

    /** stores the position of the assumed position of the interaction point - the virtual IP. */
    B2Vector3D vIP;

    /** Hit specific: determines the range for min (.first) and max (.second) distance to the IP in 3D to. */
    std::pair<double, double> hitMinMaxDist2IP3D;

    /** Sets the human readable proto-name of the sectorMap. */
    std::string secMapName;

    /** Names of Two-hitFilters to be used in that Map. */
    std::vector<std::string> twoHitFilters;

    /** Names of Three-hitFilters to be used in that Map. */
    std::vector<std::string> threeHitFilters;

    /** Names of Four-hitFilters to be used in that Map. */
    std::vector<std::string> fourHitFilters;

    /** Magnetic field value to be set for the filters. */
    double mField;

  };
}

