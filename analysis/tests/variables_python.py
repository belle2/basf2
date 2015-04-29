import variables as v

# some variables should be there
assert len(v.variables.getVariables()) > 0

v.printVars()

var = v.variables.getVariable('M')
assert 'M' == var.name
print var.description

v.variables.addAlias('sigProb', 'extraInfo(SignalProbability)')
var = v.variables.getVariable('sigProb')
assert 'extraInfo(SignalProbability)' == var.name

# used in FEI
import ROOT
assert 'extraInfoSignalProbability' == ROOT.Belle2.Variable.makeROOTCompatible('extraInfo(SignalProbability)')
