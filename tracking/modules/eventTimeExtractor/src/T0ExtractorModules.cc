/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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