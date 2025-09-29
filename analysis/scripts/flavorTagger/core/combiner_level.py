# this script applies the combiner level classifier

from flavorTagger.utils import get_Belle_or_Belle2
import os


def combiner_level(weightFiles='B2JpsiKs_mu', categories=None,
                   variablesCombinerLevel=None, categoriesCombinationCode=None,
                   TMVAfbdt=False, FANNmlp=False, downloadFlag=False,
                   useOnlyLocalFlag=False, signal_fraction=-2,
                   filesDirectory="./FlavorTagging/TrainedMethods", path=None):
    """
    Samples the input data or tests the combiner according to the selected categories.
    """

    # imports
    import basf2
    from basf2 import B2INFO, B2FATAL
    import basf2_mva

    # verbose
    B2INFO('COMBINER LEVEL')

    B2INFO("Flavor Tagger: Required Combiner for Categories:")
    for category in categories:
        B2INFO(category)

    B2INFO(f"Flavor Tagger: which corresponds to a weight file with categories\
            combination code {categoriesCombinationCode}")

    # initialise configuration variables
    if variablesCombinerLevel is None:
        variablesCombinerLevel = []
    exp_type = get_Belle_or_Belle2()

    combiner_method_prefix = f"FlavorTagger_{exp_type}_{weightFiles}Combiner{categoriesCombinationCode}"

    # Check if weight files are ready
    if TMVAfbdt:
        identifierFBDT = f"{combiner_method_prefix}FBDT"
        if downloadFlag or useOnlyLocalFlag:
            identifierFBDT = f"{filesDirectory}/{combiner_method_prefix}FBDT_1.root"

        if downloadFlag:
            if not os.path.isfile(identifierFBDT):
                basf2_mva.download(f"{combiner_method_prefix}FBDT", identifierFBDT)
                if not os.path.isfile(identifierFBDT):
                    B2FATAL(f"Flavor Tagger: Weight file {identifierFBDT} was\
                             not downloaded from Database. Please check the\
                             buildOrRevision name. Stopped")

        if useOnlyLocalFlag:
            if not os.path.isfile(identifierFBDT):
                B2FATAL(f"flavorTagger: Combinerlevel FastBDT was not trained\
                         with this combination of categories. Weight file\
                         {identifierFBDT} not found. Stopped")

        B2INFO(f"flavorTagger: Ready to be used with weightFile\
                {combiner_method_prefix}FBDT_1.root")

    if FANNmlp:
        identifierFANN = f"{combiner_method_prefix}FANN"
        if downloadFlag or useOnlyLocalFlag:
            identifierFANN = f"{filesDirectory}/{combiner_method_prefix}FANN_1.root"

        if downloadFlag:
            if not os.path.isfile(identifierFANN):
                basf2_mva.download(f"{combiner_method_prefix}FANN", identifierFANN)
                if not os.path.isfile(identifierFANN):
                    B2FATAL(f"Flavor Tagger: Weight file {identifierFANN} was\
                             not downloaded from Database. Please check the\
                             build or revision name name. Stopped")
        if useOnlyLocalFlag:
            if not os.path.isfile(identifierFANN):
                B2FATAL(f"flavorTagger: Combinerlevel FANNMLP was not trained\
                         with this combination of categories. Weight file\
                         {identifierFANN} not found. Stopped")

        B2INFO(f"flavorTagger: Ready to be used with weightFile {combiner_method_prefix}FANN_1.root")

    # At this stage, all necessary weight files should be ready.
    # Call MVAExpert or MVAMultipleExperts module.
    if TMVAfbdt and not FANNmlp:
        B2INFO(f"flavorTagger: Apply FBDTMethod {combiner_method_prefix}FBDT")
        path.add_module(
            'MVAExpert',
            listNames=[],
            extraInfoName='qrCombinedFBDT',
            signalFraction=signal_fraction,
            identifier=identifierFBDT
        )

    if FANNmlp and not TMVAfbdt:
        B2INFO('flavorTagger: Apply FANNMethod on combiner level')
        path.add_module(
            'MVAExpert',
            listNames=[],
            extraInfoName='qrCombinedFANN',
            signalFraction=signal_fraction,
            entifier=identifierFANN
        )

    if FANNmlp and TMVAfbdt:
        B2INFO('flavorTagger: Apply FANNMethod and FBDTMethod  on combiner level')
        mvaMultipleExperts = basf2.register_module('MVAMultipleExperts')
        mvaMultipleExperts.set_name('MVAMultipleExperts_Combiners')
        mvaMultipleExperts.param('listNames', [])
        mvaMultipleExperts.param('extraInfoNames', ['qrCombinedFBDT', 'qrCombinedFANN'])
        mvaMultipleExperts.param('signalFraction', signal_fraction)
        mvaMultipleExperts.param('identifiers', [identifierFBDT, identifierFANN])
        path.add_module(mvaMultipleExperts)
