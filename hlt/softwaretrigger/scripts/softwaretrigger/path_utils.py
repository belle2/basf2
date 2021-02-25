import basf2
from softwaretrigger import constants
import modularAnalysis
import stdV0s
import vertex
from geometry import check_components
import reconstruction


def add_online_dqm(path, run_type, dqm_environment, components, dqm_mode, create_hlt_unit_histograms=False):
    """
    Add DQM plots for a specific run type and dqm environment
    """

    # Local imports, as the dqm package is not checked out by default
    from daqdqm.collisiondqm import add_collision_dqm
    from daqdqm.cosmicdqm import add_cosmic_dqm

    if run_type == constants.RunTypes.beam:
        add_collision_dqm(path, components=components, dqm_environment=dqm_environment,
                          dqm_mode=dqm_mode, create_hlt_unit_histograms=create_hlt_unit_histograms)
    elif run_type == constants.RunTypes.cosmic:
        add_cosmic_dqm(path, components=components, dqm_environment=dqm_environment,
                       dqm_mode=dqm_mode)
    else:
        basf2.B2FATAL("Run type {} not supported.".format(run_type))

    if dqm_mode in ["dont_care", "all_events"]:
        path.add_module('DelayDQM', title=dqm_environment, histogramDirectoryName='DAQ')


def add_hlt_dqm(path, run_type, components, dqm_mode, create_hlt_unit_histograms=False):
    """
    Add all the DQM modules for HLT to the path
    """
    add_online_dqm(
        path,
        run_type=run_type,
        dqm_environment=constants.Location.hlt.name,
        components=components,
        dqm_mode=dqm_mode.name,
        create_hlt_unit_histograms=create_hlt_unit_histograms)
    path.add_module('StatisticsSummary').set_name('Sum_HLT_DQM_' + dqm_mode.name)


def add_expressreco_dqm(path, run_type, components):
    """
    Add all the DQM modules for ExpressReco to the path
    """
    add_online_dqm(path, run_type=run_type, dqm_environment=constants.Location.expressreco.name, components=components,
                   dqm_mode=constants.DQMModes.dont_care.name)


def add_geometry_if_not_present(path):
    """
    Add the geometry and gearbox module if it was not already added to the path
    """
    if 'Gearbox' not in path:
        path.add_module('Gearbox')

    if 'Geometry' not in path:
        path.add_module('Geometry', useDB=True)


def add_store_only_metadata_path(path):
    """
    Helper function to create a path which deletes (prunes) everything from the data store except
    things that are really needed, e.g. the event meta data and the results of the software trigger module.

    After this path was processed, you can not use the data store content any more to do reconstruction (because
    it is more or less empty), but can only output it to a (S)ROOT file.
    """
    path.add_module("PruneDataStore", matchEntries=constants.ALWAYS_SAVE_OBJECTS).set_name("KeepMetaData")


def add_store_only_rawdata_path(path, additonal_store_arrays_to_keep=None):
    """
    Helper function to create a path which deletes (prunes) everything from the data store except
    raw objects from the detector and things that are really needed, e.g. the event meta data and the results of the
    software trigger module.

    After this path was processed, you can not use the data store content any more to do reconstruction (because
    it is more or less empty), but can only output it to a (S)ROOT file.
    """
    entries_to_keep = constants.ALWAYS_SAVE_OBJECTS + constants.RAWDATA_OBJECTS

    if additonal_store_arrays_to_keep:
        entries_to_keep += additonal_store_arrays_to_keep

    path.add_module("PruneDataStore", matchEntries=entries_to_keep).set_name("KeepRawData")


def add_filter_software_trigger(path,
                                store_array_debug_prescale=0,
                                use_random_numbers_for_prescale=True):
    """
    Add the SoftwareTrigger for the filter cuts to the given path.

    Only the calculation of the cuts is implemented here - the cut logic has to be done
    using the module return value.

    :param path: The path to which the module should be added.
    :param store_array_debug_prescale: When not 0, store each N events the content of the variables needed for the
     cut calculations in the data store.
    :param use_random_numbers_for_prescale: If True, the prescales are applied using randomly generated numbers,
     otherwise are applied using an internal counter.
    :return: the software trigger module
    """
    hlt_cut_module = path.add_module("SoftwareTrigger",
                                     baseIdentifier="filter",
                                     preScaleStoreDebugOutputToDataStore=store_array_debug_prescale,
                                     useRandomNumbersForPreScale=use_random_numbers_for_prescale)

    return hlt_cut_module


def add_skim_software_trigger(path, store_array_debug_prescale=0):
    """
    Add the SoftwareTrigger for the skimming (after the filtering) to the given path.

    Only the calculation of the cuts is implemented here - the cut logic has to be done

    :param path: The path to which the module should be added.
    :param store_array_debug_prescale: When not 0, store each N events the content of the variables needed for the
     cut calculations in the data store.
    :return: the software trigger module
    """
    # ECL cluster and track particle lists
    modularAnalysis.fillParticleList("pi+:skim", 'pt>0.2 and abs(d0) < 2 and abs(z0) < 4', path=path)
    modularAnalysis.fillParticleList("pi+:hadb", 'p>0.1 and abs(d0) < 2 and abs(z0) < 4', path=path)
    modularAnalysis.fillParticleList("pi+:tau", 'abs(d0) < 2 and abs(z0) < 8', path=path)
    modularAnalysis.fillParticleList("gamma:skim", 'E>0.1', loadPhotonBeamBackgroundMVA=False, path=path)
    stdV0s.stdKshorts(path=path, fitter='KFit')
    modularAnalysis.cutAndCopyList('K_S0:dstSkim', 'K_S0:merged', 'goodBelleKshort == 1', True, path=path)
    stdV0s.stdLambdas(path=path)
    modularAnalysis.fillParticleList("K+:dstSkim", 'abs(d0) < 2 and abs(z0) < 4', path=path)
    modularAnalysis.fillParticleList("pi+:dstSkim", 'abs(d0) < 2 and abs(z0) < 4', path=path)
    modularAnalysis.fillParticleList("gamma:loose", 'theta > 0.296706 and theta < 2.61799 and \
    [[clusterReg == 1 and E > 0.03] or [clusterReg == 2 and E > 0.02] or [clusterReg == 3 and E > 0.03]] and \
    [abs(clusterTiming) < formula(1.0 * clusterErrorTiming) or E > 0.1] and [clusterE1E9 > 0.3 or E > 0.1]',
                                     loadPhotonBeamBackgroundMVA=False, path=path)
    modularAnalysis.reconstructDecay('pi0:loose -> gamma:loose gamma:loose', '0.075 < M < 0.175', 1, True, path=path)
    modularAnalysis.cutAndCopyList('pi0:veryLooseFit', 'pi0:loose', '', True, path=path)
    vertex.kFit('pi0:veryLooseFit', 0.0, 'mass', path=path)
    D0_Cut = '1.7 < M < 2.1'
    D0_Ch = ['K-:dstSkim pi+:dstSkim',
             'K-:dstSkim pi+:dstSkim pi0:veryLooseFit',
             'K-:dstSkim pi+:dstSkim pi-:dstSkim pi+:dstSkim',
             'K_S0:dstSkim pi+:dstSkim pi-:dstSkim']

    for chID, channel in enumerate(D0_Ch):
        chID += 1
        modularAnalysis.reconstructDecay('D0:ch' + str(chID) + ' -> ' + str(channel), D0_Cut, dmID=chID, path=path)

    Dst_Cut = 'useCMSFrame(p) > 2.2 and massDifference(0) < 0.16'
    Dst_List = []

    for chID, channel in enumerate(D0_Ch):
        chID += 1
        modularAnalysis.reconstructDecay('D*+:ch' + str(chID) + ' -> D0:ch' + str(chID) + ' pi+:all', Dst_Cut, dmID=chID, path=path)
        Dst_List.append('D*+:ch' + str(chID))
    modularAnalysis.copyLists(outputListName='D*+:d0pi', inputListNames=Dst_List, path=path)
    modularAnalysis.fillParticleList("pi+:offip", '[abs(d0) > 1 and abs(z0) > 2] and [nSVDHits >=3 or nCDCHits >= 20]', path=path)

    path.add_module("SoftwareTrigger", baseIdentifier="skim",
                    preScaleStoreDebugOutputToDataStore=store_array_debug_prescale)


def add_filter_reconstruction(path, run_type, components, **kwargs):
    """
    Add everything needed to calculation a filter decision and if possible,
    also do the HLT filtering. This is only possible for beam runs (in the moment).

    Please note that this function adds the HLT decision, but does not branch
    according to it.
    """
    check_components(components)

    if run_type == constants.RunTypes.beam:
        reconstruction.add_prefilter_reconstruction(
            path,
            skipGeometryAdding=True,
            pruneTracks=False,
            add_trigger_calculation=False,
            components=components,
            event_abort=hlt_event_abort,
            **kwargs)

        add_filter_software_trigger(path, store_array_debug_prescale=1)
        path.add_module('StatisticsSummary').set_name('Sum_HLT_Filter_Calculation')

    elif run_type == constants.RunTypes.cosmic:
        reconstruction.add_cosmics_reconstruction(path, skipGeometryAdding=True, pruneTracks=False,
                                                  components=components, **kwargs)
    else:
        basf2.B2FATAL(f"Run Type {run_type} not supported.")


def add_filter_module(path):
    """
    Add and return a skim module, which has a return value dependent
    on the final HLT decision.
    """
    return path.add_module("TriggerSkim", triggerLines=["software_trigger_cut&all&total_result"])


def add_post_filter_reconstruction(path, run_type, components):
    """
    Add all modules which should run after the HLT decision is taken
    and only on the accepted events.
    This includes reconstruction modules not essential
    to calculate filter decision and then the skim calculation.
    """
    check_components(components)

    if run_type == constants.RunTypes.beam:
        reconstruction.add_postfilter_reconstruction(path, components=components, add_trigger_calculation=False,
                                                     pruneTracks=False)

        add_skim_software_trigger(path, store_array_debug_prescale=1)
        path.add_module('StatisticsSummary').set_name('Sum_HLT_Skim_Calculation')
    elif run_type == constants.RunTypes.cosmic:
        pass
    else:
        basf2.B2FATAL(f"Run Type {run_type} not supported.")


def hlt_event_abort(module, condition, error_flag):
    """
    Create a discard path suitable for HLT processing, i.e. set an error flag and
    keep only the metadata.
    """
    p = basf2.Path()
    p.add_module("EventErrorFlag", errorFlag=error_flag)
    add_store_only_metadata_path(p)
    module.if_value(condition, p, basf2.AfterConditionPath.CONTINUE)
