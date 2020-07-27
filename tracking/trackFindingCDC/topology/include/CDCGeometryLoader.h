/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Thomas Hauth                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Load the CDC geometry from the conditions database
    class CDCGeometryLoader {

    public:
      /// Helper function to open the database - only for tests offside the standard event processing
      static void loadDatabase();

      /// Helper function to close and reset the database - only for tests offside the standard event processing
      static void closeDatabase();
    };
  }
}
