# Full Local Test

All tests is this folder use docker-compose to create a certain set of machines (e.g. some hlt machines) and start `nsmd2` in those machines. Via specific scripts, the needed `daq_slc` apps can be started and the apps can be controlled via `rcreqquest`, `rcview` and all nsm tools as normal.

## Prerequisites

The following assumes you have docker in a recent version installed and you are able to run docker containers. It also assumes you have both `basf2` as well as the `daq_slc` already downloaded, compiled and set up. `daq_slc` should be in the branch `feature/add-new-zmq-hlt-apps` for now (will be merged to master at a later point in time).

You can test this by running

    docker ps

which should show at least an empty list,

    basf2 --info

which should show some basf2 information and

    nsmd2 --help

which should show the nsmd2 help.

## Starting the machines and login

Navigate into one of the sub folders and call

    docker-compose up -d

to start the "machines" (actually, they are not real machines but only processes with some fancy control group management).
Once done, they will continue running in the background until you call

    docker-compose stop

or (for a force stop)

    docker-compose kill

You can monitor the status of the "machines" via

    docker-compose ps

To "log in" into one of the machines call

    docker exec -it <machine-name> /bin/bash

which will not really log in (like ssh would do), but start another bash process in the correct control group.

For all processes running in the docker container, it feels like they would be on a dedicated machine. Just always remember that you can not "ssh" into it.
The machine names are given in the `docker-compose` file, but are chosen to resemble the real HLT machine names (e.g. hltin, hltout etc).

## Start the daq slc apps

In every folder there is a script called `restart_apps.sh`.

    bash restart_apps.sh

Its only content is to call `scripts/restart.sh` on every running container. The real business logic is handled in this script. Fell free to look into the script and change it as needed.

The now following parts assume you are logged into the control machine (e.g. hltctl, or erctl) via

    docker exec -it hltctl /bin/bash

## Runcontrol

As the normal HLT data taking, also this setup consists of a typical `daq_slc` environment with `nsmd2` and `runcontrold`, so you can use the typical commands.
`nsmd2` runs with a 9120 network in the containers and the main runcontrol is named `rc_zhlttest` (for the HLT setup)

    root@hltctl:/work# nsminfo2 -p 9120
    root@hltctl:/work# rcview rc_hlt_docker -c rc_hlt_docker -n view
    root@hltctl:/work# rcrequest runcontrol_hlt_docker <REQUEST> -c runcontrol_hlt_docker

where `<REQUEST>` can be `LOAD`, `ABORT`, `STOP` or `START 1 1` (the experiment and run numbers do actually not play any role, but you have to give then otherwise the `daq_slc` framework won't work). Remember that you have to be on hltctl to send the requests.

All apps expose their internal status via nsm variables, so you might be interested in them:

    root@hltctl:/work# nsmvlistget <app> -c <app>
    root@hltctl:/work# nsmvget <app> -c <app> <variable>

where `<app>` is one of the started apps (you can get the names either via the `scripts/restart.sh` file or from `rcview`) and `<variable>` is one of the variables shown in `nsmvlistget`.

## Monitoring

After loading, the started basf2 programs can be monitored as normal with

    b2hlt_monitor.py address ...

Have a look into `b2hlt_monitor.py --help` to see how to use the tool.

## Event Builder and Storage

The setup includes a `test_eb1` and a `test_storage` nsm node, which also participate in the run. The `test_eb1` starts an input process from file on `START` reading from the prepared file in the folders and sending to the `distributor`. The `test_storage` starts and output process which just receives events without writing them to disk. Both use the same format as the correct apps later - but are only useful for testing purposes.

## Things to Test

Apart from the different event builder and storage and less workers, it is exactly the same setup as the later HLT/ERECO one. One notable difference is the usage of less CPUs per worker and the passthrough script instead of the real reconstruction (as most local machines/desktop can not handle more). If you want, you can change this by replacing the python script.

You can test all later applications:

* starting, stopping, loading, aborting, mix-of-everything
* kill workers/other applications by sending a `kill` signal to some of them, watching how the system behaves
* check the monitoring and the logs
* check the written out DQM files