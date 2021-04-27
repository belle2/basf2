from softwaretrigger import constants
from softwaretrigger.test_support import test_folder


if __name__ == "__main__":
    test_folder(location=constants.Location.expressreco,
                run_type=constants.RunTypes.beam,
                exp_number=1003,
                phase="phase3",
                passthrough=True)
