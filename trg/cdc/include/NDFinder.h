/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <array>
#include <utility>
#include <boost/multi_array.hpp>
#include <cstdint>

#include "trg/cdc/NDFinderPeakFinder.h"
#include "trg/cdc/dataobjects/CDCTriggerNDFinderLUTs.h"
#include "framework/database/DBObjPtr.h"

#define BOOST_MULTI_ARRAY_NO_GENERATORS

namespace Belle2 {

  // Typedefs for the LUT tables and the Hough space
  typedef boost::multi_array<unsigned short, 2> c2array;
  typedef boost::multi_array<unsigned short, 3> c3array;
  typedef boost::multi_array<unsigned short, 4> c4array;
  typedef c3array::index c3index;
  typedef c4array::index c4index;
  typedef std::array<c3index, 3> cell_index;

  // Struct of NDFinder parameters
  struct NDFinderParameters {
    // Required number of axial super layers
    unsigned short minSuperAxial;
    // Required number of stereo super layers
    unsigned short minSuperStereo;
    // Peak finding: Number of iterations for the peak search in each Hough space quadrant
    unsigned short iterations;
    // Peak finding: Number of deleted cells in each omega direction from the maximum
    unsigned short omegaTrim;
    // Peak finding: Number of deleted cells in each phi direction from the maximum
    unsigned short phiTrim;
    // Switch for saving the full Hough space
    bool storeHoughSpace;
    // Axial and stereo hit representations that should be used
    std::string axialFile;
    std::string stereoFile;
  };

  // Struct containing the track segment (hit) information from the Track Segment Finder (TSF)
  struct HitInfo {
    unsigned short hitID;
    unsigned short hitSLID;
    short hitPrioTime;
  };

  // Struct for a found NDFinder track
  struct RawFinderTrack {
    double omega{};
    double phi{};
    double cot{};
    HoughPeak peak;
    std::vector<uint8_t> houghSpace;

    RawFinderTrack(std::array<double, 3> estimatedParameters,
                   HoughPeak&& houghPeak,
                   std::vector<uint8_t>&& readoutHoughSpace)
      : omega{estimatedParameters[0]},
        phi{estimatedParameters[1]},
        cot{estimatedParameters[2]},
        peak{std::move(houghPeak)},
        houghSpace{std::move(readoutHoughSpace)} {}
  };

  // Class to represent the CDC NDFinder.
  class NDFinder {
  public:
    // Data type to collect a binning
    struct SectorBinning {
      unsigned short omega;
      unsigned short phi;
      unsigned short cot;
      unsigned short nHitIDs;
    };

    // Collection of the hit contribution information needed for the hit to peak relations
    struct ContributionInfo {
      unsigned short hitIndex;
      unsigned short contribution;
      unsigned short superLayer;
      short priorityTime;
    };

    // Default constructor
    NDFinder() = default;

    // Destructor
    ~NDFinder() { delete m_houghSpace; }

    // Initialization of the NDFinder (parameters and lookup tables)
    void init(const NDFinderParameters& ndFinderParameters);
    // Reset the NDFinder data structure to process next event
    void reset();
    // Add the hit info of a single track segment to the NDFinder
    void addHit(const HitInfo& hitInfo);
    // Main function for track finding
    void findTracks();
    // Retreive the results
    std::vector<RawFinderTrack>* getFinderTracks() { return &m_rawFinderTracks; }

  private:
    // NDFinder: Internal functions for track finding
    // Fills the m_hitToSectorIDs (see below) array with the hit to orientation/sectorWire/sectorID relations
    void initHitToSectorMap();
    // Fills the m_compAxialHitReps/m_compStereoHitReps (see below) arrays with the hit representations (hits to weights)
    void fillCompressedHitReps(const std::vector<unsigned short>& flatArray, const SectorBinning& compBins,
                               c4array& compHitsToWeights) const;
    // Loads the hit representations from a plain text file if the ConditionsDB is not used
    std::vector<unsigned short> loadFromFile(const std::string& fileName) const;
    // Fills the m_expAxialHitReps/m_expStereoHitReps (see below) arrays with the expanded hit representations (hits to weights)
    void fillExpandedHitReps(const SectorBinning& compBins, const c4array& compHitsToWeights, c4array& expHitsToWeights) const;
    // Process a single axial or stereo hit for the Hough space
    void processHitForHoughSpace(const unsigned short hitIdx);
    // Write (add) a single hit (Hough curve) to the Hough space
    void writeHitToHoughSpace(const unsigned short relativeWireID, const unsigned short phiSectorStart,
                              const c4array& expHitsToWeights);
    // Core track finding logic in the constructed Hough space
    void runTrackFinding();
    // Relate the hits in the peak cell of the peak. Applies a cut on the found peaks.
    std::vector<HoughPeak> relateHitsToPeaks(std::vector<HoughPeak>& peaks) const;
    // Create hits to peaks confusion matrix
    std::vector<std::vector<unsigned short>> getHitsVsPeaksTable(const std::vector<HoughPeak>& peaks) const;
    // Returns the hit contribution of a TS at a certain cell (= peak/maximum cell)
    unsigned short getHitContribution(const cell_index& peakCell, const unsigned short hitIdx) const;
    // Extract relevant hit information (hitIdx, contribution, super layer, drift time)
    std::vector<ContributionInfo> extractContributionInfos(const std::vector<unsigned short>& peakHits) const;
    // Find the hit with the maximum contribution in a given super layer
    int getMaximumHitInSuperLayer(const std::vector<ContributionInfo>& contributionInfos, unsigned short superLayer) const;
    // Cut on the number of hit axial/stereo super layers
    bool checkHitSuperLayers(const HoughPeak& peak) const;
    // Transform the center of gravity (cells) into the estimated track parameters
    std::array<double, 3> getTrackParameterEstimate(const cell_index& peakCell) const;
    // Transform to physical units
    std::array<double, 3> transformTrackParameters(const std::array<double, 3>& estimatedParameters) const;
    // Transverse momentum (which is 1/omega, in GeV/c) to radius (in cm)
    static inline double getTrackRadius(double transverseMomentum) { return transverseMomentum * 1e11 / (3e8 * 1.5); }

    // NDFinder: Member data stores
    // Result: Vector of the found tracks
    std::vector<RawFinderTrack> m_rawFinderTracks;
    // TS-IDs of the hits in the current event: Elements = [0,2335] for 2336 TS in total
    std::vector<unsigned short> m_hitIDs;
    // SL-IDs of the hits in the current event: Elements = Super layer number in [0,1,...,8]
    std::vector<unsigned short> m_hitSLIDs;
    // Drift time of the priority wire.
    std::vector<short> m_priorityWireTime;
    // Counter for the number of hits in the current event
    unsigned short m_nHits{0};
    // Configuration parameters of the 3DFinder
    NDFinderParameters m_ndFinderParams;
    // Peak finding module
    NDFinderPeakFinder m_peakFinder;
    // LUTs from the conditions database
    DBObjPtr<CDCTriggerNDFinderLUTs> m_LUTsConditionsDB;

    /*
      Since the CDC wire pattern is repeated 32 times, the hit IDs are stored for 1/32 of the CDC only.
      The total number of 2336 TS corresponds to (41 axial + 32 stereo) * 32.
      The number of track bins is (for example): (omega, phi, cot) = (40, 384, 9)
      Note: The omega dimension here represents just sign(q)/(p_T[GeV/c]) (0.25 -> inf -> -inf -> -0.25 for 0 -> 19.5 -> 39)
    */

    // Track segments
    static constexpr unsigned short m_nTS = 2336; // Number of track segments
    static constexpr unsigned short m_nSL = 9; // Number of super layers
    static constexpr unsigned short m_nAxial = 41; // Number of unique axial track segments
    static constexpr unsigned short m_nStereo = 32; // Number of unique stereo track segments

    // Full Hough space bins
    static constexpr unsigned short m_nOmega = 40; // Bins in the phi dimension
    static constexpr unsigned short m_nPhi = 384; // Bins in the omega dimension
    static constexpr unsigned short m_nCot = 9; // Bins in the cot dimension

    // CDC symmetry in phi
    static constexpr unsigned short m_phiGeo = 32; // Repetition of the wire pattern

    // Phi sectors in the CDC
    static constexpr unsigned short m_nPhiSector = m_nPhi / m_phiGeo; // Bins of one phi sector (12)

    // Data structure/binning of the compressed hit pattern files (.txt LUT files)
    static constexpr unsigned short m_nPhiComp = 15; // Bins of compressed phi: phi_start, phi_width, phi_0, ..., phi_12
    static constexpr SectorBinning m_compAxialBins = {m_nOmega, m_nPhiComp, 1, m_nAxial}; // 40, 15, 1, 41
    static constexpr SectorBinning m_compStereoBins = {m_nOmega, m_nPhiComp, m_nCot, m_nStereo}; // 40, 15, 9, 32

    // Acceptance ranges + slot sizes to convert bins to track parameters (for getBinToVal method)
    static constexpr std::array<double, 2> m_omegaRange = {-4., 4.}; // 1/4 = 0.25 GeV (minimum transverse momentum)
    static constexpr std::array<double, 2> m_phiRange = {0., 11.25}; // One phi sector (360/32)
    // These are optimized for minSuperAxial = 3 and minSuperStereo = 2
    static constexpr std::array<double, 2> m_cotRange = {2.3849627654510415, -1.0061730449796316}; // => theta in [22.75, 135.18]
    static constexpr double m_binSizeOmega = (m_omegaRange[1] - m_omegaRange[0]) / m_nOmega; // 0.2
    static constexpr double m_binSizePhi = (m_phiRange[1] - m_phiRange[0]) / m_nPhiSector; // 0.9375
    static constexpr double m_binSizeCot = (m_cotRange[1] - m_cotRange[0]) / m_nCot; // -0.377
    static constexpr std::array<std::array<double, 2>, 3> m_acceptanceRanges = {m_omegaRange, m_phiRange, m_cotRange};
    static constexpr std::array<double, 3> m_binSizes = {m_binSizeOmega, m_binSizePhi, m_binSizeCot};

    // Arrays of the hit representations and pointer to the Hough space
    /*
      m_hitToSectorIDs: 2D array mapping TS-ID ([0, 2335]) to:

      - [0]: Orientation (1 = axial, 0 = stereo)
      - [1]: Relative wire ID in the sector ([0, 40] for axials, [0, 31] for stereos)
      - [2]: Relative phi-sector ID in the super layer ([0, 31] in each SL)
    */
    c2array m_hitToSectorIDs{boost::extents[m_nTS][3]};
    /*
      m_compAxialHitReps/m_compStereoHitReps (~ Compressed in phi (width, start, values)) 4D array mapping:

      1. [hitID]: Relative hit number of the track segment (axial [0, 40], stereo [0, 31])
      2. [omegaIdx]: Omega index of the Hough space ([0, m_nOmega - 1])
      3. [phiIdx]: Phi start value, number of phi bins, phi values (0, 1, [2, 14])
      4. [cotIdx]: Cot index of the Hough space (0 for axial TS, [0, m_nCot - 1] for stereo TS)

      to the Hough space weight contribution at the corresponding bin (int, [0, 7])
    */
    c4array m_compAxialHitReps{boost::extents[m_nAxial][m_nOmega][m_nPhiComp][1]};
    c4array m_compStereoHitReps{boost::extents[m_nStereo][m_nOmega][m_nPhiComp][m_nCot]};
    /*
      m_expAxialHitReps/m_expStereoHitReps (~ expansion of the compressed representations) 4D array mapping:

      1. [hitID]: Relative hit number of the track segment (axial [0, 40], stereo [0, 31])
      2. [omegaIdx]: Omega index of the Hough space ([0, m_nOmega - 1])
      3. [phiIdx]: Phi index of the Hough space ([0, m_nPhi - 1])
      4. [cotIdx]: Cot index of the Hough space ([0, m_nCot - 1] for both axial and stereo!)

      to the Hough space weight contribution at the corresponding bin (int, [0, 7])
    */
    c4array m_expAxialHitReps{boost::extents[m_nAxial][m_nOmega][m_nPhi][m_nCot]};
    c4array m_expStereoHitReps{boost::extents[m_nStereo][m_nOmega][m_nPhi][m_nCot]};

    // The complete Hough space with the size [m_nOmega, m_nPhi, m_nCot]
    c3array* m_houghSpace = nullptr;
  };
}
