/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/reconstruction_cpp/InverseRaytracer.h>
#include <top/reconstruction_cpp/FastRaytracer.h>
#include <top/reconstruction_cpp/YScanner.h>
#include <top/reconstruction_cpp/SignalPDF.h>
#include <top/reconstruction_cpp/BackgroundPDF.h>
#include <top/reconstruction_cpp/DeltaRayPDF.h>
#include <top/geometry/TOPGeometryPar.h>
#include <vector>
#include <map>
#include <set>
#include <limits>

namespace Belle2 {
  namespace TOP {

    /**
     * PDF construction and log likelihood determination for a given track and particle hypothesis.
     */
    class PDFConstructor {

    public:

      /**
       * Signal PDF construction options
       */
      enum EPDFOption {
        c_Rough = 0,   /**< no dependence on y */
        c_Fine = 1,    /**< y dependent everywhere */
        c_Optimal = 2  /**< y dependent only where necessary */
      };

      /**
       * Options for storing signal PDF parameters
       */
      enum EStoreOption {
        c_Reduced = 0, /**< only PDF peak data */
        c_Full = 1     /**< also extra information and derivatives */
      };

      /**
       * Useful data type for returning the results of log likelihood calculation
       */
      struct LogL {
        double logL = 0; /**< extended log likelihood */
        double expPhotons = 0; /**< expected number of photons */
        unsigned numPhotons = 0; /**< detected number of photons */
        double effectiveSignalYield = 0; /**< effective number of signal photons in data */

        /**
         * Constructor
         * @param phot expected number of photons
         */
        explicit LogL(double phot): logL(-phot), expPhotons(phot)
        {}
      };

      /**
       * Data type for storing photon pull w.r.t PDF peak
       */
      struct Pull {
        int pixelID = 0;    /**< pixel ID */
        double time = 0;    /**< photon time */
        double peakT0 = 0;  /**< PDF peak time (signal) or minimal PDF peak time in pixel (background) */
        double ttsT0 = 0;   /**< TTS gaussian peak time (signal) or 0 (background) */
        double sigma = 0;   /**< peak overall sigma (signal) or 0 (background) */
        double phiCer = 0;  /**< azimuthal Cerenkov angle (signal) or 0 (background)*/
        double wt = 0;      /**< weight */

        /**
         * Constructor
         * @param pix pixel ID
         * @param t photon time
         * @param t0 PDF peak time
         * @param tts0 TTS gaussian peak time
         * @param sig peak overall sigma
         * @param phi azimuthal Cerenkov angle
         * @param w weight
         */
        Pull(int pix, double t, double t0, double tts0, double sig, double phi, double w):
          pixelID(pix), time(t), peakT0(t0), ttsT0(tts0), sigma(sig), phiCer(phi), wt(w)
        {}
      };

      /**
       * Class constructor
       * @param track track at TOP
       * @param hypothesis particle hypothesis
       * @param PDFOption signal PDF construction option
       * @param storeOption signal PDF store option
       * @param overrideMass alternative mass value to be used intestead of the one from hypothesis. Ignored if <= 0.
       */
      PDFConstructor(const TOPTrack& track, const Const::ChargedStable& hypothesis,
                     EPDFOption PDFOption = c_Optimal, EStoreOption storeOption = c_Reduced, double overrideMass = 0);

      /**
       * Checks the object status
       * @return true if track is valid and all requited reconstruction objects are available
       */
      bool isValid() const {return m_valid;}

      /**
       * Exclude delta-ray PDF in log likelihood calculation
       */
      void switchOffDeltaRayPDF() const {m_deltaPDFOn = false;}

      /**
       * Include delta-ray PDF in log likelihood calculation (this is default)
       */
      void switchOnDeltaRayPDF() const {m_deltaPDFOn = true;}

      /**
       * Include or exclude delta-ray PDF in log likelihood calculation
       * @param deltaPDFOn true = include, false = exclude
       */
      void switchDeltaRayPDF(bool deltaPDFOn) const {m_deltaPDFOn = deltaPDFOn;}

      /**
       * Returns slot ID
       * @return slot ID
       */
      int getModuleID() const {return m_moduleID;}

      /**
       * Returns particle hypothesis
       * @return particle hypothesis
       */
      const Const::ChargedStable& getHypothesis() const {return m_hypothesis;}

      /**
       * Returns selected photon hits belonging to this slot
       * @return selected photon hits
       */
      const std::vector<TOPTrack::SelectedHit>& getSelectedHits() const {return m_selectedHits;}

      /**
       * Returns estimated background hit rate
       * @return background hit rate per module
       */
      double getBkgRate() const {return m_bkgRate;}

      /**
       * Returns cosine of total reflection angle
       * @return cosine of total reflection angle at mean photon energy for beta = 1
       */
      double getCosTotal() const {return m_cosTotal;}

      /**
       * Returns cosine of Cerenkov angle at given photon energy
       * @param E photon energy [eV]
       * @return cosine of Cerenkov angle
       */
      double getCosCerenkovAngle(double E) const;

      /**
       * Returns signal PDF
       * @return signal PDF in pixels (index = pixelID - 1)
       */
      const std::vector<SignalPDF>& getSignalPDF() const {return m_signalPDFs;}

      /**
       * Returns background PDF
       * @return background PDF
       */
      const BackgroundPDF* getBackgroundPDF() const {return m_backgroundPDF;}

      /**
       * Returns delta-ray PDF
       * @return delta-ray PDF
       */
      const DeltaRayPDF& getDeltaRayPDF() const {return m_deltaRayPDF;}

      /**
       * Returns the expected number of signal photons within the default time window
       * @return expected number of signal photons
       */
      double getExpectedSignalPhotons() const {return m_signalPhotons;}

      /**
       * Returns the expected number of delta-ray photons within the default time window
       * @return expected number of delta-ray photons (0 if modeling turned off)
       */
      double getExpectedDeltaPhotons() const {return m_deltaPDFOn ? m_deltaPhotons : 0;}

      /**
       * Returns the expected number of background photons within the default time window.
       * Photons from other tracks in the module are counted here as background.
       * @return expected number of background photons
       */
      double getExpectedBkgPhotons() const
      {
        double photons = m_bkgPhotons;
        for (const auto* other : m_pdfOtherTracks) {
          photons += other->getExpectedSignalPhotons() + other->getExpectedDeltaPhotons();
        }
        return photons;
      }

      /**
       * Returns the expected number of all photons within the default time window
       * @return expected number of photons (signal + background + delta-ray)
       */
      double getExpectedPhotons() const
      {
        return getExpectedSignalPhotons() + getExpectedDeltaPhotons() + getExpectedBkgPhotons();
      }

      /**
       * Returns the expected number of signal photons within a given time window
       * @param minTime time window lower edge
       * @param maxTime time window upper edge
       * @return expected number of signal photons
       */
      double getExpectedSignalPhotons(double minTime, double maxTime) const
      {
        double ps = 0;
        for (const auto& signalPDF : m_signalPDFs) {
          ps += signalPDF.getIntegral(minTime, maxTime);
        }
        return ps * m_signalPhotons;
      }

      /**
       * Returns the expected number of delta-ray photons within a given time window
       * @param minTime time window lower edge
       * @param maxTime time window upper edge
       * @return expected number of delta-ray photons (0 if modeling turned off)
       */
      double getExpectedDeltaPhotons(double minTime, double maxTime) const
      {
        return m_deltaPDFOn ? m_deltaRayPDF.getIntegral(minTime, maxTime) * m_deltaPhotons : 0;
      }

      /**
       * Returns the expected number of background photons within a given time window.
       * Photons from other tracks in the module are counted here as background.
       * @param minTime time window lower edge
       * @param maxTime time window upper edge
       * @return expected number of background photons
       */
      double getExpectedBkgPhotons(double minTime, double maxTime) const
      {
        double photons = (maxTime - minTime) / (m_maxTime - m_minTime) * m_bkgPhotons;
        for (const auto* other : m_pdfOtherTracks) {
          photons += other->getExpectedSignalPhotons(minTime, maxTime) + other->getExpectedDeltaPhotons(minTime, maxTime);
        }
        return photons;
      }

      /**
       * Returns the expected number of all photons within a given time window
       * @param minTime time window lower edge
       * @param maxTime time window upper edge
       * @return expected number of photons (signal + background + delta-ray)
       */
      double getExpectedPhotons(double minTime, double maxTime) const
      {
        return getExpectedSignalPhotons(minTime, maxTime) + getExpectedDeltaPhotons(minTime, maxTime) +
               getExpectedBkgPhotons(minTime, maxTime);
      }

      /**
       * Returns PDF value.
       * @param pixelID pixel ID
       * @param time photon hit time
       * @param timeErr uncertainty of hit time
       * @param sigt additional time smearing
       * @return PDF value
       */
      double getPDFValue(int pixelID, double time, double timeErr, double sigt = 0) const
      {
        return pdfValue(pixelID, time, timeErr, sigt) / getExpectedPhotons();
      }

      /**
       * Returns extended log likelihood (using the default time window)
       * @return log likelihood
       */
      LogL getLogL() const;

      /**
       * Returns extended log likelihood for PDF shifted in time
       * @param t0 time shift
       * @param sigt additional time smearing
       * @return log likelihood
       */
      LogL getLogL(double t0, double sigt = 0) const {return getLogL(t0, m_minTime, m_maxTime, sigt);}

      /**
       * Returns extended log likelihood for PDF shifted in time and using different time window
       * @param t0 time shift
       * @param minTime time window lower edge
       * @param maxTime time window upper edge
       * @param sigt additional time smearing
       * @return log likelihood
       */
      LogL getLogL(double t0, double minTime, double maxTime, double sigt = 0) const;

      /**
       * Returns extended log likelihood for background hypothesis using default time window
       * @return log likelihood
       */
      LogL getBackgroundLogL() const {return getBackgroundLogL(m_minTime, m_maxTime);}

      /**
       * Returns extended log likelihood for background hypothesis
       * @param minTime time window lower edge
       * @param maxTime time window upper edge
       * @return log likelihood
       */
      LogL getBackgroundLogL(double minTime, double maxTime) const;

      /**
       * Returns extended log likelihoods in pixels for PDF shifted in time.
       * @param t0 time shift
       * @param sigt additional time smearing
       * @return pixel log likelihoods (index = pixelID - 1)
       */
      const std::vector<LogL>& getPixelLogLs(double t0, double sigt = 0) const
      {return getPixelLogLs(t0, m_minTime, m_maxTime, sigt);}

      /**
       * Returns extended log likelihoods in pixels for PDF shifted in time and using diferent time window
       * @param t0 time shift
       * @param minTime time window lower edge
       * @param maxTime time window upper edge
       * @param sigt additional time smearing
       * @return pixel log likelihoods (index = pixelID - 1)
       */
      const std::vector<LogL>& getPixelLogLs(double t0, double minTime, double maxTime, double sigt = 0) const;

      /**
       * Returns photon pulls w.r.t PDF peaks
       * @return pulls
       */
      const std::vector<Pull>& getPulls() const;

      /**
       * Returns number of calls of template function setSignalPDF<T> for a given peak type
       * @param type PDF peak type
       * @return number of calls
       */
      int getNCalls_setPDF(SignalPDF::EPeakType type) const {return m_ncallsSetPDF[type];}

      /**
       * Returns number of calls of function expandSignalPDF for a given peak type
       * @param type PDF peak type
       * @return number of calls
       */
      int getNCalls_expandPDF(SignalPDF::EPeakType type) const {return m_ncallsExpandPDF[type];}

      /**
       * Returns a collection of derivatives for debugging purposes.
       * The derivatives are available only for EStoreOption::c_Full
       * @return map of xD and derivatives
       */
      const std::map <double, YScanner::Derivatives>& getDerivatives() const {return m_derivatives;}

      /**
       * Append most probable PDF of other track in this module
       * @param pdfOther PDF of other track
       */
      void appendPDFOther(const PDFConstructor* pdfOther)
      {
        if (not pdfOther) return;
        m_pdfOtherTracks.push_back(pdfOther);
      }

      /**
       * Clear the container of PDF's of other tracks
       */
      void clearPDFOther() {m_pdfOtherTracks.clear();}


    private:

      /**
       * Solution of inverse raytracing in prism
       */
      struct PrismSolution {
        double len = 0;    /**< propagation length */
        double L = 0;      /**< emission position distance along particle trajectory */
        double cosFic = 0; /**< cosine of azimuthal Cerenkov angle */
        double sinFic = 0; /**< sine of azimuthal Cerenkov angle */
      };

      /**
       * Structure that enables defining a template function: direct photons
       */
      struct InverseRaytracerDirect {
        const InverseRaytracer* inverseRaytracer = 0; /**< inverse ray-tracer */
        const SignalPDF::EPeakType type = SignalPDF::c_Direct; /**< PDF peak type */

        /**
         * Solve inverse ray-tracing for direct photon.
         * @param xD unfolded position in x of photon at detection plane (unfolding w.r.t mirror)
         * @param zD position of detection plane
         * @param assumedEmission photon emission position and track angles
         * @param cer sine and cosine of Cerenkov angle
         * @param step step for numerical derivative calculation
         * @return index of solution on success or InverseRaytracer::ErrorCodes on fail
         */
        int solve(double xD, double zD, int, double, double,
                  const TOPTrack::AssumedEmission& assumedEmission,
                  const InverseRaytracer::CerenkovAngle& cer, double step = 0) const
        {
          return inverseRaytracer->solveDirect(xD, zD, assumedEmission, cer, step);
        }
      };

      /**
       * Structure that enables defining a template function: reflected photons
       */
      struct InverseRaytracerReflected {
        const InverseRaytracer* inverseRaytracer = 0; /**< inverse ray-tracer */
        const SignalPDF::EPeakType type = SignalPDF::c_Reflected; /**< PDF peak type */

        /**
         * Solve inverse ray-tracing for reflected photon.
         * @param xD unfolded position in x of photon at detection plane (unfolding w.r.t mirror)
         * @param zD position of detection plane
         * @param Nxm signed number of reflections in x before mirror
         * @param xmMin lower limit of the reflection positions on the mirror
         * @param xmMax upper limit of the reflection positions on the mirror
         * @param assumedEmission photon emission position and track angles
         * @param cer sine and cosine of Cerenkov angle
         * @param step step for numerical derivative calculation
         * @return index of solution on success or InverseRaytracer::ErrorCodes on fail
         */
        int solve(double xD, double zD, int Nxm, double xmMin, double xmMax,
                  const TOPTrack::AssumedEmission& assumedEmission,
                  const InverseRaytracer::CerenkovAngle& cer, double step = 0) const
        {
          return inverseRaytracer->solveReflected(xD, zD, Nxm, xmMin, xmMax, assumedEmission, cer, step);
        }
      };

      /**
       * Template function: sets signal PDF in a pixel column and for specific reflection in x
       * @param t template parameter of type InverseRaytracerDirect or InverseRaytracerReflected
       * @param col pixel column (0-based)
       * @param xD unfolded detection position in x
       * @param zD detection position in z
       * @param Nxm signed number of reflections in x before mirror (dummy for direct PDF)
       * @param xmMin lower limit of the reflection positions on the mirror (dummy for direct PDF)
       * @param xmMax upper limit of the reflection positions on the mirror (dummy for direct PDF)
       */
      template<class T>
      void setSignalPDF(T& t, unsigned col, double xD, double zD, int Nxm = 0, double xmMin = 0, double xmMax = 0);

      /**
       * Returns cosine and sine of cerenkov angle
       * @param dE energy difference to mean photon energy
       * @return cosine and sine of cerenkov angle
       */
      const InverseRaytracer::CerenkovAngle& cerenkovAngle(double dE = 0);

      /**
       * Sets signal PDF
       */
      void setSignalPDF();

      /**
       * Sets signal PDF for direct photons
       */
      void setSignalPDF_direct();

      /**
       * Sets signal PDF for reflected photons
       */
      void setSignalPDF_reflected();

      /**
       * Sets signal PDF for track crossing prism
       */
      void setSignalPDF_prism();

      /**
       * Sets signal PDF for reflected photons at given reflection number
       * @param Nxm reflection number in x before mirror
       * @param xmMin lower limit of the reflection positions on the mirror
       * @param xmMax upper limit of the reflection positions on the mirror
       */
      void setSignalPDF_reflected(int Nxm, double xmMin, double xmMax);

      /**
       * Calculates unfolded detection position from known reflection position on the mirror and emission point
       * @param xM reflection position x on the mirror
       * @param Nxm reflection number before the mirror
       * @param xDs unfolded detection positions (appended at each call) [in/out]
       * @param minLen minimum of propagation lengths (updated at each call) [in/out]
       * @return true on success
       */
      bool detectionPositionX(double xM, int Nxm, std::vector<double>& xDs, double& minLen);

      /**
       * Corrects the solution of inverse ray-tracing with fast ray-tracing.
       * Corrected solution is available in m_fastRaytracer.
       * @param sol solution of inverse raytracing
       * @param dFic_dx derivative of Cerenkov azimuthal angle over photon detection coordinate x
       * @param xD unfolded detection position in x
       * @return true on success
       */
      bool doRaytracingCorrections(const InverseRaytracer::Solution& sol, double dFic_dx, double xD);

      /**
       * Returns the difference in xD between ray-traced solution rotated by dFic and input argument
       * @param dFic rotation angle around track direction (delta Cerenkov azimuthal angle)
       * @param sol solution of inverse raytracing
       * @param xD unfolded detection position in x
       * @return difference in xD
       */
      double deltaXD(double dFic, const InverseRaytracer::Solution& sol, double xD);

      /**
       * Sets the derivatives (numerically) using forward ray-tracing
       * @param D derivatives to be set
       * @param dL step in trajectory running parameter [cm]
       * @param de step in photon energy [eV]
       * @param dFic step in Cherenkov azimuthal angle
       * @return true on success
       */
      bool setDerivatives(YScanner::Derivatives& D, double dL, double de, double dFic);

      /**
       * Forward ray-tracing (called by setDerivatives)
       * @param rayTracer forward ray-tracer object
       * @param dL value to be added to trajectory running parameter [cm]
       * @param de value to be added to photon energy [eV]
       * @param dFic value to be added to Cherenkov azimuthal angle
       * @return true on success
       */
      bool raytrace(const FastRaytracer& rayTracer, double dL = 0, double de = 0, double dFic = 0);

      /**
       * Returns photon propagation losses (bulk absorption, surface reflectivity, mirror reflectivity)
       * @param E photon energy
       * @param propLen propagation length
       * @param nx total number of reflections in x
       * @param ny total number of reflections in y
       * @param type peak type, e.g. direct or reflected
       * @return survival probability due to propagation losses
       */
      double propagationLosses(double E, double propLen, int nx, int ny, SignalPDF::EPeakType type) const;

      /**
       * Expands signal PDF in y (y-scan)
       * @param col pixel column (0-based)
       * @param D derivatives
       * @param type peak type, e.g. direct or reflected
       */
      void expandSignalPDF(unsigned col, const YScanner::Derivatives& D, SignalPDF::EPeakType type);

      /**
       * Estimates range of unfolded x coordinate of the hits on given plane perpendicular to z-axis
       * @param z position of the plane
       * @param xmi lower limit [out]
       * @param xma upper limit [out]
       * @return true if at least some photons can reach the plane within the time window
       */
      bool rangeOfX(double z, double& xmi, double& xma);

      /**
       * Finds the position on the mirror of the extreme reflection.
       * @param xE true emission position in x
       * @param zE emission position in z
       * @param zD detection position in z
       * @param Nxm signed number of reflections before mirror
       * @param A width of the mirror segment
       * @param mirror spherical mirror data
       * @return position of the extreme if exists or -A/2
       */
      double findReflectionExtreme(double xE, double zE, double zD, int Nxm, double A,
                                   const RaytracerBase::Mirror& mirror) const;

      /**
       * Returns the derivative of reflected position at given x.
       * This function is used to find the position of reflection extreme.
       * All arguments must be given in the mirror frame and in units of mirror radius.
       * @param x  position in x on the mirror
       * @param xe unfolded emission position in x
       * @param ze emission position in z
       * @param zd unfolded detection position in z
       * @return the derivative
       */
      double derivativeOfReflectedX(double x, double xe, double ze, double zd) const;

      /**
       * Do forward raytracing of inverse raytracing solution in prism
       * @param sol solution of inverse raytracing in prism
       * @param dL step in length along trajectory for derivative calculation
       * @param dFic step in Cherenkov azimuthal angle for derivative calculation
       * @param de step in photon energy for derivative calculation
       * @return true on success
       */
      bool prismRaytrace(const PrismSolution& sol, double dL = 0, double dFic = 0, double de = 0);

      /**
       * General solution of inverse raytracing in prism: iterative procedure calling basic solution
       * @param pixel pixel data
       * @param k index of unfolded prism exit window
       * @param nx number of reflections in x
       * @return solution
       */
      PrismSolution prismSolution(const PixelPositions::PixelData& pixel, unsigned k, int nx);

      /**
       * Basic solution of inverse raytracing in prism: assuming straight line particle trajectory
       * @param rD unfolded pixel position
       * @param L emission position distance along particle trajectory
       * @return solution
       */
      PrismSolution prismSolution(const ROOT::Math::XYZPoint& rD, double L);

      /**
       * Returns the value of signal PDF normalized to the number of expected photons.
       * @param pixelID pixel ID
       * @param time photon hit time
       * @param timeErr uncertainty of hit time
       * @param sigt additional time smearing
       * @return PDF value
       */
      double pdfValueSignal(int pixelID, double time, double timeErr, double sigt = 0) const;

      /**
       * Returns the value of signal + deltaRay PDF normalized to the number of expected photons.
       * @param pixelID pixel ID
       * @param time photon hit time
       * @param timeErr uncertainty of hit time
       * @param sigt additional time smearing
       * @return PDF value
       */
      double pdfValueSignalDelta(int pixelID, double time, double timeErr, double sigt = 0) const;

      /**
       * Returns the value of PDF normalized to the number of expected photons.
       * @param pixelID pixel ID
       * @param time photon hit time
       * @param timeErr uncertainty of hit time
       * @param sigt additional time smearing
       * @return PDF value
       */
      double pdfValue(int pixelID, double time, double timeErr, double sigt = 0) const;

      /**
       * Initializes pixel log likelihoods
       * @param minTime time window lower edge
       * @param maxTime time window upper edge
       */
      void initializePixelLogLs(double minTime, double maxTime) const;

      /**
       * Appends pulls of a photon hit
       * @param hit photon hit
       */
      void appendPulls(const TOPTrack::SelectedHit& hit) const;

      int m_moduleID = 0; /**< slot ID */
      const TOPTrack& m_track;   /**< temporary reference to track at TOP */
      const Const::ChargedStable m_hypothesis; /**< particle hypothesis */
      const InverseRaytracer* m_inverseRaytracer = 0; /**< inverse ray-tracer */
      const FastRaytracer* m_fastRaytracer = 0; /**< fast ray-tracer */
      std::vector<FastRaytracer> m_rayTracers; /**< copies of fast ray-tracer used to compute derivatives */
      const YScanner* m_yScanner = 0; /**< PDF expander in y */
      const BackgroundPDF* m_backgroundPDF = 0; /**< background PDF */
      DeltaRayPDF m_deltaRayPDF; /**< delta-ray PDF */
      EPDFOption m_PDFOption = c_Optimal; /**< signal PDF construction option */
      EStoreOption m_storeOption = c_Reduced; /**< signal PDF storing option */
      bool m_valid = false; /**< cross-check flag, true if track is valid and all the pointers above are valid */

      double m_beta = 0; /**< particle hypothesis beta */
      double m_tof = 0; /**< time-of-flight from IP to average photon emission position */
      double m_groupIndex = 0; /**< group refractive index at mean photon energy */
      double m_groupIndexDerivative = 0; /**< derivative (dn_g/dE) of group refractive index at mean photon energy */
      double m_cosTotal = 0; /**< cosine of total reflection angle */
      double m_minTime = 0; /**< time window lower edge */
      double m_maxTime = 0; /**< time window upper edge */
      std::vector<TOPTrack::SelectedHit> m_selectedHits; /**< selected photon hits */
      double m_bkgRate = 0; /**< estimated background hit rate */

      std::vector<SignalPDF> m_signalPDFs; /**< parameterized signal PDF in pixels (index = pixelID - 1) */
      double m_signalPhotons = 0; /**< expected number of signal photons */
      double m_bkgPhotons = 0; /**< expected number of uniform background photons */
      double m_deltaPhotons = 0; /**< expected number of delta-ray photons */

      std::map<double, InverseRaytracer::CerenkovAngle> m_cerenkovAngles; /**< sine and cosine of Cerenkov angles */
      double m_dFic = 0; /**< temporary storage for dFic used in last call to deltaXD */
      double m_Fic = 0;  /**< temporary storage for Cerenkov azimuthal angle */
      mutable std::map <SignalPDF::EPeakType, int> m_ncallsSetPDF; /**< number of calls to setSignalPDF<T> */
      mutable std::map <SignalPDF::EPeakType, int> m_ncallsExpandPDF; /**< number of calls to expandSignalPDF */
      mutable std::vector<LogL> m_pixelLLs; /**< pixel log likelihoods (index = pixelID - 1) */
      mutable std::vector<Pull> m_pulls; /**< photon pulls w.r.t PDF peaks */
      mutable bool m_deltaPDFOn = true; /**< include/exclude delta-ray PDF in likelihood calculation */
      std::map <double, YScanner::Derivatives> m_derivatives; /**< a map of xD and derivatives */
      mutable std::set<int> m_zeroPixels; /**< collection of pixelID's with zero pdfValue */

      std::vector<const PDFConstructor*> m_pdfOtherTracks; /**< most probable PDF's of other tracks in the module */
      mutable double m_f0 = 0; /**< temporary value of signal PDF */

    };

    //--- inline functions ------------------------------------------------------------

    inline double PDFConstructor::pdfValueSignal(int pixelID, double time, double timeErr, double sigt) const
    {
      unsigned k = pixelID - 1;
      if (k < m_signalPDFs.size() and m_valid) {
        return  m_signalPhotons * m_signalPDFs[k].getPDFValue(time, timeErr, sigt);
      }
      return 0;
    }

    inline double PDFConstructor::pdfValueSignalDelta(int pixelID, double time, double timeErr, double sigt) const
    {
      double f = pdfValueSignal(pixelID, time, timeErr, sigt);
      if (m_deltaPDFOn) f += m_deltaPhotons * m_deltaRayPDF.getPDFValue(pixelID, time);
      return f;
    }

    inline double PDFConstructor::pdfValue(int pixelID, double time, double timeErr, double sigt) const
    {

      if (not m_valid) return 0;

      double f = pdfValueSignal(pixelID, time, timeErr, sigt); // signal
      m_f0 = f;
      if (m_deltaPDFOn) f += m_deltaPhotons * m_deltaRayPDF.getPDFValue(pixelID, time); // delta electrons
      f += m_bkgPhotons * m_backgroundPDF->getPDFValue(pixelID); // uniform background
      for (const auto* other : m_pdfOtherTracks) f += other->pdfValueSignalDelta(pixelID, time, timeErr, sigt); // other tracks

      return f;
    }

    inline double PDFConstructor::deltaXD(double dFic, const InverseRaytracer::Solution& sol, double xD)
    {
      m_dFic = dFic;
      m_fastRaytracer->propagate(m_inverseRaytracer->getReconstructedPhoton(sol, dFic), true);
      if (not m_fastRaytracer->getPropagationStatus()) return std::numeric_limits<double>::quiet_NaN();
      return m_fastRaytracer->getXD() - xD;
    }

    inline double PDFConstructor::getCosCerenkovAngle(double E) const
    {
      double refind = TOPGeometryPar::Instance()->getPhaseIndex(E);
      return std::min(1 / m_beta / refind, 1.0);
    }

    inline const InverseRaytracer::CerenkovAngle& PDFConstructor::cerenkovAngle(double dE)
    {
      auto& cer = m_cerenkovAngles[dE];
      if (cer.cosThc == 0 and cer.sinThc == 0) {
        double meanE = m_yScanner->getMeanEnergy();
        double cosThc = getCosCerenkovAngle(meanE + dE);
        cer = InverseRaytracer::CerenkovAngle(cosThc);
      }
      return cer;
    }


    template<class T>
    void PDFConstructor::setSignalPDF(T& t, unsigned col, double xD, double zD, int Nxm, double xmMin, double xmMax)
    {
      m_ncallsSetPDF[t.type]++;

      m_inverseRaytracer->clear();
      t.inverseRaytracer = m_inverseRaytracer;

      // central solutions

      int i0 = t.solve(xD, zD, Nxm, xmMin, xmMax, m_track.getEmissionPoint(), cerenkovAngle());
      if (i0 < 0 or not m_inverseRaytracer->getStatus()) return;
      int n = 0;
      for (unsigned i = 0; i < 2; i++) {
        if (not m_inverseRaytracer->getStatus(i)) continue;
        const auto& solutions = m_inverseRaytracer->getSolutions(i);
        const auto& sol = solutions[i0];
        double time = m_tof + sol.len * m_groupIndex / Const::speedOfLight;
        if (time > m_maxTime + 1.0) continue;
        n++;
      }
      if (n == 0) return;

      // solutions with xD displaced by dx

      double dx = 0.1; // cm
      int i_dx = t.solve(xD + dx, zD, Nxm, xmMin, xmMax, m_track.getEmissionPoint(), cerenkovAngle(), dx);
      if (i_dx < 0) return;
      int k = 0;
      while (m_inverseRaytracer->isNymDifferent()) { // get rid of discontinuities
        if (k > 8) {
          B2DEBUG(20, "TOP::PDFConstructor::setSignalPDF: failed to find the same Nym (dx)");
          return;
        }
        dx = - dx / 2;
        i_dx = t.solve(xD + dx, zD, Nxm, xmMin, xmMax, m_track.getEmissionPoint(), cerenkovAngle(), dx);
        if (i_dx < 0) return;
        k++;
      }

      // loop over the two solutions, do ray-tracing corrections, compute the derivatives and expand PDF in y

      for (unsigned i = 0; i < 2; i++) {
        if (not m_inverseRaytracer->getStatus(i)) continue;
        const auto& solutions = m_inverseRaytracer->getSolutions(i);
        const auto& sol = solutions[i0];
        const auto& sol_dx = solutions[i_dx];

        // compute dFic/dx needed in raytracing corrections

        double dFic_dx = YScanner::Derivatives::dFic_d(sol, sol_dx);

        // do raytracing corrections

        m_dFic = 0;
        bool ok = doRaytracingCorrections(sol, dFic_dx, xD);
        if (not ok) continue;
        m_Fic = sol.getFic() + m_dFic;

        // check if time is still within the reconstruction range

        double time = m_tof + m_fastRaytracer->getPropagationLen() * m_groupIndex / Const::speedOfLight;
        if (time > m_maxTime) continue;

        // compute the derivatives

        YScanner::Derivatives D;
        ok = setDerivatives(D, 0.1, 0.1, 0.001);
        if (not ok) {
          B2DEBUG(20, "TOP::PDFConstructor::setSignalPDF: failed to determine derivatives: "
                  << LogVar("track momentum", m_track.getMomentumMag())
                  << LogVar("impact local z", m_track.getEmissionPoint().position.Z())
                  << LogVar("xD", xD)
                  << LogVar("Nxm", Nxm)
                  << LogVar("time", time)
                  << LogVar("peak type", t.type));
          continue;
        }
        if (m_storeOption == c_Full) m_derivatives[xD] = D;

        // expand PDF in y

        expandSignalPDF(col, D, t.type);
      }
    }

  } // namespace TOP
} // namespace Belle2
