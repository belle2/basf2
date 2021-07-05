/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/dataobjects/VXDIntercept.h>

namespace Belle2 {

  /** PXDIntercept stores the U,V coordinates and uncertainties of the intersection of a track
   *   with an PXD sensor.
   */
  class PXDIntercept : public VXDIntercept {

    //! Needed to make the ROOT object storable
    ClassDef(PXDIntercept, 2);
  };
}
