# This script contains the variables used for flavor tagging

# imports
import variables as va
from categories import get_available_categories

# aliases
# aliases for FT output vars
va.variables.addAlias('FBDT_qrCombined', 'qrOutput(FBDT)')
va.variables.addAlias('FANN_qrCombined', 'qrOutput(FANN)')
va.variables.addAlias('qrGNN', 'extraInfo(qrGNN)')

# alias for FT target var
va.variables.addAlias('qrMC', 'isRelatedRestOfEventB0Flavor')

# aliases for FT category info vars
for iCategory in get_available_categories():
    aliasForQp = f'qp{iCategory}'
    aliasForTrueTarget = f'hasTrueTarget{iCategory}'
    aliasForIsRightCategory = f'isRightCategory{iCategory}'
    va.variables.addAlias(aliasForQp, f'qpCategory({iCategory})')
    va.variables.addAlias(aliasForTrueTarget, f'hasTrueTargets({iCategory})')
    va.variables.addAlias(aliasForIsRightCategory, f'isTrueFTCategory({iCategory})')


# something
# variables that are to be saved after Flavor Tagging
flavor_tagging_vars = ['FBDT_qrCombined', 'FANN_qrCombined', 'qrMC', 'mcFlavorOfOtherB', 'qrGNN',
                       'qpElectron', 'hasTrueTargetElectron', 'isRightCategoryElectron',
                       'qpIntermediateElectron', 'hasTrueTargetIntermediateElectron', 'isRightCategoryIntermediateElectron',
                       'qpMuon', 'hasTrueTargetMuon', 'isRightCategoryMuon',
                       'qpIntermediateMuon', 'hasTrueTargetIntermediateMuon', 'isRightCategoryIntermediateMuon',
                       'qpKinLepton', 'hasTrueTargetKinLepton', 'isRightCategoryKinLepton',
                       'qpIntermediateKinLepton', 'hasTrueTargetIntermediateKinLepton', 'isRightCategoryIntermediateKinLepton',
                       'qpKaon', 'hasTrueTargetKaon', 'isRightCategoryKaon',
                       'qpSlowPion', 'hasTrueTargetSlowPion', 'isRightCategorySlowPion',
                       'qpFastHadron', 'hasTrueTargetFastHadron', 'isRightCategoryFastHadron',
                       'qpLambda', 'hasTrueTargetLambda', 'isRightCategoryLambda',
                       'qpFSC', 'hasTrueTargetFSC', 'isRightCategoryFSC',
                       'qpMaximumPstar', 'hasTrueTargetMaximumPstar', 'isRightCategoryMaximumPstar',
                       'qpKaonPion', 'hasTrueTargetKaonPion', 'isRightCategoryKaonPion'
                       ]
va.utils.add_collection(flavor_tagging_vars, 'flavor_tagging')
