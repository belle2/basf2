/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
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
   * Database object of CDC.
   */
  class CDCTimeZero: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCTimeZero():
      m_wireID(), m_t0(0.0)
    {}

    /**
     * Constructor
     */
    CDCTimeZero(WireID wid, float t0):
      m_wireID(wid), m_t0(t0)
    {
    }

    /**
     * Get layer ID.
     */
    unsigned short getILayer() const { return m_wireID.getICLayer();}

    /**
     * Get wire ID.
     */
    unsigned short getIWire() const { return m_wireID.getIWire();}

    /**
     * Get t0.
     */
    float getT0() const { return m_t0;}

  private:
    WireID m_wireID; /**< Wire ID. */
    float m_t0; /**< t0. */

    ClassDef(CDCTimeZero, 1); /**< ClassDef */

  };

} // end namespace Belle2
