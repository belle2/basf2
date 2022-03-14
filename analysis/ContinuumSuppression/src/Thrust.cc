/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//////////////////////////////////////////
//                                      //
//  Thrust.cc                           //
//                                      //
//  calculation of the thrust quantity  //
//  from a vector of 3D momenta.        //
//  Returns the thrust axes (vector).   //
//  {cc,h}                              //
//  Brandt, S., Dahmen, H.D. Axes and   //
//  scalar measures of two-jet and      //
//  three-jet events. Z. Phys. C -      //
//  Particles and Fields 1, 61–70       //
//  (1979).                             //
//  https://doi.org/10.1007/BF01450381  //
//  http://home.thep.lu.se/~torbjorn/ppp2011/lec8.pdf
//  https://home.fnal.gov/~mrenna/lu_update/lutp0613man2/node235.html
//                                      //
//  Last modified 2021 March 15th       //
//////////////////////////////////////////


/* ******************** NOTES ******************** *
 * INPUT : std::vector<XYZVector> : momenta        *
 * OUTPUT: XYZVector              : thrust axis    *
 *                                                 *
 * Thrust Axis defined as:                         *
 *    thrust_axis = ( vec{n} ÷ ||n|| ) * thrust    *
 *                                                 *
 * Thrust (Scalar) defined as:                     *
 *                   Σ(p_i · vec{n})               *
 *    thrust = max(        ÷           )           *
 *                  Σ(||p_i||) * ||n||             *
 *                                                 *
 * *********************************************** */

/* *************************** STEPS *************************** *
 *      Initialization:                                          *
 *  1. Initialize all variables                                  *
 *  2. Parse momenta vector to compute magnitude Σ(||p_i||)      *
 *                                                               *
 *      Pick a thrust axis and optimize (max Σ(p_i · vec{n})):   *
 *  3. Define p_k as thrust axis (initial conditions)            *
 *  4. Define p_j = Sum (± p_i) as thrust axis (trial)           *
 *      - note: ± = to sign of (p_i · p_k)                       *
 *  5. Check if ( p_i · p_j ) * ( p_i · p_k ) < 0 for all p_i    *
 *  6. While True, p_k = p_j and repeat 4 -> 5, else see go to 7 *
 *  7. Compute thrust associated to selected trial axis          *
 *                                                               *
 *      Find best local Thrust (scalar) maxima possible:         *
 *  8. Keep trial axis as thrust axis if trial > base thrust     *
 *  9. Repeat from 3 -> 8 for each momentum in momenta vector    *
 *                                                               *
 *      Return thrust axis associated to best Thrust:            *
 * 10. Multiply normalized thrust axis by Thrust (<= 1)          *
 *                                                               *
 * ************************************************************* */

#include <analysis/ContinuumSuppression/Thrust.h>

using namespace Belle2;
using namespace ROOT::Math;

XYZVector Thrust::calculateThrust(const std::vector<XYZVector>& momenta)
{

  /* STEP 1: Initialization of Variables */
  XYZVector thrust_axis, trial_axis, base_axis;
  auto end = momenta.end();
  auto begin = momenta.begin();
  decltype(begin) itr;

  double sum_magnitude_mom = 0.;
  double thrust = 0.;

  /*
    STEP 2: Parse momenta vector to compute magnitude Σ(||p_i||)
  */

  for (auto const& momentum : momenta)
    sum_magnitude_mom += momentum.R();

  /*
    STEP 3: For each momentum in momenta vector,
            use momentum as initial axis to optimize
  */

  for (auto const& mom : momenta) {
    // By convention, thrust axis in same direction as Z axis
    trial_axis = (mom.z() >= 0.) ? XYZVector(mom) : XYZVector(-mom);

    // Normalize if magnitude != 0
    trial_axis = trial_axis.Unit();

    /*
      STEP 4: Store the previous trial axis as a base axis and initialize
              a new trial axis as the Z-aligned sum of the momentum vectors
    */

    itr = begin;
    while (itr != end) {
      base_axis = trial_axis;
      trial_axis = XYZVector();

      // Z-alignment of momenta and addition to trial axis
      for (auto const& momentum : momenta)
        trial_axis += (momentum.Dot(base_axis) >= 0.) ? \
                      XYZVector(momentum) : XYZVector(-momentum);

      /*
        STEP 5: Check ( p_i · trial_axis ) * ( p_i · base_axis ) < 0 ∀ p_i
      */

      for (itr = begin; itr != end; itr++)
        if ((*itr).Dot(trial_axis) * (*itr).Dot(base_axis) < 0.) break;

      /*
        STEP 6: While condition True:
                    base_axis = trial_axis
                    repeat STEP 4 and STEP 5;
                else:
                    go to STEP 7 (while from STEP 4 is done)
      */
    }

    double trial_mag(trial_axis.R()); // pre-compute
    // trial_axis = trial_axis.Unit();

    /*
      STEP 7: Compute the thrust associated to the selected trial axis
                                Σ(p_i · vec{n})
                  thrust = max(       ÷           )
                                Σ(||p_i||) * ||n||
    */
    double trial_thrust(0.);
    for (auto const& momentum : momenta)
      trial_thrust += std::fabs(momentum.Dot(trial_axis));

    trial_thrust /= (sum_magnitude_mom * trial_mag);
    // trial_thrust /= sum_magnitude_mom;

    /*
      STEP 8: Keep trial axis as thrust axis better
    */
    if (trial_thrust > thrust) {
      thrust = trial_thrust;
      thrust_axis = trial_axis / trial_mag;
      // thrust_axis = trial_axis;
    }

    /*
      STEP 9: Repeat from STEP 3 to STEP 8 (select base axis)
              to avoid falling in local maxima
    */
  }

  /*
    STEP 10: Multiply normalized thrust axis by Thrust (<= 1)
  */
  thrust_axis *= thrust;
  return thrust_axis;
}
