/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <stdexcept>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Enumeration for the positional information to be used.
    enum class EFitPos {
      /// Use the wire position as positional information.
      c_WirePos = 0,

      /// Use the right left signed dirft circle
      c_RLDriftCircle = 1,

      /// Use the reconstructed position as positional information
      c_RecoPos = 2
    };

    /**
     *  Helper function to translate user provided strings to the enum values of the postion information
     *  information used in the two dimensional fit
     */
    inline EFitPos getFitPos(const std::string& fitPosString)
    {
      if (fitPosString == std::string("recoPos")) {
        return EFitPos::c_RecoPos;
      } else if (fitPosString == std::string("rlDriftCircle")) {
        return EFitPos::c_RLDriftCircle;
      } else if (fitPosString == std::string("wirePos")) {
        return EFitPos::c_WirePos;
      } else {
        throw std::invalid_argument("Unexpected fit postion string : '" + fitPosString + "'");
      }
    }

  }
}
