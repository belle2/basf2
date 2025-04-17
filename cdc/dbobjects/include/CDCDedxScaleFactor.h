/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    explicit CDCDedxScaleFactor(double scale): m_scale(scale) {};

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
