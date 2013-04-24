/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDTFINFOBOARD_H
#define VXDTFINFOBOARD_H

#include <vector>

#include <TObject.h>

namespace Belle2 {
  /** \addtogroup dataobjects
   * @{
   */

  /** The VXD Track Finder Info Board Class
   * This class is needed for information transport between the VXD Track Finder and its analysis tool. It simply stores a
   * vector of integers. These integers are index numbers of (depending on the case) TFHits, VXDSegmentCells or VXDTFTrackCandidates
   */
  class VXDTFInfoBoard: public TObject {
  public:

    /** Default constructor for the ROOT IO. */
    VXDTFInfoBoard():
      m_fitSuccessful(false),
      m_probValue(0),
      m_assignedGFTC(-1) { m_indexNumbers.clear(); }

    /** getter - getIndex returns vector of indices assigned to InfoBoard, currently not used */
    const std::vector<int> getIndex() const { return m_indexNumbers; }

    /** getter - isFitPossible returns whether fit using GenFit within the VXDTFModule was possible or not */
    bool isFitPossible() const { return m_fitSuccessful; }

    /** getter - returns index number of assigned GFTrackCand */
    int getAssignedGFTC() const { return m_assignedGFTC; }

    /** getter - returns calculated probability that assigned GFTC is real track */
    double getProbValue() const { return m_probValue; }

    /** setter - add new index value (currently not used) */
    void addIndexValue(int newVal) { m_indexNumbers.push_back(newVal); }

    /** setter - submits indexnumber of  assigned GFTC for unique identification */
    void assignGFTC(int index) { m_assignedGFTC = index; }

    /** setter - set true, if fit was possible*/
    void fitIsPossible(bool yesNo) { m_fitSuccessful = yesNo; }

    /** setter - submit probability value that assigned GFTC is real track */
    void setProbValue(double value) { m_probValue = value; }


  protected:
    std::vector<int> m_indexNumbers; /**< contains index numbers of (depending on the case) TFHits, VXDSegmentCells or VXDTFTrackCandidates */
    bool m_fitSuccessful; /**< is true, when fit was successfull */
    double m_probValue; /**< probability-value calculated by kalman fit (probability that his TC is real track) */
    int m_assignedGFTC; /**< index number of assigned GFTrackCand for unique identification */

    ClassDef(VXDTFInfoBoard, 1)
  };

  /** @}*/
} //Belle2 namespace
#endif
