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
#include <map>

namespace Belle2 {

  /**
   *   dE/dx wire gain calibration constants
   */

  class CDCDedxWireGain: public TObject {

  public:

    /**
     * Default constructor
     */
    CDCDedxWireGain(): m_means() {};

    /**
     * Constructor
     */
    CDCDedxWireGain(std::map<int, double>& means): m_means(means) {};

    /**
     * Destructor
     */
    ~CDCDedxWireGain() {};

    /** Return wire gain
     * @return wire gain
     */
    float getWireGain(int wire) const
    {
      double mean = m_means.at(wire);
      return mean;
    };

  private:
    /** dE/dx gains for each wire: key is continuous wire number (0-14336) */
    std::map<int, double> m_means;

    ClassDef(CDCDedxWireGain, 2); /**< ClassDef */
  };
} // end namespace Belle2
