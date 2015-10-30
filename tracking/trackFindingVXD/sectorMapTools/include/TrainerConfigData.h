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
  struct TrainerConfigData {
    // TODO:
    // JAKOB will change the name of this class into SectorMapConfig which is
    // more meaningful, and will put it among the dataobjects since it will be
    // stored together with the cut values

    /** stores pTCuts for min (.first) and max (.second) ptCut. */
    // Eugenio: Why not use pTmin and pTmax instead of pTCuts.first and pTCuts.second which is awkward?
    std::pair<double, double> pTCuts = {0.02, 3.5};

    /** allows smearing of the cuts. Values greater 0 stretch the cuts, values smaller 0 narrow them down. */
    double pTSmear = 0.;

    /** stores allowed layers to be used, with min (.first) and max (.second) layer to be named. Numbers below 0 assume no cut on that side. */
    std::pair<int, int> minMaxLayer = {3, 6};

    /** stores the cuts in u-direction in local relative coordinates of a sensor (0-1), to be used for determining the sectorID.
     * First one has to be 0, last one has to be 1. */
    // TODO: These are not Cuts, perhaps upperValues Low values?
    // Can we get rid og 0 and 1 since they are not conveing any information?
    std::vector<double> uDirectionCuts = {0., .15, .5, .85, 1.};


    /** stores the cuts in v-direction in local relative coordinates of a sensor (0-1), to be used for determining the sectorID.
     * First one has to be 0, last one has to be 1. */
    // TODO: These are not Cuts, perhaps upperValues Low values?
    // Can we get rid og 0 and 1 since they are not conveing any information?
    std::vector<double> vDirectionCuts = {0., .1, .3, .5, .7, .9, 1.};

    /** stores all the pdgCodes which are allowed to be used by the SecMap. if empty all types are allowed. */
    std::vector<int> pdgCodesAllowed;

    /** stores a cut for maximum distance of the seed in xy of the given virtual IP. WARNING only working if MC-TCs are used, VXDTF-TCs use innermost hit as seed != mctc-seed! */
    double seedMaxDist2IPXY = 23.5;

    /** stores a cut for maximum distance of the seed in z of the given virtual IP. WARNING only working if MC-TCs are used, VXDTF-TCs use innermost hit as seed != mctc-seed!  */
    double seedMaxDist2IPZ = 23.5;

    /** stores the minimal number of hits a TC must have to be accepted as TC (vIP-Hits are ignored). */
    unsigned nHitsMin = 3;

    /** stores the position of the assumed position of the interaction point - the virtual IP. */
    B2Vector3D vIP = B2Vector3D(0, 0, 0);

//     /** Hit specific: determines the range for min (.first) and max (.second) distance to the IP in 3D to. */
//     std::pair<double, double> hitMinMaxDist2IP3D = { -1, -1};

    /** Sets the human readable proto-name of the sectorMap. */
    std::string secMapName = "testMap";

    /** Names of Two-hitFilters to be used in that Map. */
    std::vector<std::string> twoHitFilters = { "distance3D", "distanceXY", "slopeRZ"};

    /** Names of Three-hitFilters to be used in that Map. */
//  std::vector<std::string> threeHitFilters = { "angles3D", "deltaSlopeRZ", "distance2IP"};
    std::vector<std::string> threeHitFilters = { "angles3D", "deltaSlopeRZ"};

    /** Names of Four-hitFilters to be used in that Map. */
//  std::vector<std::string> fourHitFilters = {"deltaDistance2IP"};
    std::vector<std::string> fourHitFilters = {};

    /** Magnetic field value to be set for the filters. */
    double mField = 1.5;

    /** defined 1 == 100%, if relative frequency of sec-combi to the outer-sector is less than threshold, sector-combi will be deleted. */
    double rarenessThreshold = 0.001;

    /** the quantiles to be chosen in the end for determining the cuts first is quantile, second is 1-quantile. */
    std::pair<double, double> quantiles = {0.005, 0.005};
  };
}

