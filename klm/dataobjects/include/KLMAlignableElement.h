/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* C++ headers. */
#include <cstdint>

namespace Belle2 {

  /**
   * KLM alignable element identifier.
   */
  class KLMAlignableElement {

  public:

    /**
     * Type of EKLM element.
     */
    enum ElementType {
      c_EKLMSector,  /**< EKLM sector (module). */
      c_EKLMSegment, /**< EKLM segment. */
      c_BKLMModule,  /**< BKLM module. */
    };

    /**
     * Constructor.
     */
    KLMAlignableElement();

    /**
     * Constructor.
     * @param[in] type    Element type.
     * @param[in] section Section number.
     * @param[in] sector  Sector number.
     * @param[in] layer   Layer number.
     * @param[in] plane   Plane number (for EKLM segments only).
     * @param[in] segment Segment number (for EKLM segments only).
     */
    KLMAlignableElement(enum ElementType type, int section, int sector,
                        int layer, int plane = 0, int segment = 0);

    /**
     * Constructor.
     * @param[in] globalNumber Element global number.
     */
    explicit KLMAlignableElement(int globalNumber);

    /**
     * Destructor.
     */
    ~KLMAlignableElement();

    /**
     * Set element type.
     */
    void setType(ElementType type);

    /**
     * Get element type.
     */
    ElementType getType() const;

    /**
     * Set section number.
     */
    void setSection(int section);

    /**
     * Get section number.
     */
    int getSection() const;

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

    /**d
     * Get segment number.
     */
    int getSegment() const;

    /**
     * Get KLM module number (for EKLM sectors and BKLM modules).
     */
    uint16_t getModuleNumber() const;

    /**
     * Get EKLM segment number.
     */
    int getEKLMSegmentNumber() const;

    /**
     * Get alignable element number.
     */
    int getNumber() const;

  private:

    /** Element type. */
    ElementType m_Type;

    /** Subdetector. */
    int m_Subdetector;

    /** Section number. */
    int m_Section;

    /** Sector number. */
    int m_Sector;

    /** Layer number. */
    int m_Layer;

    /** Plane number. */
    int m_Plane;

    /** Segment number. */
    int m_Segment;

    /** EKLM segment offset for global number. */
    const int c_EKLMSegmentOffset = 65536;

  };

}
