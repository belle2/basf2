/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Martin Heck                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <TObject.h>

#include <bitset>
#include <algorithm>

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
    HitPatternCDC(const unsigned long& initValue) : m_pattern(initValue)
    {}

    /** Getter for underlying integer type. */
    unsigned long getInteger() {
      return m_pattern.to_ulong();
    }

    /** Getter for underlying bit set. */
    std::bitset<64> getBitSet() {
      return m_pattern;
    }

    /** Get the approximate total Number of CDC hits in the fit.
     *
     *  If in a Super-Layer there are more than two layers with two or more hits,
     *  this results in undercounting of the number of hits.
     */
    unsigned short getNHits() const {
      return static_cast<unsigned short>(m_pattern.count());
    }

    /** Set bit corresponding to layer to true.
     *
     *  This function may throw an out-of-range exception.
     *  @return Value of the bit before setting. This maybe relevant
     *          e.g. to set double occupation bit.
     */
    bool setLayer(const unsigned short layer) {
      //The return value may be nonsense, if out-of range...
      bool valueBefore = m_pattern[layer];
      m_pattern.set(layer);
      return valueBefore;
    }

    /** Set bit corresponding to layer to false.
     *
     *  This function may throw an out-of-range exception.
     *  @return Value of the bit before setting. This maybe relevant
     *          e.g. to set double occupation bit.
     */
    bool resetLayer(const unsigned short layer) {
      bool valueBefore = m_pattern[layer];
      m_pattern.reset(layer);
      return valueBefore;
    }

    /** Set bit without range check or readout. */
    void setLayerFast(const unsigned short layer, const bool value = true) {
      m_pattern[layer] = value;
    }

    /** Getter for single layer.*/
    bool hasLayer(const unsigned short layer) const {
      return m_pattern[layer];
    }

    /** Getter for Super-Layer match.*/
    bool hasSLayer(const unsigned short sLayer) const {
      return ((m_pattern & s_sLayerMasks[sLayer]).any());
    }

    /** Reset complete superLayer, e.g. because segment shouldn't belong to that track.*/
    void resetSLayer(const unsigned short sLayer) {
      for (unsigned short int ii = 0; ii < m_pattern.size(); ++ii) {
        if ((s_sLayerMasks[sLayer])[ii]) {resetLayer(ii);}
      }
    }

    /** Reset the complete hit pattern. */
    void resetPattern() {
      m_pattern.reset();
    }

    /** Getter for the approximate number of hits in one super-layer.
     *
     *  In case of multiple layers with two or more hits or
     *  any layers with more than two hits leads to under-counting.
     */
    unsigned short getSLayerNHits(const unsigned short sLayer) const {
      return static_cast<unsigned short>((m_pattern & s_sLayerMasks[sLayer]).count());
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

    /** Getter for longest run of consecutive layers with hits in the Super-Layer.
     *
     *  @TODO Not yet implemented, perhaps having statics of the Super-Layer boundaries
     *        is vastly superior to bit logic here?
     */
    unsigned short getLongestContRunInSL(const unsigned short /*sLayer*/) const {
      return 0;
    }

  private:
    std::bitset<64> m_pattern;                     /**<  Saves the actual pattern.*/
    static const std::bitset<64> s_sLayerMasks[9]; /**<  Masks to zero out all bits from other layers.*/

    //-----------------------------------------------------------------------------------
    /** Make it a ROOT object.
     *
     *  ClassVersionID is set to 0, as there is no need for streamer,
     *  as this the integer equivalent of the bitset is used as object member.
     */
    ClassDef(HitPatternCDC, 0);
  };
}
