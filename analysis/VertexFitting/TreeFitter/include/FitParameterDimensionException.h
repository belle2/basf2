/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * right(C) 2019 - Belle II Collaboration                                 *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <stdexcept>
#pragma once
namespace TreeFitter {
  /** exception template, runtime_error implements what() */
  class FitParameterDimensionException : public std::runtime_error {
  public:
    /** throw a helpful message like this one. */
    explicit FitParameterDimensionException(std::string const& msg):
      runtime_error(msg)
    {}
  };
}
