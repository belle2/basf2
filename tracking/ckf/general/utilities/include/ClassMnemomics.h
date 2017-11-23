/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>

/**
 *  @file
 *
 *  Additional class mnemonics only used in the CKF.
 *
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
