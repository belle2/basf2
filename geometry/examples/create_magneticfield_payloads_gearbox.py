#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
import shutil
import multiprocessing


def safe_process(*args, **kwargs):
    """Run `basf2.process` with the given path in a child process using
    `multiprocessing.Process`

    This avoids side effects (`safe_process` can be safely called multiple times)
    and doesn't kill this script even if a segmentation violation or a `FATAL
    <LogLevel.FATAL>` error occurs during processing.

    It will return the exitcode of the child process which should be 0 in case of no error
    """
    process = multiprocessing.Process(target=basf2.process, args=args, kwargs=kwargs)
    process.start()
    process.join()
    return process.exitcode


# remove existing local database
shutil.rmtree("localdb", ignore_errors=True)

path = basf2.Path()
path.add_module("EventInfoSetter")
path.add_module("Gearbox")
path.add_module("Geometry", createPayloads=True, payloadIov=[0, 0, -1, -1])
for field in ["", "Phase2", "Phase2QCSoff"]:
    basf2.set_module_parameters(path, "Geometry", components=[f"MagneticField{field}"])
    basf2.prepend_testing_payloads(f"localdb/MagneticField{field}.txt")
    safe_process(path)
    txtfile = open(f"localdb/MagneticField{field}.txt").readlines()
    with open(f"localdb/MagneticField{field}.txt", "w") as newfile:
        for line in txtfile:
            if line.find("Magnetic") >= 0:
                newfile.write(line)
