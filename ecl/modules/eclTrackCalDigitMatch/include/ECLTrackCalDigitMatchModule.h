/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <ecl/geometry/ECLGeometryPar.h>

#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <analysis/dataobjects/ECLEnergyCloseToTrack.h>

namespace Belle2 {

  /**
   * Module to find the closest ECLCalDigits to an extrapolated track
   */
  class ECLTrackCalDigitMatchModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ECLTrackCalDigitMatchModule();

    /** initialize */
    virtual void initialize();

    /** event */
    virtual void event();

    /** Check if extrapolated hit is inside ECL. */
    bool isECLHit(const ExtHit& extHit) const;

  private:

    // module parameter(s)
    double m_extRadius {130.0}; /**< radius to which track is extrapolated (using track parameters, not GEANT) */
    double m_angleFWDGap {31.80}; /**< FWD gap angle [deg] */
    double m_angleBWDGap {129.70}; /**< BWD gap angle [deg]*/
    int m_trackHypothesis {11}; /**< Track hypothesis to be used (will use closest one) */

    // other
    double m_extZFWD {0.}; /**< z position (FWD) to which track is extrapolated */
    double m_extZBWD {0.}; /**< z position (BWD) to which track is extrapolated */

    std::vector < int > m_eclCalDigitsArray; /**< eclCalDigits in array */

    std::map <int , std::vector<int> > m_FWD3Barrel; /**< map between phi id and FWD3 cell ids on the barrel side */
    std::map <int , std::vector<int> > m_FWD3Endcap; /**< map between phi id and FWD3 cell ids on the endcap side */
    std::map <int , std::vector<int> > m_BWD3Barrel; /**< map between phi id and BWD3 cell ids on the barrel side */
    std::map <int , std::vector<int> > m_BWD3Endcap; /**< map between phi id and BWD3 cell ids on the endcap side */

    ECL::ECLGeometryPar* geom; /**<ECL geometry */

    StoreArray<ECLCalDigit> m_eclCalDigits; /**< Required input array of ECLCalDigits  */
    StoreArray<Track> m_tracks; /**< Required input array of Tracks */

    StoreArray<ECLEnergyCloseToTrack> m_anaEnergyCloseToTrack; /**< Analysis level information attached to a track*/
  };
}

