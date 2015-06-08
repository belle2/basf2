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

  /** The PXD Raw Hit class
    * This class stores information about PXD Pixel hits
    * and makes them available in a root tree
    */
  class PXDRawCluster : public RelationsObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDRawCluster():
      m_length(0), m_vxdID(0)
    {};

    /**
     * @param vxdID
     * @param length
     */

    PXDRawCluster(void* data, unsigned int length, VxdID vxdID):
      m_length(length), m_vxdID(vxdID)
    {};

    /** Get the dhe ID.
     * @return ID of the sensor.
     */
    VxdID getVxdID() const
    {
      return m_vxdID;
    }

    /** Get length  of frame
    * @return length of rame
    */
    unsigned int getLength() const
    {
      return m_length;
    }

  protected:
    unsigned short m_vxdID;
    unsigned int m_length;

    // ~PXDRawCluster();

    ClassDef(PXDRawCluster, 1);
  };


} //Belle2 namespace
#endif
