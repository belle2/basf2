/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <string>
#include <memory>

namespace Belle2 {

  class AnalysisConfiguration
  /**
   * Singleton class keeping configurables of analysis components.
   * Object of this class is created on the first call and remains
   * in memory such, that once parameters are configured they
   * remain the same for every call. For example:
   * AnalysisConfigurationModule creates AnalysisConfiguration
   * object and set configurable value by line:
   *
   * AnalysisConfiguration::instance()->setTupleStyle(m_tupleStyle);
   *
   * Than, NTupleTools use functions inherited from DecayDescriptor
   * and in these get parameter value from AnalysisConfiguration object:
   *
   * AnalysisConfiguration::instance()->getTupleStyle()
   *
   * If object was not created before by AnalysisConfiguration module,
   * it is created on first call with default values by NTupleTool.
   *
   * By now, this class cnofigures only style of variables in tuples
   * and thus has two specific functions:
   *
   * const string getTupleStyle() which returns chosen style
   * void setTupleStyle(string v) which sets style
   *
   * Also, it has
   * static AnalysisConfiguration *instance()
   * which returns pointer to previously created AnalysisClass object
   * or, if it is the first call, create object and return pointer to it.
   * Constructor of class is prvate, so the only way to create object is
   * through instance() function.
   *
   */
  {
    std::string m_tupleStyle; /**< branch naming style */
    bool m_legacyMCMatching; /**< specifies the version of MC matching algorithm to be used */
    static AnalysisConfiguration* s_instance; /**< Singleton instance */

    /** Constructor */
    AnalysisConfiguration(): m_tupleStyle("Default"), m_legacyMCMatching(false)
    { }
  public:

    /** Configurable-specific function to obtain value */
    const std::string getTupleStyle()
    {
      //B2INFO("Returning style '"<<m_tupleStyle<<"'");
      return m_tupleStyle;
    }

    /** Configurable-specific function to set value*/
    void setTupleStyle(const std::string& v)
    {
      //B2INFO("Setting style '"<<v<<"'");
      m_tupleStyle = v;
    }

    /**
     * Determines wheter to use the legacy MCMatching algorithm (true) or not (false).
     */
    void useLegacyMCMatching(const bool flag)
    {
      m_legacyMCMatching = flag;
    }

    /**
     * Specifies whether to use the legacy MCMatching algorithm (true) or not (false).
     */
    bool useLegacyMCMatching() const
    {
      return m_legacyMCMatching;
    }

    /**
     * Returns a pointer to the singleton instance
     */
    static AnalysisConfiguration* instance()
    {
      if (!s_instance)
        s_instance = new AnalysisConfiguration;
      return s_instance;
    }
  };

} // Belle2 namespace


