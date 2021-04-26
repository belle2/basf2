/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunliffe                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/variables/BasicParticleInformation.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>

namespace Belle2 {
  namespace Variable {

    double particleIsFromECL(const Particle* part)
    {
      return (part->getParticleSource() == Particle::EParticleSourceObject::c_ECLCluster);
    }

    double particleIsFromKLM(const Particle* part)
    {
      return (part->getParticleSource() == Particle::EParticleSourceObject::c_KLMCluster);
    }

    double particleIsFromTrack(const Particle* part)
    {
      return (part->getParticleSource() == Particle::EParticleSourceObject::c_Track);
    }

    double particleIsFromV0(const Particle* part)
    {
      return (part->getParticleSource() == Particle::EParticleSourceObject::c_V0);
    }

    double particleSource(const Particle* part)
    {
      return part->getParticleSource();
    }

    double particleMdstArrayIndex(const Particle* part)
    {
      return part->getMdstArrayIndex();
    }

    double uniqueParticleIdentifier(const Particle* part)
    {
      return part->getMdstSource();
    }

    double particleIsUnspecified(const Particle* part)
    {
      int properties = part->getProperty();
      return (properties & Particle::PropertyFlags::c_IsUnspecified) ? 1.0 : 0.0;
    }

    double particlePvalue(const Particle* part)
    {
      return part->getPValue();
    }

    double particleNDaughters(const Particle* part)
    {
      return part->getNDaughters();
    }

    double particleFlavorType(const Particle* part)
    {
      return part->getFlavorType();
    }

    double particleCharge(const Particle* part)
    {
      return part->getCharge();
    }

    VARIABLE_GROUP("Basic particle information");
    REGISTER_VARIABLE("isFromECL", particleIsFromECL, "Returns 1.0 if this particle was created from an ECLCluster, 0 otherwise.");
    REGISTER_VARIABLE("isFromKLM", particleIsFromKLM, "Returns 1.0 if this particle was created from a KLMCluster, 0 otherwise.");
    REGISTER_VARIABLE("isFromTrack", particleIsFromTrack, "Returns 1.0 if this particle was created from a track, 0 otherwise.");
    REGISTER_VARIABLE("isFromV0", particleIsFromV0, "Returns 1.0 if this particle was created from a V0, 0 otherwise.");
    REGISTER_VARIABLE("particleSource", particleSource, R"DOC(
      Returns mdst source used to create the particle. The meaning of the values are

      * 0: undefined
      * 1: created from track
      * 2: created from ECL cluster
      * 3: created from KLM cluster
      * 4: created from V0
      * 5: MC particle
      * 6: composite particle

      )DOC");
    REGISTER_VARIABLE("mdstIndex", particleMdstArrayIndex, R"DOC(
Store array index (0 - based) of the MDST object from which the Particle was created. 
It's 0 for composite particles.

.. tip:: 
    It is not a unique identifier of particle. For example, a pion and a gamma can have the same `mdstIndex`:
    pions are created from tracks whereas gammas are created from ECL clusters; tracks and
    ECL clusters are stored in different arrays. A photon may be created from ECL cluster with index 0 and a
    pion may be created from track with index 0 will both have :b2:var:`mdstIndex` equal to 0, but they will be different particles.

.. tip:: 
    Two particles of the same type can also have the same :b2:var:`mdstIndex`. This would mean that they are created from the same object. 
    For example, if pion and kaon have the same :b2:var:`mdstIndex` it means that they are created from the same track.


 .. tip::
    If you are looking for unique identifier of the particle, please use :b2:var:`uniqueParticleIdentifier`.
    )DOC");
    REGISTER_VARIABLE("uniqueParticleIdentifier", uniqueParticleIdentifier, R"DOC(
Returns unique identifier of final state particle.
Particles created from the same object (e.g. from the same track) have different :b2:var:`uniqueParticleIdentifier` value.)DOC");

    REGISTER_VARIABLE("isUnspecified", particleIsUnspecified,
                      "Returns 1 if the particle is marked as an unspecified object (like B0 -> @Xsd e+ e-), 0 otherwise");
    REGISTER_VARIABLE("chiProb", particlePvalue, R"DOC(
A context-dependent :math:`\chi^2` probability for 'the fit' related to this particle.

* If this particle is track-based, then this is the pvalue of the track fit (identical to :b2:var:`pValue`).
* If this particle is cluster-based then this variable is currently unused.
* If this particle is composite and a vertex fit has been performed, then this is the :math:`\chi^2` probability of the vertex fit result.

.. tip:: 
    If multiple vertex fits are performed then the last one sets the ``chiProb`` overwriting all previous.

.. seealso:: :b2:var:`pValue` for tracks
    )DOC");
    REGISTER_VARIABLE("nDaughters", particleNDaughters,
                      "Returns number of daughter particles");
    REGISTER_VARIABLE("flavor", particleFlavorType,
                      "Returns 1.0 if particle is flavored type, 0.0 if it is unflavored.");
    REGISTER_VARIABLE("charge", particleCharge, "Returns electric charge of particle.");
  }
}
