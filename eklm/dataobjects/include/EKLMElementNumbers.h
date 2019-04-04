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
     * Get software strip number by firmware strip number.
     * @param[in] stripFirmware Firmware strip number.
     */
    int getStripSoftwareByFirmware(int stripFirmware) const;

    /**
     * Get firmware strip number by software strip number.
     * @param[in] stripFirmware Firmware strip number.
     */
    int getStripFirmwareBySoftware(int stripSoftware) const;

    /**
     * Get ASIC and channel numbers (both are 0-based)
     * by plane and strip numbers.
     * @param[in]  plane   Plane.
     * @param[in]  strip   Strip,
     * @param[out] asic    ASIC.
     * @param[out] channel Channel.
     */
    void getAsicChannel(int plane, int strip, int* asic, int* channel) const;

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
    static constexpr int getMaximalSectorNumber()
    {
      return m_MaximalSectorNumber;
    }

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
    static constexpr int getMaximalStripNumber()
    {
      return m_MaximalStripNumber;
    }

    /**
     * Get maximal detector layer global number.
     */
    static constexpr int getMaximalLayerGlobalNumber()
    {
      return m_MaximalDetectorLayerNumber[0] + m_MaximalDetectorLayerNumber[1];
    }

    /**
     * Get maximal sector global number.
     */
    static constexpr int getMaximalSectorGlobalNumber()
    {
      return m_MaximalSectorNumber * getMaximalLayerGlobalNumber();
    }

    /**
     * Get maximal plane global number.
     */
    static constexpr int getMaximalPlaneGlobalNumber()
    {
      return m_MaximalPlaneNumber * getMaximalSectorGlobalNumber();
    }

    /**
     * Get maximal segment global number.
     */
    static constexpr int getMaximalSegmentGlobalNumber()
    {
      return m_MaximalSegmentNumber * getMaximalPlaneGlobalNumber();
    }

    /**
     * Get maximal strip global number.
     */
    static constexpr int getMaximalStripGlobalNumber()
    {
      return m_MaximalStripNumber * getMaximalPlaneGlobalNumber();
    }

    /**
     * Get number of strips in a layer.
     */
    static constexpr int getNStripsLayer()
    {
      return getNStripsSector() * m_MaximalSectorNumber;
    }

    /**
     * Get number of strips in a sector.
     */
    static constexpr int getNStripsSector()
    {
      return m_MaximalPlaneNumber * m_MaximalStripNumber;
    }

    /**
     * Get number of strips in a segment.
     */
    static constexpr int getNStripsSegment()
    {
      return m_NStripsSegment;
    }

  protected:

    /** Maximal endcap number. */
    static constexpr int m_MaximalEndcapNumber = 2;

    /** Maximal layer number. */
    static constexpr int m_MaximalLayerNumber = 14;

    /** Maximal detector layer number. */
    static constexpr int m_MaximalDetectorLayerNumber[2] = {12, 14};

    /** Maximal sector number. */
    static constexpr int m_MaximalSectorNumber = 4;

    /** Maximal plane number. */
    static constexpr int m_MaximalPlaneNumber = 2;

    /** Maximal segment number. */
    static constexpr int m_MaximalSegmentNumber = 5;

    /** Maximal strip number. */
    static constexpr int m_MaximalStripNumber = 75;

    /** Number of strips in a segment. */
    static constexpr int m_NStripsSegment = 15;

  private:

    /** Class version. */
    ClassDef(EKLMElementNumbers, 3);

  };

}
