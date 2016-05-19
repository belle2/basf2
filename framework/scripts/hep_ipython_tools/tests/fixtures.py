try:
    from queue import Empty
except ImportError:
    from Queue import Empty


class MockQueue:

    def __init__(self):
        self.internal_storage = []

    def put(self, item):
        self.internal_storage.append(item)

    def get_nowait(self):
        try:
            return self.internal_storage.pop(0)
        except IndexError:
            raise Empty
