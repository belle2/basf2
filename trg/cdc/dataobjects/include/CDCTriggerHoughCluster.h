#ifndef CDCTRIGGERHOUGHCLUSTER_H
#define CDCTRIGGERHOUGHCLUSTER_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /** Cluster created by the Hough finder of the CDC trigger. */
  class CDCTriggerHoughCluster : public RelationsObject {
  public:
    /** default constructor */
    CDCTriggerHoughCluster():
      xmin(0), xmax(0), ymin(0), ymax(0) { }

    /** constructor with arguments */
    CDCTriggerHoughCluster(int x1, int x2, int y1, int y2,
                           const std::vector<TVector2>& cellList):
      xmin(x1), xmax(x2), ymin(y1), ymax(y2), cells(cellList) { }

    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTriggerHoughCluster() { }

    // accessors
    /** get size of bounding rectangle of cluster */
    TVector2 getClusterArea() const { return TVector2(xmax - xmin + 1, ymax - ymin + 1); }
    /** get left bottom corner */
    TVector2 getCornerBL() const { return TVector2(xmin, ymin); }
    /** get top right corner */
    TVector2 getCornerTR() const { return TVector2(xmax, ymax); }
    /** get list of cell indices */
    std::vector<TVector2> getCells() const { return cells; }

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
    std::vector<TVector2> cells = {};

    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerHoughCluster, 1);
  };
}
#endif
