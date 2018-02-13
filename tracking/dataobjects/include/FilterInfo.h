/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>


namespace Belle2 {
  /// helper class to store the information for a Filter
  class FilterInfo : public TObject {
  public:
    /// default constructor needed by root
    FilterInfo() :
      m_name(""),
      m_result(-666.),
      m_wasAccepted(false),
      m_wasUsed(false)
    {
    };

    /// no setters so use this one to put in the information!
    FilterInfo(std::string aname, double aresult, bool accepted, bool used) :
      m_name(aname),
      m_result(aresult),
      m_wasAccepted(accepted),
      m_wasUsed(used)
    {
    };

    /// destructor
    ~FilterInfo() {};

    /// getter function to get the name
    std::string getName() { return m_name; };
    /// returns the result of the filtervariable attached to this filter
    double getResult() { return m_result; };
    /// returns if the event was accepted
    bool getWasAccepted() { return m_wasAccepted; };
    /// returns if filter was evaluated
    bool getWasUsed() { return m_wasUsed; };

  private:
    /// name of the FilterVariable
    std::string m_name;
    /// the result of the filter variable
    double m_result;
    /// stores if filter was accepted
    bool m_wasAccepted;
    /// stores if filter was evaluated
    bool m_wasUsed;

    ClassDef(FilterInfo, 1);
  };
}
