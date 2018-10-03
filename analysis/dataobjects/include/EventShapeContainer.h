/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVENTSHAPECONTAINER_H
#define EVENTSHAPECONTAINER_H

#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>

#include <TVector3.h>
#include <TLorentzVector.h>
#include <TMatrixT.h>
#include <vector>
#include <set>

namespace Belle2 {
  /**
   * Class for collecting the basic objects related to the event shape.
   * This object is written by the EventShapeCalculator module, that stores here the quantities that
   * require long calculations like the FW moments, the sphericity(-like) tensors and the shape axes
   * (like thrust axis or jet axes). These quantities will be then used by the VaribaleManager functions
   * to get the final EventShape variable, which are usually very simple to calculate.
   * For example, the  EventShapeCalculator will store here the Fox-Wolfram moments, while the ratios of them will be calculated
   * on fly by the Variable Manager. We prefer ths approach rather than storing every event shape variable direcly
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
   * - Multipole moments from 0 to 9
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
      m_forwardEmisphere4Momentum.SetXYZT(0., 0., 0., 0.);
      m_backwardEmisphere4Momentum.SetXYZT(0., 0., 0., 0.);

      m_thrustAxis.SetXYZ(0., 0., 0.);
      for (int i = 0; i < 3; i++)
        m_sphericityEigenvector[i].SetXYZ(0., 0., 0.);
    };


    // --------------------------------------
    // Setters
    // --------------------------------------


    /**
     * Sets the i-th sphericity eigenvector
     * @param short the order (0,1,2)
     * @param TVector3 the eigenvector
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
     * @param short the order (0,1,2)
     * @param float the eigenvector
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
     * @param short The order (0 to 9)
     * @param float The Fox-Wolfram moment value
     */
    void setFWMoment(short order, float moment)
    {
      if (order < 0 || order > 9)
        B2ERROR("Invalid Fox-Wolfram moment order (n = " << order << "). The order must be in the [0,9] range.");
      else
        m_foxWolframMoments[order] = moment;
    };

    /**
     * Sets the thrustness of the event
     * @param float The thrustness  value
     */
    void setThrustness(float thrustness)
    {
      m_thrustness = thrustness;
    };

    /**
     * Sets the thrust axis, normalizing it
     * @param TVector3 thrust axis.
     */
    void setThrustAxis(TVector3 thrustAxis)
    {
      if (thrustAxis.Mag() < 1.E-10)
        B2WARNING("The thrust axis you are trying to set has magnitude numerically compatible with 0.");
      m_thrustAxis = (1. / thrustAxis.Mag()) * thrustAxis;
    };

    /**
     * Sets the spherocity axis, normalizing it
     * @param TVector3 sperocity axis. Must have magnitude equal to the thrustness value.
     */
    void setSpherocityAxis(TVector3 spherocityAxis)
    {
      if (spherocityAxis.Mag() < 1.E-10)
        B2WARNING("The spherocity axis you are trying to set has magnitude numerically compatible with 0.");
      m_spherocityAxis = (1. / spherocityAxis.Mag()) * spherocityAxis;
    };

    /**
     * Sets the multipole moment of order n, calculated using the thrust axis.
     * @param short The order (0 to 9)
     * @param float The multipole moment value
     */
    void setMultipoleMomentThrust(short order, float moment)
    {
      if (order < 0 || order > 9)
        B2ERROR("Invalid multipole moment order. It  must be in the [0,9] range.");
      else
        m_multipoleMomentsThrust[order] = moment;
    };

    /**
     * Sets the multipole moment of order n, calculated using the collision axis.
     * @param short The order (0 to 9)
     * @param float The multipole moment value
     */
    void setMultipoleMomentCollision(short order, float moment)
    {
      if (order < 0 || order > 9)
        B2ERROR("Invalid multipole moment order. It  must be in the [0,9] range.");
      else
        m_multipoleMomentsCollision[order] = moment;
    };

    /**
     * Sets the Cleo cone of order n, calculated using the thrust axis.
     * @param short The order (0 to 9)
     * @param float The Cleo cone value
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
     * @param short The order (0 to 9)
     * @param float The Cleo cone value
     */
    void setCleoConeCollision(short order, float moment)
    {
      if (order < 0 || order > 9)
        B2ERROR("Invalid Cleo cone order. It must be in the [0,9] range.");
      else
        m_cleoConesCollision[order] = moment;
    };

    /**
     * Sets the 4-momentum of the forward emisphere, as defined by the thrust axis.
     * @param TLorentzVector 4-momentum of the particles in the emisphere
     */
    void setForwardEmisphere4Momentum(TLorentzVector mom)
    {
      m_forwardEmisphere4Momentum = mom;
    };

    /**
     * Sets the 4-momentum of the backward emisphere, as defined by the thrust axis.
     * @param TLorentzVector 4-momentum of the particles in the emisphere
     */
    void setBackwardEmisphere4Momentum(TLorentzVector mom)
    {
      m_backwardEmisphere4Momentum = mom;
    };


    // --------------------------------------
    // Getters
    // --------------------------------------


    /**
     * Returns the thrustness
     * @return TVector3 the thrust axis, normalized
     */
    float getThrustness()
    {
      return m_thrustness;
    };

    /**
     * Returns the thrust axis (normalized).
     * @return TVector3 the thrust axis, normalized
     */
    TVector3 getThrustAxis()
    {
      return m_thrustAxis;
    };

    /**
     * Returns the spherocity axis (normalized).
     * @return TVector3 the spherocity axis, normalized
     */
    TVector3 getSpherocityAxis()
    {
      return m_spherocityAxis;
    };

    /**
     * Returns the i-th sphericity matrix eigenvalue.
     * @param The number of the eigenvalue (0,1,2)
     * @return float the eigenvalue
     */
    float getSphericityEigenvalue(short i)
    {
      if (i < 0 || i > 2) {
        B2ERROR("Invalid  Eigenvalue number  (n = " << i << "). There are only 3 eigenvalues...");
        return std::numeric_limits<float>::quiet_NaN();
      } else
        return m_sphericityEigenvalue[i];
    };


    /**
     * Returns the i-th sphericity matrix eigenvector.
     * @param The number of the eigenvalue (0,1,2)
     * @return float the eigenvector
     */
    TVector3 getSphericityEigenvector(short i)
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
     * @param short Order of the FW moment
     * @return float FW moment
     */
    float getFWMoment(short order)
    {
      if (order < 0 || order > 9) {
        B2ERROR("Invalid Fox-Wolfram moment order. It must be in the [0,9] range.");
        return std::numeric_limits<float>::quiet_NaN();
      } else
        return m_foxWolframMoments[order];
    };

    /**
     * Returns the multipole moment of a given order, calculated respect to the beam axis
     * @param short Order of the multipole moment
     * @return float multipole moment
     */
    float getMultipoleMomentCollision(short order)
    {
      if (order < 0 || order > 9) {
        B2ERROR("Invalid multipole moment order. It  must be in the [0,9] range.");
        return std::numeric_limits<float>::quiet_NaN();
      } else
        return m_multipoleMomentsCollision[order];
    };

    /**
     * Returns the multipole moment of a given order, calculated respect to the thrust axis
     * @param short Order of the multipole moment
     * @return float multipole moment
     */
    float getMultipoleMomentThrust(short order)
    {
      if (order < 0 || order > 9) {
        B2ERROR("Invalid multipole moment order. The order must be in the [0,9] range.");
        return std::numeric_limits<float>::quiet_NaN();
      } else
        return m_multipoleMomentsThrust[order];
    };


    /**
     * Returns the Cleo cone of a given order, calculated respect to the beam axis
     * @param short Order of the multipole moment
     * @return float Cleo cone
     */
    float getCleoConeCollision(short order)
    {
      if (order < 0 || order > 9) {
        B2ERROR("Invalid CLEO cone order. The order must be in the [0,9] range.");
        return std::numeric_limits<float>::quiet_NaN();
      } else
        return m_cleoConesCollision[order];
    };

    /**
     * Returns the Cleo cone of a given order, calculated respect to the thrust axis
     * @param short Order of the multipole moment
     * @return float Cleo cone
     */
    float getCleoConeThrust(short order)
    {
      if (order < 0 || order > 9) {
        B2ERROR("Invalid CLEO cone order. The order must be in the [0,9] range.");
        return std::numeric_limits<float>::quiet_NaN();
      } else
        return m_cleoConesThrust[order];
    };


    /**
     * Returns the 4-momentum of the forward emisphere, as defined by the thrust axis.
     * @return TLorentzVector 4-momentum of the particles in the emisphere
     */
    TLorentzVector getForwardEmisphere4Momentum()
    {
      return m_forwardEmisphere4Momentum;
    };

    /**
     * Return the 4-momentum of the backward emisphere, as defined by the thrust axis.
     * @return TLorentzVector 4-momentum of the particles in the emisphere
     */
    TLorentzVector getBackwardEmisphere4Momentum()
    {
      return m_backwardEmisphere4Momentum;
    };


  private:

    // Axes
    TVector3 m_thrustAxis; /**< Thrust axis. It is not normalized, and it's lenght is the thrust value */
    TVector3 m_spherocityAxis; /**< Spherocity axis. It is not normalized, and it's lenght is the spherocity value */

    // Axis-indepentend quantities
    TVector3 m_sphericityEigenvector[3]; /**< Sphericity tensor eigenvectors*/
    float m_sphericityEigenvalue[3] = {0.}; /**< Sphericity tensor eigenvalues*/
    float m_foxWolframMoments[10] = {0.}; /**< Fox Wolfram moments up to order 9. */

    // Axis-dependent quantities
    float m_thrustness = 0; /**< Thrustness value */
    float m_multipoleMomentsThrust[10] = {0.}; /**< Multipole moments up to order 9, calculated respect to the thrust axis */
    float m_cleoConesThrust[10] = {0.}; /**< Cleo cones up to order 9, calculated respect to the thrust axis. */
    float m_multipoleMomentsCollision[10] = {0.}; /**< Multipole moments up to order 9, calculated respect to the collision axis. */
    float m_cleoConesCollision[10] = {0.}; /**< Cleo cones up to order 9, calculated respect to the collision axis. */

    // Emisphere related quantities
    TLorentzVector m_forwardEmisphere4Momentum; /**< Total 4-momentum of the particles in the forward emisphere */
    TLorentzVector m_backwardEmisphere4Momentum; /**< Total 4-momentum of the particles in the backward emisphere */

    ClassDef(EventShapeContainer, 1) /**< class definition */

  };


} // end namespace Belle2

#endif
