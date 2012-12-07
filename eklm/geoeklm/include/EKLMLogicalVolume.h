/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Kirill Chilikin                                         *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/

#ifndef G4PVPLACEMENTGT_H_
#define G4PVPLACEMENTGT_H_

#include <G4LogicalVolume.hh>

namespace Belle2 {

  /** Mode of detector operation. */
  enum EKLMDetectorMode {
    /** Normal mode. */
    EKLM_DETECTOR_NORMAL = 0,
    /** Background study. */
    EKLM_DETECTOR_BACKGROUND = 1,
    /**
     * Debug mode.
     * When constructing geometry, print masses of all volumes and exit.
     */
    EKLM_DETECTOR_PRINTMASSES = 2,
  };

  /** Type of sensitive detector corresponding to this volume. */
  enum EKLMSensitiveType {
    /** Not sensitive. */
    EKLM_NOT_SENSITIVE = -1,
    /** Strip. */
    EKLM_SENSITIVE_STRIP = 0,
    /** SiPM. */
    EKLM_SENSITIVE_SIPM = 1,
    /** Readout boards. */
    EKLM_SENSITIVE_BOARD = 2,
  };

  /**
   * G4PVPlacement with Global Transformation information.
   */
  class EKLMLogicalVolume : public G4LogicalVolume {

  public:

    /**
     * Constructor.
     */
    EKLMLogicalVolume(EKLMLogicalVolume* mlv,
                      G4VSolid* pSolid,
                      G4Material* pMaterial,
                      const G4String& name,
                      int id = 0,
                      G4VSensitiveDetector* pSDetector = 0,
                      enum EKLMDetectorMode mode = EKLM_DETECTOR_NORMAL,
                      enum EKLMSensitiveType type = EKLM_NOT_SENSITIVE);

    /**
     * Get mother logical volume.
     */
    EKLMLogicalVolume* getMother() const;

    /**
     * Get identifier.
     */
    int getID() const;

    /**
     * Get detector mode
     */
    enum EKLMDetectorMode getMode() const;

    /**
     * Get volume type.
     */
    enum EKLMSensitiveType getVolumeType() const;


  private:

    /** Identifier. */
    int m_id;

    /** Mother. */
    EKLMLogicalVolume* m_mother;

    /** Detector mode. */
    enum EKLMDetectorMode m_mode;

    /** Sensitive detectior type. */
    enum EKLMSensitiveType m_type;

  };

}

#endif

