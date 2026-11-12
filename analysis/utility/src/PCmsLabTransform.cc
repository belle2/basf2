/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/utility/PCmsLabTransform.h>

#include <initializer_list>

using namespace Belle2;

namespace {
  /** Beam parameters from the database and the quantities derived from them, shared by all PCmsLabTransform instances. */
  struct CmsTransformCache {
    /** DB object for the invariant mass. */
    DBObjPtr<CollisionInvariantMass> invariantMass;
    /** DB object for the boost vector. */
    DBObjPtr<CollisionBoostVector> boostVector;
    /** DB object for the collision axis in CM system from the boost. */
    DBObjPtr<CollisionAxisCMS> axisCms;

    /** True if the cached values below are up to date */
    bool valid = false;
    /** CMS energy, i.e. the invariant mass of the e+e- system (sqrt(s)) */
    double cmsEnergy = 0;
    /** Boost vector */
    ROOT::Math::XYZVector boost;
    /** Lorentz transformation from Lab to CMS */
    ROOT::Math::LorentzRotation labToCms;
    /** Lorentz transformation from CMS to Lab */
    ROOT::Math::LorentzRotation cmsToLab;
    /** LAB four-momentum of e+e- */
    ROOT::Math::PxPyPzEVector beamFourMomentum;

    CmsTransformCache()
    {
      for (DBAccessorBase* accessor : std::initializer_list<DBAccessorBase*> {&invariantMass, &boostVector, &axisCms}) {
        // Invalidate when a payload changes (new run or intra-run dependency) and when the DBStore entry is
        // destroyed (DBStore reset): in the latter case the accessor is reattached on its next use, which
        // happens when the cache is recomputed.
        accessor->addCallback([this]() { valid = false; }, false);
        accessor->addCallback([this]() { valid = false; }, true);
      }
    }

    /** Returns the cached values, recomputing them if needed. */
    const CmsTransformCache& get()
    {
      if (!valid) {
        cmsEnergy = invariantMass->getMass();
        boost = ROOT::Math::XYZVector(boostVector->getBoost());
        labToCms = LabToCms::rotateLabToCms(boost, axisCms->getAngleXZ(), axisCms->getAngleYZ());
        cmsToLab = labToCms.Inverse();
        beamFourMomentum = cmsToLab * ROOT::Math::PxPyPzEVector(0, 0, 0, cmsEnergy);
        valid = true;
      }
      return *this;
    }
  };

  /** Lazily constructed on first use, i.e. when the framework and the database are set up. */
  const CmsTransformCache& cmsTransform()
  {
    static CmsTransformCache cache;
    return cache.get();
  }
}

ROOT::Math::XYZVector PCmsLabTransform::getBoostVector()
{
  return cmsTransform().boost;
}

double PCmsLabTransform::getCMSEnergy()
{
  return cmsTransform().cmsEnergy;
}

ROOT::Math::PxPyPzEVector PCmsLabTransform::getBeamFourMomentum()
{
  return cmsTransform().beamFourMomentum;
}

const ROOT::Math::LorentzRotation PCmsLabTransform::rotateLabToCms()
{
  return cmsTransform().labToCms;
}

const ROOT::Math::LorentzRotation PCmsLabTransform::rotateCmsToLab()
{
  return cmsTransform().cmsToLab;
}

ROOT::Math::PxPyPzMVector PCmsLabTransform::labToCms(const ROOT::Math::PxPyPzMVector& vector)
{
  PCmsLabTransform T;
  return T.rotateLabToCms() * vector;
}

ROOT::Math::PxPyPzMVector PCmsLabTransform::cmsToLab(const ROOT::Math::PxPyPzMVector& vector)
{
  PCmsLabTransform T;
  return T.rotateCmsToLab() * vector;
}

ROOT::Math::PxPyPzEVector PCmsLabTransform::labToCms(const ROOT::Math::PxPyPzEVector& vector)
{
  PCmsLabTransform T;
  return T.rotateLabToCms() * vector;
}

ROOT::Math::PxPyPzEVector PCmsLabTransform::cmsToLab(const ROOT::Math::PxPyPzEVector& vector)
{
  PCmsLabTransform T;
  return T.rotateCmsToLab() * vector;
}

PCmsLabTransform::PCmsLabTransform() = default;
