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
  // helper class to store the information for a Filter
  class FilterInfo : public TObject {
  public:
    FilterInfo() :
      m_name(""),
      m_result(-666.),
      m_wasAccepted(false),
      m_wasUsed(false)
    {
    };

    // no setters so use this one to put in the information!
    FilterInfo(std::string aname, double aresult, bool accepted, bool used) :
      m_name(aname),
      m_result(aresult),
      m_wasAccepted(accepted),
      m_wasUsed(used)
    {
    };

    ~FilterInfo() {};

    //getter functions
    std::string getName() { return m_name; };
    double getResult() { return m_result; };
    bool getWasAccepted() { return m_wasAccepted; };
    bool getWasUsed() { return m_wasUsed; };

  private:
    std::string m_name;
    double m_result;
    bool m_wasAccepted;
    bool m_wasUsed;

    ClassDef(FilterInfo, 1);
  };
}
