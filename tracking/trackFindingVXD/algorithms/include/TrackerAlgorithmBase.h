/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>

namespace Belle2 {

  /// base class for TrackerAlgorithms. shall allow a common base for algorithms like the cellular automaton
  template<class ContainerType, class ValidatorType>
  class TrackerAlgorithmBase {
  protected:
    /// something which checks the quality of the test so far (will be applied by the apply-function
    ValidatorType m_validator;

  public:
    /** getter */
    /// returns the validator of the algorithm
    virtual ValidatorType& getValidator() { return m_validator; }

    /// returns current logging info of the algorithm (some stuff one wants to log about that algorithm
    virtual std::string printStatistics() { return ""; }

    /** setter */
    /// allows to set some condition for validating the algorithm
    virtual void setValidator(ValidatorType& aValidator) { m_validator = aValidator; }

    /// applies actual algorithm on parameter passed, returns some debuging results
    virtual int apply(ContainerType&) { return 0; }

    /// searches for seeds among the entries of parameter passed (bool sets strictnes for finding seeds),
    /// uses m_validator for determining them, returns number of seeds found
    virtual unsigned int findSeeds(ContainerType&, bool) { return 0; }
  };
}
