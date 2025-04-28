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
#include <Math/Vector3D.h>
#include "trg/cdc/NDFinderDefs.h"
#include "trg/cdc/Clusterizend.h"

namespace Belle2 {

  // Struct of NDFinder parameters
  struct NDFinderParameters {
    // Required number of axial super layers
    unsigned short minSuperAxial;
    // Required number of stereo super layers
    unsigned short minSuperStereo;
    // Clustering: Minimum peak cell weight
    unsigned short minPeakWeight;
    // Clustering: Number of iterations for the cluster search in each Hough space quadrant
    unsigned short iterations;
    // Clustering: Number of deleted cells in each omega direction from the maximum
    unsigned short omegaTrim;
    // Clustering: Number of deleted cells in each phi direction from the maximum
    unsigned short phiTrim;
    // Switch for writing the full Hough space and the cluster information to the 3DFinderInfo class
    bool storeAdditionalReadout;
    // Axial and stereo hit representations that should be used
    std::string axialFile;
    std::string stereoFile;
  };

  // Struct containing the track segment (hit) information from the Track Segment Finder (TSF)
  struct HitInfo {
    unsigned short hitID;
    unsigned short hitSLID;
    unsigned short hitPrioPos;
    short hitPrioTime;
  };

  // Class for a found NDFinder track
  class NDFinderTrack {
  public:
    NDFinderTrack(std::array<double, 3> estimatedParameters,
                  SimpleCluster&& cluster,
                  std::vector<ROOT::Math::XYZVector>&& readoutHoughSpace,
                  std::vector<ROOT::Math::XYZVector>&& readoutCluster)
      : m_cluster(std::move(cluster)),
        m_houghSpace(std::move(readoutHoughSpace)),
        m_readoutCluster(std::move(readoutCluster))
    {
      m_omega = estimatedParameters[0]; // This is the "real" omega (curvature), i.e., sign(q)/(r_2d[cm])
      m_phi = estimatedParameters[1];
      m_cotCot = estimatedParameters[2];
    }

    // Get the track parameters (z always 0)
    double getOmega() const { return m_omega; }
    double getPhi0() const { return m_phi; }
    double getCot() const { return m_cotCot; }
    // Get the number of related Hits
    std::vector<unsigned short> getRelatedHits() const { return m_cluster.getClusterHits(); }
    // Hough space readout (if storeAdditionalReadout true)
    std::vector<ROOT::Math::XYZVector> getHoughSpace() const { return m_houghSpace; }
    // Cluster readout (if storeAdditionalReadout true)
    std::vector<ROOT::Math::XYZVector> getClusterReadout() const { return m_readoutCluster; }

  private:
    // 2D track curvature
    double m_omega;
    // 2D azimuthal angle
    double m_phi;
    // 3D polar angle
    double m_cotCot;
    // Vector of the indices of the related hits in the list of CDC hits (StoreArray<CDCHits>)
    std::vector<unsigned short> m_relHits;
    // Vector of the weights for each related hit.
    std::vector<double> m_hitWeights;
    // The found cluster of the track
    SimpleCluster m_cluster;
    // Vector storing the complete Hough space for analysis
    std::vector<ROOT::Math::XYZVector> m_houghSpace;
    // Vector storing cluster informations for analysis
    std::vector<ROOT::Math::XYZVector> m_readoutCluster;
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
      unsigned short nPriorityWires;
    };

    // Collection of the hit information needed for the hit representations
    struct WireInfo {
      unsigned short relativeWireID;
      unsigned short phiSectorStart;
      unsigned short priorityWire;
    };

    // Collection of the hit contribution information needed for the hit to cluster relations
    struct ContributionInfo {
      unsigned short hitIndex;
      unsigned short contribution;
      unsigned short superLayer;
      short priorityTime;
    };

    // Default constructor
    NDFinder() = default;

    // Destructor
    virtual ~NDFinder()
    {
      delete m_hitToSectorIDs;
      delete m_compAxialHitReps;
      delete m_compStereoHitReps;
      delete m_expAxialHitReps;
      delete m_expStereoHitReps;
      delete m_houghSpace;
    }

    // Initialization of the NDFinder (parameters and lookup tables)
    void init(const NDFinderParameters& ndFinderParameters);
    // Reset the NDFinder data structure to process next event
    void reset();
    // Add the hit info of a single track segment to the NDFinder
    void addHit(const HitInfo& hitInfo);
    // Main function for track finding
    void findTracks();
    // Retreive the results
    std::vector<NDFinderTrack>* getFinderTracks() { return &m_ndFinderTracks; }

    // NDFinder: Internal functions for track finding
  protected:
    // Initialize the arrays LUT arrays
    void initLookUpArrays();
    // Fills the m_hitToSectorIDs (see below) array with the hit to orientation/sectorWire/sectorID relations
    void initHitToSectorMap();
    // Fills the m_compAxialHitReps/m_compStereoHitReps (see below) arrays with the hit representations (hits to weights)
    void loadCompressedHitReps(const std::string& fileName, const SectorBinning& compBins, c5array& compHitsToWeights);
    // Fills the m_expAxialHitReps/m_expStereoHitReps (see below) arrays with the expanded hit representations (hits to weights)
    void fillExpandedHitReps(const SectorBinning& compBins, const c5array& compHitsToWeights, c5array& expHitsToWeights);
    // Process a single axial or stereo hit for the Hough space
    void processHitForHoughSpace(const unsigned short hitIdx);
    // Computes the phi bin of the sector start, given the relative SectorID. Saves the result in m_phiSectorStarts (see below).
    unsigned short computePhiSectorStart(unsigned short relativeSectorID);
    // Write (add) a single hit (Hough curve) to the Hough space
    void writeHitToHoughSpace(const WireInfo& hitInfo, const c5array& expHitsToWeights);
    // Core track finding logic in the constructed Hough space
    void runTrackFinding();
    // Relate the hits in the peak of the cluster to the cluster. Applies a cut on the clusters.
    std::vector<SimpleCluster> relateHitsToClusters(std::vector<SimpleCluster>& clusters);
    // Create hits to clusters confusion matrix
    std::vector<std::vector<unsigned short>> getHitsVsClustersTable(const std::vector<SimpleCluster>& clusters);
    // Returns the hit contribution of a TS at a certain cluster cell (= peak/maximum cell)
    unsigned short getHitContribution(const cell_index& peakCell, const unsigned short hitIdx);
    // Extract relevant hit information (hitIdx, contribution, super layer, drift time)
    std::vector<ContributionInfo> extractContributionInfos(const std::vector<unsigned short>& clusterHits);
    // Find the hit with the maximum contribution in a given super layer
    int getMaximumHitInSuperLayer(const std::vector<ContributionInfo>& contributionInfos, unsigned short superLayer);
    // Cut on the number of hit axial/stereo super layers
    bool checkHitSuperLayers(const SimpleCluster& cluster);
    // Calculate the center of gravity (weighted mean) for the track parameters
    std::array<double, 3> calculateCenterOfGravity(const SimpleCluster& cluster);
    // Transform the center of gravity (cells) into the estimated track parameters
    std::array<double, 3> getTrackParameterEstimate(const std::array<double, 3>& centerOfGravity);
    // Transform to physical units
    std::array<double, 3> transformTrackParameters(const std::array<double, 3>& estimatedParameters);
    // Transverse momentum (which is 1/omega, in GeV/c) to radius (in cm)
    static inline double getTrackRadius(double transverseMomentum) { return transverseMomentum * 1e11 / (3e8 * 1.5); }

    // NDFinder: Member data stores
  private:
    // Result: Vector of the found tracks
    std::vector<NDFinderTrack> m_ndFinderTracks;
    // TS-IDs of the hits in the current event: Elements = [0,2335] for 2336 TS in total
    std::vector<unsigned short> m_hitIDs;
    // SL-IDs of the hits in the current event: Elements = Super layer number in [0,1,...,8]
    std::vector<unsigned short> m_hitSLIDs;
    // Priority positon within the TS. Elements basf2: [0,3] first, left, right, no hit
    std::vector<unsigned short> m_priorityWirePos;
    // Drift time of the priority wire.
    std::vector<short> m_priorityWireTime;
    // Start phi-sector index of the hit representation (11/32) in full track parameter space
    std::vector<unsigned short> m_phiSectorStarts;
    // Counter for the number of hits in the current event
    unsigned short m_nHits{0};
    // Configuration parameters of the 3DFinder
    NDFinderParameters m_ndFinderParams;
    // Clustering module
    Clusterizend m_clusterer;

    /*
      Since the CDC wire pattern is repeated 32 times, the hit IDs are stored for 1/32 of the CDC only.
      The total number of 2336 TS corresponds to (41 axial + 32 stereo) * 32.
      The number of track bins (full phi) is: (omega, phi, cot) = (40, 384, 9)
      Note: The omega dimension here represents just sign(q)/(p_T[GeV/c]) (0.2 -> inf -> -inf -> -0.2 for 0 -> 19.5 -> 39)
    */

    // Track segments
    static constexpr unsigned short m_nTS = 2336; // Number of track segments
    static constexpr unsigned short m_nSL = 9; // Number of super layers
    static constexpr unsigned short m_nAxial = 41; // Number of unique axial track segments
    static constexpr unsigned short m_nStereo = 32; // Number of unique stereo track segments
    static constexpr unsigned short m_nPrio = 3; // Number of priority wires

    // Full Hough space bins
    static constexpr unsigned short m_nOmega = 40; // Bins in the phi dimension
    static constexpr unsigned short m_nPhi = 384; // Bins in the omega dimension
    static constexpr unsigned short m_nCot = 9; // Bins in the cot dimension

    // CDC symmetry in phi
    static constexpr unsigned short m_phiGeo = 32; // Repetition of the wire pattern
    static constexpr unsigned short m_nExpPhiSectors = 13; // Number of phi sectors defining the range of the expanded hits

    // Phi sectors in the CDC
    static constexpr unsigned short m_nPhiSector = m_nPhi / m_phiGeo; // Bins of one phi sector (12)
    static constexpr unsigned short m_nPhiComp = 15; // Bins of compressed phi: phi_start, phi_width, phi_0, ..., phi_12
    static constexpr unsigned short m_nPhiExp =  m_nExpPhiSectors * m_nPhiSector; // Bins of 11 phi sectors (132)

    // Binnings in different hit pattern arrays
    static constexpr SectorBinning m_compAxialBins = {m_nOmega, m_nPhiComp, 1, m_nAxial, m_nPrio}; // 40, 15, 1, 41, 3
    static constexpr SectorBinning m_compStereoBins = {m_nOmega, m_nPhiComp, m_nCot, m_nStereo, m_nPrio}; // 40, 15, 9, 32, 3
    static constexpr SectorBinning m_expAxialBins = {m_nOmega, m_nPhiExp, m_nCot, m_nAxial, m_nPrio}; // 40, 132, 9, 32, 3
    static constexpr SectorBinning m_expStereoBins = {m_nOmega, m_nPhiExp, m_nCot, m_nStereo, m_nPrio}; // 40, 132, 9, 32, 3
    static constexpr SectorBinning m_fullBins = {m_nOmega, m_nPhi, m_nCot, m_nTS, m_nPrio}; // 40, 384, 9, 2336, 3

    // Acceptance ranges + slot sizes to convert bins to track parameters (for getBinToVal method)
    static constexpr std::array<double, 2> m_omegaRange = {-4., 4.}; // 1/4 = 0.25 GeV (minimum transverse momentum)
    static constexpr std::array<double, 2> m_phiRange = {0., 11.25};
    static constexpr std::array<double, 2> m_cotRange = {1.8154040548776156, -0.7951509931203085};
    static constexpr double m_binSizeOmega = (m_omegaRange[1] - m_omegaRange[0]) / m_nOmega; // 0.2
    static constexpr double m_binSizePhi = (m_phiRange[1] - m_phiRange[0]) / m_nPhiSector; // 0.9375
    static constexpr double m_binSizeCot = (m_cotRange[1] - m_cotRange[0]) / m_nCot; // -0.29
    static constexpr std::array<std::array<double, 2>, 3> m_acceptanceRanges = {m_omegaRange, m_phiRange, m_cotRange};
    static constexpr std::array<double, 3> m_binSizes = {m_binSizeOmega, m_binSizePhi, m_binSizeCot};

    // Array pointers to the hit representations and Hough space
    /*
      m_hitToSectorIDs: 2D array mapping TS-ID ([0, 2335]) to:

      - [0]: Orientation (1 = axial, 0 = stereo)
      - [1]: Relative wire ID in the sector ([0, 40] for axials, [0, 31] for stereos)
      - [2]: Relative phi-sector ID in the super layer ([0, 31] in each SL)
    */
    c2array* m_hitToSectorIDs = nullptr;
    /*
      m_compAxialHitReps/m_compStereoHitReps (~ Compressed in phi (width, start, values)) 5D array mapping:

      1. [hitID]: Relative hit number of the track segment (axial [0, 40], stereo [0, 31])
      2. [priorityWire]: Hit priority wire ([0, 2])
      3. [omegaIdx]: Omega index of the Hough space ([0, 39])
      4. [phiIdx]: Phi start value, number of phi bins, phi values (0, 1, [2, 14])
      5. [cotIdx]: Cot index of the Hough space (0 for axial TS, [0, 8] for stereo TS)

      to the Hough space weight contribution at the corresponding bin (int, [0, 7])
    */
    c5array* m_compAxialHitReps = nullptr;
    c5array* m_compStereoHitReps = nullptr;
    /*
      m_expAxialHitReps/m_expStereoHitReps (~ Expands to 11/32 phi sectors) 5D array mapping:

      1. [hitID]: Relative hit number of the track segment (axial [0, 40], stereo [0, 31])
      2. [priorityWire]: Hit priority wire ([0, 2])
      3. [omegaIdx]: Omega index of the Hough space ([0, 39])
      4. [phiIdx]: The actual relative phi index in the Hough space ([0, 131]), phiSectorStart must be added
      5. [cotIdx]: Cot index of the Hough space ([0, 8] for both axial and stereo!)

      to the Hough space weight contribution at the corresponding bin (int, [0, 7])
    */
    c5array* m_expAxialHitReps = nullptr;
    c5array* m_expStereoHitReps = nullptr;
    // The complete Hough space with the size [m_nOmega, m_nPhi, m_nCot]
    c3array* m_houghSpace = nullptr;
  };
}
