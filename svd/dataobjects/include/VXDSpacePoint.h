/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDSPACEPOINT_H
#define VXDSPACEPOINT_H

#include <TObject.h>

#define DEFAULT_VXDSPACEPOINTS "VXDSpacePoints"
#define DEFAULT_VXDSPACEPOINTSREL "MCParticlesToVXDSpacePoints"

namespace Belle2 {

  /*
   ** The spacepoint class for SVD and PXD.
   *
   * The SpacePoint class carries basically the same data as PXD/SVD hit,
   * but in global coordinates.
   */

  class VXDSpacePoint : public TObject {

  public:

    /** Default constructor for ROOT IO. */
    VXDSpacePoint() {;}

    /** Useful Constructor. */
    VXDSpacePoint(int sensorUniID, int layerID, int ladderID, int sensorID,
                  float x, float y, float z, float energyDep) :
        m_sensorUID(sensorUniID), m_layerID(layerID), m_ladderID(ladderID),
        m_sensorID(sensorID), m_x(x), m_y(y), m_z(z), m_energyDep(energyDep)
    {;}


    /** Set compressed layer/ladder/sensor id.*/
    void setSensorUniID(int uniID) { m_sensorUID = uniID; }

    /** Set layer id.*/
    void setLayerID(int layerID) { m_layerID = layerID; }

    /** Set compressed ladder id.*/
    void setLadderID(int ladderID) { m_ladderID = ladderID; }

    /** Set sensor id.*/
    void setSensorID(int sensorID) { m_sensorID = sensorID; }

    /** Set x coordinate.*/
    void setX(float x) { m_x = x; }

    /** Set y coordinate.*/
    void setY(float y) { m_y = y; }

    /** Set z coordinate.*/
    void setZ(float z) { m_z = z; }

    /** Set deposited energy.*/
    void setEnergyDep(float energyDep) { m_energyDep = energyDep; }

    /** Get the compact ID.*/
    int getSensorUniID() const { return m_sensorUID; }

    /** Get the compact ID.*/
    int getLayerID() const { return m_layerID; }

    /** Get ladder ID.*/
    int getLadderID() const { return m_ladderID; }

    /** Get sensor ID.*/
    int getSensorID() const { return m_sensorID; }

    /** Get x coordinate.*/
    float getX() const { return m_x; }

    /** Get y coordinate.*/
    float getY() const { return m_y; }

    /** Get z coordinate.*/
    float getZ() const { return m_z; }

    /** Get deposited energy. */
    float getEnergyDep() const { return m_energyDep; }

  private:

    int m_sensorUID;                /**< Compressed sensor identifier.*/
    int m_layerID;                  /**< Layer ID. */
    int m_ladderID;                 /**< Ladder ID. */
    int m_sensorID;                 /**< SensorID. */
    float m_x;                      /**< x coordinate of the hit.*/
    float m_y;                      /**< y coordinate of the hit.*/
    float m_z;                      /**< z coordinate of the hit.*/
    float m_energyDep;              /**< deposited energy.*/

    ClassDef(VXDSpacePoint, 1);

  };

} // end namespace Belle2

#endif
