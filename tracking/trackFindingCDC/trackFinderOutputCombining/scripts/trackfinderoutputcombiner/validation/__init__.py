from tracking.run.event_generation import StandardEventGenerationRun
from tracking.validation.module import SeparatedTrackingValidationModule
from tracking.metamodules import IfStoreArrayNotPresentModule


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
    # The vectors with __Temp get registered but not created.
    if tmva_cut < 0 or tmva_cut > 1:
        print "Given tmva_cut %.1f is not in the valid range [0, 1]. Not adding the module" % tmva_cut
        return

    background_hit_finder_module = StandardEventGenerationRun.get_basf2_module("SegmentFinderCDCFacetAutomatonDev",
                                                                               ClusterFilter="tmva",
                                                                               ClusterFilterParameters={"cut": str(tmva_cut)},
                                                                               SegmentsStoreObjName="__TempCDCRecoSegment2DVector",
                                                                               CreateGFTrackCands=False,
                                                                               WriteGFTrackCands=False,
                                                                               FacetFilter="none",
                                                                               FacetRelationFilter="none",
                                                                               TracksStoreObjName="__TempCDCTracksVector")

    if tmva_cut > 0:
        path.add_module(background_hit_finder_module)


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

    legendre_tracking_module = StandardEventGenerationRun.get_basf2_module('CDCLegendreTracking',
                                                                           WriteGFTrackCands=False,
                                                                           TracksStoreObjName=output_track_cands_store_vector_name)

    if delete_hit_information:
        legendre_tracking_module.param('SkipHitsPreparation', False)
    else:
        legendre_tracking_module.param('SkipHitsPreparation', True)

    last_tracking_module = legendre_tracking_module

    cdc_stereo_combiner = StandardEventGenerationRun.get_basf2_module('CDCLegendreHistogramming',
                                                                      SkipHitsPreparation=True,
                                                                      TracksStoreObjNameIsInput=True,
                                                                      WriteGFTrackCands=False,
                                                                      TracksStoreObjName=output_track_cands_store_vector_name)

    if assign_stereo_hits:
        path.add_module(last_tracking_module)
        last_tracking_module = cdc_stereo_combiner

    if output_track_cands_store_array_name is not None:
        last_tracking_module.param({'WriteGFTrackCands': True,
                                    'GFTrackCandsStoreArrayName': output_track_cands_store_array_name})

    path.add_module(last_tracking_module)


def add_local_track_finder(path, delete_hit_information=False,
                           output_track_cands_store_array_name=None,
                           output_segments_store_vector_name="CDCRecoSegments2DVector"):

    # Add the local finder to the path.
    # If delete_hit_information is True, all already marked hits get unmarked (not default)
    # If output_track_cands_store_array_name is None, do not write out the track cands to a genfit store array (default)
    # With output_segments_store_vector_name you can control the output vector of the CDCRecoSegments2D. Be aware that every content
    # in this vector will be deleted before executing this module!

    # The vector with _Temp gets registered but not created

    local_track_finder_module = StandardEventGenerationRun.get_basf2_module('SegmentFinderCDCFacetAutomaton',
                                                                            SegmentsStoreObjName=output_segments_store_vector_name,
                                                                            WriteGFTrackCands=False,
                                                                            FitSegments=True,
                                                                            TracksStoreObjName="__TempCDCTracksVector")

    if delete_hit_information:
        local_track_finder_module.param('SkipHitsPreparation', False)
    else:
        local_track_finder_module.param('SkipHitsPreparation', True)

    if output_track_cands_store_array_name is not None:
        local_track_finder_module.param({'WriteGFTrackCands': True,
                                         'GFTrackCandsStoreArrayName': output_track_cands_store_array_name,
                                         'CreateGFTrackCands': True})

    path.add_module(local_track_finder_module)


def add_old_combiner(path, output_track_cands_store_array_name=None,
                     track_cands_store_vector_name="CDCTrackVector",
                     segments_store_vector_name="CDCRecoSegments2DVector"):

    # Add the old combiner module to the path
    # If output_track_cands_store_array_name is None, do not write out the track cands to a genfit store array (default)
    # With track_cands_store_vector_name you can control the input vector of the CDCTracks. Be aware that the content
    # of this vector will be replaced by the output of this module
    # With segments_store_vector_name you can control the input vector of the CDCRecoSegments2D. Be aware that the content
    # of this vector will be replaced by the output of this module

    not_assigned_hits_combiner_module = StandardEventGenerationRun.get_basf2_module(
        "NotAssignedHitsCombiner",
        SkipHitsPreparation=True,
        TracksStoreObjNameIsInput=True,
        WriteGFTrackCands=False,
        SegmentsStoreObjName=segments_store_vector_name,
        TracksStoreObjName=track_cands_store_vector_name)

    if output_track_cands_store_array_name is not None:
        not_assigned_hits_combiner_module.param({'WriteGFTrackCands': True,
                                                 'GFTrackCandsStoreArrayName': output_track_cands_store_array_name})
    import basf2
    not_assigned_hits_combiner_module.set_log_level(basf2.LogLevel.DEBUG)
    path.add_module(not_assigned_hits_combiner_module)


def add_new_combiner(path, output_track_cands_store_array_name=None,
                     track_cands_store_vector_name="CDCTrackVector",
                     segments_store_vector_name="CDCRecoSegments2DVector",
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

    combiner_module = StandardEventGenerationRun.get_basf2_module("SegmentTrackCombinerDev",
                                                                  SegmentTrackChooser=segment_track_chooser_filter,
                                                                  SegmentTrainFilter=segment_train_filter,
                                                                  SegmentTrackFilter=segment_track_filter,
                                                                  WriteGFTrackCands=False,
                                                                  SkipHitsPreparation=True,
                                                                  TracksStoreObjNameIsInput=True,
                                                                  SegmentsStoreObjName=segments_store_vector_name,
                                                                  TracksStoreObjName=track_cands_store_vector_name)

    if segment_track_chooser_filter == "tmva":
        combiner_module.param('SegmentTrackChooserParameters', {"cut": str(segment_track_chooser_cut)})

    if output_track_cands_store_array_name is not None:
        combiner_module.param({'WriteGFTrackCands': True,
                               'GFTrackCandsStoreArrayName': output_track_cands_store_array_name})

    path.add_module(combiner_module)


def add_validation(path, track_candidates_store_array_name, output_file_name):

    # Add a validation and a mc track matcher to the path
    # With track_candidates_store_array_name you can choose the name of the genfit::TrackCands this validation should be created for
    # With output_file_name you can choose the file name for the results

    mc_track_matcher_module = StandardEventGenerationRun.get_basf2_module('MCTrackMatcher',
                                                                          UseCDCHits=True,
                                                                          UseSVDHits=False,
                                                                          UsePXDHits=False,
                                                                          RelateClonesToMCParticles=True,
                                                                          MCGFTrackCandsColName="MCTrackCands",
                                                                          PRGFTrackCandsColName=track_candidates_store_array_name)

    path.add_module(mc_track_matcher_module)

    validation_module = SeparatedTrackingValidationModule(
        name="",
        contact="",
        output_file_name=output_file_name,
        trackCandidatesColumnName=track_candidates_store_array_name,
        expert_level=2)

    path.add_module(validation_module)
