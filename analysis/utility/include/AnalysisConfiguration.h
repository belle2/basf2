/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CONFIGURE_H
#define CONFIGURE_H
#include <string>
#include <memory>
//#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {

  class AnalysisConfiguration {
    string m_TupleStyle;
    static AnalysisConfiguration* s_instance;
    AnalysisConfiguration(string v = "Default")
    {
      m_TupleStyle = v;
    }
  public:
    string getTupleStyle()
    {
      //B2INFO("Returning style '"<<m_TupleStyle<<"'");
      return m_TupleStyle;
    }
    void setTupleStyle(string v)
    {
      //B2INFO("Setting style '"<<v<<"'");
      m_TupleStyle = v;
    }
    static AnalysisConfiguration* instance()
    {
      if (!s_instance)
        s_instance = new AnalysisConfiguration;
      return s_instance;
    }
  };

} // Belle2 namespace

#endif



