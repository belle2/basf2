/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/utilities/Advancer.h>

namespace Belle2 {
  /// The PXD advancer is just a synonym of the normal advancer (but may change in the future).
  using SVDAdvancer = Advancer;
}