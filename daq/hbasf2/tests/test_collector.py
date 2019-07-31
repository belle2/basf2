from zmq_daq import test_support
from unittest import main


class CollectorTestCase(test_support.BaseCollectorTestCase):
    """Test case"""
    #: which programs are started
    needed_programs = {
        "collector": ["b2hlt_collector", "--input", f"tcp://*:{test_support.BaseCollectorTestCase.input_port}",
                      "--output", f"tcp://*:{test_support.BaseCollectorTestCase.output_port}",
                      "--monitor", f"tcp://*:{test_support.BaseCollectorTestCase.monitoring_port}"]}
    #: this is not a final collector test case
    final_collector = False


class FinalCollectorTestCase(test_support.BaseCollectorTestCase):
    """Test case"""
    #: which programs are started
    needed_programs = {
        "collector": ["b2hlt_finalcollector", "--input", f"tcp://*:{test_support.BaseCollectorTestCase.input_port}",
                      "--output", f"tcp://localhost:{test_support.BaseCollectorTestCase.output_port}",
                      "--monitor", f"tcp://*:{test_support.BaseCollectorTestCase.monitoring_port}"]}
    #: this is a final collector test case
    final_collector = True


if __name__ == '__main__':
    main()
