#ifndef CDCTRIGGER3DFITTERModule_H
#define CDCTRIGGER3DFITTERModule_H

#include "framework/core/Module.h"
#include <string>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <framework/dataobjects/EventT0.h>

namespace Belle2 {

  /** Module for the 3D Fitter of the CDC trigger. */
  class CDCTrigger3DFitterModule : public Module {

  public:

    /** Constructor, for setting module description and parameters. */
    CDCTrigger3DFitterModule();

    /** Destructor */
    virtual ~CDCTrigger3DFitterModule() {}

    /** Initialize the module and register DataStore arrays. */
    virtual void initialize();

    /** Run the 3D fitter for an event. */
    virtual void event();

  protected:
    /** Select stereo hits.
     * @param charge      charge of 2D track
     * @param rho         radius of 2D track
     * @param phi         angle to circle center of 2D track
     * @param bestTSIndex will contain the selected hit indices
     * @param bestTSPhi   will contain the wire phi of selected hits
     */
    void finder(int charge, double rho, double phi,
                std::vector<int>& bestTSIndex, std::vector<double>& bestTSPhi);
    /** Perform the 3D fit.
     * @param bestTSIndex list of selected hit indices
     * @param bestTSPhi   list of wire phi for selected hits
     * @param charge      charge of 2D track
     * @param rho         radius of 2D track
     * @param phi         angle to circle center of 2D track
     * @param z0          will contain the fitted z-vertex
     * @param cot         will contain the fitted cot(theta)
     * @param chi2        will contain the chi2 value for the fit
     */
    void fitter(std::vector<int>& bestTSIndex, std::vector<double>& bestTSPhi,
                int charge, double rho, double phi,
                double& z0, double& cot, double& chi2);

    /** Name of the StoreArray containing the input track segment hits. */
    std::string m_hitCollectionName;
    /** name of the event time StoreObjPtr */
    std::string m_EventTimeName;
    /** Name of the StoreArray containing the input tracks from the 2D fitter. */
    std::string m_inputCollectionName;
    /** Name of the StoreArray containing the resulting 3D tracks. */
    std::string m_outputCollectionName;
    /** Minimal number of hits required for fitting. */
    unsigned m_minHits;
    /** Switch between nominal drift velocity and xt table */
    bool m_xtSimple;

  private:
    /** geometry constants: number of wires per super layer */
    std::vector<double> nWires = {};
    /** geometry constants: radius of priority layers */
    std::vector<double> rr = {};
    /** geometry constants: backward z of priority layers */
    std::vector<double> zToStraw = {};
    /** geometry constants: wire shift of stereo layers */
    std::vector<int> nShift = {};
    /** geometry constants: stereo angle */
    std::vector<double> angleSt = {};
    /** geometry constants: drift length - drift time relation */
    std::vector<std::vector<double>> xtTables = {};

    /** list of 2D input tracks */
    StoreArray<CDCTriggerTrack> m_tracks2D;
    /** list of 3D output tracks*/
    StoreArray<CDCTriggerTrack> m_tracks3D;
    /** list of track segment hits */
    StoreArray<CDCTriggerSegmentHit> m_hits;
    /** StoreObjPtr containing the event time */
    StoreObjPtr<EventT0> m_eventTime;
  };

} // namespace Belle2

#endif // CDCTrigger3DFitterModule_H
