/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include "tracking/vxdCaTracking/FilterExceptions.h"
#include <tuple>
#include <TVector3.h>
#include <vector>
#include <boost/assign/std/vector.hpp> // overloading + to fill many values at once in vector;
#include <array>
#include "tracking/vxdCaTracking/ThreeHitFilters.h"
#include "tracking/vxdCaTracking/SharedFunctions.h" // e.g. PositionInfo



namespace Belle2 {

  /** bundles filter methods using any number of hits (starts making sense when using 4 or more hits). */
  class TrackletFilters {

  public:

    /** Empty constructor. For pre-inizialisation only, an object generated this way is useless unless resetValues(a, b) is called at least once */
    TrackletFilters():
      m_radius(0),
      m_chi2(0),
      m_numHits(0)
    {
      m_lineParameters.clear();
      m_hits = NULL;
      using namespace boost::assign;
      m_lineParameters += 0, 0, 0, 0;
    }



    /** Constructor. expects a vector of TVector3 formatted hits ordered by magnitude in x-y (first entry should be the outermost hit. Atm not needed yet, but relevant for possible future changes where a dependency of related classes like the ThreeHitFilters expect a sorted input that way) */
    TrackletFilters(const std::vector<PositionInfo*>* hits, double magneticFieldStrength = 1.5):
      m_hits(hits),
      m_radius(0),
      m_chi2(0)
    {
      m_lineParameters.clear();
      m_numHits = m_hits->size();
      resetMagneticField(magneticFieldStrength);
      using namespace boost::assign;
      m_lineParameters += 0, 0, 0, 0;
    }



    /** Destructor. */
    ~TrackletFilters() {}



    /** Overrides Constructor-Setup. Needed if you want to reuse the instance instead of recreating one. expects a vector of PostitionInof-structs (TVector3 formatted hits + their sigmaU and sigmaV) ordered by magnitude in x-y where the first entry should be the outermost hit */
    void resetValues(const std::vector<PositionInfo*>* hits)
    {
      m_lineParameters.clear();
      m_hits = hits;
      m_numHits = hits->size();
      using namespace boost::assign;
      m_lineParameters += 0, 0, 0, 0;
    }



    /** Overrides Constructor-Setup for magnetic field. if no value is given, magnetic field is assumed to be Belle2-Detector standard of 1.5T */
    void resetMagneticField(double magneticFieldStrength = 1.5) { m_3hitFilterBox.resetMagneticField(magneticFieldStrength); }



    /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the X-Y-plane, returns true, if they are ziggzagging */
    bool ziggZaggXY();



    /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the X-Y-plane, returns true, if they are ziggzagging. This functions uses the sigma to consider also approximately straight tracks as not zigg-zagging  */
    bool ziggZaggXYWithSigma();



    /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the R-Z-plane, returns true, if they are ziggzagging */
    bool ziggZaggRZ();



    /** using double circleFit(double&, double&, double&) to calculate the vector for the transverse momentum.
     * .first is the chi2-value of the fit
     * .second is the Vector of the transverse momentum
     * Pt-vector is for the innermost hit (= the last hit in 'hits', useBackwards = false)
     * or the outermost one (useBackwards = true)
     * */
    std::pair<double, TVector3> circleFit(const std::vector<PositionInfo*>* hits, bool useBackwards = false,
                                          double setMomentumMagnitude = 0);



    /** using circleFit(const std::vector<PositionInfo*>*, bool, double) to calculate the vector for the transverse momentum.
     *
     * it uses the standard-values of that member and fills it with the hits stored in current trackletFilter-instance.
     * .first is the chi2-value of the fit
     * .second is the Vector of the transverse momentum
     * Pt-vector is for the innermost hit (= the last hit in 'hits', useBackwards = false)
     * or the outermost one (useBackwards = true) */
    std::pair<double, TVector3> circleFit() { return circleFit(m_hits); }



    /** This is a circleFit for the coordinates given by constructor or resetValues.
    *
    * taken from paper "Effective circle fitting for particle trajectories" from V. Karimäki (Nucl.Instr.and Meth. in Physics Research, A305 (1991), Elsevier)
    * calculates chi2-value of a circle including these hits.
    * Return value is chi2,
    * input parameters will be calculated during process (so please fill in variables which shall be rewritten) and are:
    * first: the phi-value of the direction of movement of the particle at POCA (point of closest approach) (0 <= phi <= 2pi).
    * second: the distance to the origin of the POCA, has same sign as curvature if POCA is on the same quadrant as the circle-center, different sign if not.
    * third: the curvature of the fitted circle, value is < 0 if direction of movement is counterclockwise.
    */
    double circleFit(double& pocaPhi, double& pocaD, double& curvature);



    /** does a tripletFit of the given hits
     * The filter is based on the paper 'A New Three-Dimensional Track Fit with Multiple Scattering'
     * by Andre Schoening et al. https://arxiv.org/abs/1606.04990*/
    std::pair<double, TVector3> tripletFit(const std::vector<PositionInfo*>* hits, bool useBackwards = false);

    /** As it seems to be customary to use another layer: */
    std::pair<double, TVector3> tripletFit() { return tripletFit(m_hits); }



    /** does a complete helixFit of the given hits */
    std::pair<double, TVector3> helixFit(const std::vector<PositionInfo*>* hits, bool useBackwards = false,
                                         double setMomentumMagnitude = 0);



    /** using paper "Helix fitting by an extended Riemann fit" from R. Frühwirth, (Nucl.Instr.and Meth. in Physics Research, A490 (2002), Elsevier) does a full fit onto a helix and calculates the chi2 value (.first) including the momentum vector (.second) */
    std::pair<double, TVector3> helixFit() { return helixFit(m_hits); }
    //Thomas: returns radius (.first) and not chi2. (as of feb 2014) WARNING still the case?



    /** a straight line fit for 3D coordinates (in fact, it is using 2 2D-Fits and ignores covariances (due to nonparallel and nonrotated sensors) and only expects errors in 1D per 2D-fit) calculating a direction for a seed value. The return values are chi2 (.first) and direction vector (.second, similar to momentum vector of the helixFit) */
    std::pair<double, TVector3> simpleLineFit3D() { return simpleLineFit3D(m_hits); }



    /** straight line fits expecting the hits stored in a vector of PositionInfo */
    std::pair<double, TVector3> simpleLineFit3D(const std::vector<PositionInfo*>* hits, bool useBackwards = false,
                                                double setMomentumMagnitude = 0);



    /** if simpleLineFit3D has been executed before, this getter returns the estimated values using an array. At position0: slopeYest, 1: interceptYest, 2: slopeZest: 3: interceptZest. Else values are 0 */
    std::vector<double> getStraightLineFitResults() { return m_lineParameters; }



    /** producing a reasonable guess for the abs(pT) of the tracklet, not compatible with setups of very small values for magnetic field, if a radius is already known, the value can be passed and shortens the process */
    double calcPt(double r = 0)
    {
      if (r != 0) { return m_3hitFilterBox.calcPt(r); }

      if (m_radius == 0.) {m_chi2 = circleFit().first; }
//      double = pocaPhi, pocaD, curvature;
//      m_chi2 = circleFit(pocaPhi, pocaD, curvature);
      if (m_radius < 0) { m_radius = -m_radius; }
      if (fabs(m_radius) < 0.00001) { throw FilterExceptions::Circle_too_small(); } // WARNING: hardcoded value, solution for such a case is needed
      m_3hitFilterBox.filterNan(m_radius);
      return m_3hitFilterBox.calcPt(m_radius);
    }



    /** calculates the momentum vector on first or last hitPosition in the hitVector. This means, you can define the hit to be chosen by two ways: fast: if useBackwards = true, then the first hit in the hitList is taken (check VXDTrackCandidate.h for the sequence of hits), if useBackwards = false, the last hit will be taken. The second method means that you have to sort your hits beforehand and therefore you can place the hit of your choice in the front or back-position... Return values: .first: Momentum-vector of seed, .second, sign-curvature */
    std::pair<TVector3, int> calcMomentumSeed(bool useBackwards = false, double setMomentumMagnitude = 0);



    /** Calculates Curvature: True means clockwise, False means counterclockwise.  (from out to in) */
    bool CalcCurvature();



    /** prints positions and sigmas of all hits in given vector of PositionInfos TODO use SpacePoints instead of PositionInfo for all filters, SpacePoints shall get a print-Function */
    std::string printHits(const std::vector<PositionInfo*>* hits) const ;



  protected:



    const std::vector<PositionInfo*>* m_hits; /**< stores hits using TVector3 format in a vector */
    double m_radius; /**< stores radius of tracklet-circle */
    double m_chi2; /**< stores chi2 of tracklet-circle */
    int m_numHits; /**< stores number of hits for some speed optimizations */
    std::vector<double> m_lineParameters; /**< position0: slopeYest, 1: interceptYest, 2: slopeZest: 3: interceptZest */
    ThreeHitFilters m_3hitFilterBox; /**< instance of ThreeHitFilters-class used for some internal calculations */

  }; //end class TrackletFilters
} //end namespace Belle2
