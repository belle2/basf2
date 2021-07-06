/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <vxd/dataobjects/VxdID.h>
#include <vector>


namespace Belle2 {

  /** The PXD Cluster class
    * This class stores all information about reconstructed PXD clusters
    * The position error matrix has the form
    * ((m_uSigma^2, m_uvRho * m_uSigma * m_vSigma),
    *  (m_uvRho * m_uSigma * m_vSigma, m_vSigma^2))
    *  The correlation coefficient is derived from the shape of the cluster.
    *  The cluster shape ID is use for correction of bias and error of position.
    */
  class PXDCluster: public RelationsObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDCluster():
      m_sensorID(0), m_uPosition(0), m_vPosition(0), m_uPositionSigma(1),
      m_vPositionSigma(1), m_uvRho(0),  m_clsCharge(0), m_seedCharge(0),
      m_clsSize(0), m_uSize(0), m_vSize(0), m_uStart(0), m_vStart(0),
      m_clsKind(0) {}

    /** Constructor.
     * @param sensorID Sensor compact ID.
     * @param uPosition Cluster u coordinate (r-phi).
     * @param vPosition Cluster v coordinate (z).
     * @param uError Error (estimate) of uPosition.
     * @param vError Error (estiamte) of vPosition.
     * @param uvRho u-v error correlation coefficient.
     * @param clsCharge The cluster charge.
     * @param seedCharge The charge of the cluster seed.
     * @param clsSize size of the cluster in pixels.
     * @param uSize number of pixel columns contributing to the cluster.
     * @param vSize number of pixel rows contributing to the cluster.
     * @param uStart Start column of the cluster.
     * @param vStart Start row of the cluster.
     */
    PXDCluster(VxdID sensorID, float uPosition, float vPosition, float uError,
               float vError, float uvRho, unsigned short clsCharge, unsigned short seedCharge,
               unsigned short clsSize, unsigned short uSize, unsigned short vSize,
               unsigned short uStart, unsigned short vStart):
      m_sensorID(sensorID), m_uPosition(uPosition), m_vPosition(vPosition),
      m_uPositionSigma(uError), m_vPositionSigma(vError),
      m_uvRho(uvRho), m_clsCharge(clsCharge),
      m_seedCharge(seedCharge),  m_clsSize(clsSize), m_uSize(uSize),
      m_vSize(vSize), m_uStart(uStart), m_vStart(vStart), m_clsKind(0)
    {}

    /** Constructor for who want to add also cluster kind in one step.
     * @param sensorID Sensor compact ID.
     * @param uPosition Cluster u coordinate (r-phi).
     * @param vPosition Cluster v coordinate (z).
     * @param uError Error (estimate) of uPosition.
     * @param vError Error (estiamte) of vPosition.
     * @param uvRho u-v error correlation coefficient.
     * @param clsCharge The cluster charge.
     * @param seedCharge The charge of the cluster seed.
     * @param clsSize size of the cluster in pixels.
     * @param uSize number of pixel columns contributing to the cluster.
     * @param vSize number of pixel rows contributing to the cluster.
     * @param uStart Start column of the cluster.
     * @param vStart Start row of the cluster.
     * @param clsKind ClusterKind of the cluster.
     */
    PXDCluster(VxdID sensorID, float uPosition, float vPosition, float uError,
               float vError, float uvRho, unsigned short clsCharge, unsigned short seedCharge,
               unsigned short clsSize, unsigned short uSize, unsigned short vSize,
               unsigned short uStart, unsigned short vStart, int clsKind):
      m_sensorID(sensorID), m_uPosition(uPosition), m_vPosition(vPosition),
      m_uPositionSigma(uError), m_vPositionSigma(vError),
      m_uvRho(uvRho), m_clsCharge(clsCharge),
      m_seedCharge(seedCharge),  m_clsSize(clsSize), m_uSize(uSize),
      m_vSize(vSize), m_uStart(uStart), m_vStart(vStart), m_clsKind(clsKind)
    {}

    /** Constructor for who want to add also cluster kind, shape indices and eta values in one step.
     * @param sensorID Sensor compact ID.
     * @param uPosition Cluster u coordinate (r-phi).
     * @param vPosition Cluster v coordinate (z).
     * @param uError Error (estimate) of uPosition.
     * @param vError Error (estiamte) of vPosition.
     * @param uvRho u-v error correlation coefficient.
     * @param clsCharge The cluster charge.
     * @param seedCharge The charge of the cluster seed.
     * @param clsSize size of the cluster in pixels.
     * @param uSize number of pixel columns contributing to the cluster.
     * @param vSize number of pixel rows contributing to the cluster.
     * @param uStart Start column of the cluster.
     * @param vStart Start row of the cluster.
     * @param clsKind ClusterKind of the cluster.
     * @param etaValues The eta values of the cluster for sectors of incidence angles
     * @param shapeIndices The shapeIndices of the cluster for sectors of incidence angles
     */
    PXDCluster(VxdID sensorID, float uPosition, float vPosition, float uError,
               float vError, float uvRho, unsigned short clsCharge, unsigned short seedCharge,
               unsigned short clsSize, unsigned short uSize, unsigned short vSize,
               unsigned short uStart, unsigned short vStart, int clsKind, const std::vector<float>& etaValues,
               const std::vector<int>& shapeIndices):
      m_sensorID(sensorID), m_uPosition(uPosition), m_vPosition(vPosition),
      m_uPositionSigma(uError), m_vPositionSigma(vError),
      m_uvRho(uvRho), m_clsCharge(clsCharge),
      m_seedCharge(seedCharge),  m_clsSize(clsSize), m_uSize(uSize),
      m_vSize(vSize), m_uStart(uStart), m_vStart(vStart), m_clsKind(clsKind), m_etaValues(etaValues), m_shapeIndices(shapeIndices)
    {}

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const { return m_sensorID; }

    /** Get u coordinate of hit position.
     * @return u coordinate of hit position.
     */
    float getU() const { return m_uPosition; }

    /** Get v coordinate of hit position.
     * @return v coordinate of hit position.
     */
    float getV() const { return m_vPosition; }

    /** Get error of u coordinate of hit position.
     * @return Error of u coordinate of hit position.
     */
    float getUSigma() const { return m_uPositionSigma; }

    /** Get error in v coordinate of hit position.
     * @return Error in v coordinate of hit position.
     */
    float getVSigma() const { return m_vPositionSigma; }

    /** Get hit position error covariance coefficient
     * @return Covariance coefficient of hit position errors.
     */
    float getRho() const { return m_uvRho; }

    /** Get collected charge.
     * @return charge collected in the cluster.
     */
    unsigned short getCharge() const { return m_clsCharge; }

    /** Get seed charge.
     * @return seed charge of the cluster.
     */
    unsigned short getSeedCharge() const { return m_seedCharge; }

    /** Get cluster size.
     * @return number of pixels contributing to the cluster.
     */
    unsigned short getSize() const { return m_clsSize; }

    /** Get cluster size in u direction.
     * @return number of pixel columns contributing to the cluster.
     */
    unsigned short getUSize() const { return m_uSize; }

    /** Get cluster size in v direction.
     * @return number of pixel rows contributing to the cluster.
     */
    unsigned short getVSize() const { return m_vSize; }

    /** Get cluster start cell in u direction.
     * @return first column contributing to the cluster.
     */
    unsigned short getUStart() const { return m_uStart; }

    /** Get cluster start cell in v direction.
     * @return first row contributing to the cluster.
     */
    unsigned short getVStart() const { return m_vStart; }

    /** Get cluster kind.
     * @return cluster kind of the cluster.
     */
    int getKind() const { return m_clsKind; }

    /** Get cluster kind.
     * @return cluster shape of the cluster.
     * FIXME REMOVE THIS member
     */
    short getShape() const { return 0; }

    /** Set cluster kind.
     */
    void setKind(short NewClsKind) { m_clsKind = NewClsKind; }

    /** Set u coordinate of hit position.
     */
    void setU(float NewuPosition) { m_uPosition = NewuPosition; }

    /** Set v coordinate of hit position.
     */
    void setV(float NewvPosition) { m_vPosition = NewvPosition; }

    /** Set error of u coordinate of hit position.
     */
    void setUSigma(float NewuError) { m_uPositionSigma = NewuError; }

    /** Set error in v coordinate of hit position.
     */
    void setVSigma(float NewvError) { m_vPositionSigma = NewvError; }

    /** Set error covariance coefficient of hit position.
     */
    void setRho(float NewRho) { m_uvRho = NewRho; }

    /** Get vector of sector shape indices. Sectors in thetaU and thetaV are numbered ++, -+, --, +-.
     */
    const std::vector<int>& getSectorShapeIndices() const { return m_shapeIndices; }

    /** Get vector of sector shape indices. Sectors in thetaU and thetaV are numbered ++, -+, --, +-.
     */
    std::vector<int>& getSectorShapeIndices() { return m_shapeIndices; }

    /** Get vector of sector eta values. Sectors in thetaU and thetaV are numbered ++, -+, --, +-.
     */
    const std::vector<float>& getSectorEtaValues() const { return m_etaValues; }

    /** Get vector of sector eta values. Sectors in thetaU and thetaV are numbered ++, -+, --, +-.
     */
    std::vector<float>& getSectorEtaValues() { return m_etaValues; }

  protected:
    unsigned short m_sensorID;    /**< Compressed sensor identifier.*/
    float m_uPosition;            /**< Absolute cell position in r-phi. */
    float m_vPosition;            /**< Absolute cell position in z. */
    float m_uPositionSigma;       /**< Error in u position. */
    float m_vPositionSigma;       /**< Error in v position. */
    float m_uvRho;                /**< Cluster shape correlation. */
    unsigned short m_clsCharge;   /**< Deposited charge in ADC units. */
    unsigned short m_seedCharge;  /**< Cluster seed charge in ADC units. */
    unsigned short m_clsSize;     /**< Cluster size in pixels */
    unsigned short m_uSize;       /**< Cluster size in pixel columns */
    unsigned short m_vSize;       /**< Cluster size in pixel rows  */
    unsigned short m_uStart;      /**< Start column of the cluster */
    unsigned short m_vStart;      /**< Start row of the cluster */
    int m_clsKind;                /**< Cluster kind */
    std::vector<float> m_etaValues;    /**< Vector of eta values for four angle sectors */
    std::vector<int> m_shapeIndices;   /**< Vector of shape indices for four angle sectors */


    ClassDef(PXDCluster, 5)
  };


} //Belle2 namespace
