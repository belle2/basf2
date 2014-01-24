/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:Bjoern Spruck                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDRAWROIS_H
#define PXDRAWROIS_H


#include <TObject.h>

namespace Belle2 {
  /** \addtogroup dataobjects
    * @{
    */

  /** The PXD Raw ROIs class
    * This class stores information about the ROIs processed by ONSEN
    * and makes them available in a root tree
    */
  class PXDRawROIs : public TObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDRawROIs():
      m_2timesNrROIs(0), m_header1(0), m_header2(0), m_header3(0), m_header4(0), m_rois(0) {};


    /**
     * @param nr rois.
     */
    PXDRawROIs(unsigned int nrroi, unsigned int h1, unsigned int h2, unsigned int h3, unsigned int h4, int* data):
      m_2timesNrROIs(2 * nrroi), m_header1(h1), m_header2(h2), m_header3(h3), m_header4(h4), m_rois(0) {
      m_rois = new int[m_2timesNrROIs];
      memcpy(m_rois, data, m_2timesNrROIs * sizeof(int));
    };

    /** Get the nr of ROIs.
     * @return Nr of ROIs.
     */
    unsigned int getNrROIs() const {
      return m_2timesNrROIs / 2;
    }

    int getDHHID(int j) const {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return (m_rois[2 * j] & 0x3F0) >> 4;
    }
    int getRow1(int j) const {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return ((m_rois[2 * j] & 0x00F) << 6) | ((m_rois[2 * j + 1] & 0x03FC0000) >> 26) ;
    }
    int getRow2(int j) const {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return (m_rois[2 * j + 1] & 0x0003FF00) >> 8;
    }
    int getCol1(int j) const {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return (m_rois[2 * j + 1] & 0x03FC0000) >> 18;
    }
    int getCol2(int j) const {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return (m_rois[2 * j + 1] & 0xFF);
    }
    int getType(int j) const {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return (m_rois[2 * j] & 0x400) >> 10;
    }

  protected:
    unsigned int m_2timesNrROIs;
    unsigned int m_header1;
    unsigned int m_header2;
    unsigned int m_header3;
    unsigned int m_header4;
    int* m_rois; // [m_2timesNrROIs] /**< Buffer of size 2*m_NrROIs ints  */


    // ~PXDRawROIs();

    ClassDef(PXDRawROIs, 1)
  };

  /** @}*/

} //Belle2 namespace
#endif
