/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
