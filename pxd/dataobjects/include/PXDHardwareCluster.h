/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDHARDWARECLUSTER_H
#define PXDHARDWARECLUSTER_H


#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/RelationsObject.h>
#include <pxd/reconstruction/Pixel.h>

namespace Belle2 {

  /** The PXD Hardware Cluster class
    * This class stores information of Hardware Clusters
    */
  class PXDHardwareCluster : public RelationsObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDHardwareCluster():
      m_nrPixel(0), m_pixelU(0), m_pixelV(0), m_pixelQ(0), m_seedCharge(0), m_seedU(0), m_seedV(0), m_clusterCharge(0), m_vxdID(0),
      m_chipID(0) {};

    /**
     * @param nrPixel   //number of pixel in one cluster
     * @param pixelU    //U coordinate of the pixel, row
     * @param pixelV    //V coordiante of the Pixel, column
     * @param pixelQ    //Charge of the Pixel
     * @param seedCharge  //seed charge of the cluster (pixel with highest charge)
     * @param seedU //U position (row) of seed pixel
     * @param seedV //V position (column) of seed pixel
     * @param clusterCharge //total cluster charge
     * @param vxdID   //Vertex Detector ID of the half ladder from which the cluster originates
     * @param chipID //ID of dhp chip the cluster originates from
     */

    ~PXDHardwareCluster();

    PXDHardwareCluster(unsigned short* pixelU, unsigned short* pixelV, unsigned char* pixelQ, unsigned int nrPixel,
                       unsigned char seedCharge,
                       unsigned short seedU, unsigned short seedV, unsigned int clusterCharge, VxdID vxdID, unsigned int chipID);


    /** Get the number of pixel in cluster.
    * @return number of pixel.
    */
    unsigned int getNrPixels() const
    {
      return m_nrPixel;
    }

    /** Get static pointer to U coordinate of Pixel.
     * @param j Index of Pixel in cluster
     * @return pointer.
     */
    unsigned short getPixelU(unsigned int j) const
    {
      return (m_pixelU[j]);
    }

    /** Get static pointer to V coordinate of Pixel.
    * @param j Index of Pixel in cluster
    * @return pointer.
    */
    unsigned short getPixelV(unsigned int j) const
    {
      return (m_pixelV[j]);
    }

    /** Get static pointer to Charge of Pixel.
     * @param j Index of Pixel in cluster
     * @return pointer.
     */
    unsigned char getPixelQ(unsigned int j) const
    {
      return (m_pixelQ[j]);
    }

    /** Get the clusters seed charge.
    * @return seed charge of cluster.
    */
    unsigned char getSeedCharge() const
    {
      return m_seedCharge;
    }

    /** Get the clusters seed charge.
    * @return seed charge of cluster.
    */
    unsigned short getSeedU() const
    {
      return m_seedU;
    }

    /** Get the clusters seed charge.
    * @return seed charge of cluster.
    */
    unsigned short getSeedV() const
    {
      return m_seedV;
    }

    /** Get the clusters total charge.
    * @return total charge of cluster.
    */
    unsigned int getClusterCharge() const
    {
      return m_clusterCharge;
    }

    /** Get the Vertex Detector ID of the half ladder the cluster originates from.
    * @return Vertex Detector ID.
    */
    VxdID getVxdID() const
    {
      return m_vxdID;
    }

    /** Get the ID of the dhp chip the cluster originates from.
    * @return Vertex Detector ID.
    */
    unsigned int getChipID() const
    {
      return m_chipID;
    }

  protected:
    unsigned int m_nrPixel;
    unsigned short* m_pixelU; // [m_nrPixel] /**< Buffer of size m_nrPixel shorts  */
    unsigned short* m_pixelV; // [m_nrPixel] /**< Buffer of size m_nrPixel shorts  */
    unsigned char* m_pixelQ;  // [m_nrPixel] /**< Buffer of size m_nrPixel shorts  */
    unsigned char m_seedCharge;
    unsigned short m_seedU;
    unsigned short m_seedV;
    unsigned int m_clusterCharge;
    unsigned short m_vxdID;
    unsigned int m_chipID;
    // ~PXDHardwareCluster();

    ClassDef(PXDHardwareCluster, 11);
  };


} //Belle2 namespace
#endif
