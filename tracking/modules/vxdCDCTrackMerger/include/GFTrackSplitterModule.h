#ifndef GFTRACKSPLITTERMODULE_H
#define GFTRACKSPLITTERMODULE_H

#include <framework/core/Module.h>
#include "genfit/Track.h"
//#include "genfit/TrackCandidate.h"
#include "genfit/RKTrackRep.h"
//#include "genfit/AbsTrackRep.h"

//root stuff
#include <TTree.h>
#include <TFile.h>


namespace Belle2 {
  /** A module siCDCTrackMerger.
   *
   *  A detailed description of your module.
   */
  class GFTrackSplitterModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    GFTrackSplitterModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~GFTrackSplitterModule();

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

    std::string m_SiGFTracksColName; /**< Name of output GFtrack collection from vertex detector*/
    std::string m_CDCGFTracksColName;  /**< Name of output GFtrack collection from CDC*/
    std::string m_GFTracksColName; /**< Name of input GFtrack collection*/
    std::string m_storeTrackCandName; /**< Name of output GFTrackCand collection*/
  };
}
#endif
