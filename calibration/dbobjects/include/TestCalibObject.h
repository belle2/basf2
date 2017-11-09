/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: David Dossett                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
