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
      xmin(0), xmax(0), ymin(0), ymax(0) { }

    /** constructor with arguments */
    CDCTriggerHoughCluster(int x1, int x2, int y1, int y2,
                           const std::vector<ROOT::Math::XYVector>& cellList):
      xmin(x1), xmax(x2), ymin(y1), ymax(y2), cells(cellList) { }

    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTriggerHoughCluster() { }

    // accessors
    /** get size of bounding rectangle of cluster */
    ROOT::Math::XYVector getClusterArea() const { return ROOT::Math::XYVector(xmax - xmin + 1, ymax - ymin + 1); }
    /** get left bottom corner */
    ROOT::Math::XYVector getCornerBL() const { return ROOT::Math::XYVector(xmin, ymin); }
    /** get top right corner */
    ROOT::Math::XYVector getCornerTR() const { return ROOT::Math::XYVector(xmax, ymax); }
    /** get list of cell indices */
    std::vector<ROOT::Math::XYVector> getCells() const { return cells; }

  protected:
    /** x index of left boundary cell */
    int xmin;
    /** x index of right boundary cell */
    int xmax;
    /** y index of bottom boundary cell */
    int ymin;
    /** y index of top boundary cell */
    int ymax;
    /** list of cell indices in the cluster */
    std::vector<ROOT::Math::XYVector> cells = {};

    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerHoughCluster, 2);
  };
}
#endif
