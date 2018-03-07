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
     * Check if endcap number is correct.
     * @param[in] endcap     Endcap number.
     * @param[in] fatalError Issue fatal error (default) or not.
     * @return True if the endcap number is correct.
     */
    bool checkEndcap(int endcap, bool fatalError = true) const;

    /**
     * Check if layer number is correct.
     * @param[in] layer      Layer number.
     * @param[in] fatalError Issue fatal error (default) or not.
     * @return True if the layer number is correct.
     */
    bool checkLayer(int layer, bool fatalError = true) const;

    /**
     * Check if detector layer number is correct (fatal error if not).
     * Endcap number must be checked separately.
     * @param[in] endcap     Endcap number.
     * @param[in] layer      Layer number.
     * @param[in] fatalError Issue fatal error (default) or not.
     * @return True if the detector layer number is correct.
     */
    bool checkDetectorLayer(int endcap, int layer,
                            bool fatalError = true) const;

    /**
     * Check if sector number is correct (fatal error if not).
     * @param[in] sector     Sector number.
     * @param[in] fatalError Issue fatal error (default) or not.
     * @return True if the sector number is correct.
     */
    bool checkSector(int sector, bool fatalError = true) const;

    /**
     * Check if plane number is correct (fatal error if not).
     * @param[in] plane      Plane number.
     * @param[in] fatalError Issue fatal error (default) or not.
     * @return True if the plane number is correct.
     */
    bool checkPlane(int plane, bool fatalError = true) const;

    /**
     * Check if segment number is correct (fatal error if not).
     * @param[in] segment    Segment number.
     * @param[in] fatalError Issue fatal error (default) or not.
     * @return True if the segment number is correct.
     */
    bool checkSegment(int segment, bool fatalError = true) const;

    /**
     * Check if strip number is correct (fatal error if not).
     * @param[in] strip      Strip number.
     * @param[in] fatalError Issue fatal error (default) or not.
     * @return True if the strip number is correct.
     */
    bool checkStrip(int strip, bool fatalError = true) const;

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
     * Number is from 1 to 4*26 = 104.
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
     * Get element numbers by strip global number.
     * @param[in]  stripGlobal Strip global number.
     * @param[out] endcap      Endcap number.
     * @param[out] layer       Layer number.
     * @param[out] sector      Sector number.
     * @param[out] plane       Plane number.
     * @param[out] strip       Strip number.
     */
    void stripNumberToElementNumbers(
      int stripGlobal, int* endcap, int* layer, int* sector, int* plane,
      int* strip) const;

    /**
     * Strip local number (in a plane).
     * @param[in] strip Global strip number.
     */
    int stripLocalNumber(int strip) const;

    /**
     * Get maximal endcap number.
     */
    int getMaximalEndcapNumber() const;

    /**
     * Get maximal layer number.
     */
    int getMaximalLayerNumber() const;

    /**
     * Get maximal detector layer number.
     * @param[in] endcap Endcap number.
     */
    int getMaximalDetectorLayerNumber(int endcap) const;

    /**
     * Get maximal sector number.
     */
    int getMaximalSectorNumber() const;

    /**
     * Get maximal plane number.
     */
    int getMaximalPlaneNumber() const;

    /**
     * Get maximal segment number.
     */
    int getMaximalSegmentNumber() const;

    /**
     * Get maximal strip number.
     */
    int getMaximalStripNumber() const;

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

