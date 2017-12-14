/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TELTRACKFINDERMODULE_H
#define TELTRACKFINDERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <genfit/Track.h>
#include <genfit/TrackCand.h>

#include <testbeam/vxd/dataobjects/TelCluster.h>

#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>

//root stuff
#include <TVector3.h>



namespace Belle2 {
  /**
   * Finds a track candidate including telescope clusters by taking a genfit::Track and projecting it to the telescope planes.
   * The output is a StoreArray with genfit::TrackCandidates which include all clusters from the original TrackCandidate
   * (the one which was related the genfit::Track) and in addition the telescope tracks
   *
   * Please Note: this is only meant as workaraund as track finding in the Telescope does not work with the standard
   * modules. So the results of this track finder should be taken with caution. Also the default parameter never have
   * been tuned!
   */
  class TelTrackFinderModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TelTrackFinderModule();

    /** Destructor:   */
    virtual ~TelTrackFinderModule();

    /** Register input and output data */
    virtual void initialize();

    /** currently empty   */
    virtual void beginRun();

    /** all basic steps are performed here */
    virtual void event();

    /** currently does nothing */
    virtual void endRun();

    /** currently empty  */
    virtual void terminate();

    /**
    adds the index of cluster candidates close to the track intersection with the senso to the list of cluster candidates
    @param clusterindizes: the list of indizes to which the clusters will be added
    @param avxdid: is the sensor id where the clusters are searched
    @param intersec: the track intersection on the sensor (local coordinates )
    @param du: fit uncertainty in u of the fitted track intersection
    @param dv: fit uncertainty in v of the fitted track intersection
    */
    void findClusterCands(std::vector<int>& clusterindizes, VxdID avxdid, TVector3& intersec, double du = -99., double dv = -99.);

    /** returns the track intersection for the given sensor (local coordinates)
    @param anid: the VxdID of the sensor
    @param track: the track
    @param isgood: will be false if finding the intersection has failed
    @param du: fit uncertainty in u direction
    @param dv: fit uncertainty in v direction
    */
    TVector3 getTrackIntersec(VxdID anid, const genfit::Track& track, bool& isgood, double& du, double& dv);


    /**
      adds a new genfit::trackcand to the output array of track candidates. (Currently no relations are added!)
      @param telHitIndizes: vector with indizes of the telescope clusters within the input array
      @param trackCand: the base track candidate used to add the hits
    */
    void addNewTrackCand(const  std::vector<int>& telHitIndizes, const genfit::TrackCand& trackCand);

  private:

    //the reference to the geometry
    VXD::GeoCache& m_vxdGeometry;


    int m_mintelLadder; /** the "HARDCODED" ladder which is used for the telescope */
    int m_maxtelLadder; /** the "HARDCODED" ladder which is used for the telescope */

    double m_distanceCut;/**<cut in cm on the distance of a cluster to the track intersection to be considered to belong to the track*/
    int m_minTelLayers;

    std::string m_inputTracksName;  /**< name of store array with input genfit::Tracks */
    std::string m_outputTrackCandsName;  /**< name of store array with output genfit::TrackCands  */
    std::string m_inputClustersName; /**< name of store array with the input clusters (not sure which format yet)*/


    StoreArray<genfit::Track> m_tracks; /**< store array with genfit::Tracks */
    StoreArray<genfit::TrackCand> m_inputTrackCands; /**< store array with track cands the genfit::Tracks have been created from*/
    StoreArray<genfit::TrackCand> m_trackCands; /**< store array with output track cands */

    StoreArray<TelCluster> m_clusters; /** type might have to be PXDCluster ?!?!*/
  };
}

#endif /* TELTRACKFINDERMODULE_H */
