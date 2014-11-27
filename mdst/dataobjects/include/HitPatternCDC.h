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
#include <vector>
#include <map>
#include <utility>

namespace Belle2 {
  /** Hit pattern of CDC hits within a track and efficient getters.
   *
   *  The pattern is stored using a std::bitset. This allows to use some stuff from the
   *  STL, which most likely is very efficiently programmed rather than implementing the
   *  stuff oneself with integer types.
   *  For each layer there is one bit. In addition there is a bit for the superLayers 2-8,
   *  which can be set to indicate, that at least one of the layers has two hits for the track.
   *  This is the idea at the moment. Perhaps we will later decide to switch this to some
   *  better description of the outermost super-layer...
   *  Note, that super Layer counting goes from 0 to 8.
   *  GENERAL COMMENT: I think the non-static members and the interface are largely OK,
   *                   but the back-end implementation maybe not so great.
   */
  class HitPatternCDC : public TObject {
  public:
    /** Create empty pattern.*/
    HitPatternCDC() {}

    /** Initialize the pattern with some long int.*/
    HitPatternCDC(ULong64_t initValue) : m_pattern(initValue)
    {}

    /** Getter for underlying integer type. */
    ULong64_t getInteger() const {
      if (sizeof(unsigned long) >= 8) {
        return m_pattern.to_ulong();
      } else {
        return m_pattern.to_ullong();
      }
    }

    /** Getter for underlying bit set. */
    std::bitset<64> getBitSet() const {
      return m_pattern;
    }

    /** Get the approximate total Number of CDC hits in the fit. */
    unsigned short getNHits() const {
      // Shift the 8 MSBs to the right and return their value as integer.
      return static_cast<unsigned short int>((m_pattern >> 56).to_ulong());
    }

    /** Sets the 8 MSBs to the total number of hits in the CDC.*/
    void setNHits(unsigned short nHits) {
      if (nHits > 256) {
        // Maximum with 8 available bits
        nHits = 255;
      }
      // Reset the 8 MSBs to zero.
      m_pattern = m_pattern & ~s_infoLayerMask;
      // Set the total number of hits as the 8 MSBs
      std::bitset<64> numberOfHits(nHits);
      numberOfHits <<= 56;
      // Set the 8 MSBs to the total number of hits.
      // The 8 MSBs have to be zero, otherwise this breaks.
      m_pattern = numberOfHits | m_pattern;
    }

    // ----------------------------------------------------------------
    // ---------------- LAYER FUNCTIONS -------------------------------
    // ----------------------------------------------------------------

    /** Set bit corresponding to layer to true.
     *
     *  This function may throw an out-of-range exception.
     */
    void setLayer(const unsigned short layer) {
      B2ASSERT("Layer is out of range.", layer <= 55);
      m_pattern.set(layer);
    }

    /** Set bit corresponding to layer to false.
     *
     *  This function may throw an out-of-range exception.
     */
    void resetLayer(const unsigned short layer) {
      B2ASSERT("Layer is out of range.", layer <= 55);
      m_pattern.reset(layer);
    }

    /** Getter for single layer.*/
    bool hasLayer(const unsigned short layer) const {
      B2ASSERT("Layer is out of range.", layer <= 55);
      return m_pattern[layer];
    }


    /** Returns the index of the first layer with a hit.
     * If there is no hit in the whole pattern, -1 is returned.
     */
    short getFirstLayer() const {
      for (unsigned int i = 0; i < m_pattern.size(); ++i) {
        if ((m_pattern & ~s_infoLayerMask).test(i)) return i;
      }
      return -1;
    }

    /** Returns the index of the last layer with a hit.
     * If there is no hit in the whole pattern, -1 is returned.
     */
    short getLastLayer() const {
      // m_pattern.size()-8 because the first 8 bits are not pattern
      for (unsigned int i = m_pattern.size() - 8; i > 0; --i) {
        // -1 because of the index couting...
        if ((m_pattern & ~s_infoLayerMask).test(i - 1)) return i - 1;
      }
      return -1;
    }
    // ----------------------------------------------------------------
    // ---------------- SUPER LAYER FUNCTIONS -------------------------
    // ----------------------------------------------------------------

    /** Getter for Super-Layer match.*/
    bool hasSLayer(const unsigned short sLayer) const {
      B2ASSERT("Super layer outof range.", sLayer <= 8);
      return ((m_pattern & s_sLayerMasks[sLayer]).any());
    }

    /** Reset complete superLayer, e.g. because segment shouldn't belong to that track.*/
    void resetSLayer(const unsigned short sLayer) {
      B2ASSERT("Super layer outof range.", sLayer <= 8);
      for (unsigned short int ii = 0; ii < m_pattern.size(); ++ii) {
        if ((s_sLayerMasks[sLayer])[ii]) {resetLayer(ii);}
      }
    }

    /** Get the pattern in a specific super layer. */
    std::bitset<64> getSLayerPattern(const unsigned short sLayer) {
      return m_pattern & s_sLayerMasks[sLayer];
    }

    /** Getter for the approximate number of hits in one super-layer.
     *
     *  In case of multiple layers with two or more hits or
     *  any layers with more than two hits leads to under-counting.
     */
    unsigned short getSLayerNHits(const unsigned short sLayer) const {
      B2ASSERT("Super layer outof range.", sLayer <= 8);
      return static_cast<unsigned short>((m_pattern & s_sLayerMasks[sLayer]).count());
    }

    /** Getter for longest run of consecutive layers with hits in the Super-Layer.
     * TODO: Maybe a better solution can be found here*/
    unsigned short getLongestContRunInSL(const unsigned short sLayer) const {
      B2ASSERT("Super layer outof range.", sLayer <= 8);
      unsigned short max = 0;
      unsigned short counter = 0;
      std::pair<unsigned short, unsigned short> indices = s_superLayerIndices.at(sLayer);
      for (unsigned short i = indices.first; i <= indices.second; ++i) {
        counter += m_pattern[i];
        if (m_pattern[i] == 0) {
          if (counter > max) {
            max = counter;
          }
          counter = 0;
        }
      }
      return std::max(max, counter);
    }

    // ----------------------------------------------------------------
    // ---------------- Others ----------------------------------------
    // ----------------------------------------------------------------

    /** Reset the complete hit pattern. */
    void resetPattern() {
      m_pattern.reset();
    }

    /** True, if at least one axial layer is true.*/
    bool hasAxialLayer() const {
      return ((s_sLayerMasks[0] | s_sLayerMasks[2] | s_sLayerMasks[4] | s_sLayerMasks[6] | s_sLayerMasks[8])
              & m_pattern).any();
    }

    /** True, if at least one axial layer is true.*/
    bool hasStereoLayer() const {
      return ((s_sLayerMasks[1] | s_sLayerMasks[3] | s_sLayerMasks[5] | s_sLayerMasks[7])
              & m_pattern).any();
    }


  private:
    std::bitset<64> m_pattern;                     /**<  Saves the actual pattern.*/
    static const std::bitset<64> s_sLayerMasks[9]; /**<  Masks to zero out all bits from other layers.*/
    static const std::bitset<64> s_infoLayerMask;  /**<  Mask to zero out all bits from other layers. */

    /** Holds the indices for super layer access. */
    static const std::map<unsigned short, std::pair<unsigned short, unsigned short>> s_superLayerIndices;

    //-----------------------------------------------------------------------------------
    /** Make it a ROOT object.
     *
     *  ClassVersionID is set to 0, as there is no need for streamer,
     *  as this the integer equivalent of the bitset is used as object member.
     */
    ClassDef(HitPatternCDC, 0);
  };
}
