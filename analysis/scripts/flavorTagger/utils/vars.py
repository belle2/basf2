# this script sets the aliases for different variable sets

# relative imports
from .categories import get_available_categories
from .helpers import read_yaml


def set_output_vars():
    """
    Sets the aliases for FT output vars
    """

    # imports
    import basf2
    from variables import variables as va
    import variables.utils as vu

    # aliases for FT output vars
    va.addAlias('FBDT_qrCombined', 'qrOutput(FBDT)')
    va.addAlias('FANN_qrCombined', 'qrOutput(FANN)')
    va.addAlias('qrGNN', 'extraInfo(qrGNN)')

    # alias for FT target var
    va.addAlias('qrMC', 'isRelatedRestOfEventB0Flavor')

    # aliases for FT category info vars
    for category in get_available_categories():
        aliasForQp = f'qp{category}'
        aliasForTrueTarget = f'hasTrueTarget{category}'
        aliasForIsRightCategory = f'isRightCategory{category}'
        va.addAlias(aliasForQp, f'qpCategory({category})')
        va.addAlias(aliasForTrueTarget, f'hasTrueTargets({category})')
        va.addAlias(aliasForIsRightCategory, f'isTrueFTCategory({category})')

    # get output vars list
    filepath = basf2.find_file('data/analysis/output_vars.yaml')
    flavor_tagging_vars = read_yaml(filepath)["output_vars"]

    # add output vars to flavor tagger collection
    vu.add_collection(flavor_tagging_vars, 'flavor_tagging')


def set_FT_pid_aliases(type="Current", exp_type="Belle2"):
    """
    This function adds the pid aliases needed by the flavor tagger.
    """
    # imports
    import basf2
    from variables import variables as va

    # define a function to set alias
    def set_alias(alias, var): return va.addAlias(alias, var)

    # get list of aliases
    filepath = basf2.find_file('data/analysis/pid_alias.yaml')
    alias_list = read_yaml(filepath)[type]

    for alias, var in alias_list["Common"].items():
        set_alias(alias, var)   # common alias
    for alias, var in alias_list[exp_type].items():
        set_alias(alias, var)   # exp specific alias


def set_GFlat_aliases(categories, usePIDNN):
    """
    This function adds aliases for the GNN-based flavor tagger.
    """
    # imports
    from variables import variables as va

    # will be used for target variable 0:B0bar, 1:B0
    va.addAlias('qrCombined_bit', '(qrCombined+1)/2')
    alias_list = ['qrCombined_bit']

    var_dict = {
        # position
        'dx': 'dx',
        'dy': 'dy',
        'dz': 'dz',
        # mask
        'E': 'E',
        # charge,
        'charge': 'charge',
        # feature
        'px_c': 'px*charge',
        'py_c': 'py*charge',
        'pz_c': 'pz*charge',
        'electronID_c': 'electronIDNN*charge' if usePIDNN else 'electronID*charge',
        'muonID_c': 'muonIDNN*charge' if usePIDNN else 'muonID*charge',
        'pionID_c': 'pionIDNN*charge' if usePIDNN else 'pionID*charge',
        'kaonID_c': 'kaonIDNN*charge' if usePIDNN else 'kaonID*charge',
        'protonID_c': 'protonIDNN*charge' if usePIDNN else 'protonID*charge',
        'deuteronID_c': 'deuteronIDNN*charge' if usePIDNN else 'deuteronID*charge',
        'electronID_noSVD_noTOP_c': 'electronID_noSVD_noTOP*charge',
    }

    # 16 charged particles are used at most
    for rank in range(1, 17):

        for cat in categories:
            listName = get_available_categories()[cat].particleList
            varName = f'QpTrack({listName}, isRightCategory({cat}), isRightCategory({cat}))'

            varWithRank = f'ifNANgiveX(getVariableByRank(pi+:inRoe, FT_p, {varName}, {rank}), 0)'
            aliasWithRank = f'{cat}_rank{rank}'

            va.addAlias(aliasWithRank, varWithRank)
            alias_list.append(aliasWithRank)

        for alias, var in var_dict.items():
            varWithRank = f'ifNANgiveX(getVariableByRank(pi+:inRoe, FT_p, {var}, {rank}), 0)'
            aliasWithRank = f'{alias}_rank{rank}'

            va.addAlias(aliasWithRank, varWithRank)
            alias_list.append(aliasWithRank)

    return alias_list


def set_masked_vars(mask_name="all"):
    """
    Returns variables with a certain mask
    """
    # imports
    from variables import variables as va

    masked_var_dict = {
        'pMissTag_withMask': f'pMissTag({mask_name})',
        'cosTPTO_withMask': f'cosTPTO({mask_name})',
        'ptTracksRoe_withMask': f'ptTracksRoe({mask_name})',
        'pt2TracksRoe_withMask': f'pt2TracksRoe({mask_name})',
    }

    for key, value in masked_var_dict.items():
        va.addAlias(key, value)

    return masked_var_dict


def set_and_get_aliases(var_dict):
    """
    Sets aliases from a dict of alias, var pair and returns a list of aliases.
    """
    from variables import variables as va

    for key, value in var_dict.items():
        va.addAlias(key, value)

    return list(var_dict.keys())
