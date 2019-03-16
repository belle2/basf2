/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDRAWCLUSTER_H
#define PXDRAWCLUSTER_H


#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /** The PXD Raw Cluster class
    * This class stores Hardware Cluster as whole without unpacking
    */
  class PXDRawCluster : public RelationsObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDRawCluster():
      m_length(0), m_cluster(0), m_vxdID(0) {};

    /**
     * @param length  //length of cluster frame in shorts
     * @param data  //pointer to the first word in cluster frame
     * @param vxdID //Vertex Detector ID of the half ladder the cluster originates
     */

    ~PXDRawCluster();

    PXDRawCluster(unsigned short* data, unsigned int length, VxdID vxdID);

    unsigned int getLength() const //length of cluster in short words
    {
      return m_length;
    }

    /** Get static pointer to data.
     * @param j Index of pointer in data
     * @return pointer.
     */
    unsigned short getData(unsigned int j) const
    {
      return (m_cluster[j]);
    }

    /** Get the Vertex Detector ID of the half ladder the cluster originates from.
    * @return Vertex Detector ID.
    */
    VxdID getVxdID() const
    {
      return m_vxdID;
    }

  protected:
    unsigned int m_length;
    unsigned short* m_cluster; // [m_length] /**< Buffer of size m_length shorts  */
    unsigned short m_vxdID;

    // ~PXDRawCluster();

    ClassDef(PXDRawCluster, 5);
  };


} //Belle2 namespace
#endif
