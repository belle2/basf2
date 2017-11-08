/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {

  /**
   *   dE/dx run gain calibration constants
   */

  class CDCDedxScaleFactor: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCDedxScaleFactor(): m_scale(0) {};

    /**
     * Constructor
     */
    CDCDedxScaleFactor(double scale): m_scale(scale) {};

    /**
     * Destructor
     */
    ~CDCDedxScaleFactor() {};

    /** Return scale factor
     * @return scale factor
     */
    double getScaleFactor() const {return m_scale; };

    /** Set scale factor
     * @param scale factor
     */
    void setScaleFactor(double scale) {m_scale = scale; };

  private:
    double m_scale; /**< Scale factor to make electron dE/dx ~ 1 */

    ClassDef(CDCDedxScaleFactor, 1); /**< ClassDef */
  };
} // end namespace Belle2
