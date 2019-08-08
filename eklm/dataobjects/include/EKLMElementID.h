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

namespace Belle2 {

  /**
   * EKLM element identifier.
   */
  class EKLMElementID {

  public:

    /**
     * Type of EKLM element.
     */
    enum ElementType {
      c_Endcap,  /**< Endcap. */
      c_Layer,   /**< Layer. */
      c_Sector,  /**< Sector. */
      c_Plane,   /**< Plane. */
      c_Segment, /**< Segment. */
    };

    /**
     * Constructor.
     */
    EKLMElementID();

    /**
     * Constructor (sector).
     * @param[in] endcap  Endcap number.
     * @param[in] layer   Layer number.
     * @param[in] sector  Sector number.
     */
    EKLMElementID(int endcap, int layer, int sector);

    /**
     * Constructor (segment).
     * @param[in] endcap  Endcap number.
     * @param[in] layer   Layer number.
     * @param[in] sector  Sector number.
     * @param[in] plane   Plane number.
     * @param[in] segment Segment number.
     */
    EKLMElementID(int endcap, int layer, int sector, int plane, int segment);

    /**
     * Constructor.
     * @param[in] globalNumber Element global number.
     */
    explicit EKLMElementID(int globalNumber);

    /**
     * Destructor.
     */
    ~EKLMElementID();

    /**
     * Set element type.
     */
    void setType(ElementType type);

    /**
     * Get element type.
     */
    ElementType getType() const;

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
     * Get global detector layer number.
     */
    int getLayerNumber() const;

    /**
     * Get global sector number.
     */
    int getSectorNumber() const;

    /**
     * Get global plane number.
     */
    int getPlaneNumber() const;

    /**
     * Get global segment number.
     */
    int getSegmentNumber() const;

    /**
     * Get global element number.
     */
    int getGlobalNumber() const;

  private:

    /** Element type. */
    ElementType m_Type;

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
