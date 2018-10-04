/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>

#include <vector>
#include <string>

namespace Belle2 {
  class Particle;

  namespace Variable {


    /**
     * Returns the ratio of the i-th to the 0-th order Fox Wolfram moments. The order can go up to 8th.
     */
    Manager::FunctionPtr foxWolframR(const std::vector<std::string>& arguments);

    /**
     * Returns the i-th  order Fox Wolfram moments. The order can go up to 8th.
     */
    Manager::FunctionPtr foxWolframH(const std::vector<std::string>& arguments);

    /**
     * Returns the ratio of the i-th order cleo cone with respect to either the thrust axis or the z axis (i.e. collision axis).
     */
    Manager::FunctionPtr cleoCone(const std::vector<std::string>& arguments);

    /**
     * Returns the ratio of the 1st to the 0th order Fox-Wolfram moment
     */
    double foxWolframR1(const Particle* particle);

    /**
     * Returns the ratio of the 2nd to the 0th order Fox-Wolfram moment
     */
    double foxWolframR2(const Particle* particle);

    /**
     * Returns the ratio of the 3rd to the 0th order Fox-Wolfram moment
     */
    double foxWolframR3(const Particle* particle);

    /**
     * Returns the ratio of the 4th to the 0th order Fox-Wolfram moment
     */
    double foxWolframR4(const Particle* particle);


    /**
     * Returns the sphericity value
     */
    double sphericity(const Particle* particle);

    /**
     * Returns the aplanarity value
     */
    double aplanarity(const Particle* particle);

    /**
     * Returns the thrust value
     */
    double thrust(const Particle* particle);

    /**
     * Returns the X component of the thrust axis
     */
    double thrustAxisX(const Particle* particle);

    /**
     * Returns the Y component of the thrust axis
     */
    double thrustAxisY(const Particle* particle);

    /**
     * Returns the Y component of the thrust axis
     */
    double thrustAxisY(const Particle* particle);

    /**
     * Returns cos(polar_angle) of the thrust axis
     */
    double thrustAxisCosTheta(const Particle* particle);


    /**
     * Returns the invariant mass of the forward emisphere, as defined by the thrust axis
     */
    double forwardEmisphereMass(const Particle* particle);

    /**
     * Returns the X component of the momentum of the forward emisphere, as defined by the thrust axis
     */
    double forwardEmisphereX(const Particle* particle);

    /**
     * Returns the Y component of the momentum of the forward emisphere, as defined by the thrust axis
     */
    double forwardEmisphereY(const Particle* particle);

    /**
     * Returns the Z component of the momentum of the forward emisphere, as defined by the thrust axis
     */
    double forwardEmisphereZ(const Particle* particle);

    /**
     * Returns  the momentum of the forward emisphere, as defined by the thrust axis
     */
    double forwardEmisphereMomentum(const Particle* particle);



    /**
     * Returns the invariant mass of the backward emisphere, as defined by the thrust axis
     */
    double backwardEmisphereMass(const Particle* particle);

    /**
     * Returns the X component of the momentum of the backward emisphere, as defined by the thrust axis
     */
    double backwardEmisphereX(const Particle* particle);

    /**
     * Returns the Y component of the momentum of the backward emisphere, as defined by the thrust axis
     */
    double backwardEmisphereY(const Particle* particle);

    /**
     * Returns the Z component of the momentum of the backward emisphere, as defined by the thrust axis
     */
    double backwardEmisphereZ(const Particle* particle);

    /**
     * Returns  the momentum of the backward emisphere, as defined by the thrust axis
     */
    double backwardEmisphereMomentum(const Particle* particle);


    /**
     * Returns the variable in respect to rotated coordinates, in which z lies on the specified thrust axis.
     */
    Manager::FunctionPtr useThrustFrame(const std::vector<std::string>& arguments);

  }
}
