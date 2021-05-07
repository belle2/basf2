/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/reconstruction_cpp/RaytracerBase.h>
#include <top/reconstruction_cpp/InverseRaytracer.h>
#include <top/reconstruction_cpp/PixelPositions.h>
#include <top/reconstruction_cpp/PixelMasks.h>
#include <top/reconstruction_cpp/PixelEfficiencies.h>
#include <top/reconstruction_cpp/EnergyMask.h>
#include <vector>
#include <algorithm>


namespace Belle2 {
  namespace TOP {

    /**
     * Utility for expanding the PDF in y direction.
     */
    class YScanner : public RaytracerBase {

    public:

      /**
       * Derivatives
       */
      struct Derivatives {
        double dLen_dx = 0; /**< propagation length over photon detection coordinate x */
        double dLen_de = 0; /**< propagation length over photon energy */
        double dLen_dL = 0; /**< propagation length over running parameter of particle trajectory */
        double dyD_dx = 0;  /**< unfolded y coordinate at detection over photon detection coordinate x */
        double dyD_de = 0;  /**< unfolded y coordinate at detection over photon energy */
        double dyD_dL = 0;  /**< unfolded y coordinate at detection over running parameter of particle trajectory */
        double dyB_dx = 0;  /**< unfolded y coordinate at prism entrance over photon detection coordinate x */
        double dyB_de = 0;  /**< unfolded y coordinate at prism entrance over photon energy */
        double dyB_dL = 0;  /**< unfolded y coordinate at prism entrance over running parameter of particle trajectory */
        double dFic_dx = 0; /**< Cerenkov azimuthal angle over photon detection coordinate x */
        double dFic_de = 0; /**< Cerenkov azimuthal angle over photon energy */
        double dFic_dL = 0; /**< Cerenkov azimuthal angle over running parameter of particle trajectory */

        /**
         * Default constructor
         */
        Derivatives()
        {}

        /**
         * Constructor
         * @param sol central solution
         * @param sol_dx solution displaced by dx
         * @param sol_de solution displaced by de
         * @param sol_dL solution dispalced by dL
         */
        Derivatives(const InverseRaytracer::Solution& sol,
                    const InverseRaytracer::Solution& sol_dx,
                    const InverseRaytracer::Solution& sol_de,
                    const InverseRaytracer::Solution& sol_dL);

        /**
         * Calculates the derivative of propagation length
         * @param sol0 central solution
         * @param sol1 displaced solution
         * @return the derivative
         */
        double dLen_d(const InverseRaytracer::Solution& sol0, const InverseRaytracer::Solution& sol1);

        /**
         * Calculates the derivative of unfolded y coordinate at detection
         * @param sol0 central solution
         * @param sol1 displaced solution
         * @return the derivative
         */
        double dyD_d(const InverseRaytracer::Solution& sol0, const InverseRaytracer::Solution& sol1);

        /**
         * Calculates the derivative of unfolded y coordinate at prism entrance
         * @param sol0 central solution
         * @param sol1 displaced solution
         * @return the derivative
         */
        double dyB_d(const InverseRaytracer::Solution& sol0, const InverseRaytracer::Solution& sol1);

        /**
         * Calculates the derivative of Cerenkov azimuthal angle
         * @param sol0 central solution
         * @param sol1 displaced solution
         * @return the derivative
         */
        double dFic_d(const InverseRaytracer::Solution& sol0, const InverseRaytracer::Solution& sol1);
      };


      /**
       * Table entry
       */
      struct TableEntry {
        double y = 0;   /**< value */
        double x = 0;   /**< mean x */
        double xsq = 0; /**< mean x^2 */

        /**
         * Constructor
         * @param Y value
         * @param X mean x
         * @param Xsq mean x^2
         */
        TableEntry(double Y, double X, double Xsq):
          y(Y), x(X), xsq(Xsq)
        {}
      };


      /**
       * A table of equidistant entries
       */
      struct Table {
        double x0 = 0;         /**< x of first entry */
        double step = 0;       /**< step size */
        std::vector<TableEntry> entries; /**< table entries */

        /**
         * Default constructor
         */
        Table()
        {}

        /**
         * Clear the content entirely
         */
        void clear();

        /**
         * Sets the first x and the step, and clears the entries
         * @param X0 x of the first entry
         * @param Step step size
         */
        void set(double X0, double Step);

        /**
         * Sets the first x and the step, and clears the entries
         * @param T the table
         */
        void set(const Table& T);

        /**
         * Returns x for a given index
         * @return x
         */
        double getX(int i) const;

        /**
         * Returns x of the first entry
         * @return minimal x
         */
        double getXmin() const;

        /**
         * Returns x of the last entry
         * @return maximal x
         */
        double getXmax() const;

        /**
         * Returns index
         * @return index (note: the range is not limited to the range of entries)
         */
        int getIndex(double x) const;

        /**
         * Returns y for a given index
         * @return y or 0 if index is out of range
         */
        double getY(int i) const;
      };


      /**
       * Down-stream projection of a pixel to prism entrance window w/ a clip on bar exit thickness.
       */
      struct PixelProjection {
        double yc = 0; /**< center in y of clipped pixel projection */
        double Dy = 0; /**< size in y of clipped pixel projection */
        const EnergyMask* mask = 0; /**< the corresponding energy mask */

        /** operator "less than" needed for sorting */
        bool operator<(const PixelProjection& other) const {return yc < other.yc;}
      };


      /**
       * Single PDF peak data
       */
      struct Result {
        int pixelID = 0;  /**< pixel ID (1-based) */
        double sum = 0;   /**< peak area proportional to number of photons */
        double e0 = 0;    /**< mean photon energy of the peak */
        double sigsq = 0; /**< width of the peak squared, in photon energy units */

        /**
         * Constructor with pixel ID
         * @param ID pixel ID (1-based)
         */
        explicit Result(int ID): pixelID(ID)
        {}

        /**
         * Sets the mean and width-squared from the accumulated values
         */
        void set();
      };


      /**
       * Class constructor.
       * Sets pixel positions of a given module and nominal photon detection efficiency.
       * @param moduleID slot ID
       * @param N size of nominal photon detection efficiency table
       */
      explicit YScanner(int moduleID, unsigned N = 64);

      /**
       * Sets parameters for selection between expand methods.
       * @param maxReflections maximal number of reflections in y to perform scan
       */
      static void setScanLimits(int maxReflections)
      {
        s_maxReflections = maxReflections;
      }

      /**
       * Clear mutable variables
       */
      void clear() const;

      /**
       * Prepare for the PDF expansion in y for a given track mass hypothesis.
       * Sets photon energy and quasy-energy distributions, mean and r.m.s of photon energy, number of photons
       * and aboveThreshold flag.
       * @param momentum particle momentum
       * @param beta particle beta
       * @param length length of particle trajectory within the quartz
       */
      void prepare(double momentum, double beta, double length) const;

      /**
       * Returns above Cerenkov threshold flag which is set in the prepare method.
       * @return true, if beta is above the Cherenkov threshold for at least one PDE data point
       */
      bool isAboveThreshold() const {return m_aboveThreshold;}

      /**
       * Performs the PDF expansion in y for a given pixel column using scan or merge methods.
       * Results accessable with getResults() method.
       * @param col pixel column number (0-based)
       * @param yB unfolded coordinate y of photon at prism entrance (= Bar exit) plane
       * @param dydz photon slope in y-z projection at prism entrance (dy/dz)
       * @param D the derivatives
       * @param doScan if true decide between scan and merge methods, if false always use merge method
       */
      void expand(unsigned col, double yB, double dydz, const Derivatives& D, bool doScan) const;

      /**
       * Returns pixel positions and their sizes
       * @return pixel positions and their sizes in module local frame
       */
      const PixelPositions& getPixelPositions() const {return m_pixelPositions;}

      /**
       * Returns pixel masks
       * @return pixel masks
       */
      const PixelMasks& getPixelMasks() const {return m_pixelMasks;}

      /**
       * Returns pixel relative efficiencies
       * @return pixel relative efficiencies
       */
      const PixelEfficiencies& getPixelEfficiencies() const {return m_pixelEfficiencies;}

      /**
       * Returns nominal photon detection efficiencies (PDE)
       * @return nominal photon detection efficiencies
       */
      const Table& getEfficiencies() const {return m_efficiency;}

      /**
       * Returns cosine of total reflection angle
       * @return cosine of total reflection angle at mean photon energy for beta = 1
       */
      double getCosTotal() const {return m_cosTotal;}

      /**
       * Returns particle momentum
       * @return particle momentum
       */
      double getMomentum() const {return m_momentum;}

      /**
       * Returns particle beta
       * @return particle beta
       */
      double getBeta() const {return m_beta;}

      /**
       * Returns particle trajectory lenght inside quartz
       * @return particle trajectory lenght inside quartz
       */
      double getTrackLengthInQuartz() const {return m_length;}

      /**
       * Returns number of photons per Cerenkov azimuthal angle per track length
       * @return number of photons per radian per centimeter
       */
      double getNumPhotonsPerLen() const {return m_numPhotons;}

      /**
       * Returns number of photons per Cerenkov azimuthal angle
       * @return number of photons per radian
       */
      double getNumPhotons() const {return m_numPhotons * m_length;}

      /**
       * Returns mean photon energy
       * @return mean photon energy
       */
      double getMeanEnergy() const {return m_meanE;}

      /**
       * Returns r.m.s of photon energy
       * @return r.m.s of photon energy
       */
      double getRMSEnergy() const {return m_rmsE;}

      /**
       * Returns mean photon energy for beta = 1
       * @return mean photon energy for beta = 1
       */
      double getMeanEnergyBeta1() const {return m_meanE0;}

      /**
       * Returns r.m.s of photon energy for beta = 1
       * @return r.m.s of photon energy for beta = 1
       */
      double getRMSEnergyBeta1() const {return m_rmsE0;}

      /**
       * Returns r.m.s of multiple scattering angle in quartz converted to photon energy
       * @return r.m.s of multiple scattering angle in quartz [eV]
       */
      double getSigmaScattering() const {return m_sigmaScat;}

      /**
       * Returns photon energy distribution
       * @return photon energy distribution
       */
      const Table& getEnergyDistribution() const {return m_energyDistribution;}

      /**
       * Returns photon energy distribution convoluted with multiple scattering
       * @return photon energy distribution convoluted with multiple scattering
       */
      const Table& getQuasyEnergyDistribution() const {return m_quasyEnergyDistribution;}

      /**
       * Returns the results of PDF expansion in y
       * @return results
       */
      const std::vector<Result>& getResults() const {return m_results;}

      /**
       * Checks which expansion method was used.
       * @return true if scan, false if merge
       */
      bool isScanDone() const {return m_scanDone;}


    private:

      /**
       * Sets photon energy distribution and mean photon energy according to nominal PDE and particle beta
       * @param beta particle beta
       * @return integral of distribution before normalization
       */
      double setEnergyDistribution(double beta) const;

      /**
       * Integrates quasy energy distribution multiplied with energy mask
       * @param energyMask energy mask (pointer must be valid)
       * @param Ecp position of the mask
       * @param result updated result [in/out]
       */
      void integrate(const EnergyMask* energyMask, double Ecp, Result& result) const;

      /**
       * Calculates y at prism entrance from detection position, reflection number and photon slope
       * @param y detection position in y
       * @param k valid index of vector of unfolded prism exit windows
       * @param dydz photon slope at prism entrance
       * @return y position at prism entrance
       */
      double prismEntranceY(double y, int k, double dydz) const;

      /**
       * Calculates a projection of a pixel to prism entrance window (going down-stream the photon).
       * @param yc pixel center in y
       * @param size pixel size in y
       * @param k valid index of vector of unfolded prism exit windows
       * @param dydz photon slope at prism entrance
       * @param proj projection of a pixel to prism entrance [out]
       */
      void projectPixel(double yc, double size, int k, double dydz, PixelProjection& proj) const;

      /**
       * Performs expansion w/ the scan over reflections.
       * @param col pixel column number (0-based)
       * @param yB unfolded coordinate y of photon at prism entrance (= Bar exit) plane
       * @param dydz photon slope in y-z projection at prism entrance (dy/dz)
       * @param D the derivatives
       * @param j1 first reflection number in y
       * @param j2 last (exclusive) reflection number in y
       */
      void scan(unsigned col, double yB, double dydz, const Derivatives& D, int j1, int j2) const;

      /**
       * Performs expansion by merging all reflections.
       * @param col pixel column number (0-based)
       * @param dydz photon slope in y-z projection at prism entrance (dy/dz)
       * @param j1 first reflection number in y
       * @param j2 last (exclusive) reflection number in y
       */
      void merge(unsigned col, double dydz, int j1, int j2) const;

      /**
       * Returns non-const pixel masks
       * @return pixel masks
       */
      PixelMasks& pixelMasks() {return m_pixelMasks;}

      /**
       * Returns non-const pixel relative efficiencies
       * @return pixel relative efficiencies
       */
      PixelEfficiencies& pixelEfficiencies() {return m_pixelEfficiencies;}

      // variables set in constructor (slot dependent)
      PixelPositions m_pixelPositions; /**< positions and sizes of pixels */
      PixelMasks m_pixelMasks; /**< pixel masks */
      PixelEfficiencies m_pixelEfficiencies; /**< pixel relative efficiencies */
      Table m_efficiency; /**< nominal photon detection efficiencies (PDE) */
      double m_meanE0 = 0; /**< mean photon energy for beta = 1 */
      double m_rmsE0 = 0; /**< r.m.s of photon energy for beta = 1 */
      double m_cosTotal = 0; /**< cosine of total reflection angle */

      // variables set in prepare method (track/hypothesis dependent)
      mutable double m_momentum = 0; /**< particle momentum magnitude */
      mutable double m_beta = 0; /**< particle beta */
      mutable double m_length = 0; /**< length of particle trajectory inside quartz */
      mutable double m_numPhotons = 0; /**< number of photons per Cerenkov azimuthal angle per track length */
      mutable double m_meanE = 0; /**< mean photon energy */
      mutable double m_rmsE = 0; /**< r.m.s of photon energy */
      mutable double m_sigmaScat = 0; /**< r.m.s. of multiple scattering angle in photon energy units */
      mutable Table m_energyDistribution; /**< photon energy distribution */
      mutable Table m_quasyEnergyDistribution; /**< photon energy distribution convoluted with multiple scattering */
      mutable bool m_aboveThreshold = false; /**< true if beta is above the Cerenkov threshold */

      // results of expand method (pixel column dependent)
      mutable std::vector<Result> m_results;   /**< results of PDF expansion in y */
      mutable bool m_scanDone = false;  /**< true if scan performed, false if reflections just merged */

      static int s_maxReflections; /**< maximal number of reflections to perform scan */

      friend class TOPRecoManager;

    };


    //--- inline functions ------------------------------------------------------------

    inline double YScanner::Derivatives::dLen_d(const InverseRaytracer::Solution& sol0,
                                                const InverseRaytracer::Solution& sol1)
    {
      return (sol1.len - sol0.len) / sol1.step;
    }

    inline double YScanner::Derivatives::dyD_d(const InverseRaytracer::Solution& sol0,
                                               const InverseRaytracer::Solution& sol1)
    {
      return (sol1.yD - sol0.yD) / sol1.step;
    }

    inline double YScanner::Derivatives::dyB_d(const InverseRaytracer::Solution& sol0,
                                               const InverseRaytracer::Solution& sol1)
    {
      return (sol1.yB - sol0.yB) / sol1.step;
    }

    inline double YScanner::Derivatives::dFic_d(const InverseRaytracer::Solution& sol0,
                                                const InverseRaytracer::Solution& sol1)
    {
      if (abs(sol0.cosFic) > abs(sol0.sinFic)) {
        return (sol1.sinFic - sol0.sinFic) / sol0.cosFic / sol1.step;
      } else {
        return -(sol1.cosFic - sol0.cosFic) / sol0.sinFic / sol1.step;
      }
    }

    inline void YScanner::Table::clear()
    {
      x0 = 0;
      step = 0;
      entries.clear();
    }

    inline void YScanner::Table::set(double X0, double Step)
    {
      x0 = X0;
      step = Step;
      entries.clear();
    }

    inline void YScanner::Table::set(const Table& T)
    {
      x0 = T.x0;
      step = T.step;
      entries.clear();
    }

    inline double YScanner::Table::getX(int i) const {return x0 + step * i;}

    inline double YScanner::Table::getXmin() const {return x0;}

    inline double YScanner::Table::getXmax() const {return x0 + step * (entries.size() - 1);}

    inline int YScanner::Table::getIndex(double x) const {return lround((x - x0) / step);}

    inline double YScanner::Table::getY(int i) const
    {
      unsigned k = i;
      if (k >= entries.size()) return 0;
      return entries[k].y;
    }

    inline void YScanner::Result::set()
    {
      if (sum == 0) return;
      e0 /= sum;
      sigsq = std::max(sigsq / sum - e0 * e0, 0.0);
    }

  } // namespace TOP
} // namespace Belle2

