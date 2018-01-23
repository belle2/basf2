#pragma once
//Object with performing the actual algorithm:
#include <tracking/v0Finding/fitter/V0Fitter.h>

#include <mdst/dataobjects/Track.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

#include <string>
#include <memory>

namespace Belle2 {
  /** A V0 finder module.
   *
   *  Pairs up all positive and negative tracks,
   *  tries to find vertices between them,
   *  stores found vertices.
   *  Only vertices outside the beam pipe are saved, the others are
   *  recovered in a following analysis level
   *  for use inside the beam pipe, one outside (the difference being
   *
   *  Cut outside the beam pipe is maximum chi^2.
   *  The value used as beam pipe radius is an option.
   *
   *  FIXME once particle hypotheses are thought through, we should
   *  also deal with photon conversion.
   *
   *  The resulting pairs of tracks are stored as mdst::V0.
   */
  class V0FinderModule : public Module {
  public:
    /** Setting of module description, parameters. */
    V0FinderModule();

    /** Acknowledgement of destructor. */
    ~V0FinderModule() override = default;

    /** Registration of StoreArrays, Relations, check proper GenFit setup. */
    void initialize() override;

    /** Creates Belle2::V0s from Belle2::Tracks as described in the class documentation. */
    void event() override;

  private:
    std::string m_arrayNameTrack;     ///< StoreArray name of the Tracks          (Input).
    StoreArray<Track> m_tracks;       ///< Actually array of mdst Tracks.

    std::unique_ptr<V0Fitter> m_v0Fitter; ///< Object containing the actual algorithm.
    std::string m_arrayNameRecoTrack;     ///< StoreArray name of the RecoTracks      (Input).
    std::string m_arrayNameTFResult;      ///< StoreArray name of the TrackFitResults (In- and Output).
    std::string m_arrayNameV0;            ///< StoreArray name of the V0              (Output).

    double m_beamPipeRadius;          ///< Radius where inside/outside beampipe is defined.
    double m_vertexChi2CutOutside;    ///< Chi2 cut for V0s outside of the beampipe. Applies to all.

    bool m_validation;                         ///< Flag if use validation.
    std::string m_arrayNameV0ValidationVertex; ///< StoreArray name of the V0ValidationVertex.
  };
}
