/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <vxd/dataobjects/VxdID.h>
//#include <framework/logging/Logger.h>

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>


namespace Belle2 {

  /** The payload telling which PXD pixel to mask (ignore)
   */

  class PXDMaskedPixelPar: public TObject {
  public:
    /** Structure to hold set of masked single pixels indexed by their unique id (unsigned int), stored in hash table */
    typedef std::unordered_set< unsigned int> MaskedSinglePixelsSet;

    /** Default constructor */
    PXDMaskedPixelPar() : m_MapSingles(0) {}
    /** Destructor */
    ~PXDMaskedPixelPar() {}

    /** Mask single pixel
     *
     * @param id VxdID of the required sensor
     * @param uid uCell of single pixel to mask
     * @param vid vCell of single pixel to mask
     */
    void maskSinglePixel(VxdID id, unsigned int uid, unsigned int vid);

    /** Check whether a pixel on a given sensor is OK or not.
     * @param id VxdID of the sensor
     * @param uid uCell of single pixel to mask
     * @param vid vCell of single pixel to mask
     * @return true if pixel or the id is not found in the list, otherwise false.
     */
    bool pixelOK(VxdID id, unsigned int uid, unsigned int vid) const;

    /** Return unordered_map with all masked single pixels in PXD. */
    const std::unordered_map<unsigned short, MaskedSinglePixelsSet>& getMaskedPixelMap() const {return m_MapSingles;}

  private:

    /** Structure holding sets of masked single pixels for all sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, MaskedSinglePixelsSet> m_MapSingles;

    ClassDef(PXDMaskedPixelPar, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
