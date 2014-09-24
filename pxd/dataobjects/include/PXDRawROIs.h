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


#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /** The PXD Raw ROIs class
    * This class stores information about the ROIs processed by ONSEN
    * and makes them available in a root tree.
    * This is mainly for debugging and crosscheck.
    * Attention: Format is depending on hardware implementation and not yet fixed!
    */
  class PXDRawROIs : public RelationsObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDRawROIs():
      m_2timesNrROIs(0), m_rois(0) {};


    /** Constructor with data
     * @param nrroi Nr of Rois to follow in data ptr.
     * @param data pointer to ROIs .
     */
    PXDRawROIs(unsigned int nrroi, unsigned int* data):
      m_2timesNrROIs(2 * nrroi), m_rois(0) {
      m_rois = new int[m_2timesNrROIs];
      for (unsigned int i = 0; i < m_2timesNrROIs; i++) m_rois[i] = ((data[i] >> 16) & 0xFFFF) | ((data[i] & 0xFFFF) << 16); // word swap
    };

    /** Get the nr of ROIs.
     * @param j Index of ROI
     * @return Nr of ROIs.
     */
    unsigned int getNrROIs() const {
      return m_2timesNrROIs / 2;
    }

    /** Return DHH ID of ROI j
     * @param j Index of ROI
     * @return DHH ID
     */
    int getDHHID(int j) const {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return (m_rois[2 * j] >> 4) & 0x3F; // & 0x3F0
    }

    /** Return Row 1 of ROI j
     * @param j Index of ROI
     * @return  Row 1
     */
    int getRow1(int j) const {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return ((m_rois[2 * j] << 6) & 0x3C0) | ((m_rois[2 * j + 1] >> 26) & 0x3F) ;//  & 0x00F , & 0xFC000000
    }

    /** Return Row 2 of ROI j
     * @param j Index of ROI
     * @return Row 2
     */
    int getRow2(int j) const {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return (m_rois[2 * j + 1] >> 8) & 0x3FF;  // & 0x0003FF00
    }

    /** Return Col 1 of ROI j
     * @param j Index of ROI
     * @return Column 1
     */
    int getCol1(int j) const {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return (m_rois[2 * j + 1] >> 18) & 0xFF; // & 0x03FC0000
    }

    /** Return Col 1 of ROI j
     * @param j Index of ROI
     * @return Column 2
     */
    int getCol2(int j) const {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return (m_rois[2 * j + 1]) & 0xFF;
    }

    /** Return Type (Datcon or HLT) of ROI j
     * @param j Index of ROI
     * @return Type of Roi
     */
    int getType(int j) const {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return (m_rois[2 * j] >> 10) & 0x1; //  & 0x400
    }

  protected:
    unsigned int m_2timesNrROIs;/**< Number of ROIs times two (size of one ROI is 2*32bit) */
    int* m_rois; // [m_2timesNrROIs] /**< Buffer of size 2*m_NrROIs ints  */


    // ~PXDRawROIs();

    ClassDef(PXDRawROIs, 3)
  };


} //Belle2 namespace
#endif
