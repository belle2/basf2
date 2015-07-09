from multiprocessing import Queue
from Queue import Empty


class Basf2CalculationQueue():

    """
    This class is a wrapper around a multiprocessing.Queue

    It can be used to send and receive values from the modules while processing the path.
    You can use it to save - for example - filepaths of outputfiles that you create on the fly.
    The added items are all of the type _Basf2CalculationQueueItem.
    The _Basf2CalculationQueue can be used as a dict. After the termination of the underlaying process
    you can access the different entries by their names you gave them when putting them on the queue.
    """

    def __init__(self):
        self.queue = Queue()
        self.results = dict()

    def put(self, name, item, **kwargs):
        """
        Put an item on the queue with the given name. Please keep that adding two items with the same name
        overrides one of them!
        """
        self.queue.put(Basf2CalculationQueueItem(name, item), **kwargs)

    def fill_results(self):
        """
        Fill the internal dict with the information of the queue.
        Do not call this but rather use the functions from _Basf2Calculation.
        Do only call this when the underlying process has ended.
        """
        while True:
            try:
                result = self.queue.get_nowait()
                self.results.update({result.name: result.item})
            except Empty:
                return

    def get(self, name):
        """
        Return the item with the given name or an Exception when it is not found.
        Do not call this but rather use the functions from _Basf2Calculation.
        """
        self.fill_results()
        return self.results[name]

    def get_keys(self):
        """
        Return all possible names of items saved in this queue.
        Do not call this but rather use the functions from _Basf2Calculation.
        """
        self.fill_results()
        return self.results.keys()


class Basf2CalculationQueueItem():

    """
    A placeholder for a tuple string, object.
    Do not create them by yourself.
    """

    def __init__(self, name, item):
        self.name = name
        self.item = item


class Basf2CalculationQueueStatistics():

    """
    As the basf2 statistics is not pickable, wa can not store it into the result_queue.
    So we write a wrapper which unpacks the needed properties.
    """

    def __init__(self):
        self.module = []

    def init_with_cpp(self, statistics):
        for stats in statistics.modules:
            self.append_module_statistics(stats, statistics)

    def get_dict(self, function, statistics):
        return {
            category: function(category) for category in [
                statistics.INIT,
                statistics.BEGIN_RUN,
                statistics.EVENT,
                statistics.END_RUN,
                statistics.TERM]}

    def append_module_statistics(self, stats, statistics):
        module_stats = {
            "name": stats.name, "time": self.get_dict(
                stats.time, statistics), "calls": self.get_dict(
                stats.calls, statistics)}
        self.module.append(module_stats)
