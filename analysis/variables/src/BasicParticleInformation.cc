/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/variables/BasicParticleInformation.h>

// include VariableManager
#include <analysis/VariableManager/Manager.h>

#include <analysis/dataobjects/Particle.h>

namespace Belle2 {
  namespace Variable {

    bool particleIsFromECL(const Particle* part)
    {
      return (part->getParticleSource() == Particle::EParticleSourceObject::c_ECLCluster);
    }

    bool particleIsFromKLM(const Particle* part)
    {
      return (part->getParticleSource() == Particle::EParticleSourceObject::c_KLMCluster);
    }

    bool particleIsFromTrack(const Particle* part)
    {
      return (part->getParticleSource() == Particle::EParticleSourceObject::c_Track);
    }

    bool particleIsFromV0(const Particle* part)
    {
      return (part->getParticleSource() == Particle::EParticleSourceObject::c_V0);
    }

    int particleSource(const Particle* part)
    {
      return part->getParticleSource();
    }

    int particleMdstArrayIndex(const Particle* part)
    {
      return part->getMdstArrayIndex();
    }

    int uniqueParticleIdentifier(const Particle* part)
    {
      return part->getMdstSource();
    }

    bool particleIsUnspecified(const Particle* part)
    {
      int properties = part->getProperty();
      return (properties & Particle::PropertyFlags::c_IsUnspecified) ? true : false;
    }

    double particlePvalue(const Particle* part)
    {
      return part->getPValue();
    }

    int particleNDaughters(const Particle* part)
    {
      return part->getNDaughters();
    }

    int particleFlavorType(const Particle* part)
    {
      return part->getFlavorType();
    }

    double particleCharge(const Particle* part)
    {
      return part->getCharge();
    }

    VARIABLE_GROUP("Basic particle information");
    REGISTER_VARIABLE("isFromECL", particleIsFromECL,
                      "Returns 1.0 if this particle was created from an ``ECLCluster`` or 0.0 otherwise.");
    REGISTER_VARIABLE("isFromKLM", particleIsFromKLM,
                      "Returns 1.0 if this particle was created from a ``KLMCluster`` or 0.0 otherwise.");
    REGISTER_VARIABLE("isFromTrack", particleIsFromTrack, "Returns 1.0 if this particle was created from a track or 0.0 otherwise.");
    REGISTER_VARIABLE("isFromV0", particleIsFromV0, R"DOC(

.. only:: not light

    Returns 1.0 if this particle was created from a :ref:`V0 particle <tracking_v0Finding>` or 0.0 otherwise.
    
)DOC");
    REGISTER_VARIABLE("particleSource", particleSource, R"DOC(

.. only:: not light

    Returns the mDST source use to create the particle. 

    The meaning of the values are:

    * 0: undefined
    * 1: created from track
    * 2: created from an ``ECLCluster``
    * 3: created from a ``KLMCluster``
    * 4: reated from a ref:`V0 particle <tracking_v0Finding>`
    * 5: MC particle
    * 6: composite particle

)DOC");
    REGISTER_VARIABLE("mdstIndex", particleMdstArrayIndex, R"DOC(
Returns the store array index (0 - based) of the mDST object from which the particle was created. For composite particles, this returns 0.0.

.. caution:: 
    Two particles of the same type can also have the same :b2:var:`mdstIndex`. This would mean that they are created from the same object.
    For example, if pion and kaon have the same :b2:var:`mdstIndex` it means that they are created from the same track.

.. warning:: 
    This variable is not a unique identifier of the particle. For example, a pion and a photon can have the same `mdstIndex`; even though
    pions are created from tracks and photons are created from ECL clusters, as tracks and
    ECL clusters are kept in different store arrays they can return the same index.

.. seealso::
    If you are looking for unique identifier of the particle, please use :b2:var:`uniqueParticleIdentifier`.

)DOC");
    REGISTER_VARIABLE("uniqueParticleIdentifier", uniqueParticleIdentifier, R"DOC(
Returns the unique identifier of a final state particle.
Particles created from the same object (e.g. from the same track) have different :b2:var:`uniqueParticleIdentifier` values.

)DOC");

    REGISTER_VARIABLE("isUnspecified", particleIsUnspecified, R"DOC(
Returns 1.0 if the particle is marked as an unspecified object (like B0 -> @Xsd e+ e-) or 0.0 otherwise.

.. note:: You can read more about how to define such particles here: :ref:`Marker of unspecified particle <Marker_of_unspecified_particle>`.

)DOC");
    REGISTER_VARIABLE("chiProb", particlePvalue, R"DOC(
Returns a context-dependent :math:`\chi^2` probability for 'the fit' related to this particle.

The contexts are:

* If this particle is track-based, then it returns the p-value of the track fit (identical to :b2:var:`pValue`).
* If this particle is composite, and a vertex fit has been performed, then it returns the :math:`\chi^2` probability of the vertex fit result.
* If this particle is cluster-based then this variable is currently unused.

.. attention:: 
    If multiple vertex fits have performed then the last one sets the ``chiProb`` value and overwrites all the previous ones.

)DOC");
    REGISTER_VARIABLE("nDaughters", particleNDaughters, "Returns number of daughter particles or 0.0 otherwise for a particle with no daughters.");
    REGISTER_VARIABLE("flavor", particleFlavorType, "Returns 1.0 if particle has flavour or 0.0 if it is unflavored.");
    REGISTER_VARIABLE("charge", particleCharge, "Returns the electric charge of particle in units of :math:`e`.");
  }
}
