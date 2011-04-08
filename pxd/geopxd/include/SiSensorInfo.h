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
        m_layerID(-1), m_ladderID(-1), m_sensorID(-1), m_SensorUID(-1),
        m_shape(c_otherShape), m_uSize(0), m_vSize(0), m_vSize2(0), m_thickness(0),
        m_uPitch(1), m_vPitch(1), m_vPitch2(1), m_uCells(1), m_vCells(1)
    {;}


    /**
     * Constructor.
     *
     * @param a TGeoNode from which to make a new entry.
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
    int getSensorUID() const { return m_SensorUID; }


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
    double getUSize() const { return m_uSize; }


    /**
     * Get sensor width in Z.
     * @return width in Z (beam direction).
     */
    double getVSize() const {
      if (m_shape != c_rectangular) {
        B2ERROR("Incorrect SiSensorInfo method for non-rectangular detector !!!")
        return -1;
      } else
        return m_vSize;
    }


    /**
     * Get sensor width in R-Phi.
     * @param u R-Phi coordinate, at which the width is desired (for trapezoidal sensors)
     * @return width in R-Phi (perpendicular to the beam direction).
     */
    double getVSize(double u) const {
      return m_vSize *(0.5*m_uSize + u) + m_vSize2 *(0.5*m_uSize - u);
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
    double getUPitch() const { return m_uPitch; }


    /**
     * Get sensor pitch in R-Phi. For rectangular sensors.
     * @return pitch in v (R-Phi)
     */
    double getVPitch() const {
      if (m_shape != c_rectangular) {
        B2ERROR("Incorrect SiSensorInfo method for non-rectangular detector !!!")
        return -1;
      } else
        return m_vPitch;
    }


    /**
     * Get sensor pitch in RPhi. For trapezoidal sensors.
     * @param u U ("Z") coordinate.
     * @return pitch in v (R-Phi) (perpendicular to the beam axis).
     */
    double getVPitch(double u) const {
      return m_vPitch *(0.5*m_uSize + u) + m_vPitch2 *(0.5*m_uSize - u);
    }


    //Pixel dim getters

    /**
     * Get number of cells (pixels/strips) in Z.
     * @return number of cells in Z direction
     */
    int getUCells() const { return m_uCells; }


    /**
     * Get number of cells in R-Phi in a given sensor.
     * @return number of cells in R-Phi direction.
     */
    int getVCells() const { return m_vCells; }


    // Cell IDs to coordinates and v.v.

    /**
     * Find cell number corresponding to a given Z coordinate.
     * @param u Z coordinate in the sensor.
     * @return cell number (strip number or pixel row) corresponding to the given Z coordinate,
     * -1 if no such cell.
     */
    int getUCellID(double u) const ;


    /**
     * Find cell number corresponding to a given R-Phi coordinate.
     * @param v R-Phi coordinate in the sensor.
     * @return cell number (strip number or pixel column) corresponding to the given R-Phi coordinate,
     * -1 if no such cell.
     */
    int getVCellID(double v) const ;


    /**
     * Find cell number corresponding to a given R-Phi coordinate.
     * @param u Z cooridnate in the sensor, needed for trapezoidal sensors.
     * @param v R-Phi coordinate in the sensor.
     * @return cell number (strip number or pixel column) corresponding to the given R-Phi coordinate,
     * -1 if no such cell.
     */
    int getVCellID(double u, double v) const ;


    /**
     * Find Z coordinate of the centre of a given cell.
     * @param uID cell number in Z.
     * @return u (Z) coordinate of the cell's center.
     */
    double getUCellPosition(int uID) const {
      return (uID + 0.5) * m_uPitch - 0.5*m_uSize;
    }


    /**
     * Find R-Phi coordinate of the centre of a given cell.
     * @param vID cell number in r-phi.
     * @return v (r-phi) coordinate of the cell's center.
     */
    double getVCellPosition(int vID) const {
      if (m_shape != c_rectangular) {
        B2ERROR("Incorrect SiSensorInfo method for non-rectangular detector !!!")
        return 0;
      }
      return (vID + 0.5) * m_vPitch - 0.5*m_vSize;
    }


    /**
     * Find R-Phi coordinate of the centre of a given cell.
     * @param uID cell number in z, needed for trapezoidal centers.
     * @param vID cell number in r-phi.
     * @return v (r-phi) coordinate of the cell's center.
     */
    double getVCellPosition(int uID, int vID) const {
      double u = getUCellPosition(uID);
      return (vID + 0.5) * getVPitch(u) - 0.5 * getVSize(u);
    }


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
    int m_layerID;      /**< Layer ID of the sensor. */
    int m_ladderID;     /**< Ladder ID of the sensor. */
    int m_sensorID;     /**< Sensor ID of the sensor. */
    int m_SensorUID;        /**< Compact ID of the sensor. */

    // Dimensions and shape.
    SensorShape m_shape;  /**< c_rectangular or c_trapezoidal. */
    double m_uSize;     /**< width of sensor in u (Z). */
    double m_vSize;     /**< width of sensor in v (R-Phi). */
    double m_vSize2;    /**< width of sensor in v (R-Phi) for trapezoidal sensors. */
    double m_thickness;   /**< sensor thickness. */

    // Readout
    double m_uPitch;    /**< pitch in U (Z). */
    double m_vPitch;    /**< pitch in V (R-Phi). */
    double m_vPitch2;   /**< other pitch in V (R-Phi) for trapezoidal sensors. */
    int m_uCells;     /**< number of cells in u. */
    int m_vCells;     /**< number of cells in v. */

    // Transform
    const TGeoHMatrix* m_transform;  /**< Pointer to the sensor's transformation matrix. */

  }; // class SiSensorInfo



} // mamespace Belle2

#endif /* SISENSORINFO_H_ */
