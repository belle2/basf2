import ROOT
from ROOT.Belle2 import TestCalibrationAlgorithm as TestAlgo
from caf.state_machines import CalibrationMachine
from caf.framework import Calibration

alg = TestAlgo()
cal = Calibration('TestCalibrationClass2', "CaTest", alg, '/path/to/file.root')

c = CalibrationMachine(cal)
c.save_graph("graph.dot", "CalibrationMachine")
