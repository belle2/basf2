/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* Belle2 headers. */
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/eklm/dbobjects/EKLMAlignment.h>
#include <klm/eklm/dbobjects/EKLMSegmentAlignment.h>
#include <klm/eklm/geometry/GeometryData.h>

namespace Belle2 {

  /**
   * Module for generation of KLM displacement or alignment data.
   */
  class KLMDisplacementGenerator {

  public:

    /**
     * Constructor.
     */
    KLMDisplacementGenerator();

    /**
     * Destructor.
     */
    ~KLMDisplacementGenerator();

    /**
     * Fill EKLMAlignment with zero displacements.
     * @param[out] alignment        Displacements.
     * @param[out] segmentAlignment Segment displacements.
     */
    void fillZeroDisplacements(EKLMAlignment* alignment,
                               EKLMSegmentAlignment* segmentAlignment);

    /**
     * Generation of zero displacements.
     */
    void generateZeroDisplacement();

    /**
     * Generation of fixed sector displacements.
     * @param[in] deltaU     Shift in U.
     * @param[in] deltaV     Shift in V.
     * @param[in] deltaGamma Rotation in Gamma.
     */
    void generateFixedSectorDisplacement(double deltaU, double deltaV,
                                         double deltaGamma);

    /**
     * Generation of random displacements.
     *
     * @param[out] alignment
     * Displacements.
     *
     * @param[out] segmentAlignment
     * Segment displacements.
     *
     * @param[in] displaceSector
     * Whether sectors should be displaced.
     *
     * @param[in] displaceSegment
     * Whether segments should be displaced.
     *
     * @param[in] sectorSameDisplacement
     * If the displacement should be the same for all sectors.
     *
     * @param[in] sectorZeroDeltaU
     * Fix sector deltaU at 0.
     *
     * @param[in] sectorZeroDeltaV
     * Fix sector deltaV at 0.
     *
     * @param[in] sectorZeroDeltaGamma
     * Fix sector deltaGamma at 0.
     */
    void generateRandomDisplacement(
      EKLMAlignment* alignment, EKLMSegmentAlignment* segmentAlignment,
      bool displaceSector, bool displaceSegment,
      bool sectorSameDisplacement = false, bool sectorZeroDeltaU = false,
      bool sectorZeroDeltaV = false, bool sectorZeroDeltaGamma = false);

    /**
     * Read displacement from ROOT file.
     * @param[out] alignment        Displacements.
     * @param[out] segmentAlignment Segment displacements.
     * @param[in]  inputFile        Input file.
     */
    void readDisplacementFromROOTFile(EKLMAlignment* alignment,
                                      EKLMSegmentAlignment* segmentAlignment,
                                      const char* inputFile);

    /**
     * Generate random sector displacements and check if they are correct
     * (no overlaps).
     * @oaram[in] f Output file.
     */
    void studySectorAlignmentLimits(TFile* f);

    /**
     * Save displacements to a ROOT file.
     * @param[in] alignment        Displacements.
     * @param[in] segmentAlignment Segment displacements.
     * @param[in] outputFile       Output file.
     */
    void saveDisplacement(EKLMAlignment* alignment,
                          EKLMSegmentAlignment* segmentAlignment,
                          const char* outputFile);

  private:

    /**
     * Generate random segment displacements and check if they are correct
     * (no overlaps).
     * @oaram[in] f Output file.
     */
    void studySegmentAlignmentLimits(TFile* f);

    /**
     * Generate random displacements and check if they are correct
     * (no overlaps).
     * @param[in] outputFile Output file.
     */
    void studyAlignmentLimits(const char* outputFile);

    /** Mode. */
    std::string m_Mode;

    /** What should be randomly displaced */
    std::string m_RandomDisplacement;

    /** Name of input file. */
    std::string m_InputFile;

    /** Name of output file. */
    std::string m_OutputFile;

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

  };

}
