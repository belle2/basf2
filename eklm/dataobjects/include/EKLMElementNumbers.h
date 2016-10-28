/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMELEMENTNUMBERS_H
#define EKLMELEMENTNUMBERS_H

/* External headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * EKLM element numbers.
   * All class members are constants, they are not stored.
   */
  class EKLMElementNumbers : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMElementNumbers();

    /**
     * Destructor.
     */
    virtual ~EKLMElementNumbers();

    /**
     * Check if endcap number is correct (fatal error if not).
     * @param[in] endcap Endcap number.
     */
    void checkEndcap(int endcap) const;

    /**
     * Check if layer number is correct (fatal error if not).
     * @param[in] layer Layer number.
     */
    void checkLayer(int layer) const;

    /**
     * Check if detector layer number is correct (fatal error if not).
     * Endcap number must be checked separately.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     */
    void checkDetectorLayer(int endcap, int layer) const;

    /**
     * Check if sector number is correct (fatal error if not).
     * @param[in] sector Sector number.
     */
    void checkSector(int sector) const;

    /**
     * Check if plane number is correct (fatal error if not).
     * @param[in] plane Plane number.
     */
    void checkPlane(int plane) const;

    /**
     * Check if segment number is correct (fatal error if not).
     * @param[in] segment Segment number.
     */
    void checkSegment(int segment) const;

    /**
     * Check if strip number is correct (fatal error if not).
     * @param[in] strip Strip number.
     */
    void checkStrip(int strip) const;

    /**
     * Get detector layer number.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     * @return Number of layer.
     * @details
     * Number is from 1 to 12 + 14 = 26.
     */
    int detectorLayerNumber(int endcap, int layer) const;

    /**
     * Get element numbers by detector layer global number.
     * @param[in]  layerGlobal Layer global number.
     * @param[out] endcap      Endcap number.
     * @param[out] layer       Layer number.
     */
    void layerNumberToElementNumbers(
      int layerGlobal, int* endcap, int* layer) const;

    /**
     * Get sector number.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     * @param[in] sector Sector number.
     * @return Number of sector.
     * @details
     * Number is from 1 to 4*14*2 = 112.
     */
    int sectorNumber(int endcap, int layer, int sector) const;

    /**
     * Get element numbers by sector global number.
     * @param[in]  sectorGlobal Sector global number.
     * @param[out] endcap       Endcap number.
     * @param[out] layer        Layer number.
     * @param[out] sector       Sector number.
     */
    void sectorNumberToElementNumbers(
      int sectorGlobal, int* endcap, int* layer, int* sector) const;

    /**
     * Get plane number.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     * @param[in] sector Sector number.
     * @param[in] plane  Plane number.
     * @return Number of plane.
     * @details
     * Number is from 1 to 2*4*26 = 208.
     */
    int planeNumber(int endcap, int layer, int sector, int plane) const;

    /**
     * Get element numbers by plane global number.
     * @param[in]  planeGlobal Plane global number.
     * @param[out] endcap      Endcap number.
     * @param[out] layer       Layer number.
     * @param[out] sector      Sector number.
     * @param[out] plane       Plane number.
     */
    void planeNumberToElementNumbers(
      int planeGlobal, int* endcap, int* layer, int* sector, int* segment)
    const;

    /**
     * Get segment number.
     * @param[in] endcap  Endcap number.
     * @param[in] layer   Layer number.
     * @param[in] sector  Sector number.
     * @param[in] plane   Plane number.
     * @param[in] segment Segment number.
     * @return Number of strip.
     * @details
     * Number is from 1 to 5*2*4*26 = 1040.
     */
    int segmentNumber(int endcap, int layer, int sector, int plane,
                      int segment) const;

    /**
     * Get element numbers by segment global number.
     * @param[in]  segmentGlobal Segment global number.
     * @param[out] endcap        Endcap number.
     * @param[out] layer         Layer number.
     * @param[out] sector        Sector number.
     * @param[out] plane         Plane number.
     * @param[out] segment       Segment number.
     */
    void segmentNumberToElementNumbers(
      int segmentGlobal, int* endcap, int* layer, int* sector, int* plane,
      int* segment) const;

    /**
     * Get strip number.
     * @param[in] endcap Endcap number.
     * @param[in] layer  Layer number.
     * @param[in] sector Sector number.
     * @param[in] plane  Plane number.
     * @param[in] strip  Strip number.
     * @return Number of strip.
     * @details
     * Number is from 1 to 75*2*4*26 = 15600.
     */
    int stripNumber(int endcap, int layer, int sector, int plane,
                    int strip) const;

    /**
     * Strip local number (in a plane).
     * @param[in] strip Global strip number.
     */
    int stripLocalNumber(int strip) const;

    /**
     * Get maximal detector layer global number.
     */
    int getMaximalLayerGlobalNumber() const;

    /**
     * Get maximal sector global number.
     */
    int getMaximalSectorGlobalNumber() const;

    /**
     * Get maximal plane global number.
     */
    int getMaximalPlaneGlobalNumber() const;

    /**
     * Get maximal segment global number.
     */
    int getMaximalSegmentGlobalNumber() const;

    /**
     * Get maximal strip global number.
     */
    int getMaximalStripGlobalNumber() const;

  protected:

    /** Maximal endcap number. */
    const int m_MaximalEndcapNumber;           //!

    /** Maximal layer number. */
    const int m_MaximalLayerNumber;            //!

    /** Maximal detector layer number. */
    const int m_MaximalDetectorLayerNumber[2]; //!

    /** Maximal sector number. */
    const int m_MaximalSectorNumber;           //!

    /** Maximal plane number. */
    const int m_MaximalPlaneNumber;            //!

    /** Maximal segment number. */
    const int m_MaximalSegmentNumber;          //!

    /** Maximal strip number. */
    const int m_MaximalStripNumber;            //!

  private:

    /** Makes objects storable. */
    ClassDef(EKLMElementNumbers, 1);

  };

}

#endif

