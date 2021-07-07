/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef VTX_DATAOBJECTS_VTXDIGIT_H
#define VTX_DATAOBJECTS_VTXDIGIT_H

#include <pxd/dataobjects/PXDDigit.h>

namespace Belle2 {
  /**
   * The VTX digit class.
   *
   * This is a development implementation which is intentionally kept
   * somewhat bulky.
   */

  class VTXDigit : public PXDDigit {
  public:
    /** default constructor for ROOT */
    VTXDigit(): PXDDigit() {}

    /** Useful Constructor.
     * @param sensorID Sensor compact ID.
     * @param uCellID Cell ID in "r-phi".
     * @param vCellID Cell ID in "z".
     * @param charge The charge collected in the cell.
     */
    VTXDigit(VxdID sensorID, unsigned short uCellID, unsigned short vCellID,
             unsigned short charge): PXDDigit(sensorID, uCellID, vCellID, charge)
    {}

  private:
    ClassDef(VTXDigit, 5)
  };
} // end namespace Belle2

#endif