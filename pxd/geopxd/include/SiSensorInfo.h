/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef SISENSORINFO_H_
#define SISENSORINFO_H_

#include <framework/logging/Logger.h>

#include <TVector3.h>
#include <TGeoMatrix.h>
#include <TGeoNode.h>

namespace Belle2 {


  /**
   * SiSensorInfo: active sensor information and trandformations.
   */
  class SiSensorInfo {
  public:

    /** Supported detector types. */
    enum DetectorType {
      c_PXD,
      c_SVD,
      c_otherDetector
    };

    /**
     * Sensor shape types.
     *
     * For trapezoidal sensors, a different set of getters and setters should be used.
     */
    enum SensorShape {
      c_rectangular,
      c_trapezoidal,
      c_otherShape
    };

    /**
     * Default constructor.
     */
    SiSensorInfo():
        m_detectorType(c_otherDetector),
        m_layerID(-1), m_ladderID(-1), m_sensorID(-1), m_sensorUID(-1),
        m_shape(c_otherShape), m_vSize(0), m_uSize(0), m_uSizeD(0), m_thickness(0),
        m_vPitch(1), m_uPitch(1), m_uPitchD(1), m_vCells(1), m_uCells(1)
    {;}


    /**
     * Constructor.
     *
     * @param a TGeoNode from which to make a new active sensor entry.
     */
    SiSensorInfo(TGeoNode* pNode);


    /**
     * Destructor.
     */
    virtual ~SiSensorInfo();


    /**
     * Get detector type.
     * @return DetectorType::c_PXD or DetectorType::c_SVD.
     */
    DetectorType getDetectorType() const { return m_detectorType; }

    /**
    * Get layer ID of the sensor.
    * @return layer number of the sensor.
    */
    int getLayerID() const { return m_layerID; }

    /**
    * Get ladder ID of the sensor.
    * @return ladder number of the sensor.
    */
    int getLadderID() const { return m_ladderID; }

    /**
     * Get sensor ID of the sensor.
     * @return sensor number of the sensor.
     */
    int getSensorID() const { return m_sensorID; }

    /**
     * Get sensor CID.
     * @return CID (compact layer/ladder/sensor ID of the sensor.
     */
    int getSensorUID() const { return m_sensorUID; }


    //Sensor shape and dimension getters

    /**
     * Get sensor shape type.
     * @return SensorShape type, c_rectangular or c_trapezoidal.
     */
    SensorShape getShape() const { return m_shape; }


    /**
     * Get sensor width in Z.
     * @return width in Z (beam direction).
     */
    double getVSize() const { return m_vSize; }

    /**
     * Get sensor width in R-Phi.
     * @param u R-Phi coordinate, at which the width is desired (for trapezoidal sensors)
     * @return width in R-Phi (perpendicular to the beam direction).
     */
    double getUSize(double v = 0) const {
      return m_uSize + m_uSizeD * v;
    }


    /**
     * Get sensor thickness.
     * @return sensor thickness.
     */
    double getThickness() const { return m_thickness; }


    //Pitch getters

    /**
     * Get sensor pitch in Z.
     * @return pitch in Z (beam direction)
     */
    double getVPitch() const { return m_vPitch; }


    /**
     * Get sensor pitch in RPhi.
     * @param v "Z" coordinate.
     * @return pitch in u (R-Phi)
     */
    double getUPitch(double v = 0) const {
      return m_uPitch + m_uPitchD * v;
    }


    //Pixel dim getters

    /**
     * Get number of cells (pixels/strips) in Z.
     * @return number of cells in Z direction
     */
    int getVCells() const { return m_vCells; }


    /**
     * Get number of cells in R-Phi in a given sensor.
     * @return number of cells in R-Phi direction.
     */
    int getUCells() const { return m_uCells; }


    // Cell IDs to coordinates and v.v.

    /**
     * Find cell number corresponding to a given Z coordinate.
     * @param v Z coordinate in the sensor.
     * @return cell number (strip number or pixel row) corresponding to the given Z coordinate,
     * -1 if no such cell.
     */
    int getVCellID(double v) const ;

    /**
     * Find cell number corresponding to a given R-Phi coordinate.
     * @param u R-Phi coordinate in the sensor.
     * @param v Z coordinate in the sensor, needed for trapezoidal sensors.
     * @return cell number (strip number or pixel column) corresponding to the given R-Phi coordinate,
     * -1 if no such cell.
     */
    int getUCellID(double u, double v = 0) const ;


    /**
     * Find Z coordinate of the centre of a given cell.
     * @param uID cell number in Z.
     * @return u (Z) coordinate of the cell's center.
     */
    double getVCellPosition(int vID) const {
      return (vID + 0.5) * m_vPitch - 0.5 * m_vSize;
    }


    /**
     * Find R-Phi coordinate of the centre of a given cell.
     * @param uID cell number in z, needed for trapezoidal centers.
     * @param vID cell number in r-phi.
     * @return v (r-phi) coordinate of the cell's center.
     */
    double getUCellPosition(int uID, int vID = -1) const;


    // Transforms

    /**
     * Transform a position vector from local (sensor plane) to global frame.
     * @param local TVector3 of local position coordinates.
     * @param master TVector3 of global position coordinates.
     */
    void localToMaster(const TVector3& local, TVector3& master) const ;


    /**
     * Transform a vector from local (sensor plane) to global frame.
     * @param local TVector3 in local coordinates.
     * @param master TVector3 in global coordinates.
     */
    void localToMasterVec(const TVector3& local, TVector3& master) const ;


    /**
     * Transform a position vector from global local frame.
     * @param master TVector3 of global position coordinates.
     * @param local TVector3 of local position coordinates.
     */
    void masterToLocal(const TVector3& master, TVector3& local) const ;


    /**
     * Transform a vector from global to local (sensor) frame.
     * @param local TVector3 in global coordinates.
     * @param master TVector3 in local coordinates.
     */
    void masterToLocalVec(const TVector3& master, TVector3& local) const ;


  private:

    // Detector
    DetectorType m_detectorType; /**< PXD or SVD. */

    // Sensor IDs
    int m_layerID; /**< Layer ID of the sensor. */
    int m_ladderID; /**< Ladder ID of the sensor. */
    int m_sensorID; /**< Sensor ID of the sensor. */
    int m_sensorUID; /**< Compact ID of the sensor. */

    // Dimensions and shape.
    SensorShape m_shape;  /**< c_rectangular or c_trapezoidal. */
    double m_vSize;     /**< width of sensor in v (Z). */
    double m_uSize;     /**< (mean) width of sensor in u (R-Phi) at v=0. */
    double m_uSizeD;    /**< gradient of width in u (R-Phi) along v. */
    double m_thickness;   /**< sensor thickness. */

    // Readout
    double m_vPitch;    /**< pitch in v (Z). */
    double m_uPitch;    /**< pitch in u (R-Phi) at v=0. */
    double m_uPitchD;   /**< gradient of u pitch along v.*/
    int m_vCells;     /**< number of cells in v. */
    int m_uCells;     /**< number of cells in u. */

    // Transform
    const TGeoHMatrix* m_transform;  /**< Pointer to the sensor's transformation matrix. */

  }; // class SiSensorInfo

} // mamespace Belle2

#endif /* SISENSORINFO_H_ */
