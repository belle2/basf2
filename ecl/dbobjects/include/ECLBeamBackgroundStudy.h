/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Calibration constants for the ECL single crystal energy calibration.   *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alexei Sibidanov                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLBeamBackgroundStudy_H
#define ECLBeamBackgroundStudy_H
#include <TObject.h>

namespace Belle2 {

  /**
   *   Flag for beam background study
   */
  class ECLBeamBackgroundStudy: public TObject {
  public:

    /**
     * Default constructor
     */
    ECLBeamBackgroundStudy() {}

    /**
     * Destructor
     */
    ~ECLBeamBackgroundStudy() {}

    /** Return beam background study flag
     * @return beam background study flag
     */
    bool getBeamBackgroundStudy() const {return m_isBeamBackgroundStudy; }

    /** Set beam background study flag
     * @param beam background study flag
     */
    void setBeamBackgroundStudy(bool study) { m_isBeamBackgroundStudy = study; }
  private:
    bool m_isBeamBackgroundStudy; /**< Flag */

    ClassDef(ECLBeamBackgroundStudy, 1); /**< ClassDef */
  };
} // end namespace Belle2
#endif
