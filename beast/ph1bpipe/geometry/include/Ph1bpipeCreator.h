/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PH1BPIPECREATOR_H_
#define PH1BPIPECREATOR_H_

#include <geometry/CreatorBase.h>
#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "TString.h"
#include "Riostream.h"

class G4Box;
class G4Tubs;
class G4LogicalVolume;
class G4VPhysicalVolume;
class G4Material;
class G4UniformMagField;
class DetectorMessenger;
class G4ProductionCuts;
class G4Region;
class G4VSolid;
class G4UnionSolid;
class G4SubtractionSolid;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the PH1BPIPE detector */
  namespace ph1bpipe {

    class SensitiveDetector;

    /** The creator for the PH1BPIPE geometry. */
    class Ph1bpipeCreator : public geometry::CreatorBase {
    public:
      Ph1bpipeCreator();
      virtual ~Ph1bpipeCreator();
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* PH1BPIPECREATOR_H_ */
