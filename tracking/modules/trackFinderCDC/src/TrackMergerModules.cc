/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/TrackMergerModules.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_TrackLinker);
REG_MODULE(TFCDC_TrackCombiner);

TFCDC_TrackLinkerModule::TFCDC_TrackLinkerModule()
  : Super( {"CDCTrackVector"})
{
}

TFCDC_TrackCombinerModule::TFCDC_TrackCombinerModule()
  : Super( {"" /*to be set externally*/, "" /*to be set externally*/, ""  /*to be set externally*/})
{
}
