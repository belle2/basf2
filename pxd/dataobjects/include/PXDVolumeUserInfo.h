/********************************** ****************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDVOLUMEUSERINFO_H_
#define PXDVOLUMEUSERINFO_H_

#include <geometry/dataobjects/VolumeUserInfoBase.h>
#include <framework/gearbox/Unit.h>

#include <TObject.h>

class G4LogicalVolume;
class TG4ROOTDetectorConstruction;

/** Default name of the G4 region of PXD active silicons. */
#define PXDACTIVE_G4REGION_NAME "PXD_Active_Region"

namespace Belle2 {

  /** Default step size.
   *  This is now the only place where Geant4 step length limit can be
   *  defined. Set to something positive to make step limit active.
   *  Will be fixed in future.
   */
  const double stepLengthInPXD = -1.0 * Unit::um;

  /**
   * Default range cut.
   * The range cut defines minimum energy (expressed as path length in Si)
   * for a knock-on (delta) electron to be explicitly simulated by Geant4.
   * We need this value to be set for Si detectors, as it must be smaller than
   * the Geant4 default for the whole BelleII detector. The rule of thumb is to
   * take a value close to the typical charge spread in the sensor.
   */
  const double rangeCutInPXD = -1.0 * Unit::um;

  /**
    * PXDVolumeUserInfo - Additional information for a PXD sensitive volume.
    *
    * This class holds additional information for PXD sensitive volumes, which are used
    * later to set this information for a PXDSimHit.
    */
  class PXDVolumeUserInfo : public VolumeUserInfoBase {

  public:

    /** Empty constructor, required for ROOT IO.*/
    PXDVolumeUserInfo(): VolumeUserInfoBase(),
        m_layerID(-1),
        m_ladderID(-1),
        m_sensorID(-1),
        m_uPitch(1),
        m_uCells(1),
        m_vPitch(1),
        m_vCells(1) {
      m_stepSize = stepLengthInPXD;
      m_rangeCut = rangeCutInPXD;
    }

    /** Full constructor.
     * @param layerID ID of the layer.
     * @param ladderID ID of the ladder.
     * @param sensorID ID of th1e sensor.
     * @param uPitch sensor pitch in u ("r-phi") direciton.
     * @param uCells number of cells in u ("r-phi") direction.
     * @param vPitch sensor pitch in v ("z") direction.
     * @param vCells number of cells in v ("z") direction.
     */
    PXDVolumeUserInfo(
      int layerID,
      int ladderID,
      int sensorID,
      double uPitch,
      int uCells,
      double vPitch,
      int vCells): VolumeUserInfoBase(),
        m_layerID(layerID),
        m_ladderID(ladderID),
        m_sensorID(sensorID),
        m_uPitch(uPitch),
        m_uCells(uCells),
        m_vPitch(vPitch),
        m_vCells(vCells) {
      m_stepSize = stepLengthInPXD;
      m_rangeCut = rangeCutInPXD;
    }

    /** Destructor */
    ~PXDVolumeUserInfo() {
      /* Does nothing. */
    }

    /** Sets the layer ID.
      * @param layerID The layer ID of the sensitive volume.
      */
    void setLayerID(int layerID) { m_layerID = layerID; }

    /** The method to set LadderID.*/
    void setLadderID(int ladderID) { m_ladderID = ladderID; }

    /** The method to set SensorID.*/
    void setSensorID(int sensorID) { m_sensorID = sensorID; }

    /** The method to set u pitch.*/
    void setUPitch(double uPitch) { m_uPitch = uPitch; }

    /** The method to set number of cells in u.*/
    void setUCells(int uCells) { m_uCells = uCells; }

    /** The method to set v pitch.*/
    void setVPitch(double vPitch) { m_vPitch = vPitch; }

    /** The method to set number of cells in v.*/
    void setVCells(int vCells) { m_vCells = vCells; }

    // NOTE: There is no setter for range cut. I don't want it to be set
    // detector-wise in the geometry xml.

    /** The method to get layer id.*/
    int getLayerID() const { return m_layerID; }

    /** The method to get ladder id.*/
    int getLadderID() const { return m_ladderID; }

    /** The method to get sensor id.*/
    int getSensorID() const { return m_sensorID; }

    /** The method to get u pitch.*/
    double getUPitch() const { return m_uPitch; }

    /** The method to get number of cells in u.*/
    int getUCells() const { return m_uCells; }

    /** The method to get v pitch.*/
    double getVPitch() const { return m_vPitch; }

    /** The method to get number of cells in v.*/
    int getVCells() const { return m_vCells; }

    /** The method to get range cut value. */
    double getRangeCut() const {return m_rangeCut; }

    /** Assignment operator.*/
    PXDVolumeUserInfo& operator=(const PXDVolumeUserInfo& other);

    /**
     * Overloaded to create the Geant4 region for active PXD volumes and set appropriate range cut.
     */
    void updateG4Volume(G4VPhysicalVolume* g4Volume, TG4RootDetectorConstruction *detConstruct);

  private:

    int m_layerID; /**< Layer number. */
    int m_ladderID; /**< Ladder number. */
    int m_sensorID; /**< Sensor number. */
    double m_uPitch; /**< Pitch in u ("r-phi"). */
    int m_uCells; /**< Number of cells in u ("r-phi"). */
    double m_vPitch; /**< Pitch in v ("z"). */
    int m_vCells; /**< Number of cells in v ("z"). */
    double m_rangeCut; /**< Range cut determines the minimum path/energy of delta e- that are explicitly simulated.*/

    ClassDef(PXDVolumeUserInfo, 1)

  };

} // end namespace Belle2

#endif /* PXDVOLUMEUSERINFO_H_ */
