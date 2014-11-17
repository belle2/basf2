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

#include <framework/logging/Logger.h>

#include <TObject.h>

#include <bitset>
#include <algorithm>
#include <iostream> // only temporary

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

    /**
     * Get total number of hits in PXD and SVD.
     * TODO: add ingoing arm ndfs
     */
    unsigned short getNdf() const {
      return 2 * getNPXDHits() + getNSVDHits();
    }

    // ----------------------------------------------------------------
    // ---------------- SVD LAYER FUNCTIONS ---------------------------
    // ----------------------------------------------------------------

    /**
     * Set the number of hits in a specific layer of the SVD.
     * @param svdLayer: SVD Layer numeration from inner to outer is 0 to 3.
     * @param uHits
     * @param vHits
     */
    void setSVDLayer(const unsigned short svdLayer, unsigned short uHits, unsigned short vHits) {
      // take care of input which could result in wrong behaviour
      B2ASSERT("SVD layer is out of range.", svdLayer <= 3);
      resetSVDLayer(svdLayer);
      if (uHits > 3) uHits = 3;
      if (vHits > 3) vHits = 3;
      // set the bits
      std::bitset<32> uHitPattern(uHits);
      uHitPattern <<= s_svdUshift + 4 * svdLayer;
      std::bitset<32> vHitPattern(vHits);
      vHitPattern <<= s_svdVshift + 4 * svdLayer;
      m_pattern |= (uHitPattern | vHitPattern);
    }

    /**
     * Get the number of hits in a specific layer of the SVD.
     * @param svdLayer: SVD Layer numeration from inner to outer is 0 to 3.
     * @return
     */
    std::pair<const unsigned short, const unsigned short> getSVDLayer(const unsigned short svdLayer) const {
      B2ASSERT("SVD layer is out of range.", svdLayer <= 3);
      std::bitset<32> uHitsPattern(m_pattern & (s_SVDLayerMasks[svdLayer] & s_SVDuvMasks[0]));
      std::bitset<32> vHitsPattern(m_pattern & (s_SVDLayerMasks[svdLayer] & s_SVDuvMasks[1]));
      const unsigned short uHits = (uHitsPattern >>= (s_svdUshift + 4 * svdLayer)).to_ulong();
      const unsigned short vHits = (vHitsPattern >>= (s_svdVshift + 4 * svdLayer)).to_ulong();
      return std::make_pair(uHits, vHits);
    }

    /**
     * Reset the number of hits in a specific layer of the SVD.
     * @param svdLayer: SVD Layer numeration from inner to outer is 0 to 3.
     */
    void resetSVDLayer(const unsigned short svdLayer) {
      B2ASSERT("SVD layer is out of range.", svdLayer <= 3);
      m_pattern &= ~s_SVDLayerMasks[svdLayer];
      return;
    }

    /**
     * Get total number of hits in the SVD.
     */
    unsigned short getNSVDHits() const {
      unsigned short svdHits = 0;
      for (unsigned short svdLayer = 0; svdLayer <= 3; ++svdLayer) {
        std::pair<const unsigned short, const unsigned short> svdHitPair = getSVDLayer(svdLayer);
        svdHits += svdHitPair.first + svdHitPair.second;
      }
      return svdHits;
    }

    // ----------------------------------------------------------------
    // ---------------- PXD LAYER FUNCTIONS ---------------------------
    // ----------------------------------------------------------------

    /**
     * Set the number of hits in a specific layer of the PXD.
     * @param nLayer: PXD layer numeration is 0 and 1.
     * @param mode: normal mode is 0, gated mode is 1.
     * @param nHits 0-3 hits possible
     */
    void setPXDLayer(const unsigned short pxdLayer, unsigned short nHits, const unsigned int mode = 0) {
      B2ASSERT("PXD layer is out of range.", pxdLayer <= 1);
      // take care of human error
      resetPXDLayer(pxdLayer, mode);
      if (nHits > 3) nHits = 3;
      // set hits
      std::bitset<32> hits(nHits);
      hits <<= 4 * pxdLayer + 2 * mode;
      m_pattern |= hits;
    }

    /**
     * Get the number of hits in a specific layer of the PXD.
     * @param nLayer: PXD layer numeration is 0 and 1.
     * @param mode: normal mode is 0, gated mode is 1.
     */
    unsigned short getPXDLayer(const unsigned short pxdLayer, const unsigned short mode = 0) const {
      B2ASSERT("PXD layer is out of range.", pxdLayer <= 1);
      std::bitset<32> hits(m_pattern & (s_PXDLayerMasks[pxdLayer] & s_PXDModeMasks[mode]));
      return (hits >>= (4 * pxdLayer + 2 * mode)).to_ulong();
    }

    /**
     * Reset the number of hits in a specific layer of the PXD.
     * @param nLayer: PXD layer numeration is 0 and 1.
     * @param mode: normal mode is 0, gated mode is 1.
     */
    void resetPXDLayer(const unsigned short pxdLayer, const unsigned short mode = 0) {
      B2ASSERT("PXD layer is out of range.", pxdLayer <= 1);
      m_pattern &= ~(s_PXDLayerMasks[pxdLayer] & s_PXDModeMasks[mode]);
    }

    /**
     * Get total number of hits in the PXD.
     */
    unsigned short getNPXDHits() const {
      unsigned short pxdHits = 0;
      for (unsigned short pxdLayer = 0; pxdLayer <= 1; ++pxdLayer) {
        for (unsigned short mode = 0; mode <= 1; ++mode) {
          pxdHits += getPXDLayer(pxdLayer, mode);
        }
      }
      return pxdHits;
    }


  private:
    /** Storing of actual hit pattern.
     * PXD and SVD layer get 4 bits each.
     * PXD has in each layer 4 bits where respectively two bits are used to store normal and gated mode hits separately.
     * SVD has in each layer 4 bits where respectively two bits are used to store hits in u and v layer separately.
     */
    std::bitset<32> m_pattern;

    static const std::bitset<32> s_PXDLayerMasks[2]; /**<  Mask to zero out all bits from other layers. */
    static const std::bitset<32> s_PXDModeMasks[2]; /**<  Mask to zero out all bits from other layers. */
    static const std::bitset<32> s_SVDLayerMasks[4]; /**<  Mask to zero out all bits from other layers. */
    static const std::bitset<32> s_SVDuvMasks[2]; /**<  Mask to zero out all bits from other layers. */
    static const std::bitset<32> s_infoLayerMask;  /**<  Mask to zero out all bits from other layers. */

    static const unsigned int s_svdUshift = 8; /**< for internal shifting to the SVD bits. */
    static const unsigned int s_svdVshift = 10; /**< for internal shifting to the SVD bits. */

    //-----------------------------------------------------------------------------------
    /** ROOTification for python access, but without I/O. */
    ClassDef(HitPatternVXD, 0);
  };

}
