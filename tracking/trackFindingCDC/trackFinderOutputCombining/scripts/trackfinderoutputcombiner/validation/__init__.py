from tracking.run.event_generation import StandardEventGenerationRun
from tracking.validation.module import SeparatedTrackingValidationModule
from tracking.metamodules import IfStoreArrayNotPresentModule

from tracking import modules


def add_printer(path):
    # Adds the printer module to the path
    print_collections_module = StandardEventGenerationRun.get_basf2_module("PrintCollections")
    path.add_module(print_collections_module)


def add_mc_track_finder(path, mc_track_cands_store_array_name="MCTrackCands"):
    # Adds the MCTrackFinder module to the path.
    # Outputs to the given StoreArray the created genfit::TrackCands
    mc_track_finder_module = StandardEventGenerationRun.get_basf2_module('TrackFinderMCTruth',
                                                                         UseCDCHits=True,
                                                                         WhichParticles=[],
                                                                         GFTrackCandidatesColName=mc_track_cands_store_array_name)

    mc_track_finder_module_if_module = IfStoreArrayNotPresentModule(mc_track_finder_module,
                                                                    storearray_name=mc_track_cands_store_array_name)
    path.add_module(mc_track_finder_module_if_module)


def add_background_hit_finder(path, tmva_cut=0.1):
    # Adds the background hit finder to the path.
    if tmva_cut > 0:
        path.add_module(modules.CDCBackgroundHitFinder(tmva_cut=tmva_cut))


def add_legendre_track_finder(path, delete_hit_information=False,
                              output_track_cands_store_array_name=None,
                              output_track_cands_store_vector_name="CDCTrackVector",
                              assign_stereo_hits=True):

    # Add the legendre finder to the path.
    # If assign_stereo_hits is True, also add the Histogramming module (default)
    # If delete_hit_information is True, all already marked hits get unmarked (not default)
    # If output_track_cands_store_array_name is None, do not write out the track cands to a genfit store array (default)
    # With output_track_cands_store_vector_name you can control the output vector of the CDCTracks. Be aware that every content
    # in this vector will be deleted before executing this module!

    path.add_module(modules.CDCLegendreTrackFinder(delete_hit_information,
                                                   output_track_cands_store_array_name,
                                                   output_track_cands_store_vector_name,
                                                   assign_stereo_hits))


def add_local_track_finder(path, delete_hit_information=False,
                           output_track_cands_store_array_name=None,
                           output_segments_store_vector_name="CDCRecoSegment2DVector"):

    # Add the local finder to the path.
    # If delete_hit_information is True, all already marked hits get unmarked (not default)
    # If output_track_cands_store_array_name is None, do not write out the track cands to a genfit store array (default)
    # With output_segments_store_vector_name you can control the output vector of the CDCRecoSegments2D. Be aware that every content
    # in this vector will be deleted before executing this module!

    # The vector with _Temp gets registered but not created

    path.add_module(modules.CDCLocalTrackFinder(delete_hit_information,
                                                output_track_cands_store_array_name,
                                                output_segments_store_vector_name))


def add_old_combiner(path, output_track_cands_store_array_name=None,
                     track_cands_store_vector_name="CDCTrackVector",
                     segments_store_vector_name="CDCRecoSegment2DVector",
                     use_second_stage=False):

    # Add the old combiner module to the path
    # If output_track_cands_store_array_name is None, do not write out the track cands to a genfit store array (default)
    # With track_cands_store_vector_name you can control the input vector of the CDCTracks. Be aware that the content
    # of this vector will be replaced by the output of this module
    # With segments_store_vector_name you can control the input vector of the CDCRecoSegments2D. Be aware that the content
    # of this vector will be replaced by the output of this module

    path.add_module(modules.CDCNotAssignedHitsCombiner(output_track_cands_store_array_name,
                                                       track_cands_store_vector_name,
                                                       segments_store_vector_name,
                                                       use_second_stage))


def add_new_combiner(path, output_track_cands_store_array_name=None,
                     track_cands_store_vector_name="CDCTrackVector",
                     segments_store_vector_name="CDCRecoSegment2DVector",
                     segment_track_chooser_filter="simple",
                     segment_track_chooser_cut=0,
                     segment_train_filter="simple",
                     segment_track_filter="simple"):

    # Add the new combiner module to the path
    # If output_track_cands_store_array_name is None, do not write out the track cands to a genfit store array (default)
    # With track_cands_store_vector_name you can control the input vector of the CDCTracks. Be aware that the content
    # of this vector will be replaced by the output of this module
    # With segments_store_vector_name you can control the input vector of the CDCRecoSegments2D. Be aware that the content
    # of this vector will be replaced by the output of this module
    # With the other parameters you can control the filters of the SegmentTrackCombiner

    path.add_module(modules.CDCSegmentTrackCombiner(output_track_cands_store_array_name,
                                                    track_cands_store_vector_name,
                                                    segments_store_vector_name,
                                                    segment_track_chooser_filter,
                                                    segment_track_chooser_cut,
                                                    segment_train_filter,
                                                    segment_track_filter))


def add_validation(path, track_candidates_store_array_name, output_file_name):

    # Add a validation and a mc track matcher to the path
    # With track_candidates_store_array_name you can choose the name of the genfit::TrackCands this validation should be created for
    # With output_file_name you can choose the file name for the results

    path.add_module(modules.CDCValidation(track_candidates_store_array_name,
                                          output_file_name))
