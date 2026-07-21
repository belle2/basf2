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
   * Time calibration precisions (per module)
   */
  class TOPCalPrecision: public TObject {
  public:

    /**
     * Default constructor
     */
    TOPCalPrecision()
    {}

    /**
     * Setter
     * @param pf profile histogram filled with option "S" with bins corresponding to modules
     */
    void set(const TProfile* pf);

    /**
     * Getter
     * @param moduleID slot ID
     * @return calibration precision
     */
    double get(int moduleID) const;

    /**
     * Returns a vector of calibration constants
     * @return vector of constants (index = slot - 1)
     */
    const std::vector<float>& getConstants() const {return m_precisions;}

    /**
     * Returns calibration status
     * @return true, if calibrated
     */
    bool isCalibrated() const {return !m_precisions.empty();}

  private:

    std::vector<float> m_precisions;  /**< time calibration precisions (index = slot - 1) */

    ClassDef(TOPCalPrecision, 1); /**< ClassDef */

  };

} // end namespace Belle2
