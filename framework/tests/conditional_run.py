import basf2


class PrinterModule(basf2.Module):
    def __init__(self, print_string):
        super().__init__()
        self._print_string = print_string

    def event(self):
        print(self._print_string)


def simulate_run(run_number):
    path = basf2.create_path()

    path.add_module("EventInfoSetter", runList=[run_number], evtNumList=[1])
    path.add_module("EventInfoPrinter")

    # Phase 2 path
    phase2_path = basf2.create_path()
    phase2_path.add_module(PrinterModule("Phase 2 is here"))

    # Phase 3 path
    phase3_path = basf2.create_path()
    phase3_path.add_module(PrinterModule("Phase 3 is here"))

    # Condition for phase2
    condition = path.add_module("RunDependentCondition", trueOnRun=1002)
    condition.if_true(phase2_path, basf2.AfterConditionPath.END)

    # Condition for phase3
    condition = path.add_module("RunDependentCondition", trueOnRun=1)
    condition.if_true(phase3_path, basf2.AfterConditionPath.END)

    # None at all
    path.add_module(PrinterModule("No condition met"))

    basf2.process(path)


if __name__ == "__main__":
    basf2.set_random_seed(1)
    simulate_run(1)
    simulate_run(1002)
    simulate_run(4242)
