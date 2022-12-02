/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TVector3.h>
#include <Math/Vector3D.h>

namespace Belle2 {
  /** Anonymous helper function to convert XYZVector to TVector3 */
  static constexpr auto XYZToTVector = [](const ROOT::Math::XYZVector& a) {return TVector3(a.X(), a.Y(), a.Z());};
}
