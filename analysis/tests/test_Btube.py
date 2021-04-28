import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu
import vertex as vx
import stdCharged as stdc
import b2test_utils

from variables import variables

variables.addAlias('TubeX', 'extraInfo(TubePosX)')
variables.addAlias('TubeY', 'extraInfo(TubePosY)')
variables.addAlias('TubeZ', 'extraInfo(TubePosZ)')
variables.addAlias('Tube00', 'extraInfo(TubeCov00)')
variables.addAlias('Tube01', 'extraInfo(TubeCov01)')
variables.addAlias('Tube02', 'extraInfo(TubeCov02)')
variables.addAlias('Tube10', 'extraInfo(TubeCov10)')
variables.addAlias('Tube11', 'extraInfo(TubeCov11)')
variables.addAlias('Tube12', 'extraInfo(TubeCov12)')
variables.addAlias('Tube20', 'extraInfo(TubeCov20)')
variables.addAlias('Tube21', 'extraInfo(TubeCov21)')
variables.addAlias('Tube22', 'extraInfo(TubeCov22)')

variables.addAlias('avf00', 'extraInfo(prod_vtx_cov00)')
variables.addAlias('avf01', 'extraInfo(prod_vtx_cov01)')
variables.addAlias('avf02', 'extraInfo(prod_vtx_cov02)')
variables.addAlias('avf10', 'extraInfo(prod_vtx_cov10)')
variables.addAlias('avf11', 'extraInfo(prod_vtx_cov11)')
variables.addAlias('avf12', 'extraInfo(prod_vtx_cov12)')
variables.addAlias('avf20', 'extraInfo(prod_vtx_cov20)')
variables.addAlias('avf21', 'extraInfo(prod_vtx_cov21)')
variables.addAlias('avf22', 'extraInfo(prod_vtx_cov22)')

variables.addAlias('avfPx', 'extraInfo(Px_after_avf)')
variables.addAlias('avfPy', 'extraInfo(Py_after_avf)')
variables.addAlias('avfPz', 'extraInfo(Pz_after_avf)')
variables.addAlias('avfE', 'extraInfo(E_after_avf)')

variables.addAlias('mcPxCms', 'useCMSFrame(mcPX)')
variables.addAlias('mcPyCms', 'useCMSFrame(mcPY)')
variables.addAlias('mcPzCms', 'useCMSFrame(mcPZ)')
variables.addAlias('mcECms', 'useCMSFrame(mcE)')

variables.addAlias('tube_dir_X', 'extraInfo(TubeDirX)')
variables.addAlias('tube_dir_Y', 'extraInfo(TubeDirY)')
variables.addAlias('tube_dir_Z', 'extraInfo(TubeDirZ)')
variables.addAlias('tube_B_aparent_Momentum_from_creatorB', 'extraInfo(TubeB_p_estimated)')

variables.addAlias('truex', 'matchedMC(x)')
variables.addAlias('truey', 'matchedMC(y)')
variables.addAlias('truez', 'matchedMC(z)')
# create path
my_path = b2.create_path()

# load input ROOT file

ma.inputMdst('default',
             b2test_utils.require_file('analysis/tests/mdst.root'),
             path=my_path)

stdc.stdPi(listtype='all', path=my_path)
stdc.stdPi(listtype='good', path=my_path)
stdc.stdK(listtype='all', path=my_path)
stdc.stdK(listtype='good', path=my_path)
stdc.stdMu(listtype='all', path=my_path)
stdc.stdMu(listtype='good', path=my_path)

ma.reconstructDecay('D0:kpi -> K-:good pi+:good', '', path=my_path)
ma.matchMCTruth('D0:kpi', path=my_path)
ma.applyCuts('D0:kpi', 'abs(mcPDG)==421', path=my_path)
vx.raveFit('D0:kpi', 0.0, path=my_path)

ma.reconstructDecay('J/psi:mumu -> mu+:good mu-:good', '', path=my_path)
vx.raveFit('J/psi:mumu', 0.0, path=my_path)

# TAGGING
ma.reconstructDecay('B+:tag -> anti-D0:kpi pi+:all', '', path=my_path)
ma.matchMCTruth('B+:tag', path=my_path)
ma.applyCuts('B+:tag', 'isSignalAcceptMissingNeutrino>0.5', path=my_path)
vx.raveFit('B+:tag', 0.0, path=my_path)

# SIGNAL
ma.reconstructDecay('B-:sigT -> J/psi:mumu K-:good pi+:good pi-:good', '', path=my_path)
ma.matchMCTruth('B-:sigT', path=my_path)
ma.applyCuts('B-:sigT', 'abs(mcPDG)==521', path=my_path)

ma.reconstructDecay('Upsilon(4S):sig -> B+:tag B-:sigT', '', path=my_path)

mytestmodule2 = ma.register_module('BtubeCreator')
# select the daughter which will be used as reference to create Btube.
# Order of daughters should be identical to decay string used in
# reconstructDecay
my_path.add_module(mytestmodule2, listName='Upsilon(4S):sig', decayString='Upsilon(4S):sig -> ^B+:tag B-:sigT')
vx.raveFit('B-:sigT', 0.0, constraint='btube', path=my_path)
# Saving variables to ntuple
output_file = 'test.root'
true_vars = ['truex', 'truey', 'truez']
avf_vars = ['avf00', 'avf01', 'avf02', 'avf10', 'avf11', 'avf12', 'avf20', 'avf21', 'avf22', 'avfPx', 'avfPy', 'avfPz', 'avfE']
tube_vars = [
    'TubeX',
    'TubeY',
    'TubeZ',
    'Tube00',
    'Tube01',
    'Tube02',
    'Tube10',
    'Tube11',
    'Tube12',
    'Tube20',
    'Tube21',
    'Tube22',
    'tube_dir_X',
    'tube_dir_Y',
    'tube_dir_Z',
    'tube_B_aparent_Momentum_from_creatorB']

common_vars = vc.kinematics + vc.vertex + vc.mc_vertex + vc.mc_truth + \
    vc.mc_kinematics + ['InvM', 'mcPxCms', 'mcPyCms', 'mcPzCms', 'mcECms']
D_vars_tagside = vu.create_aliases_for_selected(
    list_of_variables=common_vars,
    decay_string='B+:tag -> ^anti-D0:kpi pi+:all',
    prefix='D_tagside')
U4S_vars_sig = vu.create_aliases_for_selected(
    list_of_variables=common_vars + tube_vars + true_vars,
    decay_string='Upsilon(4S):sig -> B+:tag ^B-:sigT',
    prefix='sigB')
U4S_vars_tag = vu.create_aliases_for_selected(
    list_of_variables=common_vars + avf_vars + true_vars,
    decay_string='Upsilon(4S):sig -> ^B+:tag B-:sigT',
    prefix='tagB')
U4S_vars = common_vars + U4S_vars_tag + U4S_vars_sig + D_vars_tagside
ma.variablesToNtuple('Upsilon(4S):sig', U4S_vars,
                     filename=output_file, treename='u4stree', path=my_path)
ma.variablesToNtuple('B+:tag', common_vars,
                     filename=output_file, treename='tagtree', path=my_path)

with b2test_utils.clean_working_directory():
    b2test_utils.safe_process(my_path)
