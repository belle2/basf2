/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCWIREUNPACKEDFROMNAMESPACES_H
#define CDCWIREUNPACKEDFROMNAMESPACES_H

#include <tracking/trackFindingCDC/topology/CDCWire.h>

#ifdef __CINT__
// Because ROOTCINT does not like namespaces inside template parameters
// we have to make each object, with which we want the instantiate the templates,
// available outside any namespace.

typedef Belle2::TrackFindingCDC::CDCWire CDCWire;

#endif // __CINT__

#endif //CDCWIREUNPACKEDFROMNAMESPACES_H
