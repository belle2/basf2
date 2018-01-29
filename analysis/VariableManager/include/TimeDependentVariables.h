/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Li Gioi                                            *
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
     * return X component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleTagVx(const Particle* particle);

    /**
     * return Y component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleTagVy(const Particle* particle);

    /**
     * return Z component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleTagVz(const Particle* particle);


    /**
     * return MC X component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleTruthTagVx(const Particle* particle);

    /**
     * return MC Y component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleTruthTagVy(const Particle* particle);

    /**
     * return MC Z component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleTruthTagVz(const Particle* particle);

    /**
     * return the p-Value of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleTagVpVal(const Particle* particle);



    /**
     * return the error of the X component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleTagVxErr(const Particle* particle);

    /**
     * return the error of the Y component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleTagVyErr(const Particle* particle);

    /**
     * return the error of the Z component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleTagVzErr(const Particle* particle);

    /**
     * return Delta T (Brec - Btag) in ps
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleDeltaT(const Particle* particle);



    /**
     * return Delta T error in ps
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleDeltaTErr(const Particle* particle);

    /**
     * return generated Delta T (Brec - Btag) in ps
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleMCDeltaT(const Particle* particle);

    /**
     * return Delta Z (Brec - Btag) in cm
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleDeltaZ(const Particle* particle);

    /**
     * return Delta Boost direction (Brec - Btag) in cm
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleDeltaB(const Particle* particle);



    /**
     * returns the vertex component in the boost direction
     *
     */
    double vertexBoostDirection(const Particle*);

    /**
     * returns the vertex component in the direction orthogonal to the boost
     *
     */
    double vertexOrthogonalBoostDirection(const Particle*);

    /**
     * returns the MC vertex component in the boost direction
     *
     */
    double vertexTruthBoostDirection(const Particle*);

    /**
     * returns the MC vertex component in the direction orthogonal to the boost
     *
     */
    double vertexTruthOrthogonalBoostDirection(const Particle*);

    /**
     * returns the vertex error component in the boost direction
     *
     */
    double vertexErrBoostDirection(const Particle*);

    /**
     * returns the vertex error in the direction orthogonal boost direction
     *
     */
    double vertexErrOrthBoostDirection(const Particle*);



    /**
     *  Returns the TagV component in the boost direction
     *
     */
    double tagVBoostDirection(const Particle*);

    /**
     * Returns the TagV component in the direction orthogonal to the boost
     *
     */
    double tagVOrthogonalBoostDirection(const Particle*);

    /**
     * Returns the MC TagV component in the boost direction
     *
     */
    double tagVTruthBoostDirection(const Particle*);

    /**
     * Returns the MC TagV component in the direction orthogonal to the boost
     *
     */
    double tagVTruthOrthogonalBoostDirection(const Particle*);

    /**
     * Returns the error of TagV in the boost direction
     *
     */
    double tagVErrBoostDirection(const Particle*);

    /**
     * Returns the error of TagV in the direction orthogonal to the boost
     *
     */
    double tagVErrOrthogonalBoostDirection(const Particle*);

  }
}

