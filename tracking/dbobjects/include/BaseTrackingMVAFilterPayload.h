/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <string>

namespace Belle2 {
  /** Class for the MVA filter payloads. */
  class BaseTrackingMVAFilterPayload : public TObject {
  public:
    /** Default constructor */
    BaseTrackingMVAFilterPayload() : m_cutValue(-1.), m_weightFileIdentifier("") {}

    /** Destructor */
    ~BaseTrackingMVAFilterPayload() {}

    /** Set the MVA filter cut value (-1 if not used) */
    void setCutValue(const double cutValue)
    {
      m_cutValue = cutValue;
    }

    /** Get the MVA filter cut value (-1 if not used) */
    double getCutValue() const
    {
      return m_cutValue;
    }

    /** Set the name of the weight file */
    void setIdentifierName(const std::string identifier)
    {
      m_weightFileIdentifier = identifier;
    }

    /** Get the name of the weight file */
    std::string getIdentifierName() const
    {
      return m_weightFileIdentifier;
    }

  private:
    /** MVA cut value */
    double m_cutValue;
    /** Name of the MVA weightfile */
    std::string m_weightFileIdentifier;

    ClassDef(BaseTrackingMVAFilterPayload, 1);  /**< ClassDef, necessary for ROOT */
  };
}