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
#include <Eigen/Dense>
#include <Eigen/Geometry>

#include <framework/datastore/StoreArray.h>

#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  /**
   * Track finding algorithm class for linear tracks produced by cosmics in the VXD without magnetic field.
   * The algorithm uses a simple principal component analysis approach to find a momentum and position seed
   * assuming the presence of only the one track produced by the cosmic in the detector.
   * SpacePoints produced by detector noise can be discarded by the TF by removing the biggest outlier until
   * an adequate reduced chi squared value is reached.
   * The track finder requires a minimal number of SpacePoints for a track, a reduced chi squared value below a given
   * threshold, and the number of rejected SpacePoints to be below a given maximum.
   */
  class StandaloneCosmicsCollector {

  public:
    /// Constructor
    StandaloneCosmicsCollector() = default;

    /// Destructor
    ~StandaloneCosmicsCollector() = default;


    /**
     * Set sorting mode used in addSpacePoints. One can choose to sort
     * 1: by radius
     * 2: by x-coordinate
     * 3: by y-coordinate
     * by calling this function with the respective index. If the function is not used,
     * the default set at the declaration of m_sortingMode is used.
     */
    void setSortingMode(unsigned short index)
    {
      if ((index < 1) or (index > 3)) {
        B2FATAL("Invalid sorting mode chosen! You used " << m_sortingMode
                << ". Must be one of 1 (by radius), 2 (by x) or 3 (by y).");
      }
      m_sortingMode = index;
    }

    /**
     * Function to initialize the track finder anew for an event with its set of SpacePoints
     * provided via their StoreArray.
     * This also resets all internal variables for the new event.
     * @param SPs StoreArray of SpacePoints of the event to be analyzed.
     */
    void addSpacePoints(std::vector<StoreArray<SpacePoint>> SPs)
    {
      m_spacePoints.clear();
      m_direction.clear();
      m_start.clear();
      m_reducedChi2 = 10;
      for (auto& spArray : SPs) {
        for (auto& sp : spArray) {
          addSpacePoint(&sp);
        }
      }
    }


    /**
     * Function to perform the actual line fit based on the StoreArray of SpacePoints provided.
     * The fit performs a principal component analysis based on the provided 3D-points, refitting the SpacePoints
     * again with the worst Point removed, if the given qualityCut on the reduced chi squared value is not reached.
     * This is done until the requirement is met, or a maximal number of rejected SpacePoints (maxRejected) is reached.
     * Furthermore the fitting is aborted, if not a minimal number of SpacePoints (minSPs) is provided.
     * The function returns true, if a track candidate fulfilling these requirements is found.
     * @param qualityCut
     * @param maxRejected
     * @param minSPs
     * @return boolean true, if a sufficient track candidate is found, else false.
     */
    bool doFit(double qualityCut, int maxRejected, int minSPs)
    {
      bool fitting = true;
      int rejected = 0;

      // cppcheck-suppress knownConditionTrueFalse
      while (m_reducedChi2 > qualityCut && fitting) {
        fitting = doLineFit(minSPs);
        if (not fitting) {return false;}
        if (m_reducedChi2 > qualityCut) {
          B2DEBUG(20, "Refitting without sp with index " << m_largestChi2.second
                  << " and chi2 contribution " << m_largestChi2.first << "...");
          m_spacePoints.erase(m_spacePoints.begin() + m_largestChi2.second);
          rejected++;
        }
        if (rejected > maxRejected) { B2DEBUG(20, "Rejected " << rejected << "!"); return false; }
      }
      return fitting;
    }


    /**
     * Getter for the position and momentum seed resulting from the linear fit. The momentum is
     * rather only a vector giving the direction of the straight fit with a sufficiently large magnitude.
     * The position seed is set to be the outermost SpacePoint with the direction seed pointing towards the
     * next inner hit.
     * If the fit does not meet the given requirements, the result of the last try for this event is returned.
     * If the event does not have the required minimal number of SpacePoints, vectors with zeros are returned.
     * @return pair of vectors of double. First is position seed, second the direction/momentum seed.
     */
    std::pair<std::vector<double>, std::vector<double>> getResult()
    {
      return std::pair<std::vector<double>, std::vector<double>> (m_start, m_direction);
    }


    /**
     * Getter for the sorted list of SpacePoints used for the final fit which met the given requirements.
     * The SpacePoints are sorted, starting at the position seed and towards the direction obtained by the final fit.
     * If the fit did not meet the requirements, the last set which was used for the current event is returned.
     * @return vector of sorted SpacePoints used for the final fit.
     */
    std::vector<const SpacePoint*> getSPTC()
    {
      return m_spacePoints;
    }


    /**
     * Getter for the final reduced chi squared value obtained for the set of SpacePoints used for the last
     * performed fit.
     * @return double chi square value for the last fit performed for the event.
     */
    double getReducedChi2()
    {
      return m_reducedChi2;
    }


  private:

    /**
     * Adding single SpacePoint to the sorted member vector m_spacePoints, beginning with the SpacePoint with the
     * smallest radius sqrt(x^2 + y^2).
     * @param SP
     */
    void addSpacePoint(const SpacePoint* SP)
    {
      auto forwardIt = std::lower_bound(m_spacePoints.begin(), m_spacePoints.end(), SP,
                                        [this](const SpacePoint * lhs, const SpacePoint * rhs)
                                        -> bool { return this->compareRads(lhs, rhs); });
      m_spacePoints.insert(forwardIt, SP);
    }


    /**
     * Function performing the actual line fit via a principal component analysis methode yielding a direction vector
     * based on the eigen vector corresponding to the largest eigenvalue and a seed position calculated as the mean of
     * all given SpacePoints.
     * The function sets the member m_reducedChi2 which is calculated based on the distance of all given points to the
     * fitted line. The final value is divided by the number of SPs in the track candidate, yielding a reduced chi2.
     * If the provided SpacePoints are less than a given minimal length the fit will be aborted returning false.
     * If enough SpacePoints are provided, the function returns true independent from the value of the calculated
     * reduced chi2.
     * @param minSPs
     * @return boolean
     */
    bool doLineFit(int minSPs)
    {
      int nHits = m_spacePoints.size();
      B2DEBUG(20, "Trying fit with " << nHits << " hits...");
      // Aborting fit and returning false if the minimal number of SpacePoints required is not met.
      if (nHits < minSPs) { B2DEBUG(20, "Only " << nHits << " hits!"); return false; };

      Eigen::Matrix<double, 3, 1> average = Eigen::Matrix<double, 3, 1>::Zero(3, 1);
      Eigen::Matrix<double, Eigen::Dynamic, 3> data = Eigen::Matrix<double, Eigen::Dynamic, 3>::Zero(nHits, 3);
      Eigen::Matrix<double, Eigen::Dynamic, 3> P = Eigen::Matrix<double, Eigen::Dynamic, 3>::Zero(nHits, 3);

      for (const auto& sp : m_spacePoints) {
        average(0) += sp->getPosition().X();
        average(1) += sp->getPosition().Y();
        average(2) += sp->getPosition().Z();
      }
      average *= 1. / nHits;

      int index = 0;
      for (const auto& sp : m_spacePoints) {
        data(index, 0) = sp->getPosition().X();
        data(index, 1) = sp->getPosition().Y();
        data(index, 2) = sp->getPosition().Z();

        P(index, 0) = sp->getPosition().X() - average(0);
        P(index, 1) = sp->getPosition().Y() - average(1);
        P(index, 2) = sp->getPosition().Z() - average(2);
        index++;
      }

      //Principal component analysis
      Eigen::Matrix<double, 3, 3> product = P.transpose() * P;

      Eigen::EigenSolver<Eigen::Matrix<double, 3, 3>> eigencollection(product);
      Eigen::Matrix<double, 3, 1> eigenvalues = eigencollection.eigenvalues().real();
      Eigen::Matrix<std::complex<double>, 3, 3> eigenvectors = eigencollection.eigenvectors();
      Eigen::Matrix<double, 3, 1>::Index maxRow, maxCol;
      eigenvalues.maxCoeff(&maxRow, &maxCol);

      Eigen::Matrix<double, 3, 1> e = eigenvectors.col(maxRow).real();

      // Setting starting point to the data point with larges radius
      Eigen::Matrix<double, 3, 1> start = data.row(nHits - 1).transpose();
      Eigen::Matrix<double, 3, 1> second = data.row(nHits - 2).transpose();
      m_start = {start(0), start(1), start(2)};
      // Setting direction vector towards the inside of the detector
      m_direction = {e(0), e(1), e(2)};
      Eigen::Hyperplane<double, 3> plane(e.normalized(), start);
      Eigen::ParametrizedLine<double, 3> line(second, e.normalized());
      double factor = line.intersectionParameter(plane);
      if (factor > 0) {
        m_direction = { -e(0), -e(1), -e(2)};
      }

      // Resorting SpacePoints based on the obtained line fit result.
      resortHits();

      // Calculating reduced chi2 value of the line fit using the distances of the SpacePoints to the obtained line and
      // keeping the m_spacePoints index and the chi2 contribution of the SpacePoint with the largest contribution.
      m_reducedChi2 = 0;
      m_largestChi2 = std::pair<double, int>(0., 0);
      int largestChi2_index = 0;
      for (const auto& sp : m_spacePoints) {
        Eigen::Matrix<double, 3, 1> origin(sp->getPosition().X(), sp->getPosition().Y(), sp->getPosition().Z());
        plane = Eigen::Hyperplane<double, 3>(e.normalized(), origin);

        Eigen::Matrix<double, 3, 1> point = line.intersectionPoint(plane);

        double delta_chi2 = (point - origin).transpose() * (point - origin);
        m_reducedChi2 += delta_chi2;

        if (delta_chi2 > m_largestChi2.first) {
          m_largestChi2.first = delta_chi2;
          m_largestChi2.second = largestChi2_index;
        }
        largestChi2_index++;
      }

      m_reducedChi2 *= 1. / nHits;
      B2DEBUG(20, "Reduced chi2 result is " << m_reducedChi2 << "...");
      return true;
    }


    /**
     * Compare function used by addSpacePoint to sort the member vector of SpacePoints m_spacePoints by the radius
     * calculated with x^2 + y^2. The square root is omitted, as it does not matter for the comparison.
     * @param a left hand side SpacePoint
     * @param b right hand side SpacePoint
     * @return true if radius of a smaller than radius of b
     */
    bool compareRads(const SpacePoint* a, const SpacePoint* b)
    {
      if (m_sortingMode == 1) {
        double radA = a->getPosition().X() * a->getPosition().X() + a->getPosition().Y() * a->getPosition().Y();
        double radB = b->getPosition().X() * b->getPosition().X() + b->getPosition().Y() * b->getPosition().Y();
        return radA < radB;
      } else if (m_sortingMode == 2) {
        double xA = a->getPosition().X();
        double xB = b->getPosition().X();
        return xA < xB;
      } else if (m_sortingMode == 3) {
        double yA = a->getPosition().Y();
        double yB = b->getPosition().Y();
        return yA < yB;
      } else {
        B2FATAL("Invalid sorting mode chosen! You used " << m_sortingMode
                << ". Must be one of 1 (by radius), 2 (by x) or 3 (by y).");
      }
    }


    /**
     * Function to resort the member vector of SpacePoints m_spacePoints based on the members m_start and m_direction
     * obtained by the fit, so that the track candidate starts form the outermost hit and the following hits are sorted
     * following the direction m_direction.
     * This sorting is done using the comparison function comparePars.
     */
    void resortHits()
    {
      std::vector<const SpacePoint*> sortedSPs;
      for (auto& SP : m_spacePoints) {
        auto forwardIt = std::lower_bound(sortedSPs.begin(), sortedSPs.end(), SP,
        [this](const SpacePoint * lhs, const SpacePoint * rhs) -> bool {
          return this->comparePars(lhs, rhs);
        });
        sortedSPs.insert(forwardIt, SP);
      }
      m_spacePoints = sortedSPs;
    }


    /**
     * Comparison function to compare two SpacePoints based on the distance between the start point of the fitted line
     * m_start and their intersect with the fitted line.
     * @param a left hand side SpacePoint
     * @param b right hand side SpacePoint
     * @return boolean true if a is closer to the start of the fitted line than b
     */
    bool comparePars(const SpacePoint* a, const SpacePoint* b)
    {
      Eigen::Matrix<double, 3, 1> posA(a->getPosition().X(), a->getPosition().Y(), a->getPosition().Z());
      Eigen::Matrix<double, 3, 1> posB(b->getPosition().X(), b->getPosition().Y(), b->getPosition().Z());

      Eigen::Matrix<double, 3, 1> direction(m_direction[0], m_direction[1], m_direction[2]);
      Eigen::Matrix<double, 3, 1> origin(m_start[0], m_start[1], m_start[2]);

      Eigen::ParametrizedLine<double, 3> line(origin, direction.normalized());
      Eigen::Hyperplane<double, 3> planeA(direction.normalized(), posA);
      Eigen::Hyperplane<double, 3> planeB(direction.normalized(), posB);

      double parA = line.intersectionParameter(planeA);
      double parB = line.intersectionParameter(planeB);
      return parA < parB;
    }


    /// Member vector of SpacePoints holding the SpacePoints considered for the track candidate.
    std::vector<const SpacePoint*> m_spacePoints;

    /** Storing identifier for sorting algorithm to be used for the function addSpacePoint.
     *  1: by radius (default)
     *  2: by x-value
     *  3: by y-value
     */
    unsigned short m_sortingMode = 1;

    /** Member variable containing the reduced chi squared value of the current line fit.
     *  The value is obtained by dividing the actual chi2 value by the number of SPs in the track candidate.
     *  Is reset to 10 in addSpacePoints.
     */
    double m_reducedChi2 = 10;

    /**
     * Pair containing the index of the vector m_spacePoints for the SpacePoint with the largest contribution to the
     * chi2 value of the last fit and the respectiv contribution
     * first: contribution to the total chi2 value of the last fit
     * second: index of the SpacePoint with this largest contribution in the vector m_spacePoints.
     */
    std::pair<double, int> m_largestChi2 = std::pair<double, int>(0., 0);

    /// Start point obtained by the last performed line fit.
    std::vector<double> m_start {0., 0., 0.};

    /// Direction of the line obtained by the last performed line fit.
    std::vector<double> m_direction {0., 0., 0.};
  };
}

