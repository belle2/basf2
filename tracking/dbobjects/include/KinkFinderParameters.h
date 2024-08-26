/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>

namespace Belle2 {

  /** The payload containing the parameters for the kinkFinder
   */
  class KinkFinderParameters: public TObject {
  public:

    /** Default constructor */
    KinkFinderParameters() {}
    /** Destructor */
    ~KinkFinderParameters() {}

    /**
     * Set working mode and cuts for the KinkFinder module
     * @param kinkFitterMode working mode
     * @param vertexChi2Cut cut on Chi2 for the Kink vertex
     * @param vertexDistanceCut cut on distance between tracks at the Kink vertex
     * @param precutRho preselection cut on transverse shift from the outer CDC wall for the track ending points
     * @param precutZ preselection cut on z shift from the outer CDC wall for the track ending points
     * @param precutDistance preselection cut on distance between ending points of two tracks
     * @param precutDistance2D preselection cut on 2D distance between ending points of two tracks (for bad z cases)
     * @param precutSplitNCDCHit preselection cut on maximal number of fitted CDC hits for a track candidate to be split
     * @param precutSplitPValue preselection cut on maximal p-value for a track candidate to be split
     */
    void setParameters(const unsigned char kinkFitterMode, const double vertexChi2Cut, const double vertexDistanceCut,
                       const double precutRho, const double precutZ, const double precutDistance,
                       const double precutDistance2D, const int precutSplitNCDCHit, const double precutSplitPValue)
    {
      m_kinkFitterMode = kinkFitterMode;
      m_vertexChi2Cut = vertexChi2Cut;
      m_vertexDistanceCut = vertexDistanceCut;
      m_precutRho = precutRho;
      m_precutZ = precutZ;
      m_precutDistance = precutDistance;
      m_precutDistance2D = precutDistance2D;
      m_precutSplitNCDCHit = precutSplitNCDCHit;
      m_precutSplitPValue = precutSplitPValue;
    }

    /**
     * set KinkFitter working mode
     * @param kinkFitterMode working mode
     */
    void setKinkFitterMode(const unsigned char kinkFitterMode)
    {
      m_kinkFitterMode = kinkFitterMode;
    }

    /**
     * set cut on Chi2 for the Kink vertex
     * @param vertexChi2Cut cut on Chi2 for the Kink vertex
     */
    void setVertexChi2Cut(const double vertexChi2Cut)
    {
      m_vertexChi2Cut = vertexChi2Cut;
    }

    /**
     * set cut on distance between tracks at the Kink vertex
     * @param vertexDistanceCut cut on distance between tracks at the Kink vertex
     */
    void setVertexDistanceCut(const double vertexDistanceCut)
    {
      m_vertexDistanceCut = vertexDistanceCut;
    }

    /**
     * set preselection cut on transverse shift from the outer CDC wall for the track ending points
     * @param precutRho preselection cut on transverse shift from the outer CDC wall for the track ending points
     */
    void setPrecutRho(const double precutRho)
    {
      m_precutRho = precutRho;
    }

    /**
     * set preselection cut on z shift from the outer CDC wall for the track ending points
     * @param precutZ preselection cut on z shift from the outer CDC wall for the track ending points
     */
    void setPrecutZ(const double precutZ)
    {
      m_precutZ = precutZ;
    }

    /**
     * set preselection cut on distance between ending points of two tracks
     * @param precutDistance preselection cut on distance between ending points of two tracks
     */
    void setPrecutDistance(const double precutDistance)
    {
      m_precutDistance = precutDistance;
    }

    /**
     * set preselection cut on 2D distance between ending points of two tracks (for bad z cases)
     * @param precutDistance2D preselection cut on 2D distance between ending points of two tracks (for bad z cases)
     */
    void setPrecutDistance2D(const double precutDistance2D)
    {
      m_precutDistance2D = precutDistance2D;
    }

    /**
     * set preselection cut on maximal number of fitted CDC hits for a track candidate to be split
     * @param precutSplitNCDCHit preselection cut on maximal number of fitted CDC hits for a track candidate to be split
     */
    void setPrecutSplitNCDCHit(const int precutSplitNCDCHit)
    {
      m_precutSplitNCDCHit = precutSplitNCDCHit;
    }

    /**
     * set preselection cut on maximal p-value for a track candidate to be split
     * @param precutSplitPValue preselection cut on maximal p-value for a track candidate to be split
     */
    void setPrecutSplitPValue(const double precutSplitPValue)
    {
      m_precutSplitPValue = precutSplitPValue;
    }

    /** get kinkFitter working mode */
    unsigned char getKinkFitterMode() const
    {
      return m_kinkFitterMode;
    }

    /** get cut on Chi2 for the Kink vertex */
    double getVertexChi2Cut() const
    {
      return m_vertexChi2Cut;
    }

    /** get cut on distance between tracks at the Kink vertex */
    double getVertexDistanceCut() const
    {
      return m_vertexDistanceCut;
    }

    /** get preselection cut on transverse shift from the outer CDC wall for the track ending points */
    double getPrecutRho() const
    {
      return m_precutRho;
    }

    /** get preselection cut on z shift from the outer CDC wall for the track ending points */
    double getPrecutZ() const
    {
      return m_precutZ;
    }

    /** get preselection cut on distance between ending points of two tracks */
    double getPrecutDistance() const
    {
      return m_precutDistance;
    }

    /** get preselection cut on 2D distance between ending points of two tracks (for bad z cases) */
    double getPrecutDistance2D() const
    {
      return m_precutDistance2D;
    }

    /** get preselection cut on maximal number of fitted CDC hits for a track candidate to be split */
    int getPrecutSplitNCDCHit() const
    {
      return m_precutSplitNCDCHit;
    }

    /** get preselection cut on maximal p-value for a track candidate to be split */
    double getPrecutSplitPValue() const
    {
      return m_precutSplitPValue;
    }

  private:
    /** kinkFitter working mode in binary:
     * 1st bit: reassign hits (1 is On, 0 is Off)
     * 2nd bit: flip tracks with close end points (1 is On, 0 is Off)
     * 3rd bit: fit both tracks as one (1 is On, 0 is Off)
     * 4th bit: find candidate tracks to be split and do it (1 is On, 0 is Off)
     * fitter mode is written in decimal representation */
    unsigned char m_kinkFitterMode = 0b1111;
    /** Cut on Chi2 for the Kink vertex. */
    double m_vertexChi2Cut = 10000.;
    /** Cut on distance between tracks at the Kink vertex [cm]. */
    double m_vertexDistanceCut = 2.;
    /** Preselection cut on transverse shift from the outer CDC wall for the track ending points [cm]. */
    double m_precutRho = 10.;
    /** Preselection cut on z shift from the outer CDC wall for the track ending points [cm]. */
    double m_precutZ = 0.;
    /** Preselection cut on distance between ending points of two tracks [cm]. */
    double m_precutDistance = 10.;
    /** Preselection cut on 2D distance between ending points of two tracks (for bad z cases) [cm]. */
    double m_precutDistance2D = 10.;
    /** Preselection cut on maximal number of fitted CDC hits for a track candidate to be split. */
    int m_precutSplitNCDCHit = 49;
    /** Preselection cut on maximal p-value for a track candidate to be split. */
    double m_precutSplitPValue = 0.01;

    ClassDef(KinkFinderParameters, 1);  /**< ClassDef, necessary for ROOT */
  };
}
