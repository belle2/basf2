from variables import variables

variables.printList()

var = variables.getVariable('M')
assert 'M' == var.name
print var.description

#used in FEI
import ROOT
assert 'extraInfoSignalProbability' == ROOT.Belle2.Variable.makeROOTCompatible('extraInfo(SignalProbability)')
