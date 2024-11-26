/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef CDCTRIGGERHOUGHCLUSTER_H
#define CDCTRIGGERHOUGHCLUSTER_H

#include <framework/datastore/RelationsObject.h>

#include <Math/Vector2D.h>

namespace Belle2 {

  /** Cluster created by the Hough finder of the CDC trigger. */
  class CDCTriggerHoughCluster : public RelationsObject {
  public:
    /** default constructor */
    CDCTriggerHoughCluster():
      m_xmin(0), m_xmax(0), m_ymin(0), m_ymax(0) { }

    /** constructor with arguments */
    CDCTriggerHoughCluster(int x1, int x2, int y1, int y2,
                           const std::vector<ROOT::Math::XYVector>& cellList):
      m_xmin(x1), m_xmax(x2), m_ymin(y1), m_ymax(y2), m_cells(cellList) { }

    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTriggerHoughCluster() { }

    // accessors
    /** get size of bounding rectangle of cluster */
    ROOT::Math::XYVector getClusterArea() const { return ROOT::Math::XYVector(m_xmax - m_xmin + 1, m_ymax - m_ymin + 1); }
    /** get left bottom corner */
    ROOT::Math::XYVector getCornerBL() const { return ROOT::Math::XYVector(m_xmin, m_ymin); }
    /** get top right corner */
    ROOT::Math::XYVector getCornerTR() const { return ROOT::Math::XYVector(m_xmax, m_ymax); }
    /** get list of cell indices */
    std::vector<ROOT::Math::XYVector> getCells() const { return m_cells; }

  protected:
    /** x index of left boundary cell */
    int m_xmin;
    /** x index of right boundary cell */
    int m_xmax;
    /** y index of bottom boundary cell */
    int m_ymin;
    /** y index of top boundary cell */
    int m_ymax;
    /** list of cell indices in the cluster */
    std::vector<ROOT::Math::XYVector> m_cells = {};

    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerHoughCluster, 2);
  };
}
#endif
