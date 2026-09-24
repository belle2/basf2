/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <TProfile.h>
#include <vector>

namespace Belle2 {

  /**
   * Time-of-flight corrections as a function of ExtHit z-coordinate expressed in a TOP module frame
   */
  class TOPCalTOFCorrection: public TObject {
  public:

    /**
     * Default constructor
     */
    TOPCalTOFCorrection()
    {}

    /**
     * Setter
     * @param pf profile histogram with TOF corrections as function of local z
     */
    void set(const TProfile* pf);

    /**
     * Getter
     * @param z z-coordinate of ExtHit expressed in TOP module local frame
     * @return TOF correction
     */
    double get(double z) const;

    /**
     * Returns minimal z
     * @return minimal z
     */
    double getZmin() const {return m_zmin;}

    /**
     * Returns bin width
     * @return bin width
     */
    double getZbinWidth() const {return m_dz;}

    /**
     * Returns a vector of calibration constants
     * @return vector of constants
     */
    const std::vector<float>& getConstants() const {return m_corrections;}

    /**
     * Returns calibration status
     * @return true, if calibrated
     */
    bool isCalibrated() const {return !m_corrections.empty();}

  private:

    float m_zmin = 0; /**< minimal z */
    float m_dz = 0;   /**< bin width */
    std::vector<float> m_corrections;  /**< time-of-flight corrections as function of z */

    ClassDef(TOPCalTOFCorrection, 1); /**< ClassDef */

  };

} // end namespace Belle2

