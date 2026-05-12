/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <utility>
#include <array>
#include <boost/multi_array.hpp>

#define BOOST_MULTI_ARRAY_NO_GENERATORS

namespace Belle2 {

  // Typedefs for the Hough space
  typedef boost::multi_array<unsigned short, 3> c3array;
  typedef c3array::index c3index;
  typedef std::array<c3index, 3> cell_index;

  // Struct containing the parameters for the peak finding
  struct PeakFindingParameters  {
    // Number of iterations of the peak searching for each Hough space quadrant
    unsigned short iterations;
    // Number of deleted cells in omega in each direction of the peak
    unsigned short omegaTrim;
    // Number of deleted cells in phi in each direction of the peak
    unsigned short phiTrim;
    // The Hough space dimensions
    unsigned short nOmega;
    unsigned short nPhi;
    unsigned short nCot;
  };

  // Struct for a found peak and its hits
  struct HoughPeak {
    std::vector<unsigned short> hits;
    cell_index cell{0, 0, 0};
    unsigned int weight{0};
  };


  // Peak finding module
  class NDFinderPeakFinder {
  public:
    // Struct containing the deletion bounds of a omega row
    struct DeletionBounds {
      c3index phiLowerBound;
      c3index phiUpperBound;
      c3index omega;
      c3index cot;
    };

    // Default constructor
    NDFinderPeakFinder() = default;

    // To set custom peak finding parameters
    explicit NDFinderPeakFinder(const PeakFindingParameters& parameters): m_peakFindingParams(parameters) {}

    // Set a new Hough space for peak and track finding
    void setNewPlane(c3array& houghSpace) { m_houghSpace = &houghSpace; }
    // Create all the peaks in the Hough space
    std::vector<HoughPeak> findPeaks();

  private:
    // Get the phi bounds of one quadrant section
    std::array<c3index, 2> getSectionBounds(const unsigned short quadrant, const unsigned section);
    // Iterate m_peakFindingParams.iterations times over one section
    void iterateOverSection(const std::array<c3index, 2>& sectionBounds, std::vector<HoughPeak>& candidatePeaks);
    // Finds the global section peak index
    HoughPeak getSectionPeak(const std::array<c3index, 2>& sectionBounds);
    // Deletes the surroundings of such a peak (if iterations > 1)
    void deletePeakSurroundings(const HoughPeak& peak);
    // Method to delete a omega row for the peak cluster deletion in deletePeakSurroundings method
    void clearHoughSpaceRow(const DeletionBounds& bounds);
    // The struct holding the peak finding parameters
    PeakFindingParameters m_peakFindingParams;
    // Pointer to the Hough space
    c3array* m_houghSpace{0};
  };
}
