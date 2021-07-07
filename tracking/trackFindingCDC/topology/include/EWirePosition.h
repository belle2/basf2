/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Namespace guarding the contained enum values
    namespace NWirePosition {

      /**
       *  Wire position set.
       *
       *  This corresponds to the CDC::CDCGeometryPar::EWirePosition
       *
       *  Rational:
       *  Creating this copy relieves the headers that want to use the
       *  EWirePosition from including the heavy CDCGeometryPar and its includes
       *  resulting in improved compilation speed and safer rootification.
       */
      enum EWirePosition { c_Base = 0, c_Misaligned, c_Aligned };
    }

    using EWirePosition = NWirePosition::EWirePosition;

  }
}
