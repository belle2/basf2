/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>
#include <tracking/trackingUtilities/eventdata/tracks/CDCTrack.h>
#include <tracking/modules/trackFinderCDC/AsicBackgroundLibraryCreatorModule.h>


using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

REG_MODULE(AsicBackgroundLibraryCreator);
