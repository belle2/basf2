from caf.state_machines import CalibrationMachine


class FakeCal():
    pass

cal = FakeCal()

c = CalibrationMachine(cal)
c.save_graph("graph.dot", "CalibrationMachine")
