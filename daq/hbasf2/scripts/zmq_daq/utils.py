import pandas as pd
import json
from datetime import datetime


import os


def _receive(socket, filtering=True):
    """Internal helper function to ask a socket for monitoring and get the answer as JSON"""
    _, message, _ = socket.recv_multipart()
    message = json.loads(message.decode())

    def normalize(value):
        if isinstance(value, str):
            return value.strip()
        else:
            return value

    for category, dictionary in message.items():
        for key, value in dictionary.items():
            yield f"{category}.{key}", normalize(value)


def _get_monitor_table_impl(sockets, show_detail):
    """Functor for executing the actual monitoring request and returning the JSON"""
    for socket in sockets.values():
        socket.send_multipart([b"m", b"", b""])

    for name, socket in sockets.items():
        for key, value in _receive(socket):
            if show_detail or ("[" not in key and "last_measurement" not in key):
                yield f"{name}.{key}", value


def get_monitor_table(sockets, show_detail):
    """
    Ask the given sockets for their monitoring JSON and return
    a dictionary with each answers (the socket address as the key).
    The additional flag show_detail controls how many details of the returned
    JSON should be included.
    """
    return dict(_get_monitor_table_impl(sockets=sockets, show_detail=show_detail))


def show_monitoring(df, clear=False):
    """
    Print the monitoring data produced by "get_monitor_table"
    in a human readable form to the console.
    """
    tmp = pd.Series({tuple(key.split(".")): value for key, value in df.items()}).unstack(0)
    tmp = tmp.fillna("-")
    pd.set_option("max_rows", len(tmp))
    if clear:
        os.system("clear")
    print(tmp)


def normalize_addresses(addresses):
    """
    Convert a list of addresses into a normalized format, where
    each address starts with "tcp://", includes a hostname (if not given)
    and a port number.
    Also removed duplicates.
    Useful when user input is processed.
    """
    addresses = ["tcp://localhost:" + address if address.isdigit() else address for address in addresses]
    addresses = ["tcp://" + address if "tcp://" not in address else address for address in addresses]
    addresses = set(addresses)

    return addresses


def write_monitoring(df, f):
    """
    Using the data produced by "get_monitor_table" dump it
    to disk in the msgpack data format.
    You will need to have msgpack installed for this.
    Adds the current time as an additional column
    Attention: this is the time of the function call which might or might not
    correspond to the time the values were extracted.
    """
    try:
        import msgpack
    except ImportError:
        raise ValueError("Please install msgpack with pip to use this functionality")
    df["time"] = str(datetime.now())
    f.write(msgpack.packb(df, use_bin_type=True))


def load_measurement_file(file_name):
    """
    Load a measurement data file produced by

        b2hlt_monitor.py --dat

    and create a pandas dataframe out of it.
    You need to have msgpack installed for this.
    Automatically converts the stored time into a timedelta index
    with the first measurements defined as 0.
    See the jupyter notebook on how to use this function.
    """
    try:
        import msgpack
    except ImportError:
        raise ValueError("Please install msgpack with pip to use this functionality")

    with open(file_name, "rb") as f:
        unpacker = msgpack.Unpacker(f, raw=False)
        df = pd.DataFrame(list(unpacker))
        df = df.set_index("time")
        df.index = pd.to_datetime(df.index)
        df.index = (df.index - df.index[0]).total_seconds()
        df.columns = df.columns.map(lambda x: tuple(x.split(".")))

        return df
