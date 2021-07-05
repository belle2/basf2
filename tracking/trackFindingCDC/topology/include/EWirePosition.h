/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
