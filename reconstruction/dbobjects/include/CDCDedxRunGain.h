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

    /**
     * Combine payloads
     **/
    CDCDedxRunGain& operator*=(CDCDedxRunGain const& rhs)
    {
      m_gain *= rhs.getRunGain();
      return *this;
    };

    /** Return run gain
     * @return run gain
     */
    double getRunGain() const {return m_gain; };

    /** Set run gain
     * @param gain
     */
    void setRunGain(double gain) {m_gain = gain; };

  private:
    double m_gain; /**< Run gain */

    ClassDef(CDCDedxRunGain, 4); /**< ClassDef */
  };
} // end namespace Belle2
