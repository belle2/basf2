/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef BEAST_CLAWS_GEOMETRY_CLAWSCREATOR_H
#define BEAST_CLAWS_GEOMETRY_CLAWSCREATOR_H

#include <geometry/CreatorBase.h>
#include <simulation/kernel/SensitiveDetectorBase.h>

class G4LogicalVolume;
class G4Material;
class G4UserLimits;


namespace Belle2 {
  /** Namespace to encapsulate code needed for the CLAWS detector */
  namespace claws {

    /** The creator for the CLAWS geometry. */
    class CLAWSCreator : public geometry::CreatorBase {
    public:
      /** Constructor
       * @param sensitive detector instance to use, if null a
       * claws::SensitiveDetector instance will be created.
       */
      CLAWSCreator(Simulation::SensitiveDetectorBase* sensitive = nullptr);
      virtual ~CLAWSCreator();
      void createShape(const std::string prefix, const GearDir& params, G4LogicalVolume* parent, double roffset, bool check);
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /** pointer to the sensitive detector implementation */
      Simulation::SensitiveDetectorBase* m_sensitive{nullptr};
      /** pointer to the material in the top volume to use as default */
      G4Material* m_topMaterial{nullptr};
      /** pointer to the G4Userlimits to set for sensitive volumes (if any) */
      G4UserLimits* m_stepLength{nullptr};
    };

  }
}

#endif /* BEAST_CLAWS_GEOMETRY_CLAWSCREATOR_H */
