/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>

/**
 *  @file
 *  Additional class mnemonics only used in the CKF.
 */
namespace Belle2 {
  class RecoTrack;
  class SpacePoint;

  /// Returns a short name for class RecoTrack to be used in names of parameters.
  inline std::string getClassMnemomicParameterName(const RecoTrack* dispatchTag __attribute__((unused)))
  {
    return "recoTrack";
  }

  /// Returns a short description for class RecoTrack to be used in descriptions of parameters.
  inline std::string getClassMnemomicParameterDescription(const RecoTrack* dispatchTag __attribute__((unused)))
  {
    return "Reco Track";
  }

  /// Returns a short name for class SpacePoint to be used in names of parameters.
  inline std::string getClassMnemomicParameterName(const SpacePoint* dispatchTag __attribute__((unused)))
  {
    return "spacePoint";
  }

  /// Returns a short description for class SpacePoint to be used in descriptions of parameters.
  inline std::string getClassMnemomicParameterDescription(const SpacePoint* dispatchTag __attribute__((unused)))
  {
    return "Space Point";
  }
}
