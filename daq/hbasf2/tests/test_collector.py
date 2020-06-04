from zmq_daq import test_support
from unittest import main


class CollectorTestCase(test_support.BaseCollectorTestCase):
    """Test case"""
    #: this is not a final collector test case
    final_collector = False


class FinalCollectorTestCase(test_support.BaseCollectorTestCase):
    """Test case"""
    #: this is a final collector test case
    final_collector = True


if __name__ == '__main__':
    main()
