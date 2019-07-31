# Local Test

This test starts all applications for data transportation in the ZMQ-HLT framework locally on this machine.
As input, a prepared file of ~10 events is used repeatedly, the output is just discarded.

## Start the Test

To start the test, setup basf2 and run

    python3 run_local_test.py <yaml-file> load

where `<yaml-file>` is one of the prepared scenarios in the `config/` folder (e.g. start with `config/hlt.yaml`). 
This will start all needed applications and wait.
You can have a look into the yaml file to see which applications are started and how the data flows.

## Feed in Events

You can now send events to the input (probably the distributor) by running

    python3 run_local_test.py <yaml-file> input_file <data-file>

in a second terminal, which will stream the content of the prepared file to the input. 
To do this, you need to create a data file if you do not have one.
See the files folder on how to do this.
Please note that you may want to add different argument options to this call:

* if you have a raw data file (e.g. for hlt tests), you need to give `--raw` as an option:

      python3 run_local_test.py <yaml-file> input_file <data-file> --raw

* if you want to test continuously, there is the option `--repeat`, which will reopen the file once finished:

      python3 run_local_test.py <yaml-file> input_file <data-file> --repeat

* of course, both options can also be combined


## Monitor what is going on

While doing the test, you can monitor the status of the apps, e.g. with the default program `b2hlt_monitor.py`.
You need to use the monitor port numbers, which are also given in the selected yaml-file. For example:

    b2hlt_monitor.py 8000 8001

You can also use the script in this folder, which will be a bit more convenient (but do the same thing):

    python3 run_local_test.py <yaml-file> monitor

This will monitor all applications simultaneously.

You can also send start and stop commands, either again with the default monitoring

    b2hlt_monitor.py 8000 --stop

or with the prepared python file

    python3 run_local_test.py <yaml-file> send_stop

and similarly with `--start` or `send_start`.

In the end, either send a terminate signal to all applications or just press Ctrl-C on the terminal where you have started the load process.

## Things you might want to try out

* sending start, then send events, stop sending events, send stop
* repeatedly send events and start/stop
* stopping a run without events
* monitoring of all services and writing out of the monitoring json (see `b2hlt_monitor.py --help`)

Please be aware that this is not a real test, as it is not using the `daq_slc` apps that will be used later. Also, it only runs on a single node with TCP connections done via localhost.

You can not run the test multiple times in parallel, as it reuses the same ports.
Please report problems to [nils.braun@kit.edu](mailto:nils.braun@kit.edu).