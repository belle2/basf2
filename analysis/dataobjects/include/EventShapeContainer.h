/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>

#include <TVector3.h>
#include <TLorentzVector.h>
#include <vector>

namespace Belle2 {
  /**
   * Class for collecting the basic objects related to the event shape.
   * This object is written by the EventShapeCalculator module, that stores here the quantities that
   * require long calculations like the FW moments, the sphericity(-like) tensors and the shape axes
   * (like thrust axis or jet axes). These quantities will be then used by the VaribaleManager functions
   * to get the final EventShape variable, which are usually very simple to calculate.
   * For example, the  EventShapeCalculator will store here the Fox-Wolfram moments, while the ratios of them will be calculated
   * on fly by the Variable Manager. We prefer this approach rather than storing every event shape variable directly
   * here to keep a reasonably small data object, and making adding more variables easier.
   *
   * All the quantities are defined and calculated in the center of mass frame.
   *
   * The Event shape variables stored here can be divided into two classes: axis independent and axis-based. The first
   * one are uniquely defined by the particles in the event, while the latter one
   * require also a reference axis to be calculated.
   *
   * The axis-independent quantities currently implemented are:
   * - sphericity tensor eigenvalues and eigenvectors
   * - Fow-Wolfram moments from 0 to 9
   *
   * The axis-dependent quantities currently implemented are:
   * - Harmonic moments from 0 to 9
   * - Thrust
   * - Cleo cones
   * All these quantities are can be stored in two versions:
   * - calculated respect to the e+e- collision axis (defined by the beam parameters)
   * - calculated respect to the thrust axis (calculated by the EventShapeCalculator module)
   */

  class EventShapeContainer : public RelationsObject {

  public:

    /**
     * Default constructor, resets the sphericity matrix
     */
    EventShapeContainer()
    {
      m_thrustAxis.SetXYZ(0., 0., 0.);
      m_spherocityAxis.SetXYZ(0., 0., 0.);
      m_forwardHemisphere4Momentum.SetXYZT(0., 0., 0., 0.);
      m_backwardHemisphere4Momentum.SetXYZT(0., 0., 0., 0.);

      m_thrustAxis.SetXYZ(0., 0., 0.);
      for (int i = 0; i < 3; i++)
        m_sphericityEigenvector[i].SetXYZ(0., 0., 0.);
    };


    // --------------------------------------
    // Setters
    // --------------------------------------


    /**
     * Sets the i-th sphericity eigenvector
     * @param i the order (0,1,2)
     * @param eigenvector the eigenvector
     */
    void setSphericityEigenvector(short i, TVector3 eigenvector)
    {
      if (i < 0 || i > 2)
        B2ERROR("Invalid  EigenVector number  (n = " << i << "). You can set only 3 eigenvectors.");
      else
        m_sphericityEigenvector[i] = eigenvector;
    };

    /**
     * Sets the i-th sphericity eigenvalue
     * @param i the order (0,1,2)
     * @param eigenvalue the eigenvalue
     */
    void setSphericityEigenvalue(short i, float eigenvalue)
    {
      if (i < 0 || i > 2)
        B2ERROR("Invalid  EigenVector number  (n = " << i << "). You can set only 3 eigenvectors.");
      else
        m_sphericityEigenvalue[i] = eigenvalue;
    };

    /**
     * Sets the Fox-Wolfram (FW) moment of order n. This is NOT the normalized FW moment (R1, R2...), but the original one.
     * @param order The order (0 to 9)
     * @param moment The Fox-Wolfram moment value
     */
    void setFWMoment(short order, float moment)
    {
      if (order < 0 || order > 9)
        B2ERROR("Invalid Fox-Wolfram moment order (n = " << order << "). The order must be in the [0,9] range.");
      else
        m_foxWolframMoments[order] = moment;
    };

    /**
     * Sets the thrust of the event
     * @param thrust thrust value
     */
    void setThrust(float thrust)
    {
      m_thrust = thrust;
    };

    /**
     * Sets the thrust axis, normalizing it
     * @param thrustAxis thrust axis.
     */
    void setThrustAxis(TVector3 thrustAxis)
    {
      if (thrustAxis.Mag() < 1.E-10)
        B2WARNING("The thrust axis you are trying to set has magnitude numerically compatible with 0.");
      m_thrustAxis = (1. / thrustAxis.Mag()) * thrustAxis;
    };

    /**
     * Sets the spherocity axis, normalizing it
     * @param spherocityAxis sperocity axis. Must have magnitude equal to the thrust value.
     */
    void setSpherocityAxis(TVector3 spherocityAxis)
    {
      if (spherocityAxis.Mag() < 1.E-10)
        B2WARNING("The spherocity axis you are trying to set has magnitude numerically compatible with 0.");
      m_spherocityAxis = (1. / spherocityAxis.Mag()) * spherocityAxis;
    };

    /**
     * Sets the harmonic moment of order n, calculated using the thrust axis.
     * @param order The order (0 to 9)
     * @param moment The harmonic moment value
     */
    void setHarmonicMomentThrust(short order, float moment)
    {
      if (order < 0 || order > 9)
        B2ERROR("Invalid harmonic moment order. It  must be in the [0,9] range.");
      else
        m_harmonicMomentsThrust[order] = moment;
    };

    /**
     * Sets the harmonic moment of order n, calculated using the collision axis.
     * @param order The order (0 to 9)
     * @param moment The harmonic moment value
     */
    void setHarmonicMomentCollision(short order, float moment)
    {
      if (order < 0 || order > 9)
        B2ERROR("Invalid harmonic moment order. It  must be in the [0,9] range.");
      else
        m_harmonicMomentsCollision[order] = moment;
    };

    /**
     * Sets the Cleo cone of order n, calculated using the thrust axis.
     * @param order The order (0 to 9)
     * @param moment The Cleo cone value
     */
    void setCleoConeThrust(short order, float moment)
    {
      if (order < 0 || order > 9)
        B2ERROR("Invalid Cleo cone order. It  must be in the [0,9] range.");
      else
        m_cleoConesThrust[order] = moment;
    };

    /**
     * Sets the Cleo cone of order n, calculated using the collision axis.
     * @param order The order (0 to 9)
     * @param moment The Cleo cone value
     */
    void setCleoConeCollision(short order, float moment)
    {
      if (order < 0 || order > 9)
        B2ERROR("Invalid Cleo cone order. It must be in the [0,9] range.");
      else
        m_cleoConesCollision[order] = moment;
    };

    /**
     * Sets the 4-momentum of the forward hemisphere, as defined by the thrust axis.
     * @param mom TLorentzVector 4-momentum of the particles in the forward hemisphere
     */
    void setForwardHemisphere4Momentum(TLorentzVector mom)
    {
      m_forwardHemisphere4Momentum = mom;
    };

    /**
     * Sets the 4-momentum of the backward hemisphere, as defined by the thrust axis.
     * @param mom TLorentzVector 4-momentum of the particles in the backward hemisphere
     */
    void setBackwardHemisphere4Momentum(TLorentzVector mom)
    {
      m_backwardHemisphere4Momentum = mom;
    };


    // --------------------------------------
    // Getters
    // --------------------------------------


    /**
     * Returns the thrust
     * @return TVector3 the thrust axis, normalized
     */
    float getThrust() const
    {
      return m_thrust;
    };

    /**
     * Returns the thrust axis (normalized).
     * @return TVector3 the thrust axis, normalized
     */
    TVector3 getThrustAxis() const
    {
      return m_thrustAxis;
    };

    /**
     * Returns the spherocity axis (normalized).
     * @return TVector3 the spherocity axis, normalized
     */
    TVector3 getSpherocityAxis() const
    {
      return m_spherocityAxis;
    };

    /**
     * Returns the i-th sphericity matrix eigenvalue.
     * @param i the number of the eigenvalue (0,1,2)
     * @return float the eigenvalue
     */
    float getSphericityEigenvalue(short i) const
    {
      if (i < 0 || i > 2) {
        B2ERROR("Invalid  Eigenvalue number  (n = " << i << "). There are only 3 eigenvalues...");
        return std::numeric_limits<float>::quiet_NaN();
      } else
        return m_sphericityEigenvalue[i];
    };


    /**
     * Returns the i-th sphericity matrix eigenvector.
     * @param i the number of the eigenvalue (0,1,2)
     * @return float the eigenvector
     */
    TVector3 getSphericityEigenvector(short i) const
    {
      if (i < 0 || i > 2) {
        B2ERROR("Invalid  Eigenvalue number  (n = " << i << "). There are only 3 eigenvalues...");
        return TVector3(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
                        std::numeric_limits<float>::quiet_NaN());
      } else
        return m_sphericityEigenvector[i];
    };



    /**
     * Returns the Fox-Wolfram moment of a given order.
     * @param order Order of the FW moment
     * @return float FW moment
     */
    float getFWMoment(short order) const
    {
      if (order < 0 || order > 9) {
        B2ERROR("Invalid Fox-Wolfram moment order. It must be in the [0,9] range.");
        return std::numeric_limits<float>::quiet_NaN();
      } else
        return m_foxWolframMoments[order];
    };

    /**
     * Returns the harmonic moment of a given order, calculated respect to the beam axis
     * @param order Order of the harmonic moment
     * @return float harmonic moment
     */
    float getHarmonicMomentCollision(short order) const
    {
      if (order < 0 || order > 9) {
        B2ERROR("Invalid harmonic moment order. It  must be in the [0,9] range.");
        return std::numeric_limits<float>::quiet_NaN();
      } else
        return m_harmonicMomentsCollision[order];
    };

    /**
     * Returns the harmonic moment of a given order, calculated respect to the thrust axis
     * @param order Order of the harmonic moment
     * @return float harmonic moment
     */
    float getHarmonicMomentThrust(short order) const
    {
      if (order < 0 || order > 9) {
        B2ERROR("Invalid harmonic moment order. The order must be in the [0,9] range.");
        return std::numeric_limits<float>::quiet_NaN();
      } else
        return m_harmonicMomentsThrust[order];
    };


    /**
     * Returns the Cleo cone of a given order, calculated respect to the beam axis
     * @param order Order of the harmonic moment
     * @return float Cleo cone
     */
    float getCleoConeCollision(short order) const
    {
      if (order < 0 || order > 9) {
        B2ERROR("Invalid CLEO cone order. The order must be in the [0,9] range.");
        return std::numeric_limits<float>::quiet_NaN();
      } else
        return m_cleoConesCollision[order];
    };

    /**
     * Returns the Cleo cone of a given order, calculated respect to the thrust axis
     * @param order Order of the harmonic moment
     * @return float Cleo cone
     */
    float getCleoConeThrust(short order) const
    {
      if (order < 0 || order > 9) {
        B2ERROR("Invalid CLEO cone order. The order must be in the [0,9] range.");
        return std::numeric_limits<float>::quiet_NaN();
      } else
        return m_cleoConesThrust[order];
    };


    /**
     * Returns the 4-momentum of the forward hemisphere, as defined by the thrust axis.
     * @return TLorentzVector 4-momentum of the particles in the hemisphere
     */
    TLorentzVector getForwardHemisphere4Momentum() const
    {
      return m_forwardHemisphere4Momentum;
    };

    /**
     * Return the 4-momentum of the backward hemisphere, as defined by the thrust axis.
     * @return TLorentzVector 4-momentum of the particles in the hemisphere
     */
    TLorentzVector getBackwardHemisphere4Momentum() const
    {
      return m_backwardHemisphere4Momentum;
    };


  private:

    // Axes
    TVector3 m_thrustAxis; /**< Thrust axis. It is not normalized, and it's length is the thrust value */
    TVector3 m_spherocityAxis; /**< Spherocity axis. It is not normalized, and it's length is the spherocity value */

    // Axis-independent quantities
    TVector3 m_sphericityEigenvector[3]; /**< Sphericity tensor eigenvectors*/
    float m_sphericityEigenvalue[3] = {0.}; /**< Sphericity tensor eigenvalues*/
    float m_foxWolframMoments[10] = {0.}; /**< Fox Wolfram moments up to order 9. */

    // Axis-dependent quantities
    float m_thrust = 0; /**< Thrust value */
    float m_harmonicMomentsThrust[10] = {0.}; /**< Harmonic moments up to order 9, calculated respect to the thrust axis */
    float m_cleoConesThrust[10] = {0.}; /**< Cleo cones up to order 9, calculated respect to the thrust axis. */
    float m_harmonicMomentsCollision[10] = {0.}; /**< Harmonic moments up to order 9, calculated respect to the collision axis. */
    float m_cleoConesCollision[10] = {0.}; /**< Cleo cones up to order 9, calculated respect to the collision axis. */

    // Hemisphere related quantities
    TLorentzVector m_forwardHemisphere4Momentum; /**< Total 4-momentum of the particles in the forward hemisphere */
    TLorentzVector m_backwardHemisphere4Momentum; /**< Total 4-momentum of the particles in the backward hemisphere */

    ClassDef(EventShapeContainer, 1) /**< class definition */

  };


} // end namespace Belle2
