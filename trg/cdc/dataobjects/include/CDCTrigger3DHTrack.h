/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <array>
#include <cstdint>

#include "framework/dataobjects/Helix.h"
#include "framework/geometry/BFieldManager.h"

namespace Belle2 {

  // Track created by the CDC trigger with 3DHough Finder input.
  class CDCTrigger3DHTrack : public Helix {
  public:
    // Default constructor = initializing everything to 0/-1
    CDCTrigger3DHTrack(): Helix() {}

    /** Parameter constructor (helix only)
     *  @param phi0           angle between the transverse momentum and the x axis in [-pi, pi]
     *  @param omega          signed curvature of the track where the sign is given by the charge of the particle
     *  @param z0             z coordinate of the vertex
     *  @param cotTheta       slope of the track in the sz plane (dz/ds)
     */
    CDCTrigger3DHTrack(double phi0, double omega, double z0, double cotTheta):
      Helix(0., phi0, omega, z0, cotTheta) {}

    // Default destructor
    ~CDCTrigger3DHTrack() = default;

    // Setter methods for private variables (that are not part of the Helix class)
    void setQuadrant(const short quadrant) { m_quadrant = quadrant; };
    void setNNTClassification(const double classification) { m_classificationNNT = classification; }
    void setSTTClassification(const double classification) { m_classificationSTT = classification; }
    void setTotalMomentum(const double totalMomentum) { m_totalMomentum = totalMomentum; }
    void setTime(const int time) { m_time = time; }
    void setDriftThreshold(const std::vector<bool>& driftThreshold) { m_driftThreshold = driftThreshold; }
    void setTSVector(const std::array<unsigned short, 9>& tsVector) { m_tsVector = tsVector; }
    void setValidTrackBit(const bool validTrackBit) { m_validTrackBit = validTrackBit; }
    void setNNTBit(const bool nntBit) { m_nntBit = nntBit; }
    void setSTTBit(const bool sttBit) { m_sttBit = sttBit; }
    void setFloatInput(const std::vector<float>& floatInput) { m_floatInput = floatInput; }
    void setRawInput(const std::vector<int>& rawInput) { m_rawInput = rawInput; }
    void setRawOutput(const std::vector<int>& rawOutput) { m_rawOutput = rawOutput; }
    void setRaw3DHMaximum(const std::array<int, 4>& raw3DHMaximum) { m_raw3DHMaximum = raw3DHMaximum; }
    void setHoughSpace(const std::vector<uint8_t>& houghSpace) { m_houghSpace = std::move(houghSpace); }

    // Corresponding getter methods
    short getQuadrant() const { return m_quadrant; }
    double getNNTClassification() const { return m_classificationNNT; }
    double getSTTClassification() const { return m_classificationSTT; }
    double getTotalMomentum() const { return m_totalMomentum; }
    float getTime() const { return m_time; }
    std::vector<bool> getDriftThreshold() const { return m_driftThreshold; }
    std::array<unsigned short, 9> getTSVector() const { return m_tsVector; }
    double getPt() const
    {
      const double bField = 1.5;
      return getTransverseMomentum(bField);
    }
    bool getValidTrackBit() const { return m_validTrackBit; }
    bool getNNTBit() const { return m_nntBit; }
    bool getSTTBit() const { return m_sttBit; }
    std::vector<float> getFloatInput() const { return m_floatInput; }
    std::vector<int> getRawInput() const { return m_rawInput; }
    std::vector<int> getRawOutput() const { return m_rawOutput; }
    std::array<int, 4> getRaw3DHMaximum() const { return m_raw3DHMaximum; }
    std::vector<int> getHoughSpace() const { return std::vector<int>(m_houghSpace.begin(), m_houghSpace.end()); }

  private:
    // Number of trigger clocks of (the track output - L1 trigger) (HW)
    int m_time{0};
    // Store if drift time was within the timing window (HW)
    std::vector<bool> m_driftThreshold;
    // Store the left/right info of the found track segments for each super layer (HW and SW)
    std::array<unsigned short, 9> m_tsVector{0};
    // The found CDC quadrant (HW and SW)
    short m_quadrant{-1};
    // Network classification of the nnt-bit/y-bit (z from IP) (HW and SW)
    double m_classificationNNT{-1.0};
    // Network classification of the stt-bit (p > 0.7 GeV and z from IP) (HW and SW)
    double m_classificationSTT{-1.0};
    // Network total momentum prediction
    double m_totalMomentum{-1.0};
    // Whether the track segments are sufficient for Neuro track
    bool m_validTrackBit{false};
    // NNT decision that the firmware passed to gdl (HW and SW)
    bool m_nntBit{false};
    // STT decision that the firmware passed to gdl (HW and SW)
    bool m_sttBit{false};
    // Values to store the float network input (SW)
    std::vector<float> m_floatInput;
    // Values to store the raw network input (HW)
    std::vector<int> m_rawInput;
    // Values to store the raw network output (HW)
    std::vector<int> m_rawOutput;
    // Values to store the raw 3DHough maximum (HW and SW)
    std::array<int, 4> m_raw3DHMaximum;
    // Store the full Hough space (SW)
    std::vector<uint8_t> m_houghSpace;

    ClassDef(CDCTrigger3DHTrack, 1);
  };
}
