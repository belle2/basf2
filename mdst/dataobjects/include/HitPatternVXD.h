/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Markus Prim                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

#include <bitset>
#include <algorithm>
#include <cassert>

namespace Belle2 {
  /** Hit pattern of the VXD with efficient setters and getters.
   *
   *  @TODO A lot, the interface has to be written soon.
   *
   *  This class was developed after and is similar to
   *  @sa HitPatternCDC
   *  We only save hits from the outgoing arm.
   *  GENERAL COMMENT: 32 Bits are reserved, but only 16 are used so far. Think about a application for them.
   */
  class HitPatternVXD : public TObject {
  public:
    /** Create empty pattern.*/
    HitPatternVXD();

    /** Initialize the pattern with some short.*/
    HitPatternVXD(const unsigned int initValue) : m_pattern(initValue)
    {}

    /** Getter for the underlying integer type.*/
    unsigned int getInteger() const {
      return m_pattern.to_ulong();
    }

    /** Getter for underlying bitset.*/
    std::bitset<32> getBitset() const {
      return m_pattern;
    }

    // ----------------------------------------------------------------
    // ---------------- LAYER FUNCTIONS -------------------------------
    // ----------------------------------------------------------------

    /** Setter for single layer.
     * NOTE: Each layer has to be resettet, before it can be set again.
     * This makes this function faster, but can result in ugly behaviour
     * if not used correctly. */
    void setLayer(const unsigned short layer, const unsigned short nHits) {
      assert(layer < 8);
      assert(m_pattern[s_layerBitOne[layer]] == 0);
      assert(m_pattern[s_layerBitTwo[layer]] == 0);
      std::bitset<32> hit(nHits);
      hit <<= layer * 2;
      m_pattern = m_pattern | hit;
    }

    /** Resetter for single layer.*/
    void resetLayer(const unsigned short layer) {
      assert(layer < 8);
      m_pattern.reset(s_layerBitOne[layer]);
      m_pattern.reset(s_layerBitTwo[layer]);
    }

    /** Getter for single Layer.
     *
     *  @return  True, if at lease one hit is in the layer specified in the argument.
     */
    bool hasLayer(const unsigned short layer) {
      assert(layer < 8);
      return (m_pattern & s_LayerMasks[layer]).any();
    }

    /** Getter for hits in layer.
     *
     * @return Number of hits in the layer.
     * */
    unsigned short hitsInLayer(const unsigned short layer) {
      assert(layer < 8);
      short int bitOne = m_pattern[s_layerBitOne[layer]];
      short int bitTwo = m_pattern[s_layerBitTwo[layer]];

      if (bitTwo == 0 && bitOne == 0) return 0;
      if (bitTwo == 0 && bitOne == 1) return 1;
      if (bitTwo == 1 && bitOne == 0) return 2;
      if (bitTwo == 1 && bitOne == 1) return 3;
      else return 0;
    }


  private:
    /** Storing of actual hit pattern.
     *
     *  The two PXD layers get 4 bits each,
     *  the SVD layers have 2 bits each.
     *  In the PXD there is an indication if the hit was from the rescue mechanism
     *  and if it was taken during gated mode running.
     *  1-4 = normal hit, 5-8 = rescue hit, 9-12 = normal gated mode, 13-16 = rescue during gated mode;
     */
    std::bitset<32> m_pattern;

    static const std::bitset<32> s_LayerMasks[8]; /**<  Masks to zero out all bits from other layers.*/
    static const std::bitset<32> s_infoLayerMask;  /**<  Mask to zero out all bits from other layers. */

    static const std::vector<unsigned short> s_layerBitOne; /**< For the access of a specific layer.*/
    static const std::vector<unsigned short> s_layerBitTwo; /**< For the access of a specific layer.*/

    //-----------------------------------------------------------------------------------
    /** ROOTification for python access, but without I/O. */
    ClassDef(HitPatternVXD, 0);
  };

}
