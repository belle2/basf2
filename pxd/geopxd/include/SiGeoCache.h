/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIGEOCACHE_H
#define SIGEOCACHE_H

#include <pxd/geopxd/SiSensorInfo.h>
#include <pxd/geopxd/CIDManager.h>

#include <TGeoManager.h>

#include <string>
#include <map>
#include <set>

namespace Belle2 {

  /**
   * SensorUID-SiSensorInfo map.
   * This is used to cache active sensor information.
   */
  typedef std::map<int, SiSensorInfo> SiSensorInfoMap;
  typedef std::map<int, SiSensorInfo>::iterator SiSensorInfoItr;

  /**
   * Map volumeName -> SensorUID.
   * Used to get SiSensorInfo by TGeoVolume name.
   */
  typedef std::map<std::string, int> SiSensorNameMap;
  typedef std::map<std::string, int>::iterator SiSensorNameItr;

  /**
   * SiGeoChache - Geometry cache class for silicon detectors.
   *
   * This class simplifies access to geometry information for other PXD and SVD classes.
   * It provides getters for basic geometry data (numbers of layers, ladders, sensor
   * dimensions etc.), and also for data that are currently not part of geometry (readout
   * structure - that is, pixel sizes and numbers, etc.). The class also provides some
   * "high-level" services, such as transformations between lab and sensor reference
   * frames, positions of pixels etc. Note that most getters are not const, as they
   * change the state of the class - to get the caching functionality.
   */
  class SiGeoCache {

  public:

    /**
     * Layer type.
     */
    enum SiLayerType {
      c_pixel, c_strip, c_otherLayerType
    };

    /**
     * SiGeoCache::instance() returns the pointer to an existing geocache,
     * or creates a new one.
     */
    static SiGeoCache* instance();

  private:

    /**
     * Constructor: hidden.
     */
    SiGeoCache();

    /**
     * Destructor: hidden.
     * Clears the sensor map and name map.
     */
    virtual ~SiGeoCache();

  public:

    /**
     * Initialize SiGeoCache maps.
     * Called from the constructor. Can be called at the start of a run.
     */
    void refresh();

    short int getSensorUID(int iLayer, int iLadder, int iSensor) {
      getSensor(iLayer, iLadder, iSensor);
      return m_currentSensor.getSensorUID();
    }

    /**
     * Get Layer ID of a sensor by its UID.
     * @param aSensorUID UID of the required sensor.
     * @return Layer ID of the sensor.
     */
    const int getLayerID(int aSensorUID) {
      getSensor(aSensorUID);
      return m_currentLayerID;
    }

    /**
     * Get Ladder ID of a sensor by its UID.
     * @param aSensorUID UID of the required sensor.
     * @return Ladder ID of the sensor.
     */
    const int getLadderID(int aSensorUID) {
      getSensor(aSensorUID);
      return m_currentLadderID;
    }

    /**
     * Get ID of a sensor by its UID.
     * @param aSensorUID UID of the required sensor.
     * @return ID of the sensor (number in its layer and ladder).
     */
    const int getSensorID(int aSensorUID) {
      getSensor(aSensorUID);
      return m_currentSensorID;
    }

    /* Basic layout getters.*/

    /**
     * Get layer type.
     * @iLayer layer number, 1-2 PXD (pixels, 3-6 SVD (strips).
     * @return SiLayerType: c_pixel or c_strip
     */
    SiLayerType getLayerType(int iLayer) const;

    /**
     * Get indices of PXD layers.
     * @return set containing indices of PXD layers.
     */
    const std::set<int>& getPXDLayerIDs() const {
      return m_iLayersPXD;
    }

    /**
     * Get indices of SVD layers.
     * @return set containing indices of SVD layers.
     */
    const std::set<int>& getSVDLayerIDs() const {
      return m_iLayersSVD;
    }

    /**
     * Get number of ladders in the current layer.
     * @param iLayer layer number.
     * @return number of ladders in a given layer.
     */
    const std::set<int>& getLadderIDs(int iLayer);

    /**
     * Get number of sensors in a given ladder.
     * @param iLayer layer number.
     * @param iLadder ladder number in the given layer.
     * @return set of IDs of sensors in a given ladder.
     */
    const std::set<int>& getSensorIDs(int iLayer, int iLadder);

    /* Sensor dimension getters.*/

    /**
     * Get sensor width in v (Z) by the sensor's UID.
     * @param aSensorUID sensor UID.
     * @return width in Z (beam direction).
     */
    double getVSensorSize(int aSensorUID) {
      getSensor(aSensorUID);
      return m_currentSensor.getVSize();
    }

    /**
     * Get sensor width in v (Z).
     * @param iLayer layer number.
     * @param iLadder ladder number in a given layer.
     * @param iSensor sensor number in a given ladder.
     * @return width in Z (beam direction).
     */
    double getVSensorSize(int iLayer, int iLadder, int iSensor) {
      getSensor(iLayer, iLadder, iSensor);
      return m_currentSensor.getVSize();
    }

    /**
     * Get sensor width in u (R-Phi).
     * @param iLayer layer number.
     * @param iLadder ladder number in a given layer.
     * @param iSensor sensor numbeadder.
     * @param v (optional) position r in a given lin the sensor.
     * @return width in R-Phi (at given v).
     */
    double getUSensorSize(int aSensorUID, double v = 0) {
      getSensor(aSensorUID);
      return m_currentSensor.getUSize(v);
    }

    /**
     * Get sensor width in u (R-Phi).
     * @param iLayer layer number.
     * @param iLadder ladder number in a given layer.
     * @param iSensor sensor number in a given ladder.
     * @param v (optional) position in the sensor.
     * @return width in R-Phi.
     */
    double getUSensorSize(int iLayer, int iLadder, int iSensor, double v = 0) {
      getSensor(iLayer, iLadder, iSensor);
      return m_currentSensor.getUSize(v);
    }

    /**
     * Get sensor thickness by the sensor's UID.
     * @param aSensorUID sensor UID.
     * @return sensor thickness.
     */
    double getSensorThickness(int aSensorUID) {
      getSensor(aSensorUID);
      return m_currentSensor.getThickness();
    }
    /**
     * Get sensor thickness.
     * @param iLayer layer number.
     * @param iLadder ladder number in a given layer.
     * @param iSensor sensor number in a given ladder.
     * @return sensor thickness.
     */
    double getSensorThickness(int iLayer, int iLadder, int iSensor) {
      getSensor(iLayer, iLadder, iSensor);
      return m_currentSensor.getThickness();
    }

    /* Pitch getters.*/

    /**
     * Get sensor pitch in v (Z) by the sensor's UID.
     * @param aSensorUID sensor UID.
     * @return pitch in Z (beam direction)
     */
    double getVSensorPitch(int aSensorUID) {
      getSensor(aSensorUID);
      return m_currentSensor.getVPitch();
    }

    /**
     * Get sensor pitch in v (Z).
     * @param iLayer layer number.
     * @param iLadder ladder number in a given layer.
     * @param iSensor sensor number in a given ladder.
     * @return pitch in Z (beam direction)
     */
    double getVSensorPitch(int iLayer, int iLadder, int iSensor) {
      getSensor(iLayer, iLadder, iSensor);
      return m_currentSensor.getVPitch();
    }

    /**
     * Get sensor pitch in u (RPhi).
     * @param aSensorUID sensor UID.
     * @param v Z coordinate.
     * @return pitch in r-phi (perpendicular to the beam axis)
     */
    double getUSensorPitch(int aSensorUID, double v = 0) {
      getSensor(aSensorUID);
      return m_currentSensor.getUPitch(v);
    }

    /**
     * Get sensor pitch in v (RPhi).
     * @param aSensorUID sensor number in a given ladder.
     * @param z Z coordinate, meaningful for fan strip sensors.
     * @return pitch in r-phi (perpendicular to the beam axis); for sensors
     * with fan strips, the pitch at z (minimum pitch if z omitted!)
     */
    double getUSensorPitch(int iLayer, int iLadder, int iSensor, double v = 0) {
      getSensor(iLayer, iLadder, iSensor);
      return m_currentSensor.getUPitch(v);
    }

    /* Pixel dim getters.*/

    /**
     * Get number of cells (pixels/strips) in R-Phi in a given sensor.
     * @param aSensorUID sensor UID.
     * @return number of cells in R-Phi direction
     */
    int getUSensorCells(int aSensorUID) {
      getSensor(aSensorUID);
      return m_currentSensor.getUCells();
    }

    /**
     * Get number of cells (pixels/strips) in R-Phi in a given sensor.
     * @param iLayer layer number.
     * @param iLadder ladder number in a given layer.
     * @param iSensor sensor number in a given ladder.
     * @return number of cells in R-Phi direction
     */
    int getUSensorCells(int iLayer, int iLadder, int iSensor) {
      getSensor(iLayer, iLadder, iSensor);
      return m_currentSensor.getUCells();
    }

    /**
     * Get number of cells in Z in a given sensor.
     * @param aSensorUID sensor UID.
     * @return number of cells in Z direction.
     */
    int getVSensorCells(int aSensorUID) {
      getSensor(aSensorUID);
      return m_currentSensor.getVCells();
    }

    /**
     * Get number of cells in Z in a given sensor.
     * @param iLayer layer number.
     * @param iLadder ladder number in a given layer.
     * @param iSensor sensor number in a given ladder.
     * @return number of cells in Z direction.
     */
    int getVSensorCells(int iLayer, int iLadder, int iSensor) {
      getSensor(iLayer, iLadder, iSensor);
      return m_currentSensor.getVCells();
    }

    /**
     * Find cell number corresponding to a given Z coordinate.
     * @param aSensorUID sensor UID.
     * @param u R-Phi coordinate in the sensor.
     * @param v (optional) Z coordinate in the sensor.
     * @return cell number (strip number or pixel row) corresponding to the given Z coordinate,
     * -1 if no such cell.
     */
    int getUSensorCellID(int aSensorUID, double u, double v = 0) {
      getSensor(aSensorUID);
      return m_currentSensor.getUCellID(u, v);
    }

    /**
     * Find cell number corresponding to a given Z coordinate.
     * @param iLayer layer number.
     * @param iLadder ladder number in a given layer.
     * @param iSensor sensor number in a given ladder.
     * @param u R-Phi coordinate in the sensor.
     * @param v (optional) Z coordinate in the sensor.
     * @return cell number (strip number or pixel row) corresponding to the given Z coordinate,
     * -1 if no such cell.
     */
    int getUSensorCellID(int iLayer, int iLadder, int iSensor, double u, double v = 0) {
      getSensor(iLayer, iLadder, iSensor);
      return m_currentSensor.getUCellID(u, v);
    }

    /**
     * Find cell number corresponding to a given v (R-Phi) coordinate.
     * @param aSensorUID sensor UID.
     * @param v R-Phi coordinate in the sensor.
     * @return cell number (strip number or pixel row) corresponding to the given Z coordinate,
     * -1 if no such cell exists.
     */
    int getVSensorCellID(int aSensorUID, double v) {
      getSensor(aSensorUID);
      return m_currentSensor.getVCellID(v);
    }

    /**
     * Find cell number corresponding to a given R-Phi coordinate.
     * @param iLayer layer number.
     * @param iLadder ladder number in a given layer.
     * @param iSensor sensor number in a given ladder.
     * @param v R-Phi coordinate in the sensor.
     * @return cell number (strip number or pixel column) corresponding to the given R-Phi coordinate,
     * -1 if no such cell.
     */
    int getVSensorCellID(int iLayer, int iLadder, int iSensor, double v) {
      getSensor(iLayer, iLadder, iSensor);
      return m_currentSensor.getVCellID(v);
    }

    /**
     * Find center coordinate of a given cell in Z.
     * @param aSensorUID sensor UID.
     * @param vID cell number in Z.
     * @return v (Z) coordinate of the cell's center.
     */
    double getVCellPosition(int aSensorUID, int vID) {
      getSensor(aSensorUID);
      return m_currentSensor.getVCellPosition(vID);
    }

    /**
     * Find center coordinate of a given cell in Z.
     * @param iLayer layer number.
     * @param iLadder ladder number in a given layer.
     * @param iSensor sensor number in a given ladder.
     * @param vID cell number in Z.
     * @return v (Z) coordinate of the cell's center.
     */
    double getVCellPosition(int iLayer, int iLadder, int iSensor, int vID) {
      getSensor(iLayer, iLadder, iSensor);
      return m_currentSensor.getVCellPosition(vID);
    }

    /**
     *  Find center coordinate of a given cell in u (R-Phi).
     * @param aSensorUID sensor UID.
     * @param uID cell number in r-phi.
     * @param vID (optional) cell number in Z.
     * @return u (r-phi) coordinate of the cell's center.
     */
    double getUCellPosition(int aSensorUID, int uID, int vID = -1) {
      getSensor(aSensorUID);
      return m_currentSensor.getUCellPosition(uID, vID);
    }

    /**
     *  Find center coordinate of a given cell in u (R-Phi).
     * @param iLayer layer number.
     * @param iLadder ladder number in a given layer.
     * @param iSensor sensor number in a given ladder.
     * @param uID cell number in R-Phi.
     * @param vID (optional) cell number in Z.
     * @return u (R-Phi) coordinate of the cell's center.
     */
    double getUCellPosition(int iLayer, int iLadder, int iSensor, int uID, int vID = -1) {
      getSensor(iLayer, iLadder, iSensor);
      return m_currentSensor.getUCellPosition(uID, vID);
    }


    /* Cell UIDs - currently common to all sensors. */

    /**
     * Get unique cell ID.
     * @param uCellID cell number in u (Z).
     * @param vCellID cell number in v (R-Phi).
     * @return cell unique ID.
     */
    int getCellUID(short int uCellID, short int vCellID);

    /**
     * Get cell u ID from a UID.
     * @param cellUID UID of the cell.
     * @return cell ID (number) in u (Z).
     */
    short int getUCellID(int aCellUID);

    /**
     * Get cell v ID from a UID.
     * @param cellUID UID of the cell.
     * @return cell ID (number) in v (R-Phi).
     */
    short int getVCellID(int aCellUID);

    /* Transforms. */

    /**
     * Transform a position vector from local (sensor plane) to global frame.
     * @param iLayer layer number.
     * @param iLadder ladder number in a given layer.
     * @param iSensor sensor number in a given ladder.
     * @param local TVector3 of local position coordinates.
     * @param master TVector3 of global position coordinates.
     */
    void localToMaster(int iLayer, int iLadder, int iSensor,
                       const TVector3& local, TVector3& master) {
      getSensor(iLayer, iLadder, iSensor);
      m_currentSensor.localToMaster(local, master);
    }

    /**
     * Transform a vector from local (sensor plane) to global frame.
     * @param iLayer layer number.
     * @param iLadder ladder number in a given layer.
     * @param iSensor sensor number in a given ladder.
     * @param local TVector3 in local coordinates.
     * @param master TVector3 in global coordinates.
     */
    void localToMasterVec(int iLayer, int iLadder, int iSensor,
                          const TVector3& local, TVector3& master) {
      getSensor(iLayer, iLadder, iSensor);
      m_currentSensor.localToMasterVec(local, master);
    }

    /**
     * Transform a position vector from global local frame.
     * @param iLayer layer number.
     * @param iLadder ladder number in a given layer.
     * @param iSensor sensor number in a given ladder.
     * @param master TVector3 of global position coordinates.
     * @param local TVector3 of local position coordinates.
     */
    void masterToLocal(int iLayer, int iLadder, int iSensor,
                       const TVector3& master, TVector3& local) {
      getSensor(iLayer, iLadder, iSensor);
      m_currentSensor.masterToLocal(master, local);
    }

    /**
     * Transform a vector from global to local (sensor) frame.
     * @param iLayer layer number.
     * @param iLadder ladder number in a given layer.
     * @param iSensor sensor number in a given ladder.
     * @param local TVector3 in global coordinates.
     * @param master TVector3 in local coordinates.
     */
    void masterToLocalVec(int iLayer, int iLadder, int iSensor,
                          const TVector3& master, TVector3& local) {
      getSensor(iLayer, iLadder, iSensor);
      m_currentSensor.masterToLocalVec(master, local);
    }

    /**
        * Transform a position vector from local (sensor plane) to global frame.
        * @param aSensorUID sensor UID.
        * @param local TVector3 of local position coordinates.
        * @param master TVector3 of global position coordinates.
        */
    void localToMaster(int aSensorUID, const TVector3& local, TVector3& master) {
      getSensor(aSensorUID);
      m_currentSensor.localToMaster(local, master);
    }

    /**
     * Transform a vector from local (sensor plane) to global frame.
     * @param aSensorUID sensor UID.
     * @param local TVector3 in local coordinates.
     * @param master TVector3 in global coordinates.
     */
    void localToMasterVec(int aSensorUID, const TVector3& local, TVector3& master) {
      getSensor(aSensorUID);
      m_currentSensor.localToMasterVec(local, master);
    }

    /**
     * Transform a position vector from global local frame.
     * @param aSensorUID sensor UID.
     * @param master TVector3 of global position coordinates.
     * @param local TVector3 of local position coordinates.
     */
    void masterToLocal(int aSensorUID, const TVector3& master, TVector3& local) {
      getSensor(aSensorUID);
      m_currentSensor.masterToLocal(master, local);
    }

    /**
     * Transform a vector from global to local (sensor) frame.
     * @param aSensorUID sensor UID.
     * @param local TVector3 in global coordinates.
     * @param master TVector3 in local coordinates.
     */
    void masterToLocalVec(int aSensorUID, const TVector3& master, TVector3& local) {
      getSensor(aSensorUID);
      m_currentSensor.masterToLocalVec(master, local);
    }


  protected:

    /**
     * Scan a current geometry node and its daughters for active sensor data.
     * @param aGeoManager - current geometry manager.
     */
    void scanCurrentNode();

    /**
     * Returns true if the most recently referenced sensor is required.
     * @param aSensorUID UID of the required sensor.
     * @return true, if m_currentSensor required.
     */
    bool isSameSensor(int aSensorUID) const {
      return (aSensorUID == m_currentSensorUID);
    }

    /**
     * Returns true if the most recently referenced sensor is required.
     * @param iLayer layer number.
     * @param iLadder ladder number in a given layer.
     * @param iSensor sensor number in a given ladder.
     * @return true, if m_currentSensor required.
     */
    bool isSameSensor(int iLayer, int iLadder, int iSensor) const {
      return (iLayer == m_currentLayerID) && (iLadder == m_currentLadderID)
             && (iSensor == m_currentSensorID);
    }

    /**
     * Get sensor by UID.
     * Sets m_currentLayerID/LadderID/SensorID and m_currentSensor.
     * @param aSensorUID UID of the sensor.
     * @return const reference to the required sensor, otherwise B2ERROR.
     */
    void getSensor(int aSensorUID);

    /**
     * Get sensor by iLadder/iLayer/iSensor.
     * Sets m_currentLayerID/LadderID/SensorID and m_currentSensor.
     * @param CID of the sensor.
     * @return const reference to the required sensor, otherwise B2ERROR.
     */
    void getSensor(int iLayer, int iLadder, int iSensor);

  private:

    std::set<int> m_iLayersPXD; /**< indices of PXD layers. */
    std::set<int> m_iLayersSVD; /**< indices of SVD layers. */

    std::set<int> m_iLadders; /**< indices of ladders for the current layer. */
    int m_iLaddersLayerID; /**< layer number for which indices of ladders were calculated. */
    std::set<int> m_iSensors; /**< indices of sensors for the current layer and ladder. */
    int m_iSensorsLayerID; /**< layer number for which indices of sensors were calculated. */
    int m_iSensorsLadderID; /**< ladder number for which indices of sensors were calculated. */

    int m_currentLayerID; /**< ID of the most recently referenced layer. */
    int m_currentLadderID; /**< ID of the most recently referenced ladder. */
    int m_currentSensorID; /**< ID of the most recently referenced sensor. */

    int m_currentSensorUID; /**< CID of the most recently used sensor. */

    SiSensorInfo m_currentSensor; /**< reference to the most recently used sensor. */

    SiSensorInfoMap m_sensors; /**< map <CID, SiSensorInfo>. */
    SiSensorNameMap m_names; /**< map <name, CID>. */

    CellUIDManager* m_cellUIDManager; /**< Cell UID codec is currently common to all sensors! */

    static SiGeoCache* m_instance;   /**< Static pointer to the instance of the class.*/

  }; // class SiGeoCache

} // end namespace

#endif
