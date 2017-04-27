/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMALIGNMENTMODULE_H
#define EKLMALIGNMENTMODULE_H

/* Belle2 headers. */
#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Module EKLMAlignmentModule.
   * @details
   * Module for generation of transformation and alignment data.
   */
  class EKLMAlignmentModule : public Module {

  public:

    /**
     * Constructor.
     */
    EKLMAlignmentModule();

    /**
     * Destructor.
     */
    ~EKLMAlignmentModule();

    /**
     * Initializer.
     */
    void initialize();

    /**
     * Called when entering a new run.
     */
    void beginRun();

    /**
     * This method is called for each event.
     */
    void event();

    /**
     * This method is called if the current run ends.
     */
    void endRun();

    /**
     * This method is called at the end of the event processing.
     */
    void terminate();

  private:

    /** Mode. */
    std::string m_Mode;

    /** What should be randomly displaced */
    std::string m_RandomDisplacement;

    /** If the displacement should be the same for all sectors. */
    bool m_SectorSameDisplacement;

    /** Fix sector dx at 0. */
    bool m_SectorZeroDx;

    /** Fix sector dy at 0. */
    bool m_SectorZeroDy;

    /** Fix sector dalpha at 0. */
    bool m_SectorZeroDalpha;

    /** Name of output file. */
    std::string m_OutputFile;

    /** Geometry data. */
    const EKLM::GeometryData* m_GeoDat;

    /**
     * Generation of zero displacements.
     */
    void generateZeroDisplacement();

    /**
     * Generation of random displacements.
     * @param[in] displaceSector  Whether sectors should be displaced.
     * @param[in] displaceSegment Whether segments should be displaced.
     */
    void generateRandomDisplacement(bool displaceSector, bool displaceSegment);

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
     * @param[in] alignment Displacements.
     */
    void saveDisplacement(EKLMAlignment* alignment);

  };

}

#endif

