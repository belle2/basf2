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

    /** Set the cuts for the two filters
     * @param status the option ot trun on/off flip&refit
     * @param cutPt the cut on Pt to do flip&refit
     * @param cutFirst the cut for the 1st MVA
     * @param cutSecond the cut for the 2nd MVA
     */
    void setParameters(const int kinkFitterMode, const double vertexChi2Cut, const double vertexDistanceCut,
                       const double precutRho, const double precutZ, const double precutDistance, const double precutDistance2D)
    {
      m_kinkFitterMode = kinkFitterMode;
      m_vertexChi2Cut = vertexChi2Cut;
      m_vertexDistanceCut = vertexDistanceCut;
      m_precutRho = precutRho;
      m_precutZ = precutZ;
      m_precutDistance = precutDistance;
      m_precutDistance2D = precutDistance2D;
    }


    /** get kinkFitter working mode */
    int getKinkFitterMode() const
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

  private:
    /** kinkFitter working mode in binary:
     * first bit: reassign hits (1 is On, 0 is Off)
     * second bit: flip tracks with close end points (1 is On, 0 is Off)
     * third bit: fit both tracks as one (1 is On, 0 is Off)
     * fitter mode is written in decimal representation */
    int m_kinkFitterMode = 7;
    /** Cut on Chi2 for the Kink vertex. */
    double m_vertexChi2Cut = 10000.;
    /** Cut on distance between tracks at the Kink vertex. */
    double m_vertexDistanceCut = 2.;
    /** Preselection cut on transverse shift from the outer CDC wall for the track ending points. */
    double m_precutRho = 10.;
    /** Preselection cut on z shift from the outer CDC wall for the track ending points. */
    double m_precutZ = 0.;
    /** Preselection cut on distance between ending points of two tracks. */
    double m_precutDistance = 10.;
    /** Preselection cut on 2D distance between ending points of two tracks (for bad z cases). */
    double m_precutDistance2D = 10.;

    ClassDef(KinkFinderParameters, 1);  /**< ClassDef, necessary for ROOT */
  };
}
