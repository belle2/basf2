/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDVOLUMEUSERINFO_H_
#define PXDVOLUMEUSERINFO_H_

#include <geometry/dataobjects/VolumeUserInfoBase.h>
#include <framework/gearbox/Unit.h>

#include <TObject.h>

//class G4VPhysicalVolume;
//class TG4RootDetectorConstruction;

namespace Belle2 {

  /** Default step size.
   *  This is now the only place where Geant4 step length limit can be
   *  defined. Set to something positive to make step limit active.
   *  Will be fixed in future.
   */
  const double stepLengthInPXD = -1.0 * Unit::um;

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
        m_uPitch(0),
        m_uCells(1),
        m_vPitch(0),
        m_vCells(1) {
      m_stepSize = stepLengthInPXD;
    }

    /** Full constructor.
     * @param layerID ID of the layer.
     * @param ladderID ID of the ladder.
     * @param sensorID ID of the sensor.
     * @param uPitch sensor pitch in u ("z") direciton.
     * @param uCells number of cells in u ("z") direction.
     * @param vPitch sensor pitch in v ("r-phi") direction.
     * @param vCells number of cells in v ("r-phi") direction.
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

    /** Assignment operator.*/
    PXDVolumeUserInfo& operator=(const PXDVolumeUserInfo& other);

  private:

    int m_layerID;           /**< Layer number. */
    int m_ladderID;          /**< Ladder number. */
    int m_sensorID;          /**< Sensor number. */
    double m_uPitch;        /**< Pitch in u ("z"). */
    int m_uCells;         /**< Number of cells in u ("z"). */
    double m_vPitch;        /**< Pitch in v ("r-phi"). */
    int m_vCells;         /**< Number of cells in v ("r-phi"). */

    ClassDef(PXDVolumeUserInfo, 1)

  };

} // end namespace Belle2

#endif /* PXDVOLUMEUSERINFO_H_ */
