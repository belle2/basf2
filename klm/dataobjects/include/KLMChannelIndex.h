/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMElementNumbers.h>

namespace Belle2 {

  /**
   * KLM channel index.
   */
  class KLMChannelIndex {

  public:

    /**
     * Index level (loop over KLM elements at the specified level).
     */
    enum IndexLevel {

      /** Subdetector (BKLM or EKLM). */
      c_IndexLevelSubdetector = 1,

      /** Section. */
      c_IndexLevelSection = 2,

      /** Sector. */
      c_IndexLevelSector = 3,

      /** Layer. */
      c_IndexLevelLayer = 4,

      /** Plane. */
      c_IndexLevelPlane = 5,

      /** Strip. */
      c_IndexLevelStrip = 6,

    };

    /**
     * Constructor.
     * @param[in] indexLevel Index level.
     */
    explicit KLMChannelIndex(enum IndexLevel indexLevel = c_IndexLevelStrip);

    /**
     * Constructor.
     * @param[in] subdetector Subdetector.
     * @param[in] section     Section.
     * @param[in] sector      Sector.
     * @param[in] layer       Layer.
     * @param[in] plane       Plane.
     * @param[in] strip       Strip.
     * @param[in] indexLevel  Index level.
     */
    KLMChannelIndex(int subdetector, int section, int sector,
                    int layer, int plane, int strip,
                    enum IndexLevel indexLevel = c_IndexLevelStrip);

    /**
     * Destructor.
     */
    ~KLMChannelIndex();

    /**
     * Set index level. If the index level is increased, then the index is set
     * to the first element of the higher-order level.
     */
    void setIndexLevel(enum IndexLevel indexLevel);

    /**
     * Iterate over EKLM segments instead of strips. Turning this on does not
     * make sense for BKLM, in this case the iteration would still be performed
     * over BKLM strips. Make sure that the iteration is performed for EKLM
     * only, e. g. by using beginEKLM() and endEKLM().
     */
    void useEKLMSegments(bool useSegments = true);

    /**
     * Set KLM module.
     * @param[in] module Module.
     */
    void setKLMModule(KLMModuleNumber module);

    /**
     * Set EKLM segment.
     * @param[in] segment Segment.
     */
    void setEKLMSegment(int segment);

    /**
     * Get subdetector.
     */
    int getSubdetector() const
    {
      return m_Subdetector;
    }

    /**
     * Get section.
     */
    int getSection() const
    {
      return m_Section;
    }

    /**
     * Get sector.
     */
    int getSector() const
    {
      return m_Sector;
    }

    /**
     * Get layer.
     */
    int getLayer() const
    {
      return m_Layer;
    }

    /**
     * Get plane.
     */
    int getPlane() const
    {
      return m_Plane;
    }

    /**
     * Get strip.
     */
    int getStrip() const
    {
      return m_Strip;
    }

    /**
     * Get KLM channel number.
     */
    KLMChannelNumber getKLMChannelNumber() const;

    /**
     * Get KLM plane number.
     */
    KLMPlaneNumber getKLMPlaneNumber() const;

    /**
     * Get KLM module number.
     */
    KLMModuleNumber getKLMModuleNumber() const;

    /**
     * Get KLM sector number.
     */
    KLMSectorNumber getKLMSectorNumber() const;

    /**
     * Get EKLM segment number.
     */
    int getEKLMSegmentNumber() const;

    /**
     * First channel for BKLM.
     */
    KLMChannelIndex beginBKLM();

    /**
     * Last channel for BKLM.
     */
    KLMChannelIndex& endBKLM();

    /**
     * First channel for EKLM.
     */
    KLMChannelIndex beginEKLM();

    /**
     * Last channel for EKLM.
     */
    KLMChannelIndex& endEKLM();

    /**
     * First channel.
     */
    KLMChannelIndex begin()
    {
      return beginBKLM();
    }

    /**
     * Last channel.
     */
    KLMChannelIndex& end()
    {
      return endEKLM();
    }

    /**
     * Operator ++.
     */
    KLMChannelIndex& operator++();

    /**
     * Increment (to use in Python).
     */
    KLMChannelIndex& increment();

    /**
     * Operator ==.
     */
    bool operator==(const KLMChannelIndex& index);

    /**
     * Operator !=.
     */
    bool operator!=(const KLMChannelIndex& index);

    /**
     * Operator *.
     */
    KLMChannelIndex& operator*();

  protected:

    /**
     * Set number of strips in the current plane.
     */
    void setNStripsPlane();

    /**
     * Increment the index.
     */
    void increment(enum IndexLevel indexLevel);

    /** Index level. */
    enum IndexLevel m_IndexLevel;

    /** Subdetector. */
    int m_Subdetector;

    /** Section. */
    int m_Section;

    /** Sector. */
    int m_Sector;

    /** Layer. */
    int m_Layer;

    /** Plane. */
    int m_Plane;

    /** Strip. */
    int m_Strip;

    /** Iterate over EKLM segments instead of strips. */
    bool m_UseEKLMSegments = false;

    /** Number of strips in current plane. */
    int m_NStripsPlane;

    /** KLM element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** EKLM element numbers. */
    const EKLMElementNumbers* m_eklmElementNumbers;

  };

}
