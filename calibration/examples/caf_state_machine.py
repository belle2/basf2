from basf2 import *
import ROOT
from ROOT.Belle2 import TestCalibrationAlgorithm as TestAlgo
from caf.state_machines import CalibrationMachine
from caf.framework import Calibration

alg = TestAlgo()
cal1 = Calibration('TestCalibrationClass1', "CaTest", alg, '/path/to/file.root')
cal2 = Calibration('TestCalibrationClass2', "CaTest", alg, '/path/to/file.root')

cm1 = CalibrationMachine(cal1)
cm1.save_graph("calibration_graph.dot", "CalibrationMachine")

B2INFO("Currently in "+cm1.state.name+" state.")
cm1.submit_collector(msg_before="About to submit", msg_after="Now submitted")
cm1.complete(msg_before="Finishing", msg_after="Collector finished")
B2INFO("Currently in "+cm1.state.name+" state.")
