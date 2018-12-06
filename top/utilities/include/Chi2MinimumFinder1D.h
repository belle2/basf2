/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <string>
#include <framework/logging/Logger.h>
#include <TH1F.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Minimum finder using tabulated chi^2 values in one dimension
     */
    class Chi2MinimumFinder1D {

    public:

      /**
       * Result of minimum finder
       */
      struct Minimum {
        double position = 0; /**< position of the minimum */
        double error = 0;    /**< error on the position */
        double chi2 = 0;     /**< chi2 at minimum */
        bool valid = false;  /**< valid minimum found */

        /**
         * Default constructor
         */
        Minimum()
        {}

        /**
         * Full constructor
         */
        Minimum(double pos, double err, double chi2_min, bool valid_flag):
          position(pos), error(err), chi2(chi2_min), valid(valid_flag)
        {}
      };


      /**
       * Class constructor, similar to 1D histogram
       * @param nbins number of points the search region is divided to
       * @param xmin lower limit of the search region
       * @param xmax upper limit of the search region
       */
      Chi2MinimumFinder1D(int nbins, double xmin, double xmax);

      /**
       * Clear chi^2 values
       */
      void clear()
      {
        m_chi2.clear();
        m_searched = false;
      }

      /**
       * Add chi^2 value to bin i
       * @param i bin index (0-based)
       * @param chi2 value to be added
       */
      void add(unsigned i, double chi2);

      /**
       * Returns lower limit of search region
       * @return xmin
       */
      double getXmin() const {return m_xmin;}

      /**
       * Returns upper limit of search region
       * @return xmax
       */
      double getXmax() const {return m_xmax;}

      /**
       * Returns number of bins
       * @return number of bins
       */
      int getNbins() const {return m_x.size();}

      /**
       * Returns bin (or step) size
       * @return bin size
       */
      double getBinSize() const {return m_dx;}

      /**
       * Returns vector of bin centers
       * @return bin centers
       */
      const std::vector<double>& getBinCenters() const {return m_x;}

      /**
       * Returns vector of chi^2
       * @return chi^2 values
       */
      const std::vector<double>& getChi2Values() const {return m_chi2;}

      /**
       * Returns parabolic minimum
       */
      const Minimum& getMinimum()
      {
        if (!m_searched) {
          findMinimum();
          m_searched = true;
        }
        return m_minimum;
      }

      /**
       * Returns chi^2 packed into 1D histogram
       * @param name histogram name
       * @param title histogram title
       */
      TH1F getHistogram(std::string name, std::string title) const;

    private:

      /**
       * Finds minimum
       */
      void findMinimum();

      /**
       * Calculates minimum using parabolic interpolation
       * @param yLeft bin content of left-to-minimal bin
       * @param yCenter bin content of minimal bin
       * @param yRight bin content of right-to-minimal bin
       * @return minimum given as a fraction of step to be added to central bin position
       */
      Minimum getMinimum(double yLeft, double yCenter, double yRight) const;

      double m_xmin = 0; /**< lower limit of search region */
      double m_xmax = 0; /**< upper limit of search region */

      double m_dx = 0; /**< bin size */
      std::vector<double> m_x;  /**< bin centers */
      std::vector<double> m_chi2; /**< chi^2 values at bin centers */

      bool m_searched = false; /**< internal flag */
      Minimum m_minimum; /**< result: global minimum */

    };

  } // namespace TOP
} // namespace Belle2
