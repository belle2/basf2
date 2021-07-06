/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <TObject.h>

//Number of tiles per Layer
#define N_TILES 124

namespace Belle2 {

  /**
   * The Class for ARICH aerogel tiles properties for even reconstruction.
   * This class provides aerogel refractive index at 405 nm, transmission length at 400 nm and tile thickness
   * from aerogel slot number (1-124) and layer (0 for downstream, 1 for upstream tile).
   */
  class ARICHAeroTilesInfo : public TObject {

  public:

    //! Default constructor
    ARICHAeroTilesInfo() {};

    /**
     * Get refractive index of aerogel tile from slot number
     * @param slot aerogel slot number
     * @param layer upstream or downstream layer identifier (0 for downstream, 1 for upstream tile)
     * @return refractiveIndex refractive index of aerogel tile
     */
    float getRefractiveIndex(int slot, int layer) const;

    /**
     * Get transmission length of aerogel tile from slot number
     * @param slot aerogel slot number
     * @param layer upstream or downstream layer identifier (0 for downstream, 1 for upstream tile)
     * @return transmissionLength transmission length of aerogel tile
     */
    float getTransmissionLength(int slot, int layer) const;

    /**
     * Get transmission length of aerogel tile from slot number
     * @param slot aerogel slot number
     * @param layer upstream or downstream layer identifier (0 for downstream, 1 for upstream tile)
     * @return thickness thickness of aerogel tile
     */
    float getThickness(int slot, int layer) const;

    /**
     * Add mapping entry
     * @param slot aerogel slot number
     * @param layer upstream or downstream layer identifier (0 for downstream, 1 for upstream tile)
     * @param refractiveIndex refractive index of aerogel tile
     * @param transmissionLength transmission length of aerogel tile
     * @param thickness thickness of aerogel tile
     */
    void addMapping(int slot, int layer, float refractiveIndex, float transmissionLength, float thickness);

    /**
     * Pring mapping
     */
    void print() const;


  private:

    std::map<std::vector<int>, std::vector<float>>
                                                m_aeroPos2optical; /**< map of tile slot (1-124) to optical properties and thickness */

    ClassDef(ARICHAeroTilesInfo, 2);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
