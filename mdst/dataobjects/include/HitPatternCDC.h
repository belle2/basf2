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

#include <RtypesCore.h>

#include <bitset>
#include <map>
#include <utility>
#include <string>

namespace Belle2 {

  /**
   * Hit pattern of CDC hits within a track.
   * The pattern is stored using a std::bitset. This allows to use the STL, which is very efficiently programmed.
   *
   * For each layer there is one bit. The layer counting is from 0 to 55.
   * Super-layer information is generated on demand. The super-layer counting goes from 0 to 8.
   *
   * The numbering scheme just in this class is equivalent to the one defined in the note BELLE2-NOTE-TE-2015-022:
   * https://docs.belle2.org/record/256
   *
   * @sa HitPatternVXD
   */
  class HitPatternCDC {
    friend class HitPatternCDCTest_settersNGetters_Test;
    friend class HitPatternCDCTest_getFirstLastLayer_Test;

  public:
    /** Create empty pattern.*/
    HitPatternCDC() : m_pattern(0) {}

    /** Initialize the pattern with an integer.*/
    explicit HitPatternCDC(ULong64_t initValue) : m_pattern(initValue) {}

    /** Getter for underlying integer type. */
    ULong64_t getInteger() const;

    /**
     * Get the total Number of CDC hits in the fit.
     * Nota Bene: The maximum value returned here is 255. This is because only
     * 8 bits are available to store the information. If the track had more than 255
     * hits, e.g. because it was a curler, the returned value may be to small.
     * @return Number of hits in the CDC.
     * */
    unsigned short getNHits() const;

    /**
     * Sets the 8 MSBs to the total number of hits in the CDC.
     * Nota Bene: If the whole track has more than 255 hits, e.g. because it is
     * a curler, the value is set to 255.
     * @param nHits Number of hits in the CDC.
     * */
    void setNHits(unsigned short nHits);

    // ----------------------------------------------------------------
    // ---------------- LAYER FUNCTIONS -------------------------------
    // ----------------------------------------------------------------

    /**
     * Set bit corresponding to layer to true.
     * This function may throw an out-of-range exception.
     * @param layer Layer index.
     */
    void setLayer(const unsigned short layer);

    /**
     * Set bit corresponding to layer to false.
     * This function may throw an out-of-range exception.
     * @param layer Layer index.
     */
    void resetLayer(const unsigned short layer);

    /**
     * Getter for single layer.
     * @param layer Layer index.
     * @return Boolean if layer is hit or not.
     * */
    bool hasLayer(const unsigned short layer) const;

    /**
     * Returns the index of the first layer with a hit.
     * If there is no hit in the whole pattern, -1 is returned.
     * @return Index of last layer.
     */
    short getFirstLayer() const;

    /**
     * Returns the index of the last layer with a hit.
     * If there is no hit in the whole pattern, -1 is returned.
     * @return Index of last layer.
     */
    short getLastLayer() const;

    // ----------------------------------------------------------------
    // ---------------- SUPER LAYER FUNCTIONS -------------------------
    // ----------------------------------------------------------------

    /**
     * Getter for super-layer match.
     * @param sLayer super-layer index.
     * */
    bool hasSLayer(const unsigned short sLayer) const;

    /**
     * Reset complete superLayer, e.g. because segment shouldn't belong to that track.
     * @param sLayer Super-layer index.
     * */
    void resetSLayer(const unsigned short sLayer);

    /**
     * Get the bit pattern in a specific super layer.
     * @param sLayer Super-layer index.
     * @return Super-layer bit pattern.
     * */
    std::bitset<64> getSLayerPattern(const unsigned short sLayer);

    /**
     * Getter for the number of hits in one super-layer.
     * Nota Bene: As only a bitwise information is available if a layer is hit, under-counting
     * is possible if in at least one layer more than one wire is hit.
     * @param sLayer Super-layer index.
     * @return Number of hits in super-layer.
     */
    unsigned short getSLayerNHits(const unsigned short sLayer) const;

    /**
     * Getter for longest run of consecutive layers with hits within the given Super-layer.
     * @param sLayer Super-layer index.
     * @return Length of longest run.
     * */
    unsigned short getLongestContRunInSL(const unsigned short sLayer) const;

    // ----------------------------------------------------------------
    // ---------------- Others ----------------------------------------
    // ----------------------------------------------------------------

    /** True, if at least one axial layer is true.*/
    bool hasAxialLayer() const;

    /** True, if at least one axial layer is true.*/
    bool hasStereoLayer() const;

    /** String for printing in python.*/
    std::string __str__() const;

  private:
    std::bitset<64> m_pattern;                     /**<  Saves the actual pattern.*/
    static const std::bitset<64> s_sLayerMasks[9]; /**<  Masks to zero out all bits from other layers.*/
    static const std::bitset<64> s_infoLayerMask;  /**<  Mask to zero out all bits from other layers. */

    /** Holds the indices for super layer access. */
    static const std::map<unsigned short, std::pair<unsigned short, unsigned short>> s_superLayerIndices;

    /** Getter for underlying bit set. Just for testing. */
    std::bitset<64> getBitSet() const { return m_pattern; }

    /** Reset the complete hit pattern. Just for testing. */
    void resetPattern() { m_pattern.reset(); }

  };
}
