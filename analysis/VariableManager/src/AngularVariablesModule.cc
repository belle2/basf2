/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hikari Hirata                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VariableManager/AngularVariablesModule.h>

/* --------------- WARNING ---------------------------------------------- *
If you have more complex parameter types in your class then simple int,
double or std::vector of those you might need to uncomment the following
include directive to avoid an undefined reference on compilation.
 * ---------------------------------------------------------------------- */
// #include <framework/core/ModuleParam.templateDetails.h>


namespace Belle2 {
  namespace Variable {
    double helicityAngle(const Particle* particle, const std::vector<double>& daughter_indices)
    {
      // for the calculation of the helicity angle we need particle (=particle in the argument)
      // its daughter and granddaughter. The particle is given as an argument, but the daughter
      // and granddaughter are specified as the indices in the vector (second argument)
      // daughter_indices[0] = index of the daughter
      // daughter_indices[1] = index of the granddaughter

      if (!particle)
        return -999.9;

      int nDaughters = particle->getNDaughters();

      if (nDaughters < 2)
        return -999.9;

      // get the daughter particle
      int daughterIndex = daughter_indices[0];
      const Particle* daughter = particle->getDaughter(daughterIndex);

      nDaughters = daughter->getNDaughters();

      if (nDaughters < 2)
        return -999.9;

      // get the granddaughter
      int grandDaughterIndex = daughter_indices[1];

      const Particle* grandDaughter = daughter->getDaughter(grandDaughterIndex);
      // do the calculation
      TLorentzVector particle4Vector  = particle->get4Vector();
      TLorentzVector daughter4Vector   = daughter->get4Vector();
      TLorentzVector gDaughter4Vector = grandDaughter->get4Vector();

      TVector3 boost2daughter = -(daughter4Vector.BoostVector());

      particle4Vector.Boost(boost2daughter);
      gDaughter4Vector.Boost(boost2daughter);

      TVector3 particle3Vector      = particle4Vector.Vect();
      TVector3 gDaughter3Vector = gDaughter4Vector.Vect();
      double numerator   = gDaughter3Vector.Dot(particle3Vector);
      double denominator = (gDaughter3Vector.Mag()) * (particle3Vector.Mag());

      return numerator / denominator;
    }
    VARIABLE_GROUP("AngularVariables");
    REGISTER_VARIABLE("helicityAngle(i,j)", helicityAngle,
                      "cosine of the angle between particle->getDaughter(i)->getDaughter(j) and this particle in the particle->getDaughter(i) rest frame.");


  } // Variable namespace
} // Belle2 namespace

