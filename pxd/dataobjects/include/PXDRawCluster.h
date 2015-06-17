/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck / Klemens Lautenbach                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDRAWCLUSTER_H
#define PXDRAWCLUSTER_H


#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /** The PXD Raw Cluster class
    * This class stores Cluster as whole without unpacking
    */
  class PXDRawCluster : public RelationsObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDRawCluster():
      m_length(0), m_cluster(0), m_vxdID(0) {};

    /**
     * @param length
     * @param data
     * @param vxdID
     */

    PXDRawCluster(unsigned short* data, unsigned int length, VxdID vxdID);

    unsigned int getLength() const
    {
      return m_length;
    }

    /** Get static pointer to data.
     * @param j Index of m_cluster
     * @return pointer.
     */
    unsigned short getData(unsigned int j) const
    {
      return (m_cluster[j]);
    }

    /** Get the dhe ID.
    * @return ID of the sensor.
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

    ClassDef(PXDRawCluster, 3);
  };


} //Belle2 namespace
#endif
