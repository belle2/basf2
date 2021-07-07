/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
