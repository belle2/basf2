/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLTRACKSHOWERMATCH_H
#define ECLTRACKSHOWERMATCH_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <ecl/dataobjects/ECLShower.h>

namespace Belle2 {

  /** The modules creates and saves in the DataStore a Relation between Tracks and ECLShower.
   * It uses the existing Relation between Tracks and ExtHit, from which the ECL crystals
   *  hit by the extrapolated tracks are found. All the ECLShowers containing the found
   * crystals are then associated to each Track. A Relation Track --> ECLShower is filled and saved
   * in the DataStore.
   */
  class ECLTrackShowerMatchModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    ECLTrackShowerMatchModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~ECLTrackShowerMatchModule();

    /** Use this to initialize resources or memory your module needs.
     *
     *  Also register any outputs of your module (StoreArrays, RelationArrays,
     *  StoreObjPtrs) here, see the respective class documentation for details.
     */
    virtual void initialize();

    /** Called once before a new run begins.
     *
     * This method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun();

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event();

    /** Called once when a run ends.
     *
     *  Use this method to save run information, which you aggregated over the last run.
     */
    virtual void endRun();

    /** Clean up anything you created in initialize(). */
    virtual void terminate();
  private:

    /** Minimal distance between track and shower. */
    double computeTrkMinDistance(const ECLShower&, StoreArray<Track>&) const;
  };

} //Belle2
#endif
