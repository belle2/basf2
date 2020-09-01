/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Henrik Junkerkalefeld                                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>

namespace Belle2 {


  /** The payload containing the CDC QI cut parameter.
      The CDC QI output values lies between 0 and 1. Every CDC track
      that does not exceed the cut value will be rejected in the
      tracking reconstruction. */
  class CDCQICutParameter: public TObject {
  public:
    /** Default constructor */
    CDCQICutParameter() : m_CDCQIcutvalue(0.0) {}
    /** Destructor */
    ~CDCQICutParameter() {}

    /** Set the CDC QI cut value */
    void setCDCQIcutvalue(double CDCQIcutvalue)
    {
      m_CDCQIcutvalue = CDCQIcutvalue;
    }

    /** Get the CDC QI cut value */
    double getCDCQIcutvalue() const
    {
      return m_CDCQIcutvalue;
    }

  private:
    /** store the CDC QI cut value */
    double m_CDCQIcutvalue;

    ClassDef(CDCQICutParameter, 1);  /**< ClassDef, necessary for ROOT */
  };
}
