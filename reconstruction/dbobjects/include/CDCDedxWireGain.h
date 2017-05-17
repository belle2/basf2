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
   *   dE/dx wire gain calibration constants
   */

  class CDCDedxWireGain: public TObject {

  public:

    /**
     * Default constructor
     */
    CDCDedxWireGain(): m_wireID(0), m_gain(0) {};

    /**
     * Constructor
     */
    CDCDedxWireGain(int wireID, float gain): m_wireID(wireID), m_gain(gain) {};

    /**
     * Destructor
     */
    ~CDCDedxWireGain() {};

    /** Return wire ID
     * @return wire ID
     */
    float getWireID() const {return m_wireID; };

    /** Return wire gain
     * @return wire gain
     */
    float getWireGain() const {return m_gain; };

    /** Set wire ID
     * @param wire ID
     */
    void setWireID(int wireID) {m_wireID = wireID; };

    /** Set wire gain
     * @param wire gain
     */
    void setWireGain(float gain) {m_gain = gain; };

  private:
    int m_wireID;       /**< CDC wire ID */
    float m_gain;       /**< CDC wire gain */

    ClassDef(CDCDedxWireGain, 1); /**< ClassDef */
  };
} // end namespace Belle2
