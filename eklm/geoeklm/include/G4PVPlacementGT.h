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

#include <G4PVPlacement.hh>
#include <G4LogicalVolume.hh>
#include <G4Transform3D.hh>
#include <vector>

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
  class G4PVPlacementGT : public G4PVPlacement {

  public:

    /**
     * Constructor.
     * This constructor is used when mother physical volume is not a
     * G4PVPlacementGT.
     */
    G4PVPlacementGT(const G4Transform3D& globalTransform,
                    const G4Transform3D& Transform3D,
                    G4LogicalVolume* pCurrentLogical,
                    const G4String& pName,
                    G4LogicalVolume* pMotherLogical,
                    int id = 0,
                    enum EKLMDetectorMode mode = EKLM_DETECTOR_NORMAL);

    /**
     * Constructor.
     * This constructor is used when mother physical volume is
     * a G4PVPlacementGT.
     */
    G4PVPlacementGT(G4PVPlacementGT* motherPVPlacementGT,
                    const G4Transform3D& Transform3D,
                    G4LogicalVolume* pCurrentLogical,
                    const G4String& pName,
                    int id = 0,
                    enum EKLMDetectorMode mode = EKLM_DETECTOR_NORMAL);

    /**
     * Get transformation.
     */
    G4Transform3D getTransform();

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

    /**
     * Set volume type.
     */
    void setVolumeType(enum EKLMSensitiveType) ;

    /**
     * Get mother G4PVPlacementGT.
     */
    const G4PVPlacementGT* getMother() const;

    /**
     * Get volume name.
     */
    const std::string getName() const;

  private:

    /**
     * Transformation from local to global coordinate.
     * This is the transformation of point (not coordinate system).
     */
    G4Transform3D m_transform;

    /** Identifier. */
    int m_id;

    /** Detector mode. */
    enum EKLMDetectorMode m_mode;

    /** Sensitive detectior type. */
    enum EKLMSensitiveType m_type;

    /** Name. */
    const std::string m_name;

    /** Mother G4PVPlacementGT. */
    const G4PVPlacementGT* m_mother;

  };

}

#endif

