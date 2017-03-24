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
    CDCDedxRunGain(): m_run(0), m_gain(0) {};

    /**
     * Constructor
     */
    CDCDedxRunGain(int run, float gain): m_run(run), m_gain(gain) {};

    /**
     * Destructor
     */
    ~CDCDedxRunGain() {};

    /** Return run number
     * @return run number
     */
    float getRun() const {return m_run; };

    /** Return run gain
     * @return run gain
     */
    float getRunGain() const {return m_gain; };

    /** Set run number
     * @param run number
     */
    void setRun(int run) {m_run = run; };

    /** Set run gain
     * @param run gain
     */
    void setRunGain(float gain) {m_gain = gain; };

  private:
    int m_run;          /**< Run number */
    float m_gain;       /**< Run gain */

    ClassDef(CDCDedxRunGain, 1); /**< ClassDef */
  };
} // end namespace Belle2
