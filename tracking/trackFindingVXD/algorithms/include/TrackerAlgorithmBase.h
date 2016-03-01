/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>

namespace Belle2 {

  /** base class for TrackerAlgorithms. shall allow a common base for algorithms like the cellular automaton */
  template<class ContainerType, class ValidatorType, class LoggerType>
  class TrackerAlgorithmBase {
  protected:
    /** something which checks the quality of the test so far (will be applied by the apply-function */
    ValidatorType m_validator;

    /** stores a container carrying all the information protocolled when running the apply-function. This information will be displayed when calling printStatistics(). */
    LoggerType m_log;

  public:
/// getter:
    /** returns the validator of the algorithm */
    virtual ValidatorType& getValidator() { return m_validator; }

    /** returns the logger of the algorithm */
    virtual LoggerType& getLogger() { return m_log; }

    /** returns current logging info of the algorithm (some stuff one wants to log about that algorithm */
    virtual std::string printStatistics() { return ""; }

/// setter:

    /** allows to set some condition for validating the algorithm */
    virtual void setValidator(ValidatorType& aValidator) { m_validator = aValidator; }

    /** allows to set some condition for logging the algorithm */
    virtual void setLogger(LoggerType& aLogger) { m_log = aLogger; }

    /** applies actual algorithm on parameter passed, returns some debuging results */
    virtual int apply(ContainerType&) { return 0; }

    /** searches for seeds among the entries of parameter passed (bool sets strictnes for finding seeds), uses m_validator for determining them, returns number of seeds found */
    virtual unsigned int findSeeds(ContainerType&, bool) { return 0; }
  };

} //Belle2 namespace
