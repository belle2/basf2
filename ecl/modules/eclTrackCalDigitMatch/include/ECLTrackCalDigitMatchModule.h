/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Basf2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  class Track;
  class ECLCalDigit;
  class ECLEnergyCloseToTrack;

  namespace ECL {
    class ECLGeometryPar;
  }
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
    virtual void initialize() override;

    /** event */
    virtual void event() override;

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

    std::map <int, std::vector<int> > m_FWD3Barrel;  /**< map between phi id and FWD3 cell ids on the barrel side */
    std::map <int, std::vector<int> > m_FWD3Endcap;  /**< map between phi id and FWD3 cell ids on the endcap side */
    std::map <int, std::vector<int> > m_BWD3Barrel;  /**< map between phi id and BWD3 cell ids on the barrel side */
    std::map <int, std::vector<int> > m_BWD3Endcap;  /**< map between phi id and BWD3 cell ids on the endcap side */

    ECL::ECLGeometryPar* geom{nullptr}; /**<ECL geometry */

    StoreArray<ECLCalDigit> m_eclCalDigits; /**< Required input array of ECLCalDigits  */
    StoreArray<Track> m_tracks; /**< Required input array of Tracks */

    StoreArray<ECLEnergyCloseToTrack> m_anaEnergyCloseToTrack; /**< Analysis level information attached to a track*/
  };
}
