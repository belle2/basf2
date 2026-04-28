# This script contains a list of categories available for event level training

def get_available_categories():
    """
    Gets all available categories for eventlevel FT training.
    Also returns a named tuple corresponding to each category as
    ['particleList', 'trackName', 'eventName', 'variableName', 'code']
    """
    # imports
    import collections

    # create a tuple to store category related info
    FTCategoryParameters = collections.namedtuple('FTCategoryParameters',
                                                  ['particleList', 'trackName', 'eventName', 'variableName', 'code'])

    available_categories = {
        'Electron': FTCategoryParameters(
            'e+:inRoe',
            'Electron',
            'Electron',
            'QpOf(e+:inRoe, isRightCategory(Electron), isRightCategory(Electron))',
            0
        ),
        'IntermediateElectron': FTCategoryParameters(
            'e+:inRoe',
            'IntermediateElectron',
            'IntermediateElectron',
            'QpOf(e+:inRoe, isRightCategory(IntermediateElectron), isRightCategory(IntermediateElectron))',
            1
        ),
        'Muon': FTCategoryParameters(
            'mu+:inRoe',
            'Muon',
            'Muon',
            'QpOf(mu+:inRoe, isRightCategory(Muon), isRightCategory(Muon))',
            2
        ),
        'IntermediateMuon': FTCategoryParameters(
            'mu+:inRoe',
            'IntermediateMuon',
            'IntermediateMuon',
            'QpOf(mu+:inRoe, isRightCategory(IntermediateMuon), isRightCategory(IntermediateMuon))',
            3
        ),
        'KinLepton': FTCategoryParameters(
            'mu+:inRoe',
            'KinLepton',
            'KinLepton',
            'QpOf(mu+:inRoe, isRightCategory(KinLepton), isRightCategory(KinLepton))',
            4
        ),
        'IntermediateKinLepton': FTCategoryParameters(
            'mu+:inRoe',
            'IntermediateKinLepton',
            'IntermediateKinLepton',
            'QpOf(mu+:inRoe, isRightCategory(IntermediateKinLepton), isRightCategory(IntermediateKinLepton))',
            5
        ),
        'Kaon': FTCategoryParameters(
            'K+:inRoe',
            'Kaon',
            'Kaon',
            'weightedQpOf(K+:inRoe, isRightCategory(Kaon), isRightCategory(Kaon))',
            6
        ),
        'SlowPion': FTCategoryParameters(
            'pi+:inRoe',
            'SlowPion',
            'SlowPion',
            'QpOf(pi+:inRoe, isRightCategory(SlowPion), isRightCategory(SlowPion))',
            7
        ),
        'FastHadron': FTCategoryParameters(
            'pi+:inRoe',
            'FastHadron',
            'FastHadron',
            'QpOf(pi+:inRoe, isRightCategory(FastHadron), isRightCategory(FastHadron))',
            8
        ),
        'Lambda': FTCategoryParameters(
            'Lambda0:inRoe',
            'Lambda',
            'Lambda',
            'weightedQpOf(Lambda0:inRoe, isRightCategory(Lambda), isRightCategory(Lambda))',
            9
        ),
        'FSC': FTCategoryParameters(
            'pi+:inRoe',
            'SlowPion',
            'FSC',
            'QpOf(pi+:inRoe, isRightCategory(FSC), isRightCategory(SlowPion))',
            10
        ),
        'MaximumPstar': FTCategoryParameters(
            'pi+:inRoe',
            'MaximumPstar',
            'MaximumPstar',
            'QpOf(pi+:inRoe, isRightCategory(MaximumPstar), isRightCategory(MaximumPstar))',
            11
        ),
        'KaonPion': FTCategoryParameters(
            'K+:inRoe',
            'Kaon',
            'KaonPion',
            'QpOf(K+:inRoe, isRightCategory(KaonPion), isRightCategory(Kaon))',
            12
        ),
    }

    return available_categories
