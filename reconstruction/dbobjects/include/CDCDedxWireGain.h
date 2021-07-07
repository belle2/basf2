/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>

#include <TObject.h>

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
    explicit CDCDedxWireGain(const std::vector<double>& wiregains): m_wiregains(wiregains) {};

    /**
     * Destructor
     */
    ~CDCDedxWireGain() {};

    /**
     * Combine payloads
     **/
    CDCDedxWireGain& operator*=(CDCDedxWireGain const& rhs)
    {
      for (unsigned int bin = 0; bin < m_wiregains.size(); ++bin) {
        m_wiregains[bin] *= rhs.getWireGain(bin);
      }
      return *this;
    }

    /**
     * Set wire gain
     **/
    void setWireGain(int wire, double gain)
    {
      if (wire < 0 || (unsigned)wire >= m_wiregains.size())
        B2WARNING("Asking for a CDC Wire that is not found!");
      else m_wiregains[wire] = gain;
    }

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


    ClassDef(CDCDedxWireGain, 5); /**< ClassDef */
  };
} // end namespace Belle2
