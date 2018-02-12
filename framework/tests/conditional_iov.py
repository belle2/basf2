import basf2
from iov_conditional import phase_2_conditional, make_conditional_at


class PrinterModule(basf2.Module):
    """Print the given string on each event"""

    def __init__(self, print_string):
        """Remember the given string to print later"""
        super().__init__()
        #: The string we want to print each event
        self._print_string = print_string

    def event(self):
        """And now would be a good time for printing ..."""
        print(self._print_string)


def simulate_run(run_numbers, exp_numbers):
    assert len(exp_numbers) == len(run_numbers)

    path = basf2.create_path()

    path.add_module("EventInfoSetter", evtNumList=[1] * len(run_numbers), runList=run_numbers, expList=exp_numbers)
    path.add_module("EventInfoPrinter")

    # Phase 2 path
    phase2_path = basf2.create_path()
    phase2_path.add_module(PrinterModule("Phase 2 is here"))

    # Phase 3 path
    phase3_path = basf2.create_path()
    phase3_path.add_module(PrinterModule("Phase 3 is here"))

    # No Phase 3 path
    no_phase3_path = basf2.create_path()
    no_phase3_path.add_module(PrinterModule("Phase 3 is not here"))

    # Weird path
    weird_path = basf2.create_path()
    weird_path.add_module(PrinterModule("Strange condition"))

    # Condition for phase2
    phase_2_conditional(path, path_when_in_iov=phase2_path)

    # Condition for phase3
    make_conditional_at(path, iov_list=[(0, 0, 0, -1)], path_when_in_iov=phase3_path, path_when_not_in_iov=no_phase3_path)

    # Some weird condition
    make_conditional_at(path, iov_list=[(42, 42, 47, 47)], path_when_in_iov=weird_path)

    # Finished path
    path.add_module(PrinterModule("Finish"))

    basf2.process(path)


if __name__ == "__main__":
    basf2.set_random_seed(1)
    simulate_run(exp_numbers=[0, 1002], run_numbers=[1, 67])
    simulate_run(exp_numbers=[42, 43, 47], run_numbers=[41, 1, 48])
