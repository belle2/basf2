/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/vtx/findlets/VTXStateRejecter.h>
#include <tracking/ckf/general/findlets/StateRejecter.icc.h>

#include <tracking/ckf/vtx/entities/CKFToVTXState.h>

using namespace Belle2;

template class Belle2::StateRejecter<CKFToVTXState, ChooseableOnVTXStateApplier>;
