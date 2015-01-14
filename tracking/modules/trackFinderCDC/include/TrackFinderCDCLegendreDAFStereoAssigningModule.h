#pragma once

#include <framework/core/Module.h>
#include <fstream>

#include <boost/tuple/tuple.hpp>
#include <vector>

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackWithStereohits.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreStereohitsProcesser.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackHit.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackProcessor.h>

namespace Belle2 {

//  class TrackCandidateWithStereoHit;

  class CDCLegendreDAFStereoAssigningModule: public Module {

  public:


    /** Constructor.
     *  Create and allocate memory for variables here. Add the module parameters in this method.
     */
    CDCLegendreDAFStereoAssigningModule();

    /** Destructor.
     * Use the destructor to release the memory you allocated in the constructor.
     */
    virtual ~CDCLegendreDAFStereoAssigningModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     * Use this method to initialize variables, open files etc.
     */
    virtual void initialize();

    /** Called when entering a new run;
     * Called at the beginning of each run, the method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun() {};

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    virtual void event();

    /** This method is called if the current run ends.
     * Use this method to store information, which should be aggregated over one run.
     */
    virtual void endRun() {};

    /** This method is called at the end of the event processing.
     *  Use this method for cleaning up, closing files, etc.
     */
    virtual void terminate() {};


  protected:


  private:
    static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/

    std::vector<TrackFindingCDC::TrackHit*> m_AxialHitList; /**< List of the axial hits used for track finding. This is the vector, which is used for memory management! */
    std::vector<TrackFindingCDC::TrackHit*> m_StereoHitList; /**< List of the stereo hits used for track finding. This is the vector, which is used for memory management! */
    std::list<TrackFindingCDC::TrackCandidateWithStereoHits*> m_trackList; /**< List of track candidates. Mainly used for memory management! */

    void clear_pointer_vectors();

  };

}
