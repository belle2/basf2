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
    FilterInfo(const std::string& aname, double aresult, bool accepted, bool used) :
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

    /// Class definition to make this a ROOT class.
    ClassDef(FilterInfo, 1);
  };
}
