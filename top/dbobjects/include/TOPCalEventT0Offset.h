/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <map>
#include <vector>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * Class to store the calibrated EventT0 offsets of other detector components
   * The offsets are measured relative to TOP EventT0 (e.g CDC - TOP etc)
   */
  class TOPCalEventT0Offset: public TObject {

  public:

    /**
     * Calibration constants of a detector component
     */
    struct OffsetData {
      float offset = 0; /**< offset relative to TOP */
      float sigma = 0;  /**< time resolution of the detector component */
      Const::EDetector component = Const::invalidDetector; /**< detector component */

      /**
       * Default constructor
       */
      OffsetData()
      {}

      /**
       * Full constructor
       * @param T0 offset relative to TOP
       * @param sig time resolution
       * @param det detector component
       */
      OffsetData(double T0, double sig, Const::EDetector det): offset(T0), sigma(sig), component(det)
      {}
    };

    /**
     * Default class constructor
     */
    TOPCalEventT0Offset()
    {}

    /**
     * Setter
     * @param detector detector component
     * @param offset offset relative to TOP
     * @param sigma time resolution of the detector component (must be a positive value)
     */
    void set(Const::EDetector detector, double offset, double sigma)
    {
      if (sigma > 0) {
        m_data.push_back(OffsetData(offset, sigma, detector));
      } else {
        B2ERROR("TOPCalEventT0Offset::set: time resolution must be a positive number. Entry ignored"
                << LogVar("detector component", detector));
      }
    }

    /**
     * Getter
     * @param detector detector component
     * @return calibration constants
     */
    const OffsetData& get(Const::EDetector detector) const
    {
      if (m_calibrations.empty()) set();
      return m_calibrations[detector];
    }

    /**
     * Is calibration available for a given detector component?
     * @param detector detector component
     * @return true if available
     */
    bool isAvailable(Const::EDetector detector) const
    {
      if (m_calibrations.empty()) set();
      return (m_calibrations[detector].sigma > 0);
    }

    /**
     * Is object empty?
     * @return true if empty
     */
    bool isEmpty() const {return m_data.empty();}

  private:

    /**
     * Set the cache
     */
    void set() const
    {
      for (const auto& data : m_data) m_calibrations[data.component] = data;
    }

    std::vector<OffsetData> m_data; /**< calibration data */
    /** cache */
    mutable std::map<Const::EDetector, OffsetData> m_calibrations; //! do not write out

    ClassDef(TOPCalEventT0Offset, 1); /**< ClassDef */

  };

}
