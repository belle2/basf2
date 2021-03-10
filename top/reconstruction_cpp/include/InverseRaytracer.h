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
#include <top/reconstruction_cpp/PhotonState.h>
#include <top/reconstruction_cpp/TOPTrack.h>
#include <vector>
#include <TVector3.h>
#include <cmath>

namespace Belle2 {
  namespace TOP {

    /**
     * Utility for solving inverse ray-tracing problem.
     */
    class InverseRaytracer : public RaytracerBase {

    public:

      /**
       * Error codes returned by solveDirect or solveReflected
       */
      enum ErrorCodes {
        c_NoPhysicsSolution = -1,   /**< no physics solution */
        c_NoEquationSolution = -2,  /**< no solution of equation */
        c_NoReflectionPoint = -3    /**< position on the mirror not found */
      };


      /**
       * Sine and cosine of Cerenkov angle
       */
      struct CerenkovAngle {
        double cosThc = 0; /**< cosine of Cerenkov angle */
        double sinThc = 0; /**< sine of Cerenkov angle */

        /**
         * Default constructor
         */
        CerenkovAngle()
        {}

        /**
         * Constructor from cosine of Cerenkov angle
         * @param cosTheta cosine of Cerenkov angle
         */
        explicit CerenkovAngle(double cosTheta);
      };


      /**
       * Solution of inverse ray-tracing
       */
      struct Solution {
        double cosFic = 0; /**< cosine of azimuthal Cerenkov angle */
        double sinFic = 0; /**< sine of azimuthal Cerenkov angle */
        double kx = 0;     /**< photon direction in x at emission */
        double ky = 0;     /**< photon direction in y at emission */
        double kz = 0;     /**< photon direction in z at emission */
        double len = 0;    /**< propagation length to detector plane */
        double xD = 0;     /**< unfolded coordinate x of photon at Detector plane */
        double zD = 0;     /**< coordinate z of Detector plane */
        double yD = 0;     /**< unfolded coordinate y of photon at Detector plane */
        double yB = 0;     /**< unfolded coordinate y of photon at Bar exit plane */
        int Nxm = 0;       /**< number of reflections in x before mirror */
        int Nym = 0;       /**< number of reflections in y before mirror */
        double step = 0;   /**< step for numerical derivative calculation */
        bool totRefl = true; /**< total reflection status */

        /**
         * constructor
         * @param cfi cosine of azimuthal Cerenkov angle
         * @param sfi sine of azimuthal Cerenkov angle
         */
        Solution(double cfi, double sfi);

        /**
         * Sets photon direction
         * @param cer sine and cosine of Cerenkov angle
         * @param trk sine and cosine of track polar and azimuthal angles at photon emission
         */
        void setDirection(const CerenkovAngle& cer, const TOPTrack::TrackAngles& trk);

        /**
         * Sets total reflection status for direct photon
         * @param A bar width (size in x)
         * @param B bar thickness (size in y)
         * @param cosTotal cosine of total reflection angle
         */
        void setTotalReflStatus(double A, double B, double cosTotal);

        /**
         * Sets total reflection status for reflected photon
         * @param A bar width (size in x)
         * @param B bar thickness (size in y)
         * @param cosTotal cosine of total reflection angle
         * @param Kx photon direction in x after mirror reflection
         * @param Ky photon direction in y after mirror reflection
         */
        void setTotalReflStatus(double A, double B, double cosTotal, double Kx, double Ky);

        /**
         * Returns status
         * @return true if propagation length within the limits and total reflection
         */
        bool getStatus() const;

        /**
         * Returns Cerenkov azimuthal angle
         * @return Cerenkov azimuthal angle
         */
        double getFic() const {return atan2(sinFic, cosFic);}
      };


      /**
       * Class constructor
       * @param moduleID slot ID
       * @param cosTotal cosine of total reflection angle
       */
      InverseRaytracer(int moduleID, double cosTotal): RaytracerBase(moduleID, c_Unified, c_SemiLinear),
        m_cosTotal(cosTotal)
      {}

      /**
       * Sets maximal allowed propagation length.
       * @param maxLen maximal allowed propagation length
       */
      static void setMaxPropagationLen(double maxLen) {s_maxLen = maxLen;}

      /**
       * Clear the solutions to prepare for the new round
       */
      void clear() const;

      /**
       * Solve inverse ray-tracing for direct photon.
       * @param xD unfolded position in x of photon at detection plane
       * @param zD position of detection plane
       * @param assumedEmission photon emission position and track angles
       * @param cer sine and cosine of Cerenkov angle
       * @param step step for numerical derivative calculation
       * @return index of solution or ErrorCode on fail
       */
      int solveDirect(double xD, double zD, const TOPTrack::AssumedEmission& assumedEmission,
                      const CerenkovAngle& cer, double step = 0) const;

      /**
       * Solve inverse ray-tracing for reflected photon.
       * @param xD unfolded position in x of photon at detection plane (unfolding w.r.t mirror)
       * @param zD position of detection plane
       * @param Nxm signed number of reflections before mirror
       * @param xmMin lower limit for the reflection point search range
       * @param xmMax upper limit for the reflection point search range
       * @param assumedEmission photon emission position and track angles
       * @param cer sine and cosine of Cerenkov angle
       * @param step step for numerical derivative calculation
       * @return index of solution or ErrorCode on fail
       */
      int solveReflected(double xD, double zD, int Nxm, double xmMin, double xmMax,
                         const TOPTrack::AssumedEmission& assumedEmission,
                         const CerenkovAngle& cer, double step = 0) const;

      /**
       * Solve inverse ray-tracing for a given reflection point on the mirror.
       * @param xM position in x of the reflection point on the mirror.
       * @param Nxm signed number of reflections before mirror
       * @param assumedEmission photon emission position and track angles
       * @param cer sine and cosine of Cerenkov angle
       * @return index of solution or ErrorCode on fail
       */
      int solveForReflectionPoint(double xM, int Nxm, const TOPTrack::AssumedEmission& assumedEmission,
                                  const CerenkovAngle& cer) const;

      /**
       * Returns cosine of total reflection angle
       * @return cosine of total reflection angle at mean photon energy for beta = 1
       */
      double getCosTotal() const {return m_cosTotal;}

      /**
       * Returns the solutions of inverse ray-tracing
       * @param i to select first (0) or second (1) solutions
       * @return a vector of solutions
       */
      std::vector<Solution>& getSolutions(unsigned i) const {return m_solutions[i % 2];}

      /**
       * Returns status
       * @return status
       */
      bool getStatus() const {return (m_ok[0] or m_ok[1]);}

      /**
       * Returns status
       * @param i to select status of first (0) or second (1) solutions
       * @return status
       */
      bool getStatus(unsigned i) const {return m_ok[i % 2];}

      /**
       * Checks if Nym differs between solutions front and back in at least one of the vectors
       * @return true if differs
       */
      bool isNymDifferent() const;

      /**
       * Returns reconstructed photon at emission for a given solution of inverse raytracing.
       * @param sol solution of inverse raytracing
       * @param DFic additional rotation angle around track (delta Cerenkov azimuthal angle)
       * @return reconstructed photon (note: usually meaningless if status is false!)
       */
      PhotonState getReconstructedPhoton(const Solution& sol, double DFic = 0) const;

    private:

      /**
       * Solve inverse ray-tracing for unknown azimuthal Cerenkov angle.
       * Solutions are always two or none (very rare case, phic=0: the two are the same)
       * @param dxdz photon slope (dx/dz) in x-z projection
       * @param cer sine and cosine of Cerenkov angle
       * @param trk sine and cosine of track polar and azimuthal angles at photon emission
       * @return true if solutions exits
       */
      bool solve(double dxdz, const CerenkovAngle& cer, const TOPTrack::TrackAngles& trk) const;

      /**
       * Finds reflection point on the mirror using semi-linear optics approximation.
       * @param xE unfolded emission position in x (unfolding w.r.t mirror)
       * @param zE emission position in z
       * @param xD unfolded detection position in x (unfolding w.r.t mirror)
       * @param zD detection position in z
       * @param xmMin lower limit for search range in x
       * @param xmMax upper limit for search range in x
       * &param xM reflection position in x [out]
       * &param zM reflection position in z [out]
       * @param dxdz photon slope in x-z (dx/dz) before mirror [out]
       * @return true on success
       */
      bool findReflectionPoint(double xE, double zE, double xD, double zD, double xmMin, double xmMax,
                               double& xM, double& zM, double& dxdz) const;

      /**
       * Returns the difference between input xe and the reflected position at given x.
       * This function is used to find the reflection point on the mirror.
       * All arguments must be given in the mirror frame and in units of mirror radius.
       * @param x  position in x on the mirror
       * @param xe unfolded emission position in x
       * @param ze emission position in z
       * @param xd unfolded detection position in x
       * @param zd detection position in z
       * @return the difference in units of mirror radius.
       */
      double getDeltaXE(double x, double xe, double ze, double xd, double zd) const;

      double m_cosTotal = 0; /**< cosine of total reflection angle */
      mutable std::vector<Solution> m_solutions[2]; /**< storage for the two solutions */
      mutable bool m_ok[2] = {false, false}; /**< status of solutions */
      mutable TVector3 m_emiPoint; /**< temporary storage of emission point */
      mutable CerenkovAngle m_cer; /**< temporary storage of Cerenkov angle */
      mutable TOPTrack::TrackAngles m_trk; /**< temporary storage of track polar and azimuthal angles */

      static double s_maxLen; /**< maximal allowed propagation length */

    };

  } // namespace TOP
} // namespace Belle2

