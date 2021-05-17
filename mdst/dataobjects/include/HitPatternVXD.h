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

#include <bitset>
#include <utility>
#include <list>

namespace Belle2 {
  /**
   * Hit pattern of the VXD within a track.
   * The pattern is stored using a std::bitset. This allows to use the STL, which is very efficiently programmed.
   *
   * Only hits from the outgoing arm are stored. For each layer there are two bits. The layer counting is:
   *  - PXD: layers 1 and 2;
   *  - SVD: layers 3, 4, 5 and 6.
   *
   * The numbering scheme in this class is equivalent to the one defined in the note BELLE2-NOTE-TE-2015-010:
   * https://docs.belle2.org/record/243
   *
   * @sa HitPatternCDC
   *
   * 32 Bits are reserved. 4 Bits for each layer: 2 Bits for each PXD normal/gated-mode, 2 Bits for each SVD U/V-side. In total 24 Bits are used to represent hit patterns. Remaining upper 8 Bits are resevered for information.
   * 8 Bits information is currently only used in V0Finder.
   *  - 1: V0 daughters share either side of SVDCluster in the innermost hits.
   *  - 2: V0 daughters share PXDCluster or both sides of SVDClusters in the innermost hits.
   */
  class HitPatternVXD {
    friend class HitPatternVXDTest_General_Test;
    friend class HitPatternVXDTest_NumberingScheme_Test;
    friend class HitPatternVXDTest_SVDSetterAndGetter_Test;
    friend class HitPatternVXDTest_PXDSetterAndGetter_Test;

  public:

    /** PXD modes. */
    enum class PXDMode : unsigned short { normal = static_cast<unsigned short>(0), gated = 1 };

    /** Create empty pattern. */
    HitPatternVXD() : m_pattern(0) {}

    /** Initialize the pattern with an integer. */
    explicit HitPatternVXD(const unsigned int initValue) : m_pattern(initValue) {}

    /**
     * Getter for the underlying integer.
     * @return Hit pattern encoded as integer.
     * */
    unsigned int getInteger() const { return m_pattern.to_ulong(); }

    /**
     * Get degrees of freedom in PXD and SVD.
     * Nota Bene: Only for the outgoing arm.
     * @return Degrees of freedom.
     */
    unsigned short getNdf() const;

    /**
     * Return the total number of activated layers in the VXD depending on
     * the mode of the PXD.
     * @param pxdmode normal or gated
     * @sa HitPatternVXD::PXDMode
     * @return Number of activated layers.
     */
    unsigned short getNVXDLayers(const PXDMode& pxdmode = PXDMode::normal) const;

    // ----------------------------------------------------------------
    // ---------------- SVD LAYER FUNCTIONS ---------------------------
    // ----------------------------------------------------------------

    /**
     * Set the number of hits in a specific layer of the SVD.
     * @param layerId SVD layer index.
     * @param uHits
     * @param vHits
     */
    void setSVDLayer(const unsigned short layerId, unsigned short uHits, unsigned short vHits);

    /**
     * Get the number of hits in a specific layer of the SVD.
     * @param layerId SVD layer index.
     * @return Pair of (u, v) hits in the SVD layer.
     */
    std::pair<const unsigned short, const unsigned short> getSVDLayer(const unsigned short layerId) const;

    /**
     * Reset the number of hits in a specific layer of the SVD.
     * @param layerId SVD layer index.
     */
    void resetSVDLayer(const unsigned short layerId);

    /**
     * Get total number of hits in the SVD.
     * @return Total number of hits in the SVD.
     */
    unsigned short getNSVDHits() const;

    /**
     * Returns the total number of activated layers of the SVD.
     * @return Number of activated SVD layers.
     * */
    unsigned short getNSVDLayers() const;

    /**
     * Get the first activated SVD layer index.
     * If there is no hit in the SVD pattern, -1 is returned.
     * @return Index of first layer.
     */
    short getFirstSVDLayer() const;

    /**
     * Get the last activated SVD layer index.
     * If there is no hit in the SVD pattern, -1 is returned.
     * @return Index of last layer.
     */
    short getLastSVDLayer() const;

    // ----------------------------------------------------------------
    // ---------------- PXD LAYER FUNCTIONS ---------------------------
    // ----------------------------------------------------------------

    /**
     * Set the number of hits in a specific layer of the PXD.
     * @param layerId PXD layer index.
     * @param nHits 0-3 hits possible
     * @param mode normal or gated
     * @sa HitPatternVXD::PXDMode
     */
    void setPXDLayer(const unsigned short layerId, unsigned short nHits, const PXDMode& mode = PXDMode::normal);

    /**
     * Get the number of hits in a specific layer of the PXD.
     * @param layerId PXD layer index.
     * @param mode normal or gated
     * @sa HitPatternVXD::PXDMode
     * @return Number of hits in the PXD layer.
     */
    unsigned short getPXDLayer(const unsigned short layerId, const PXDMode& mode = PXDMode::normal) const;

    /**
     * Reset the number of hits in a specific layer of the PXD.
     * @param layerId PXD layer index.
     * @param mode normal or gated
     * @sa HitPatternVXD::PXDMode
     */
    void resetPXDLayer(const unsigned short layerId, const PXDMode& mode = PXDMode::normal);

    /**
     * Get total number of hits in the PXD.
     * @return Total number of hits in the PXD.
     */
    unsigned short getNPXDHits() const;

    /**
     * Returns the total number of activated layers of the PXD.
     * @param mode normal or gated
     * @sa HitPatternVXD::PXDMode
     * @return Total number of activated PXD layers.
     */
    unsigned short getNPXDLayers(const PXDMode& mode = PXDMode::normal) const;

    /**
     * Returns the first activated layer of the PXD depending on the mode.
     * If there is no hit in the PXD pattern, -1 is returned.
     * @param mode normal or gated
     * @sa HitPatternVXD::PXDMode
     * @return Index of first layer.
     */
    short getFirstPXDLayer(const PXDMode& mode = PXDMode::normal) const;

    /** Returns the last activated layer of the PXD depending on the mode.
     * If there is no hit in the PXD pattern, -1 is returned.
     * @param mode normal or gated
     * @sa HitPatternVXD::PXDMode
     * @return Index of last layer.
     */
    short getLastPXDLayer(const PXDMode& mode = PXDMode::normal) const;

    // ----------------------------------------------------------------
    // ---------------- INFORMATION FUNCTIONS -------------------------
    // ----------------------------------------------------------------

    /**
     * Set the information flag.
     * @param information Information flag.
     */
    void setInformation(const unsigned short information);

    /**
     * Get the information flag.
     * @return Information flag.
     */
    unsigned short getInformation() const;

    /**
     * Reset the information flag.
     */
    void resetInformation();

    /** String for printing in python.*/
    std::string __repr__() const;

  private:
    /** Storing of actual hit pattern.
     * PXD and SVD layer get 4 bits each.
     * PXD has in each layer 4 bits where respectively two bits are used to store normal and gated mode hits separately.
     * SVD has in each layer 4 bits where respectively two bits are used to store hits in u and v layer separately.
     */
    std::bitset<32> m_pattern;

    static const std::bitset<32> s_LayerMasks[6]; /**<  Masks to zero out all bits from other layers. */
    static const std::bitset<32> s_PXDModeMasks[2]; /**<  Mask to zero out all bits from other layers. */
    static const std::bitset<32> s_SVDuvMasks[2]; /**<  Mask to zero out all bits from other layers. */
    static const std::bitset<32> s_infoLayerMask;  /**<  Mask to zero out all bits from other layers. */

    static const std::list<unsigned short> s_PXDLayerNumbers;  /**< For iterating through layers. */
    static const std::list<unsigned short> s_SVDLayerNumbers;  /**< For iterating through layers. */

    static const unsigned int s_svdUshift = 8; /**< for internal shifting to the SVD bits. */
    static const unsigned int s_svdVshift = 10; /**< for internal shifting to the SVD bits. */

    /** Getter for underlying bitset.*/
    std::bitset<32> getBitset() const { return m_pattern; }

    /** Getter for the proper layer masks. Makes index handling more easy. */
    std::bitset<32> getLayerMask(const unsigned short layerId) const { return HitPatternVXD::s_LayerMasks[layerId - 1]; }

  };
}
