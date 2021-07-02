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
   *   Test DBObject
   */

  class TestCalibObject: public TObject {

  public:

    /**
     * Default constructor
     */
    TestCalibObject(): m_value(0) {};

    /**
     * Constructor
     */
    explicit TestCalibObject(float value): m_value(value) {};

    /**
     * Destructor
     */
    ~TestCalibObject() {};

    /** Return constant
     * @return constant
     */
    float getConstantValue() const {return m_value; };

    /** Set value of constant
     * @param value
     */
    void setConstantValue(float value) {m_value = value; };

  private:
    float m_value;       /**< Constant value */

    ClassDef(TestCalibObject, 1); /**< ClassDef */
  };
} // end namespace Belle2
