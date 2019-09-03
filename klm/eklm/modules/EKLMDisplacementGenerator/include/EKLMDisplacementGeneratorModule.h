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
#include <framework/core/Module.h>
#include <klm/eklm/dbobjects/EKLMAlignment.h>
#include <klm/eklm/dbobjects/EKLMSegmentAlignment.h>

namespace Belle2 {

  /**
   * Module for generation of displacement or alignment data.
   */
  class EKLMDisplacementGeneratorModule : public Module {

  public:

    /**
     * Constructor.
     */
    EKLMDisplacementGeneratorModule();

    /**
     * Destructor.
     */
    ~EKLMDisplacementGeneratorModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;

  private:

    /** Payload name. */
    std::string m_PayloadName;

    /** Mode. */
    std::string m_Mode;

    /** What should be randomly displaced */
    std::string m_RandomDisplacement;

    /** If the displacement should be the same for all sectors. */
    bool m_SectorSameDisplacement;

    /** Fix sector deltaU at 0. */
    bool m_SectorZeroDeltaU;

    /** Fix sector deltaV at 0. */
    bool m_SectorZeroDeltaV;

    /** Fix sector deltaGamma at 0. */
    bool m_SectorZeroDeltaGamma;

    /** Sector deltaU */
    double m_SectorDeltaU;

    /** Sector deltaV. */
    double m_SectorDeltaV;

    /** Sector deltaGamma. */
    double m_SectorDeltaGamma;

    /** Name of input file. */
    std::string m_InputFile;

    /** Name of output file. */
    std::string m_OutputFile;

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /**
     * Fill EKLMAlignment with zero displacements.
     * @param[in,out] alignment        Displacements.
     * @param[in,out] segmentAlignment Segment displacements.
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
     * @param[in] displaceSector  Whether sectors should be displaced.
     * @param[in] displaceSegment Whether segments should be displaced.
     */
    void generateRandomDisplacement(bool displaceSector, bool displaceSegment);

    /**
     * Read displacement from ROOT file.
     */
    void readDisplacementFromROOTFile();

    /**
     * Generate random sector displacements and check if they are correct
     * (no overlaps).
     * @oaram[in] f Output file.
     */
    void studySectorAlignmentLimits(TFile* f);

    /**
     * Generate random segment displacements and check if they are correct
     * (no overlaps).
     * @oaram[in] f Output file.
     */
    void studySegmentAlignmentLimits(TFile* f);

    /**
     * Generate random displacements and check if they are correct
     * (no overlaps).
     */
    void studyAlignmentLimits();

    /**
     * Save displacements to a ROOT file.
     * @param[in] alignment        Displacements.
     * @param[in] segmentAlignment Segment displacements.
     */
    void saveDisplacement(EKLMAlignment* alignment,
                          EKLMSegmentAlignment* segmentAlignment);

  };

}
