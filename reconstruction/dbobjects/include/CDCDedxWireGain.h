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

#include <framework/logging/Logger.h>

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
    CDCDedxWireGain(): m_wiregains() {};

    /**
     * Constructor
     */
    explicit CDCDedxWireGain(std::vector<double>& wiregains): m_wiregains(wiregains) {};

    /**
     * Destructor
     */
    ~CDCDedxWireGain() {};

    /** Return wire gain
     * @param wire number
     */
    float getWireGain(int wire) const
    {
      if (wire < 0 || (unsigned)wire >= m_wiregains.size()) {
        B2WARNING("Asking for a CDC Wire that is not found!");
        return 1.0;
      }
      double gain = m_wiregains[wire];
      return gain;
    };

  private:
    /** Note, we are using dense packed wire number (0-14336) defined as follows:
    const int iwire = (superlayer == 0) ?
    160*layer+wire : m_nLayerWires[superlayer-1]+(160+32*(superlayer-1))*layer+wire;
        -see reconstruction/modules/CDCDedxPIDModule */
    std::vector<double> m_wiregains; /**< dE/dx gains for each wire */


    ClassDef(CDCDedxWireGain, 3); /**< ClassDef */
  };
} // end namespace Belle2
