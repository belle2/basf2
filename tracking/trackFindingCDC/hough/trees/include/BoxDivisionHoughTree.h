/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/hough/trees/WeightedFastHoughTree.h>
#include <tracking/trackFindingCDC/hough/baseelements/SectoredLinearDivision.h>

#include <tracking/trackFindingCDC/numerics/LookupTable.h>
#include <tracking/trackFindingCDC/utilities/EvalVariadic.h>

#include <tracking/trackFindingCDC/utilities/TupleGenerate.h>

#include <type_traits>
#include <utility>
#include <tuple>
#include <array>
#include <memory>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  A fast hough algorithm with rectangular boxes, which are split linearly by a fixed number of
     *  divisions in each coordinate up to a maximal level.
     */
    template <class AItemPtr, class AHoughBox, size_t ... divisions>
    class BoxDivisionHoughTree {

    public:
      /// Type of the box in the hough space
      using HoughBox = AHoughBox;

      /// Type of the box division strategy
      using BoxDivision = SectoredLinearDivision<HoughBox, divisions...>;

      /// Type of the fast hough tree structure
      using HoughTree = WeightedFastHoughTree<AItemPtr, HoughBox, BoxDivision>;

      /// Type of the coordinate I.
      template <size_t I>
      using Type = typename HoughBox::template Type<I>;

      /// Predicate that the given type is indeed a coordinate of the hough space
      template <class T>
      using HasType = typename HoughBox::template HasType<T>;

      /// Function to get the coordinate index from its type
      template <class T>
      using TypeIndex = typename HoughBox::template TypeIndex<T>;

      /// Type of the width in coordinate I.
      template <size_t I>
      using Width = typename HoughBox::template Width<I>;

      /// Type of the nodes used in the tree for the search.
      using Node = typename  HoughTree::Node;

    public:
      /// Constructor using the given maximal level.
      explicit BoxDivisionHoughTree(int maxLevel, int sectorLevelSkip = 0)
        : m_maxLevel(maxLevel)
        , m_sectorLevelSkip(sectorLevelSkip)
        , m_overlaps((divisions * 0)...)
      {
      }

    private:
      /// Type of the discrete value array to coordinate index I.
      template <size_t I>
      using Array = typename Type<I>::Array;

      /// Tuple type of the discrete value arrays
      using Arrays = TupleGenerateN<Array, sizeof...(divisions)>;

    public:
      /// Getter the number of divisions at each level for coordinate index I.
      size_t getDivision(size_t i) const
      {
        return m_divisions[i];
      }

      /**
       *  Construct the discrete value array at coordinate index I
       *
       *  This function is only applicable for discrete axes.
       *  For continuous axes assignArray should be call with an array containing only the
       *  lower and upper bound of the axes range and an optional overlap.
       *
       *  @param lowerBound  Lower bound of the value range
       *  @param upperBound  Upper bound of the value range
       *  @param nBinOverlap Overlap of neighboring bins. Default is no overlap.
       *                     Usuallly this is counted in number of discrete values
       *  @param nBinWidth   Width of the bins at lowest level. Default is width of 1.
       *                     Usually this is counted in numbers of discrete values
       */
      template <size_t I>
      void constructArray(double lowerBound,
                          double upperBound,
                          Width<I> nBinOverlap = 0,
                          Width<I> nBinWidth = 0)
      {
        static_assert(std::is_integral<Width<I> >::value, "Method only applicable to discrete axes");
        const size_t division = getDivision(I);
        const int granularityLevel = m_maxLevel + m_sectorLevelSkip;
        const size_t nBins = std::pow(division, granularityLevel);

        if (nBinWidth == 0) {
          nBinWidth = nBinOverlap + 1;
        }

        B2ASSERT("Width " << nBinWidth << "is not bigger than overlap " << nBinOverlap,
                 nBinOverlap < nBinWidth);

        const auto nPositions = (nBinWidth - nBinOverlap) * nBins + nBinOverlap + 1;
        std::get<I>(m_arrays) = linspace<float>(lowerBound, upperBound, nPositions);
        std::get<I>(m_overlaps) = nBinOverlap;
      }

      /// Provide an externally constructed array by coordinate index
      template <size_t I>
      void assignArray(Array<I> array, Width<I> overlap = 0)
      {
        std::get<I>(m_arrays) = std::move(array);
        std::get<I>(m_overlaps) = overlap;

        // In case of a discrete axes, check whether the size of the array is sensible
        // such that the bin width at the highest granularity level is a whole number given the overlap.
        if (std::is_integral<Width<I> >::value) {
          const int division = getDivision(I);
          const int granularityLevel = m_maxLevel + m_sectorLevelSkip;
          const long int nBins = std::pow(division, granularityLevel);
          const long int nPositions = std::get<I>(m_arrays).size();
          const long int nWidthTimeNBins = nPositions - 1 + (nBins - 1) * overlap;

          B2ASSERT("Not enough positions in array to cover all bins.\n"
                   "Expected: positions = " << nBins - (nBins - 1) * overlap + 1 << " at least.\n"
                   "Actual:   positions = " << nPositions << " (overlap = " << overlap << ", bins = " << nBins << ")\n",
                   nWidthTimeNBins >= nBins);

          B2ASSERT("Number of positions in array introduces inhomogeneous width at the highest granularity level.\n"
                   "Expected: positions = 'width * bins - (bins - 1) * overlap + 1'\n"
                   "Actual:   positions = " << nPositions << " (overlap = " << overlap << ", bins = " << nBins << ")\n",
                   nWidthTimeNBins % nBins == 0);
        }
      }

      /// Provide an externally constructed array by coordinate type
      template <class T>
      std::enable_if_t< HasType<T>::value, void>
      assignArray(Array<TypeIndex<T>::value > array, Width<TypeIndex<T>::value > overlap = 0)
      {
        assignArray<TypeIndex<T>::value>(std::move(array), overlap);
      }

    public:
      /// Initialise the algorithm by constructing the hough tree from the parameters
      void initialize()
      {
        // Compose the hough space
        HoughBox houghPlane = constructHoughPlane();
        BoxDivision boxDivision(m_overlaps, m_sectorLevelSkip);
        m_houghTree.reset(new HoughTree(std::move(houghPlane), std::move(boxDivision)));
      }

      /// Prepare the leave finding by filling the top node with given hits
      template <class AItemPtrs>
      void seed(const AItemPtrs& items)
      {
        if (not m_houghTree) initialize();
        m_houghTree->seed(items);
      }

      /// Terminates the processing by striping all hit information from the tree
      void fell()
      {
        m_houghTree->fell();
      }

      /// Release all memory that the tree aquired during the runs.
      void raze()
      {
        m_houghTree->raze();
      }

    public:
      /// Getter for the tree used in the search in the hough plane.
      HoughTree* getTree() const
      {
        return m_houghTree.get();
      }

      /// Getter for the currently set maximal level
      int getMaxLevel() const
      {
        return m_maxLevel;
      }

      /// Setter maximal level of the hough tree.
      void setMaxLevel(int maxLevel)
      {
        m_maxLevel = maxLevel;
      }

      /// Getter for number of levels to skip in first level to form a finer sectored hough space.
      int getSectorLevelSkip() const
      {
        return m_sectorLevelSkip;
      }

      /// Setter for number of levels to skip in first level to form a finer sectored hough space.
      void setSectorLevelSkip(int sectorLevelSkip)
      {
        m_sectorLevelSkip = sectorLevelSkip;
      }

      /// Getter for the array of discrete value for coordinate I.
      template <size_t I>
      const Array<I>& getArray() const
      {
        return std::get<I>(m_arrays);
      }

    private:
      /// Construct the box of the top node of the tree. Implementation unroling the indices.
      template <size_t... Is>
      HoughBox constructHoughPlaneImpl(const std::index_sequence<Is...>& is __attribute__((unused)))
      {
        return HoughBox(Type<Is>::getRange(std::get<Is>(m_arrays))...);
      }

      /// Construct the box of the top node of the tree.
      HoughBox constructHoughPlane()
      {
        return constructHoughPlaneImpl(std::make_index_sequence<sizeof...(divisions)>());
      }

    private:
      /// Number of the maximum tree level.
      int m_maxLevel;

      /// Number of levels to skip in first level to form a finer sectored hough space.
      int m_sectorLevelSkip;

      /// Array of the number of divisions at each level
      const std::array<size_t, sizeof ...(divisions)> m_divisions = {{divisions ...}};

      /// An tuple of division overlaps in each coordinate.
      typename HoughBox::Delta m_overlaps;

      /// A tuple of value arrays providing the memory for the discrete bin bounds.
      Arrays m_arrays;

      /// Dynamic hough tree structure traversed in the leaf search.
      std::unique_ptr<HoughTree> m_houghTree = nullptr;
    };
  }
}
