/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once


#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <string>


namespace Belle2 {
  class RecoTrack;

  /** Class to help managing the creation of RecoTrack StoreArray and adding of RecoTracks.
   *
   * The implementation follows the design pattern of the class `ParticleListHelper`.
   *
   * There's a bit of management involved when trying to create a new tracks store array
   *
   * - the tracks store array needs to be registered in the datastore
   * - in each event the store array need to be initialized
   * - when adding a track it needs to be added to the tracks array
   *
   * This class is managing all this in a clean interface
   *
   * 1. Add one instance as member to your class
   * 2. in the `initialize()` function call `registerArray()` with the full array
   *    name
   * 3. in the beginning of the `event()` function call `create()`
   * 4. Fill reco tracks with `add()` or `addTrack` (the second one is for the
   *    benefit of python as currently ROOT has problems with variadic template
   *    functions forwarding arguments)
   + 5. Copy over existing track with all hits with `copyTrackWithHitsAndRelations`
   */
  class TrackStoreArrayHelper {
  public:
    /** Default constructor, does nothing */
    TrackStoreArrayHelper() = default;
    /** Construct and initialize right away. Useful in cases where you create
     * these objects directly in the `initialize()` function of a module
     */
    explicit TrackStoreArrayHelper(const std::string& arrayName, bool errorIfAlreadyRegistered = false): TrackStoreArrayHelper()
    {
      registerArray(arrayName, errorIfAlreadyRegistered);
    }
    /** Register a array by name
     *
     * \warning must be called in initialize
     *
     * @param arrayName full array name
     * @param errorIfAlreadyRegistered flag to error if StoreArray exists already in data store
     */
    void registerArray(const std::string& arrayName, bool errorIfAlreadyRegistered);

    /** Add a track to the list by forwarding all arguments to the constructor
     * of the RecoTrack object: Any valid arguments to construct a Belle2::RecoTrack work here.
     *
     * It will return a reference to the created track for further modification.
     */
    template<class ...Args> RecoTrack& add(Args&& ... args)
    {
      // cppcheck doesn't seem to understand the forwarding of arguments and
      // throws an erroneous warning about reassigning appendNew
      //
      // cppcheck-suppress redundantAssignment
      // cppcheck-suppress redundantInitialization
      auto track = m_recoTracks.appendNew(std::forward<Args>(args)...);
      return *track;
    }

    /** Non-templated convenience function to add a track
     */
    RecoTrack& addTrack(const ROOT::Math::XYZVector& seedPosition, const ROOT::Math::XYZVector& seedMomentum,
                        const short int seedCharge)
    {
      return add(seedPosition, seedMomentum, seedCharge);
    }

    /** Non-templated convenience function copy a track with hits and relations but not fit results
     */
    void copyTrackWithHitsAndRelations(const RecoTrack& recoTrack);

  private:
    /** RecoTracks StoreArray name */
    std::string m_recoTracksStoreArrayName;
    /** Store array for the reco tracks */
    StoreArray<RecoTrack>  m_recoTracks;
  };
}
