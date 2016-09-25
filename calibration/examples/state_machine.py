import ROOT
from ROOT.Belle2 import TestCalibrationAlgorithm as TestAlgo
from caf.state_machines import CalibrationMachine, MachineError, ConditionError
from caf.framework import Calibration

alg = TestAlgo()
cal1 = Calibration('TestCalibrationClass1', "CaTest", alg, '/path/to/file.root')
cal2 = Calibration('TestCalibrationClass2', "CaTest", alg, '/path/to/file.root')

cal2.depends_on(cal1)


def print_msg_before(**kwargs):
    print(kwargs["msg_before"])


def print_msg_after(**kwargs):
    print(kwargs["msg_after"])

cm1 = CalibrationMachine(cal1)
cm1.transitions["submit_collector"][0]["before"] = [print_msg_before]
cm1.transitions["submit_collector"][0]["after"] = [print_msg_after]
cm2 = CalibrationMachine(cal2)
# cm1.save_graph("graph.dot", "CalibrationMachine")

print("Currently in", cm1.state, "state.")
cm1.submit_collector(msg_before="About to submit", msg_after="Now submitted")
print("Currently in", cm1.state, "state.")
