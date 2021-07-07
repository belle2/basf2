/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

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
      m_2timesNrROIs(0), m_rois(NULL) {};


    /** Constructor with data
     * @param nrroi Nr of Rois to follow in data ptr.
     * @param data pointer to ROIs .
     */
    PXDRawROIs(unsigned int nrroi, unsigned int* data);

    /** Destructor
     * */
    ~PXDRawROIs();

    /** Get the nr of ROIs.
     * @param j Index of ROI
     * @return Nr of ROIs.
     */
    unsigned int getNrROIs() const
    {
      return m_2timesNrROIs / 2;
    }

    /** Return DHH ID of ROI j
     * @param j Index of ROI
     * @return DHH ID
     */
    int getDHHID(int j) const
    {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return (m_rois[2 * j] >> 4) & 0x3F; // & 0x3F0
    }

    /** Return MinVid (Row 1) of ROI j
     * @param j Index of ROI
     * @return  MinVid
     */
    int getMinVid(int j) const
    {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return ((m_rois[2 * j] << 6) & 0x3C0) | ((m_rois[2 * j + 1] >> 26) & 0x3F) ;//  & 0x00F , & 0xFC000000
    }

    /** Return MaxVid (Row 2) of ROI j
     * @param j Index of ROI
     * @return MaxVid
     */
    int getMaxVid(int j) const
    {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return (m_rois[2 * j + 1] >> 8) & 0x3FF;  // & 0x0003FF00
    }

    /** Return MinUid (Col 1) of ROI j
     * @param j Index of ROI
     * @return MinUid
     */
    int getMinUid(int j) const
    {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return (m_rois[2 * j + 1] >> 18) & 0xFF; // & 0x03FC0000
    }

    /** Return MaxUid (Col 2) of ROI j
     * @param j Index of ROI
     * @return MaxUid
     */
    int getMaxUid(int j) const
    {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return (m_rois[2 * j + 1]) & 0xFF;
    }

    /** Return Type (Datcon or HLT) of ROI j
     * @param j Index of ROI
     * @return Type of Roi
     */
    int getType(int j) const
    {
      if (j < 0 || j >= (int)m_2timesNrROIs / 2) return -1;
      return (m_rois[2 * j] >> 10) & 0x1; //  & 0x400
    }

  private:
    unsigned int m_2timesNrROIs;/**< Number of ROIs times two (size of one ROI is 2*32bit) */
    /** Buffer of size 2*m_NrROIs ints. */
    int* m_rois; //[m_2timesNrROIs] // DONT MODIFY THIS COMMENT AS NECESSARY FOR ROOT STREAMER

    /** necessary for ROOT */
    ClassDef(PXDRawROIs, 3)
  };


} //Belle2 namespace
