/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSEGMENTID_H
#define EKLMSEGMENTID_H

/* External headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * EKLM segment identifier.
   */
  class EKLMSegmentID {

  public:

    /**
     * Constructor.
     */
    EKLMSegmentID(int endcap, int layer, int sector, int plane, int segment);

    /**
     * Destructor.
     */
    ~EKLMSegmentID();

    /**
     * Get segment number.
     */
    int getSegmentNumber() const;

  private:

    /** Endcap number. */
    int m_Endcap;

    /** Layer number. */
    int m_Layer;

    /** Sector number. */
    int m_Sector;

    /** Plane number. */
    int m_Plane;

    /** Segment number. */
    int m_Segment;

  };

}

#endif

