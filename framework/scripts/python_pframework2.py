from numpy import random
from time import sleep
import json
import zmq

INPUT_NAME = "input"
OUTPUT_NAME = "output"
CONTROL_RECEIVE_NAME = "control_receive"
CONTROL_SEND_NAME = "control_send"

DEBUG = False


class Module(object):

    def __init__(self):
        self.context = None
        self.socket = None

    def terminate(self):
        if DEBUG:
            print("Term", self.__class__.__name__)

    def event(self, **kwargs):
        if DEBUG:
            print("Event", self.__class__.__name__)
        return {}

    def initialize(self):
        if DEBUG:
            print("Init", self.__class__.__name__)

        self.context = zmq.Context()


def process_modules(module_list):
    if not module_list:
        return

    # Initialize phase
    for module in module_list:
        module.initialize()

    sleep(0.01)

    # Event phase
    should_break = False
    while not should_break:
        # "Reset DataStore"
        result = {}
        for i, module in enumerate(module_list):
            try:
                result = module.event(**result)

                if not result and i == 0:
                    should_break = True
                    break
            except KeyboardInterrupt:
                should_break = True
                break

    # Terminate phase
    for module in module_list:
        module.terminate()


class ReadEventsModule(Module):

    def __init__(self):
        super().__init__()

        self.event_number = 0

    def event(self, **kwargs):
        Module.event(self, **kwargs)

        if self.event_number >= 100:
            return None
        else:
            self.event_number += 1
            return {"data": [1] * 1000}


class SaveEventsModule(Module):

    def __init__(self):
        super().__init__()

        self.f = open("log", "w")

    def event(self, **kwargs):
        self.f.write(str(kwargs["worker"]) + "\n")

    def terminate(self):
        self.f.close()


class WorkingProcess(Module):

    def __init__(self, worker_id):
        super().__init__()

        self.worker_id = worker_id
        self.__class__.__name__ += "_" + str(self.worker_id)

    def event(self, **kwargs):
        kwargs["worker"] = self.worker_id
        sleep((1.0 + (4.0 - self.worker_id)) / 10.0)
        return kwargs


class InputTxModule(Module):

    def __init__(self, num_workers, buffer_size):
        super().__init__()

        self.num_workers = num_workers
        self.buffer_size = buffer_size

        self.next_worker = [worker_id for _ in range(self.buffer_size) for worker_id in range(self.num_workers)]

    def initialize(self):
        Module.initialize(self)

        self.socket = self.context.socket(zmq.ROUTER)
        self.socket.setsockopt(zmq.LINGER, 0)
        self.socket.bind("ipc://%s" % INPUT_NAME)

    def get_workers_ready_message(self, blocking=False):
        # Get all messages in the queue
        while (not blocking and self.socket.poll(0)) or (blocking and len(self.next_worker) == 0 and self.socket.poll(-1)):
            worker_id, message = self.socket.recv_multipart()
            message = json.loads(str(message, encoding="utf8"))
            assert "state" in message and message["state"] == "ready"

            self.next_worker.append(int(worker_id))

    def get_next_worker(self):
        if len(self.next_worker) == 0:
            self.get_workers_ready_message(blocking=True)

        return self.next_worker.pop(0)

    def send_work_to(self, next_worker, data):
        self.socket.send_multipart([bytes(str(next_worker), encoding="utf8"), bytes(json.dumps(data), encoding="utf8")])
        print("Sending work to", next_worker)

    def send_kill_to(self, next_worker):
        self.socket.send_multipart([bytes(str(next_worker), encoding="utf8"),
                                    bytes(json.dumps({"state": "finished"}), encoding="utf8")])

    def event(self, **kwargs):
        Module.event(self, **kwargs)

        # Get all workers ready messages
        self.get_workers_ready_message(blocking=False)

        print(self.next_worker)

        # Normal Mode: We have still events around
        next_worker = self.get_next_worker()

        self.send_work_to(next_worker, kwargs)

    def terminate(self):
        Module.terminate(self)
        # Terminate mode: We do not have events around
        for worker_id in range(self.num_workers):
            self.send_kill_to(worker_id)


class InputRxModule(Module):

    def __init__(self, worker_id):
        super().__init__()

        self.worker_id = worker_id
        self.__class__.__name__ += "_" + str(self.worker_id)

    def initialize(self):
        Module.initialize(self)

        self.socket = self.context.socket(zmq.DEALER)
        self.socket.setsockopt(zmq.LINGER, 0)
        self.socket.setsockopt_string(zmq.IDENTITY, str(self.worker_id))
        self.socket.connect("ipc://%s" % INPUT_NAME)

    def get_work(self):
        return self.socket.recv_json()

    def send_ready_message(self):
        self.socket.send_json({"state": "ready"})

    def event(self, **kwargs):
        Module.event(self, **kwargs)

        kwargs = self.get_work()

        if "state" in kwargs and kwargs["state"] == "finished":
            return None
        else:
            self.send_ready_message()
            return kwargs


class OutputTxModule(Module):

    def __init__(self, worker_id):
        super().__init__()

        self.worker_id = worker_id
        self.__class__.__name__ += "_" + str(self.worker_id)

    def initialize(self):
        Module.initialize(self)

        self.socket = self.context.socket(zmq.PUSH)
        self.socket.setsockopt(zmq.LINGER, 0)
        self.socket.connect("ipc://%s" % OUTPUT_NAME)

    def send_output(self, work):
        self.socket.send_json(work)

    def send_kill(self):
        self.socket.send_json({"state": "finished"})

    def event(self, **kwargs):
        Module.event(self, **kwargs)

        self.send_output(kwargs)

    def terminate(self):
        Module.terminate(self)
        self.send_kill()


class OutputRxModule(Module):

    def __init__(self, num_workers):
        super().__init__()
        self.num_workers = num_workers

    def initialize(self):
        Module.initialize(self)

        self.socket = self.context.socket(zmq.PULL)
        self.socket.setsockopt(zmq.LINGER, 0)
        self.socket.bind("ipc://%s" % OUTPUT_NAME)

    def get_work(self):
        return self.socket.recv_json()

    def event(self, **kwargs):
        Module.event(self, **kwargs)

        while True:
            kwargs = self.get_work()
            if "state" in kwargs and kwargs["state"] == "finished":
                self.num_workers -= 1

                if self.num_workers == 0:
                    return None
                else:
                    # We still have to wait for everyone to say something
                    pass
            else:
                return kwargs


if __name__ == '__main__':
    from multiprocessing import Process

    num_workers = 5
    buffer_size = 5

    def input_process():
        print("Input Process started.")
        input_path = [ReadEventsModule(), InputTxModule(num_workers, buffer_size)]
        process_modules(input_path)
        print("Input Process finished.")

    # Start workers
    def worker_process(w_id):
        print("Worker Process %d started." % w_id)
        worker_path = [InputRxModule(w_id), WorkingProcess(w_id), OutputTxModule(w_id)]
        process_modules(worker_path)
        print("Worker Process %d finished." % w_id)

    # Start sender
    def output_process():
        print("Output Process started.")
        output_path = [OutputRxModule(num_workers), SaveEventsModule()]
        process_modules(output_path)
        print("Output Process finished.")

    processes = [
        Process(target=input_process),
        Process(target=output_process),
    ]

    for worker_id in range(num_workers):
        processes.append(Process(target=worker_process, args=(worker_id,)))

    for process in processes:
        process.start()

    for process in processes:
        process.join()

    import pandas as pd

    df = pd.read_csv("log", header=None)
    print(df[0].value_counts())
    print(len(df[0]))
