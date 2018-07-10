/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <TObject.h>

//number of channels per HAPD
#define N_TILES 124

namespace Belle2 {

  //! The Class for ARICH aerogel tiles properties
  /*! This class provides aerogel refractive index and transmission length from aerogel slot number (1-124) and layer (0 for downstream, 1 for upstream tile)
  */

  class ARICHAeroTilesInfo : public TObject {

  public:

    //! Default constructor
    ARICHAeroTilesInfo() {};

    /**
     * Get refractive index of aerogel tile from slot number
     * @param slot aerogel slot number
     * @param layer upstream or downstream layer identifier
     * @return refractiveIndex refractive index of aerogel tile
     */
    float getRefractiveIndex(int slot, int layer) const;

    /**
     * Get transmission length of aerogel tile from slot number
     * @param slot aerogel slot number
     * @param layer upstream or downstream layer identifier
     * @return transmissionLength transmission length of aerogel tile
     */
    float getTransmissionLength(int slot, int layer) const;

    /**
     * Add mapping entry
     * @param slot aerogel slot number
     * @param layer upstream or downstream layer identifier
     * @param refractiveIndex refractive index of aerogel tile
     * @param transmissionLength transmission length of aerogel tile
     */
    void addMapping(int slot, int layer, float refractiveIndex, float transmissionLength);

    /**
     * Pring mapping
     */
    void print() const;


  private:

    std::map<std::vector<int>, std::vector<float>> m_aeroPos2optical; /**< map of tile slot (1-124) to optical properties */

    ClassDef(ARICHAeroTilesInfo, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
