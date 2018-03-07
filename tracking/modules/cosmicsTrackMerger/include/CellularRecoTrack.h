/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/ca/WithAutomatonCell.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  /// Type of a reco track with automaton cell
  using CellularRecoTrack = TrackFindingCDC::WithAutomatonCell<const RecoTrack*>;

  /// Returns a short name for class CellularRecoTrack to be used in names of parameters.
  inline std::string getClassMnemomicParameterName(const CellularRecoTrack* dispatchTag __attribute__((unused)))
  {
    return "recoTrack";
  }

  /// Returns a short description for class CellularRecoTrack to be used in descriptions of parameters.
  inline std::string getClassMnemomicParameterDescription(const CellularRecoTrack* dispatchTag __attribute__((unused)))
  {
    return "Reco Track";
  }
}
