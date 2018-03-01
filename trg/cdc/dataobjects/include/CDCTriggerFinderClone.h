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
      oldTrack(true) { }

    /** constructor with arguments */
    CDCTriggerFinderClone(bool inOldTrack):
      oldTrack(inOldTrack) { }

    /** destructor, empty because we don't allocate memory anywhere. */
    ~CDCTriggerFinderClone() { }

    // accessors
    /** whether this is an old track (appears to be a clone) */
    bool isOldTrack() const { return oldTrack; }

  protected:
    /** whether this is an old track */
    bool oldTrack;

    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerFinderClone, 1);
  };
}
#endif
