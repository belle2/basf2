/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <cdc/dataobjects/WireID.h>

namespace Belle2 {

  /**
   * Database object for timing offset (t0).
   */
  class CDCTimeZero: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCTimeZero():
      m_eWire(0), m_t0(0.0)
    {}

    /**
     * Constructor
     */
    CDCTimeZero(WireID wid, double t0):
      m_eWire(wid.getEWire()), m_t0(t0)
    {
    }

    /**
     * Get layer ID.
     */
    unsigned short getICLayer() const { return WireID(m_eWire).getICLayer();}

    /**
     * Get wire ID.
     */
    unsigned short getIWire() const { return WireID(m_eWire).getIWire();}

    /**
     * Get t0.
     */
    double getT0() const { return m_t0;}

  private:
    unsigned short  m_eWire; /**< Wire ID. */
    float           m_t0;    /**< t0. */

    ClassDef(CDCTimeZero, 2); /**< ClassDef */

  };

} // end namespace Belle2
