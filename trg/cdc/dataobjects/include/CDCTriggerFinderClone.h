#ifndef CDCTRIGGERFINDERCLONE_H
#define CDCTRIGGERFINDERCLONE_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /** Additional information from the 2D finder unpacker of the CDC trigger.

      Contains information for the clone study
   */
  class CDCTriggerFinderClone : public RelationsObject {
  public:
    /** default constructor */
    CDCTriggerFinderClone():
      oldTrack(true), iTracker(0) { }

    /** constructor with arguments */
    CDCTriggerFinderClone(bool inOldTrack, int inITracker):
      oldTrack(inOldTrack), iTracker(inITracker) { }

    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTriggerFinderClone() { }

    // accessors
    /** whether this is an old track (appears to be a clone) */
    bool isOldTrack() const { return oldTrack; }

    int getITracker() const {return iTracker;}

  protected:
    /** whether this is an old track */
    bool oldTrack;

    /** ID of the 2D tracker (0 to 3) */
    int iTracker;

    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerFinderClone, 1);
  };
}
#endif
