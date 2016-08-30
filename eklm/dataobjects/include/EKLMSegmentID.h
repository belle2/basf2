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
    EKLMSegmentID();

    /**
     * Constructor.
     * @param[in] endcap  Endcap number.
     * @param[in] layer   Layer number.
     * @param[in] sector  Sector number.
     * @param[in] plane   Plane number.
     * @param[in] segment Segment number.
     */
    EKLMSegmentID(int endcap, int layer, int sector, int plane, int segment);

    /**
     * Constructor.
     * @param[in] segment Segment number (global).
     */
    EKLMSegmentID(int segment);

    /**
     * Destructor.
     */
    ~EKLMSegmentID();

    /**
     * Set endcap number.
     */
    void setEndcap(int endcap);

    /**
     * Get endcap number.
     */
    int getEndcap() const;

    /**
     * Set layer number.
     */
    void setLayer(int layer);

    /**
     * Get layer number.
     */
    int getLayer() const;

    /**
     * Set sector number.
     */
    void setSector(int sector);

    /**
     * Get sector number.
     */
    int getSector() const;

    /**
     * Set plane number.
     */
    void setPlane(int plane);

    /**
     * Get plane number.
     */
    int getPlane() const;

    /**
     * Set segment number.
     */
    void setSegment(int segment);

    /**
     * Get segment number.
     */
    int getSegment() const;

    /**
     * Get segment global number.
     */
    int getSegmentGlobalNumber() const;

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

