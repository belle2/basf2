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

#include <TVector3.h>
#include <vector>
#include "ThreeHitFilters.h"



namespace Belle2 {

  /** bundles filter methods using any number of hits (starts making sense when using 4 or more hits). */
  class TrackletFilters {

  public:

    /** Empty constructor. For pre-inizialisation only, an object generated this way is useless unless resetValues(a, b) is called at least once */
    TrackletFilters():
      m_numHits(0) {}

    /** Constructor. expects a vector of TVector3 formatted hits ordered by magnitude in x-y (first entry should be the outermost hit. Atm not needed yet, but relevant for possible future changes where a dependency of related classes like the ThreeHitFilters expect a sorted input that way) */
    TrackletFilters(std::vector<TVector3> hits);


    /** Destructor. */
    ~TrackletFilters() {}

    /** Overrides Constructor-Setup. Needed if you want to reuse the instance instead of recreating one. expects a vector of TVector3 formatted hits ordered by magnitude in x-y where the first entry should be the outermost hit */
    void resetValues(std::vector<TVector3> hits);

    /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the X-Y-plane, returns true, if they are ziggzagging */
    bool ziggZaggXY();

    /** checks whether chain of segments are zigg-zagging (changing sign of curvature of neighbouring segments) in the R-Z-plane, returns true, if they are ziggzagging */
    bool ziggZaggRZ();

    /** using circleFit(double, double) but neglects clap (closest approach of fitted circle to origin), so if your are not interested in the coordinates of clap, use this one */
    double circleFit();

    /** using paper "Effective circle fitting for particle trajectories" from V. Karimäki (Nucl.Instr.and Meth. in Physics Research, A305 (1991), Elsevier) to calculate chi2-value of a circle including these hits. Return value is chi2, input parameters are the future r-phi-coordinates of clap (closest approach of fitted circle to origin), which will be calculated during process */
    double circleFit(double& clapPhi, double& clapR);  //

  protected:

    std::vector<TVector3> m_hits; /**< stores hits using TVector3 format in a vector */
    int m_numHits; /**< stores number of hits for some speed optimizations */
    ThreeHitFilters m_3hitFilterBox;

  }; //end class TrackletFilters
} //end namespace Belle2

#endif //TRACKLETFILTERS


