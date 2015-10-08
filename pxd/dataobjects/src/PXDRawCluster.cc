/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach
 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/dataobjects/PXDRawCluster.h>
#include <boost/spirit/home/support/detail/endian.hpp>

using namespace std;
using namespace Belle2;

PXDRawCluster::PXDRawCluster(unsigned short* data, unsigned int length,  VxdID vxdID):
  m_length(length), m_cluster(0), m_vxdID(vxdID)
{
  m_cluster = new unsigned short[m_length];//
  boost::spirit::endian::ubig16_t* d = (boost::spirit::endian::ubig16_t*)data;
  for (unsigned int i = 0 ; i < m_length ; i++) {
    m_cluster[i] = d[i]; //We have to do 16bit endian swap, TODO VERIFY this!
  }
};

PXDRawCluster::~PXDRawCluster()
{
  delete m_cluster;
};
