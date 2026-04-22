# this script applies the event level classifier

from flavorTagger.utils import get_available_categories
import os


def event_level(weightFiles='B2JpsiKs_mu', categories=None, files_dir="",
                downloadFlag=False, useOnlyLocalFlag=False, signal_fraction=-2,
                exp_type="Belle2", path=None):
    """
    Samples data for training or tests all categories all categories at event level.
    """

    # imports
    import basf2
    from basf2 import create_path, register_module
    from basf2 import B2INFO, B2FATAL
    import basf2_mva

    # verbose
    B2INFO('EVENT LEVEL')

    # initialise configuration variables
    ready_methods = 0
    all_categories = get_available_categories()

    # Each category has its own Path in order to be skipped if the
    # corresponding particle list is empty
    identifiers_extraInfo_dict = dict()
    identifiers_extraInfo_KaonPion = []

    # process each category
    for category in categories:
        particleList = all_categories[category].particleList

        # initialise names
        event_method_prefix = (f"FlavorTagger_{exp_type}_{weightFiles}"
                               f"EventLevel{category}FBDT")
        event_identifier = event_method_prefix
        target_variable = f"isRightCategory({category})"
        extraInfo_name = target_variable

        # setup flag specific configuration
        if downloadFlag or useOnlyLocalFlag:
            event_identifier = f"{files_dir}/{event_method_prefix}_1.root"

        if downloadFlag:
            if not os.path.isfile(event_identifier):
                basf2_mva.download(event_method_prefix, event_identifier)
                if not os.path.isfile(event_identifier):
                    B2FATAL(f"Flavor Tagger: Weight file {event_identifier}"
                            "was not downloaded from database. Please check the"
                            " build or revision name. Stopped.")

        if useOnlyLocalFlag:
            if not os.path.isfile(event_identifier):
                B2FATAL(f"Flavor Tagger: {particleList} event level was not "
                        "trained. Weight file {event_identifier} was not "
                        "found. Stopped.")

        # verbose
        B2INFO(f"Flavor Tagger: MVAExpert {event_method_prefix} ready.")

        # start application
        B2INFO(f"Flavor Tagger: Applying MVAExpert {event_method_prefix}.")

        if category == 'KaonPion':
            identifiers_extraInfo_KaonPion.append((extraInfo_name, event_identifier))
        elif particleList not in identifiers_extraInfo_dict:
            identifiers_extraInfo_dict[particleList] = [(extraInfo_name, event_identifier)]
        else:
            identifiers_extraInfo_dict[particleList].append((extraInfo_name, event_identifier))

        ready_methods += 1

    # Each category has its own Path in order to be skipped if the corresponding particle list is empty
    for particleList in identifiers_extraInfo_dict:
        eventLevelPath = create_path()
        SkipEmptyParticleList = register_module("SkimFilter")
        SkipEmptyParticleList.set_name('SkimFilter_EventLevel_' + particleList)
        SkipEmptyParticleList.param('particleLists', particleList)
        SkipEmptyParticleList.if_true(eventLevelPath, basf2.AfterConditionPath.CONTINUE)
        path.add_module(SkipEmptyParticleList)

        mvaMultipleExperts = register_module('MVAMultipleExperts')
        mvaMultipleExperts.set_name('MVAMultipleExperts_EventLevel_' + particleList)
        mvaMultipleExperts.param('listNames', [particleList])
        mvaMultipleExperts.param('extraInfoNames', [row[0] for row in identifiers_extraInfo_dict[particleList]])
        mvaMultipleExperts.param('signalFraction', signal_fraction)
        mvaMultipleExperts.param('identifiers', [row[1] for row in identifiers_extraInfo_dict[particleList]])
        eventLevelPath.add_module(mvaMultipleExperts)

    if 'KaonPion' in categories and len(identifiers_extraInfo_KaonPion) != 0:
        eventLevelKaonPionPath = create_path()
        SkipEmptyParticleList = register_module("SkimFilter")
        SkipEmptyParticleList.set_name('SkimFilter_' + 'K+:inRoe')
        SkipEmptyParticleList.param('particleLists', 'K+:inRoe')
        SkipEmptyParticleList.if_true(eventLevelKaonPionPath, basf2.AfterConditionPath.CONTINUE)
        path.add_module(SkipEmptyParticleList)

        mvaExpertKaonPion = register_module("MVAExpert")
        mvaExpertKaonPion.set_name('MVAExpert_KaonPion_' + 'K+:inRoe')
        mvaExpertKaonPion.param('listNames', ['K+:inRoe'])
        mvaExpertKaonPion.param('extraInfoName', identifiers_extraInfo_KaonPion[0][0])
        mvaExpertKaonPion.param('signalFraction', signal_fraction)
        mvaExpertKaonPion.param('identifier', identifiers_extraInfo_KaonPion[0][1])

        eventLevelKaonPionPath.add_module(mvaExpertKaonPion)

    if ready_methods != len(categories):
        return False
    else:
        return True
