/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dbobjects/eklm/EKLMAlignment.h>
#include <klm/dbobjects/eklm/EKLMSegmentAlignment.h>
#include <klm/eklm/geometry/GeometryData.h>

/* ROOT headers. */
#include <TFile.h>

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
     * Generation of fixed module displacements.
     * @param[in] deltaU     Shift in U.
     * @param[in] deltaV     Shift in V.
     * @param[in] deltaGamma Rotation in Gamma.
     */
    void generateFixedModuleDisplacement(double deltaU, double deltaV,
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
     * @param[in] displaceModule
     * Whether modules should be displaced.
     *
     * @param[in] displaceSegment
     * Whether segments should be displaced.
     *
     * @param[in] moduleSameDisplacement
     * If the displacement should be the same for all modules.
     *
     * @param[in] moduleZeroDeltaU
     * Fix module deltaU at 0.
     *
     * @param[in] moduleZeroDeltaV
     * Fix module deltaV at 0.
     *
     * @param[in] moduleZeroDeltaGamma
     * Fix module deltaGamma at 0.
     */
    void generateRandomDisplacement(
      EKLMAlignment* alignment, EKLMSegmentAlignment* segmentAlignment,
      bool displaceModule, bool displaceSegment,
      bool moduleSameDisplacement = false, bool moduleZeroDeltaU = false,
      bool moduleZeroDeltaV = false, bool moduleZeroDeltaGamma = false);

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
     * Generate random module displacements and check if they are correct
     * (no overlaps).
     * @oaram[in] f Output file.
     */
    void studyModuleAlignmentLimits(TFile* f);

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

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** EKLM element numbers. */
    const EKLMElementNumbers* m_eklmElementNumbers;

  };

}
