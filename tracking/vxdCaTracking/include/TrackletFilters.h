/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef TRACKLETFILTERS_H
#define TRACKLETFILTERS_H

#include <tuple>
#include <TVector3.h>
#include <vector>
#include "ThreeHitFilters.h"
#include "SharedFunctions.h" // e.g. PositionInfo



namespace Belle2 {

  /** bundles filter methods using any number of hits (starts making sense when using 4 or more hits). */
  class TrackletFilters {

  public:

    /** Empty constructor. For pre-inizialisation only, an object generated this way is useless unless resetValues(a, b) is called at least once */
    TrackletFilters():
      m_radius(0),
      m_chi2(0),
      m_numHits(0) { m_hits = NULL; }

    /** Constructor. expects a vector of TVector3 formatted hits ordered by magnitude in x-y (first entry should be the outermost hit. Atm not needed yet, but relevant for possible future changes where a dependency of related classes like the ThreeHitFilters expect a sorted input that way) */
    TrackletFilters(const std::vector<PositionInfo*>* hits, double magneticFieldStrength = 1.5):
      m_hits(hits),
      m_radius(0),
      m_chi2(0) {
      m_numHits = m_hits->size();
      resetMagneticField(magneticFieldStrength);
    }


    /** Destructor. */
    ~TrackletFilters() {}

    /** Overrides Constructor-Setup. Needed if you want to reuse the instance instead of recreating one. expects a vector of TVector3 formatted hits ordered by magnitude in x-y where the first entry should be the outermost hit */
    void resetValues(const std::vector<PositionInfo*>* hits) {
      m_hits = hits;
      m_numHits = hits->size();
    }

    /** Overrides Constructor-Setup for magnetic field. if no value is given, magnetic field is assumed to be Belle2-Detector standard of 1.5T */
    void resetMagneticField(double magneticFieldStrength = 1.5) { m_3hitFilterBox.resetMagneticField(magneticFieldStrength); }

    /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the X-Y-plane, returns true, if they are ziggzagging */
    bool ziggZaggXY();

    /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the R-Z-plane, returns true, if they are ziggzagging */
    bool ziggZaggRZ();

    /** using circleFit(double, double) but neglects clap (closest approach of fitted circle to origin), so if your are not interested in the coordinates of clap, use this one */
    double circleFit() {
      double phiValue, rValue;
      return circleFit(phiValue, rValue, m_radius);
    } // if you do not want to have the coordinates of the point of closest approach, use this one

    /** does a complete helixFit of the given hits */
    std::pair<double, TVector3> helixFit(const std::vector<PositionInfo*>* hits, bool useBackwards = false);

    /** using paper "Helix fitting by an extended Riemann fit" from R. Frühwirth, (Nucl.Instr.and Meth. in Physics Research, A490 (2002), Elsevier) does a full fit onto a helix and calculates the chi2 value (.first) including the momentum vector (.second) */
    std::pair<double, TVector3> helixFit() { return helixFit(m_hits); }

    /** using paper "Effective circle fitting for particle trajectories" from V. Karimäki (Nucl.Instr.and Meth. in Physics Research, A305 (1991), Elsevier) to calculate chi2-value of a circle including these hits. Return value is chi2, input parameters are the future r-phi-coordinates of clap (closest approach of fitted circle to origin), which will be calculated during process */
    double circleFit(double& clapPhi, double& clapR, double& radius);

    /** producing a reasonable guess for the pT of the tracklet */
    double calcPt() {
      if (m_radius < 0.001) { m_chi2 = circleFit(); }
      if (m_radius < 0.001) { return 0.; }
      return m_3hitFilterBox.calcPt(m_radius);
    }

    /** calculates the momentum vector on first or last hitPosition in the hitVector. This means, you can define the hit to be chosen by two ways: fast: if useBackwards = true, then the first hit in the hitList is taken (check VXDTrackCandidate.h for the sequence of hits), if useBackwards = false, the last hit will be taken. The second method means that you have to sort your hits beforehand and therefore you can place the hit of your choice in the front or back-position... Return values: .first: Momentum-vector of seed, .second, sign-curvature */
    std::pair<TVector3, int> calcMomentumSeed(bool useBackwards = false);

  protected:

    const std::vector<PositionInfo*>* m_hits; /**< stores hits using TVector3 format in a vector */
    double m_radius; /**< stores radius of tracklet-circle */
    double m_chi2; /**< stores chi2 of tracklet-circle */
    int m_numHits; /**< stores number of hits for some speed optimizations */
    ThreeHitFilters m_3hitFilterBox; /**< instance of ThreeHitFilters-class used for some internal calculations */

  }; //end class TrackletFilters
} //end namespace Belle2

#endif //TRACKLETFILTERS


