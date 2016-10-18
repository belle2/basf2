from numpy import random
from time import sleep
import traceback
import sys

import zmq

INPUT_NAME = "input"
OUTPUT_NAME = "output"
CONTROL_RECEIVE_NAME = "control_receive"
CONTROL_SEND_NAME = "control_send"


class ControlableProcess:

    def __init__(self):
        self.context = zmq.Context()

        self.control_socket_send = self.context.socket(zmq.PUB)
        self.control_socket_send.setsockopt(zmq.LINGER, 0)
        self.control_socket_send.connect("ipc://%s" % CONTROL_SEND_NAME)

        self.control_socket_receive = self.context.socket(zmq.SUB)
        self.control_socket_receive.setsockopt_string(zmq.SUBSCRIBE, "")
        self.control_socket_receive.setsockopt(zmq.LINGER, 0)
        self.control_socket_receive.connect("ipc://%s" % CONTROL_RECEIVE_NAME)

    def send_kill(self, what):
        self.control_socket_send.send_json({"what": what})


class Producer(ControlableProcess):

    def __init__(self):
        super().__init__()

        self.zmq_socket = self.context.socket(zmq.PUSH)
        self.zmq_socket.setsockopt(zmq.LINGER, 0)
        self.zmq_socket.bind("ipc://%s" % INPUT_NAME)

    def main_loop(self):
        print("Producer: Entering main loop of producer.")
        # Start your result manager and workers before you start your producers
        for num in range(10):
            work_message = {'num': [1] * 1000000}
            print("Sending work")
            self.zmq_socket.send_json(work_message)

        self.send_kill("producer")
        print("Producer: Finished.")


class Worker(ControlableProcess):

    def __init__(self, consumer_id):
        super().__init__()

        self.consumer_id = consumer_id

        print("Consumer %d: I am consumer" % self.consumer_id)

        self.consumer_receiver = self.context.socket(zmq.PULL)
        self.consumer_receiver.setsockopt(zmq.LINGER, 0)
        self.consumer_receiver.connect("ipc://%s" % INPUT_NAME)

        self.consumer_sender = self.context.socket(zmq.PUSH)
        self.consumer_sender.setsockopt(zmq.LINGER, 0)
        self.consumer_sender.connect("ipc://%s" % OUTPUT_NAME)

        self.killed = False

    def main_loop(self):
        print("Consumer %d: Entering main loop of consumer" % self.consumer_id)

        poller = zmq.Poller()
        poller.register(self.control_socket_receive, zmq.POLLIN)
        poller.register(self.consumer_receiver, zmq.POLLIN)

        total_waiting_time = 0

        while True:
            if self.killed:
                timeout = 0
            else:
                timeout = -1
            answering_sockets = dict(poller.poll(timeout=timeout))

            if not answering_sockets:
                print("Killing!!!!")
                break

            if self.control_socket_receive in answering_sockets and answering_sockets[self.control_socket_receive] == zmq.POLLIN:
                result = self.control_socket_receive.recv_json()
                if result["what"] == "producer":
                    print("Consumer %d: Worker was killed." % self.consumer_id)
                    self.killed = True
                else:
                    print("WHAT?", result)

            if self.consumer_receiver in answering_sockets and answering_sockets[self.consumer_receiver] == zmq.POLLIN:
                data = self.consumer_receiver.recv_json()
                print("Consumer waiting")
                waiting_time = random.randint(0, 10)
                total_waiting_time += waiting_time
                sleep(waiting_time)
                result = {'consumer': self.consumer_id}
                # result.update(data)
                self.consumer_sender.send_json(result)

        # Tell consumer that we are done
        self.send_kill("worker")
        print("Worker %d finished within %d" % (self.consumer_id, total_waiting_time))


class ResultCollector(ControlableProcess):

    def __init__(self, num_workers):
        super().__init__()

        self.results_receiver = self.context.socket(zmq.PULL)
        self.results_receiver.setsockopt(zmq.LINGER, 0)
        self.results_receiver.bind("ipc://%s" % OUTPUT_NAME)

        self.collected_data = []

        self.living_workers = num_workers

    def main_loop(self):
        print("Collector: Entering main loop of result collector")

        poller = zmq.Poller()
        poller.register(self.control_socket_receive, zmq.POLLIN)
        poller.register(self.results_receiver, zmq.POLLIN)

        while True:
            if self.living_workers == 0:
                timeout = 0
            else:
                timeout = -1
            answering_sockets = dict(poller.poll(timeout=timeout))

            if not answering_sockets:
                print("KILLED!!!!")
                break

            if self.results_receiver in answering_sockets and answering_sockets[self.results_receiver] == zmq.POLLIN:
                result = self.results_receiver.recv_json()
                self.collected_data.append(result)

            if self.control_socket_receive in answering_sockets and answering_sockets[self.control_socket_receive] == zmq.POLLIN:
                result = self.control_socket_receive.recv_json()
                if result["what"] == "worker":
                    self.living_workers -= 1
                    print("Collector: Worker was killed. Still living %d" % self.living_workers)

        print(self.collected_data)

        self.send_kill("collector")
        print("Collector finished")


def run_functor(functor):
    """
    Given a no-argument functor, run it and return its result. We can
    use this with multiprocessing.map and map it over a list of job
    functors to do them.

    Handles getting more than multiprocessing's pitiful exception output
    """

    def f():
        try:
            # This is where you do your actual work
            return functor()
        except:
            # Put all exception text into an exception and raise that
            raise Exception("".join(traceback.format_exception(*sys.exc_info())))

    return f


if __name__ == '__main__':
    from multiprocessing import Process

    def collector(num_workers):
        sleep(1)
        collector = ResultCollector(num_workers)
        collector.main_loop()
        print("Collector: Out")

    # Start workers
    def worker(id):
        sleep(1)
        worker = Worker(id)
        worker.main_loop()
        print("Worker: Out")

    # Start sender
    def producer():
        sleep(1)
        producer = Producer()
        producer.main_loop()
        print("Producer: Out")

    context = zmq.Context()
    send_socket = context.socket(zmq.SUB)
    send_socket.setsockopt_string(zmq.SUBSCRIBE, "")
    send_socket.bind("ipc://%s" % CONTROL_SEND_NAME)

    receive_socket = context.socket(zmq.PUB)
    receive_socket.setsockopt(zmq.LINGER, 0)
    receive_socket.bind("ipc://%s" % CONTROL_RECEIVE_NAME)

    processes = [
        Process(target=producer),
        Process(target=collector, args=(4,)),
        Process(target=worker, args=(0,)),
        Process(target=worker, args=(1,)),
        Process(target=worker, args=(2,)),
        Process(target=worker, args=(3,))
    ]

    for process in processes:
        process.start()

    for i in range(len(processes)):
        message = send_socket.recv()
        print("Proxy: Having received:", message)
        receive_socket.send(message)
        print("Proxy: Having send:", message)

    print("Proxy: Stopping proxy")
    print("Proxy: Proxy closed")

    for process in processes:
        process.join()
