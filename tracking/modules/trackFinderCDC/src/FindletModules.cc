/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/FindletModules.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(FacetCreator);
REG_MODULE(SegmentCreatorFacetAutomaton);
REG_MODULE(SegmentLinker);
REG_MODULE(SegmentFitter);
REG_MODULE(SegmentRejecter);
REG_MODULE(SegmentOrienter);
REG_MODULE(SegmentCreatorMCTruth);

REG_MODULE(TrackLinker);
REG_MODULE(TrackCombiner);

REG_MODULE(SegmentFinderFacetAutomaton);
