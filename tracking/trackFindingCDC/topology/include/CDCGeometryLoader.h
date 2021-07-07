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
