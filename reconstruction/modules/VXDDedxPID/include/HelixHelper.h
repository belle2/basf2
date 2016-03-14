/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HELIXHELPER_H
#define HELIXHELPER_H

#include <TVector3.h>
#include <Math/Functor.h>
#include <Math/BrentMinimizer1D.h>

#include <cmath>

namespace Belle2 {
  /** Helper class representing a helical track
   *
   * It is used as an alternative to Genfit's track representation
   * and provides much faster replacements for its extrapolateToLine/
   * extrapolateToPoint() methods.
   */
  class HelixHelper {
    constexpr static double c_cTimesB = (1.5 * 0.00299792458); /**< magnetic filed times speed of light */
    constexpr static double c_maxFlightLength = 150.0; /**< maximal path length (from origin) considered for extrapolation */

  public:
    /** construct a helix with given helix parameters, as defined for Track objects */
    HelixHelper(float z0, float d0, float omega, float cotTheta, float phi):
      z0(z0), d0(d0), omega(omega), cotTheta(cotTheta), phi(phi),
      poca(d0 * sin(phi), -d0 * cos(phi), z0)
    { }


    /** construct a helix at an arbitrary position 'poca' (helices built at different points are not comparable) */
    HelixHelper(const TVector3& poca, const TVector3& momentum_in_poca, int charge):
      poca(poca)
    {
      const double pt = momentum_in_poca.Pt();
      const double R = pt / c_cTimesB; //c and magnetic field, should come from some common database later...

      const TVector3& dirInPoca = momentum_in_poca.Unit();

      //determine the angle phi, distribute it from -pi to pi
      phi = atan2(dirInPoca.y() , dirInPoca.x());

      //determine sign of d0
      //calculate the sign of the projection of pt(dirInPoca) at d0(poca)
      const double d0Sign = TMath::Sign(1., poca.x() * dirInPoca.x()
                                        + poca.y() * dirInPoca.y());

      //Now set the helix parameters
      d0 = d0Sign * poca.Perp();
      omega = 1 / R * charge;
      z0 = poca.z();
      cotTheta = dirInPoca.z() / dirInPoca.Pt();
    }

    /** returns the path length (along the helix) to the helix point closest to p.
     *
     * a path length of 0 corresponds to p = poca
     */
    double pathLengthToPoint(const TVector3& p) const
    {
      minimize_distance_to_point = p;
      helix_object = this; //ok, this is ugly
      //TODO create a functor object to wrap everything up

      ROOT::Math::Functor1D functor(&distanceToPoint);
      ROOT::Math::BrentMinimizer1D bm;
      bm.SetFunction(functor, 0.0, c_maxFlightLength);
      bm.Minimize(100); //#iterations, abs. error, rel. error

      //bm.FValMinimum() is shortest distance
      //bm.XMinimum() is corresponding path length
      return bm.XMinimum();
    }

    /** returns the path length (along the helix) to the helix point closest to the line
     *  going through points a and b.
     */
    double pathLengthToLine(const TVector3& a, const TVector3& b) const
    {
      minimize_distance_to_line_a = a;
      minimize_distance_to_line_b = b;

      helix_object = this; //ok, this is ugly
      //TODO create a functor object to wrap everything up

      ROOT::Math::Functor1D functor(&distanceToLine);
      ROOT::Math::BrentMinimizer1D bm;
      bm.SetFunction(functor, 0.0, c_maxFlightLength);
      bm.Minimize(100); //#iterations, abs. error, rel. error

      //bm.FValMinimum() is shortest distance
      //bm.XMinimum() is corresponding path length
      return bm.XMinimum();
    }


    /** momentum of the particle, at the helix point
     *  corresponding to a flown path length s (from poca).
     */
    TVector3 momentum(double s = 0) const
    {
      const float pt = c_cTimesB / TMath::Abs(omega);
      return TVector3(
               pt * cos(phi - 2 * omega * s),
               pt * sin(phi - 2 * omega * s),
               pt * cotTheta
             );
    }

    /** point on helix corresponding to a flown path length s (from poca) */
    TVector3 position(double s) const
    {
      //aproximation (but it does work for straight tracks)
      return poca + TVector3(
               s * s * omega / 2 * sin(phi) + s * cos(phi),
               -s * s * omega / 2 * cos(phi) + s * sin(phi),
               s * cotTheta
             );
    }

  private:
    // helix parameters, with same convention as those stored in Track objects
    /** minimal z distance of point of closest approach to origin */
    float z0;
    /** minimal r distance of point of closest approach to origin */
    float d0;
    /** signed curvature */
    float omega;
    /** cotangens of polar angle */
    float cotTheta;
    /** Phi at the perigee [-pi, pi]. */
    float phi;

    /** point of closest approach to origin */
    TVector3 poca;

    /** minimization function, calculates distance to minimize_distance_to_point */
    static double distanceToPoint(double s)
    {
      return (helix_object->position(s) - minimize_distance_to_point).Mag();
    }

    /** same as distanceToPoint, but ignoring z coordinate */
    static double distanceToLine(double s)
    {
      const TVector3& p = helix_object->position(s);
      // d = |(p-a) \times (p-b)| / |b-a|
      return ((p - minimize_distance_to_line_a).Cross(p - minimize_distance_to_line_b)).Mag() / (minimize_distance_to_line_b -
             minimize_distance_to_line_a).Mag();
    }

    /** user supplied point we're trying to find the nearest helix point to */
    static TVector3 minimize_distance_to_point;
    // @{
    /** user supplied line we're trying to find the nearest helix point to */
    static TVector3 minimize_distance_to_line_a, minimize_distance_to_line_b;
    //@}
    /** keep a 'this' pointer around for minimization */
    static HelixHelper const* helix_object;
  };

  TVector3 HelixHelper::minimize_distance_to_point(0.0, 0.0, 0.0);
  TVector3 HelixHelper::minimize_distance_to_line_a(0.0, 0.0, 0.0);
  TVector3 HelixHelper::minimize_distance_to_line_b(0.0, 0.0, 0.0);
  HelixHelper const* HelixHelper::helix_object(0);
}
#endif
