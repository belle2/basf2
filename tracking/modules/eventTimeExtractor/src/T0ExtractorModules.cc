/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/eventTimeExtractor/T0ExtractorModules.h>
#include <tracking/eventTimeExtraction/findlets/IterativeEventTimeExtractor.icc.h>
#include <tracking/eventTimeExtraction/findlets/BaseEventTimeExtractorModule.icc.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(DriftLengthBasedT0Extractor);
REG_MODULE(Chi2BasedT0Extractor);
REG_MODULE(CDCHitBasedT0Extraction);
REG_MODULE(FullGridChi2TrackTimeExtractor);
REG_MODULE(FullGridDriftLengthTrackTimeExtractor);

CDCHitBasedT0ExtractionModule::CDCHitBasedT0ExtractionModule()
  : Super( {"CDCWireHitVector"})
{
}