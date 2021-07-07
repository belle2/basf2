/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/vtx/filters/results/ChooseableVTXResultFilter.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>

using namespace Belle2;

template class TrackFindingCDC::Chooseable<BaseVTXResultFilter>;
template class TrackFindingCDC::ChooseableFilter<VTXResultFilterFactory>;
