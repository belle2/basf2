import variables as v

#some variables should be there
assert len(v.variables.getVariables()) > 0

v.printVars()

var = v.variables.getVariable('M')
assert 'M' == var.name
print var.description

# used in FEI
import ROOT
assert 'extraInfoSignalProbability' == ROOT.Belle2.Variable.makeROOTCompatible('extraInfo(SignalProbability)')
