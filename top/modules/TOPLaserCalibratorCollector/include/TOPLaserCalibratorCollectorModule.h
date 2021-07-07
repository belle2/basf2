/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//Calibration
#include <calibration/CalibrationCollectorModule.h>

//ECL
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  class TOPDigit;

  /**
      Collector module for the TOP channelT0 calibration and, more in general, for the time resolution
      studies using the laser and pulser data.
  */
  class TOPLaserCalibratorCollectorModule : public CalibrationCollectorModule {

  public:

    /**
    Constructor: Sets the description, the properties and the parameters of the module
    */
    TOPLaserCalibratorCollectorModule();

    /**
    Define histograms
    */
    void prepare() override;

    /**
    Main mathod, called for each event.
    Applies an advances selection of the double calpulse,
    aligns the time of all the topDigits to the reference calpulse
    and saves a tree that is then passed to the calibrator class.
    A second tree containing the resolution parameters taken from the
    DB is saved as well.
    */
    void collect() override;

  private:

    // Parameters.
    bool m_useReferencePulse = true; /**< Use the electronic pulser as reference.  */
    unsigned m_refChannel = 0; /**< Reference channel. It must contain electronic pulses. 0-511. */
    int m_refSlot = 0; /**< Reference slot (1-16). Set it to 0 or negative to switch to slot-by-slot reference */
    float m_pulserDeltaT = 22.; /**< Approximate time difference between the two calpulses, in ns */
    float m_pulserDeltaTTolerance = 2.; /**< Window around the nominal deltaT used to select a double pulse, in ns */
    bool m_storeMCTruth = false; /**< Store the TOPSimHits information instead of the  TOPDigit one*/

    //Required arrays
    StoreArray<TOPDigit> m_TOPDigitArray; /**< Required input array of TOPDigits */

    // Variables to be saved in the hit tree
    short m_channel = 0;  /**< Channel number (0-511) */
    short m_window = 0;  /**< Window  number (0-???) */
    short m_sample = 0;  /**< Sample number (0-???) */
    short m_slot = 0;  /**< Slot number (1-16) */
    float m_hitTime = 0.;  /**< Hit time with respect to the reference pulse (ns) */
    float m_dVdt = 0.;  /**< Approximate dV/dt (ADC/ns) */
    float m_amplitude = 0; /**< Hit pulse amplitude (ADC) */
    float m_width = 0; /**< Hit pulse width (ns) */
    float m_refTime = 0; /**< Time of the reference pulse (ns) */
    int m_event = 0; /**< Consecutive event count. Useful to select all the hits of a given event */
    bool m_refTimeValid = false; /**< true when the time of the reference pulse is valid */
  };
}
