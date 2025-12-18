# This is the main flavortagger function

import basf2
from basf2 import B2INFO, B2FATAL, B2WARNING
import modularAnalysis as ma
from variables import variables as va
from flavorTagger.utils import (
    get_available_categories, set_FT_pid_aliases, read_yaml,
    get_Belle_or_Belle2, set_GFlat_aliases, set_masked_vars,
    fill_particle_lists, set_output_vars
)
from .event_level import event_level
from .combiner_level import combiner_level
import os


def flavorTagger(
    particleLists=None,
    weightFiles='B2nunubarBGx1',
    workingDirectory='.',
    combinerMethods=['TMVA-FBDT'],
    categories=[
        'Electron',
        'IntermediateElectron',
        'Muon',
        'IntermediateMuon',
        'KinLepton',
        'IntermediateKinLepton',
        'Kaon',
        'SlowPion',
        'FastHadron',
        'Lambda',
        'FSC',
        'MaximumPstar',
        'KaonPion'],
    maskName='FTDefaultMask',
    saveCategoriesInfo=True,
    useOnlyLocalWeightFiles=False,
    downloadFromDatabaseIfNotFound=False,
    prefix='MC15ri_light-2207-bengal_0',
    useGNN=True,
    identifierGNN='GFlaT_MC15ri_light_2303_iriomote_0',
    usePIDNN=False,
    path=None,
):
    """
      Defines the whole flavor tagging process for each selected Rest of Event (ROE) built in the steering file.
      The flavor is predicted by Multivariate Methods trained with Variables and MetaVariables which use
      Tracks, ECL- and KLMClusters from the corresponding RestOfEvent dataobject.
      This module can be used to sample the training information, to train and/or to test the flavorTagger.

      @param particleLists                     The ROEs for flavor tagging are selected from the given particle lists.
      @param weightFiles                       Weight files name. Default=
                                               ``B2nunubarBGx1`` (official weight files). If the user wants to train the
                                               FlavorTagger themselves, the weightfiles name should correspond to the
                                               analyzed CP channel in order to avoid confusions. The default name
                                               ``B2nunubarBGx1`` corresponds to
                                               :math:`B^0_{\\rm sig}\\to \\nu \\overline{\\nu}`.
                                               and ``B2JpsiKs_muBGx1`` to
                                               :math:`B^0_{\\rm sig}\\to J/\\psi (\\to \\mu^+ \\mu^-) K_s (\\to \\pi^+ \\pi^-)`.
                                               BGx1 stands for events simulated with background.
      @param workingDirectory                  Path to the directory containing the FlavorTagging/ folder.
      @param combinerMethods                   MVAs for the combiner: ``TMVA-FBDT` (default).
                                               ``FANN-MLP`` is available only with ``prefix=''`` (MC13 weight files).
      @param categories                        Categories used for flavor tagging. By default all are used.
      @param maskName                          Gets ROE particles from a specified ROE mask.
                                               ``FTDefaultMask`` (default): tentative mask definition that will be created
                                               automatically. The definition is as follows:

                                               - Track (pion): thetaInCDCAcceptance and dr<1 and abs(dz)<3
                                               - ECL-cluster (gamma): thetaInCDCAcceptance and clusterNHits>1.5 and \
                                               [[clusterReg==1 and E>0.08] or [clusterReg==2 and E>0.03] or \
                                               [clusterReg==3 and E>0.06]] \
                                               (Same as gamma:pi0eff30_May2020 and gamma:pi0eff40_May2020)

                                               ``all``: all ROE particles are used.
                                               Or one can give any mask name defined before calling this function.
      @param saveCategoriesInfo                Sets to save information of individual categories.
      @param useOnlyLocalWeightFiles           [Expert] Uses only locally saved weight files.
      @param downloadFromDatabaseIfNotFound    [Expert] Weight files are downloaded from
                                               the conditions database if not available in workingDirectory.
      @param prefix                            Prefix of weight files.
                                               ``MC15ri_light-2207-bengal_0`` (default): Weight files trained with MC15ri samples.
                                               ``MC16rd_light-2501-betelgeuse``: Weight files trained with MC16rd samples.
                                               ``''``: Weight files trained for MC13 samples.
      @param useGNN                            Use GNN-based Flavor Tagger in addition with FastBDT-based one.
                                               Please specify the weight file with the option ``identifierGNN``.
                                               [Expert] In the sampler mode,
                                               training files for GNN-based Flavor Tagger are produced.
      @param identifierGNN                     The name of weight file of the GNN-based Flavor Tagger.
                                               ``GFlaT_MC15ri_light_2303_iriomote_0`` (default): Trained with MC15ri samples
                                               ``GFlaT_MC16rd_light-2501-betelgeuse_tensorflow``: Trained with MC16rd samples
                                               [Expert] Multiple identifiers can be given with list(str).
      @param usePIDNN                          If True, PID probabilities calculated from PID neural network are used
                                               (default is False). Prefix and identifierGNN must be set accordingly.
      @param path                              Modules are added to this path

    """

    # set common config
    exp_type = get_Belle_or_Belle2()
    available_categories = get_available_categories()
    config_filepath = basf2.find_file('data/analysis/config.yaml')
    config_params = read_yaml(config_filepath)
    signal_fraction = config_params["signal_fraction"]
    download_folder = config_params["database_io"]["download_folder"]

    # sanitize and cross check inputs

    # force the particle list to be a list
    if (not isinstance(particleLists, list)):
        particleLists = [particleLists]

    # ensure unique categories in list
    if len(categories) != len(set(categories)):
        dup = [cat for cat in set(categories) if categories.count(cat) > 1]
        B2WARNING(f"Flavor Tagger: There are duplicate elements in the given\
                   categories list. The following duplicate elements are\
                   removed: {', '.join(dup)}")
        categories = list(set(categories))

    # ensure minimum category number
    if len(categories) < 2:
        B2FATAL('Flavor Tagger: At least two categories are needed.')
        B2FATAL(f"Flavor Tagger: Possible categories are {available_categories.keys()}")

    # ensure legitimate categories
    for category in categories:
        if category not in available_categories:
            B2FATAL('Flavor Tagger: ' + category + ' is not a valid category name given')
            B2FATAL(f"Flavor Tagger: Possible categories are {available_categories.keys()}")

    # ensure correct GNN config
    if useGNN and identifierGNN == '':
        B2FATAL('Please specify the name of the weight file with ``identifierGNN``')

    # ensure correct combiner method config
    if len(combinerMethods) == 0:
        B2FATAL('Flavor Tagger: Please specify at least one combinerMethods.\
                 The available methods are "TMVA-FBDT" and "FANN-MLP"')

    FANNmlp = False
    TMVAfbdt = False

    for method in combinerMethods:
        if method == 'TMVA-FBDT':
            TMVAfbdt = True
        elif method == 'FANN-MLP':
            FANNmlp = True
        else:
            B2FATAL('Flavor Tagger: Invalid list of combinerMethods. \
                    The available methods are "TMVA-FBDT" and "FANN-MLP"')

    # check if working directory exists for download
    basf2.find_file(workingDirectory)

    files_dir = f"{workingDirectory}/{download_folder}"
    if downloadFromDatabaseIfNotFound:
        if not basf2.find_file(files_dir, silent=True):
            os.makedirs(files_dir)

    # verbose
    B2INFO(f"""
           *** FLAVOR TAGGING ***
           Working directory is: {files_dir}
           """)

    # setup FT pid alias
    if prefix == '':
        set_FT_pid_aliases(type="MC13", exp_type=exp_type)
    else:
        set_FT_pid_aliases(type="Current", exp_type=exp_type)
        weightFiles = f"{prefix}_{weightFiles}"

    # set GNN aliases
    if useGNN:
        set_GFlat_aliases(categories, usePIDNN)

    # set input masked vars
    set_masked_vars()

    # Create configuration lists and code-name for given category's list
    trackLevelParticleLists = []
    eventLevelParticleLists = []
    variablesCombinerLevel = []
    categoriesCombination = []
    categoriesCombinationCode = 'CatCode'
    for category in categories:
        ftCategory = available_categories[category]

        track_tuple = (ftCategory.particleList, ftCategory.trackName)
        event_tuple = (ftCategory.particleList, ftCategory.eventName, ftCategory.variableName)

        if track_tuple not in trackLevelParticleLists and category != 'MaximumPstar':
            trackLevelParticleLists.append(track_tuple)

        if event_tuple not in eventLevelParticleLists:
            eventLevelParticleLists.append(event_tuple)
            variablesCombinerLevel.append(ftCategory.variableName)
            categoriesCombination.append(ftCategory.code)
        else:
            B2FATAL(f"Flavor Tagger:  {category} has been already given")

    for code in sorted(categoriesCombination):
        categoriesCombinationCode = categoriesCombinationCode + f'{int(code):02}'

    # Create default ROE-mask
    if maskName == 'FTDefaultMask':
        FTDefaultMask = (
            'FTDefaultMask',
            'thetaInCDCAcceptance and dr<1 and abs(dz)<3',
            'thetaInCDCAcceptance and clusterNHits>1.5 and \
            [[E>0.08 and clusterReg==1] or [E>0.03 and clusterReg==2] or [E>0.06 and clusterReg==3]]'
        )
        for name in particleLists:
            ma.appendROEMasks(list_name=name, mask_tuples=[FTDefaultMask], path=path)

    # Start ROE-routine
    roe_path = basf2.create_path()
    deadEndPath = basf2.create_path()

    # If trigger returns 1 jump into empty path skipping further modules in roe_path
    # run filter with no cut first to get rid of ROEs that are missing the mask of the signal particle
    ma.signalSideParticleListsFilter(particleLists, f'nROE_Charged({maskName}, 0) > 0', roe_path, deadEndPath)

    # Initialization of flavorTaggerInfo dataObject needs to be done in the main path
    flavorTaggerInfoBuilder = basf2.register_module('FlavorTaggerInfoBuilder')
    path.add_module(flavorTaggerInfoBuilder)

    # fill particle lists
    fill_particle_lists(maskName, categories, roe_path)

    if event_level(
        weightFiles=weightFiles,
        categories=categories,
        files_dir=files_dir,
        useOnlyLocalFlag=useOnlyLocalWeightFiles,
        downloadFlag=downloadFromDatabaseIfNotFound,
        exp_type=exp_type,
        signal_fraction=signal_fraction,
        path=roe_path
    ):

        combiner_level(
            weightFiles=weightFiles,
            categories=categories,
            variablesCombinerLevel=variablesCombinerLevel,
            categoriesCombinationCode=categoriesCombinationCode,
            TMVAfbdt=TMVAfbdt,
            FANNmlp=FANNmlp,
            downloadFlag=downloadFromDatabaseIfNotFound,
            useOnlyLocalFlag=useOnlyLocalWeightFiles,
            signal_fraction=signal_fraction,
            filesDirectory=files_dir,
            path=roe_path
        )

        flavorTaggerInfoFiller = basf2.register_module('FlavorTaggerInfoFiller')
        flavorTaggerInfoFiller.param('trackLevelParticleLists', trackLevelParticleLists)
        flavorTaggerInfoFiller.param('eventLevelParticleLists', eventLevelParticleLists)
        flavorTaggerInfoFiller.param('TMVAfbdt', TMVAfbdt)
        flavorTaggerInfoFiller.param('FANNmlp', FANNmlp)
        flavorTaggerInfoFiller.param('qpCategories', saveCategoriesInfo)
        flavorTaggerInfoFiller.param('istrueCategories', saveCategoriesInfo)
        flavorTaggerInfoFiller.param('targetProb', False)
        flavorTaggerInfoFiller.param('trackPointers', False)
        roe_path.add_module(flavorTaggerInfoFiller)  # Add FlavorTag Info filler to roe_path
        set_output_vars()

        if useGNN:
            ma.rankByHighest('pi+:inRoe', 'p', numBest=0, allowMultiRank=False,
                             outputVariable='FT_p_rank', overwriteRank=True,
                             path=roe_path)
            ma.fillParticleListFromDummy('vpho:dummy', path=roe_path)

            if isinstance(identifierGNN, str):
                roe_path.add_module(
                    'MVAExpert',
                    listNames='vpho:dummy',
                    extraInfoName='qrGNN_raw',  # the range of qrGNN_raw is [0,1]
                    identifier=identifierGNN
                )
                ma.variableToSignalSideExtraInfo(
                    'vpho:dummy',
                    {'extraInfo(qrGNN_raw)*2-1': 'qrGNN'},
                    path=roe_path
                )
            elif isinstance(identifierGNN, list):
                identifierGNN = list(set(identifierGNN))

                extraInfoNames = [f'qrGNN_{i_id}' for i_id in identifierGNN]
                roe_path.add_module(
                    'MVAMultipleExperts',
                    listNames='vpho:dummy',
                    extraInfoNames=extraInfoNames,
                    identifiers=identifierGNN
                )

                extraInfoDict = {}
                for extraInfoName in extraInfoNames:
                    extraInfoDict[f'extraInfo({extraInfoName})*2-1'] = extraInfoName
                    va.addAlias(extraInfoName, f'extraInfo({extraInfoName})')

                ma.variableToSignalSideExtraInfo(
                    'vpho:dummy',
                    extraInfoDict,
                    path=roe_path
                )

    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
