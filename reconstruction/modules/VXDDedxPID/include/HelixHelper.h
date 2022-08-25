/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <Math/Functor.h>
#include <Math/BrentMinimizer1D.h>
#include <Math/Vector3D.h>

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
      m_z0(z0), m_d0(d0), m_omega(omega), m_cotTheta(cotTheta), m_phi(phi),
      m_poca(d0 * sin(phi), -d0 * cos(phi), z0)
    { }


    /** construct a helix at an arbitrary position 'poca' (helices built at different points are not comparable) */
    HelixHelper(const ROOT::Math::XYZVector& poca, const ROOT::Math::XYZVector& momentum_in_poca, int charge):
      m_poca(poca)
    {
      const double pt = momentum_in_poca.Rho();
      const double R = pt / c_cTimesB; //c and magnetic field, should come from some common database later...

      const ROOT::Math::XYZVector& dirInPoca = momentum_in_poca.Unit();

      //determine the angle phi, distribute it from -pi to pi
      m_phi = atan2(dirInPoca.Y(), dirInPoca.X());

      //determine sign of d0
      //calculate the sign of the projection of pt(dirInPoca) at d0(poca)
      const double d0Sign = TMath::Sign(1., poca.X() * dirInPoca.X()
                                        + poca.Y() * dirInPoca.Y());

      //Now set the helix parameters
      m_d0 = d0Sign * poca.Rho();
      m_omega = 1 / R * charge;
      m_z0 = poca.Z();
      m_cotTheta = dirInPoca.Z() / dirInPoca.Rho();
    }

    /** returns the path length (along the helix) to the helix point closest to p.
     *
     * a path length of 0 corresponds to p = poca
     */
    double pathLengthToPoint(const ROOT::Math::XYZVector& p) const
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
    double pathLengthToLine(const ROOT::Math::XYZVector& a, const ROOT::Math::XYZVector& b) const
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
    ROOT::Math::XYZVector momentum(double s = 0) const
    {
      const float pt = c_cTimesB / TMath::Abs(m_omega);
      return ROOT::Math::XYZVector(
               pt * cos(m_phi - 2 * m_omega * s),
               pt * sin(m_phi - 2 * m_omega * s),
               pt * m_cotTheta
             );
    }

    /** point on helix corresponding to a flown path length s (from poca) */
    ROOT::Math::XYZVector position(double s) const
    {
      //aproximation (but it does work for straight tracks)
      return m_poca + ROOT::Math::XYZVector(
               s * s * m_omega / 2 * sin(m_phi) + s * cos(m_phi),
               -s * s * m_omega / 2 * cos(m_phi) + s * sin(m_phi),
               s * m_cotTheta
             );
    }

  private:
    // helix parameters, with same convention as those stored in Track objects
    /** minimal z distance of point of closest approach to origin */
    float m_z0;
    /** minimal r distance of point of closest approach to origin */
    float m_d0;
    /** signed curvature */
    float m_omega;
    /** cotangens of polar angle */
    float m_cotTheta;
    /** Phi at the perigee [-pi, pi]. */
    float m_phi;

    /** point of closest approach to origin */
    ROOT::Math::XYZVector m_poca;

    /** minimization function, calculates distance to minimize_distance_to_point */
    static double distanceToPoint(double s)
    {
      return (helix_object->position(s) - minimize_distance_to_point).R();
    }

    /** same as distanceToPoint, but ignoring z coordinate */
    static double distanceToLine(double s)
    {
      const ROOT::Math::XYZVector& p = helix_object->position(s);
      // d = |(p-a) \times (p-b)| / |b-a|
      return ((p - minimize_distance_to_line_a).Cross(p - minimize_distance_to_line_b)).R() / (minimize_distance_to_line_b -
             minimize_distance_to_line_a).R();
    }

    /** user supplied point we're trying to find the nearest helix point to */
    static ROOT::Math::XYZVector minimize_distance_to_point;
    /** first user supplied line we're trying to find the nearest helix point to */
    static ROOT::Math::XYZVector minimize_distance_to_line_a;
    /** second user supplied line we're trying to find the nearest helix point to */
    static ROOT::Math::XYZVector minimize_distance_to_line_b;
    /** keep a 'this' pointer around for minimization */
    static HelixHelper const* helix_object;
  };

  ROOT::Math::XYZVector HelixHelper::minimize_distance_to_point(0.0, 0.0, 0.0);
  ROOT::Math::XYZVector HelixHelper::minimize_distance_to_line_a(0.0, 0.0, 0.0);
  ROOT::Math::XYZVector HelixHelper::minimize_distance_to_line_b(0.0, 0.0, 0.0);
  HelixHelper const* HelixHelper::helix_object(0);
}
