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

  class CDCDedxRunGain: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCDedxRunGain(): m_gain(1.0) {};

    /**
     * Constructor
     */
    explicit CDCDedxRunGain(double gain): m_gain(gain) {};

    /**
     * Destructor
     */
    ~CDCDedxRunGain() {};

    /** Return run gain
     * @return run gain
     */
    double getRunGain() const {return m_gain; };

    /** Set run gain
     * @param run gain
     */
    void setRunGain(double gain) {m_gain = gain; };

  private:
    double m_gain; /**< Run gain */

    ClassDef(CDCDedxRunGain, 3); /**< ClassDef */
  };
} // end namespace Belle2
