# this script applies the combiner level classifier

from flavorTagger.utils import get_Belle_or_Belle2
import os


def combiner_level(weightFiles='B2JpsiKs_mu', categories=None,
                   variablesCombinerLevel=None, categoriesCombinationCode=None,
                   TMVAfbdt=False, downloadFlag=False,
                   useOnlyLocalFlag=False, signal_fraction=-2,
                   filesDirectory="./FlavorTagging/TrainedMethods", path=None):
    """
    Samples the input data or tests the combiner according to the selected categories.
    """

    # imports
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

    # At this stage, all necessary weight files should be ready.
    # Call MVAExpert or MVAMultipleExperts module.
    if TMVAfbdt:
        B2INFO(f"flavorTagger: Apply FBDTMethod {combiner_method_prefix}FBDT")
        path.add_module(
            'MVAExpert',
            listNames=[],
            extraInfoName='qrCombinedFBDT',
            signalFraction=signal_fraction,
            identifier=identifierFBDT
        )
