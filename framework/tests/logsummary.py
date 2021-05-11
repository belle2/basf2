#!/usr/bin/env python3

# this is a test executable, not a module so we don't need doxygen warnings
# @cond SUPPRESS_DOXYGEN

"""
Test if the log summary is shown correctly in all cases
"""

from basf2 import create_path, process, Module, logging, LogLevel, LogInfo, \
    B2WARNING, B2FATAL, B2ERROR
import multiprocessing


def fork_process(*args, target=process):
    """Run function in forked child to eliminate side effects"""
    sub = multiprocessing.Process(target=target, args=args)
    sub.start()
    sub.join()


class FatalError(Module):
    """Simple module to just print a warning, an error and a fatal error"""

    def event(self):
        """print messages"""
        B2WARNING("warning")
        B2ERROR("error")
        B2FATAL("fatal error")


logging.log_level = LogLevel.WARNING
for level in LogLevel.values.values():
    logging.set_info(level, LogInfo.LEVEL | LogInfo.MODULE | LogInfo.PACKAGE | LogInfo.MESSAGE)

main = create_path()
main.add_module("EventInfoSetter")
module = main.add_module(FatalError())

print("run in default settings")
fork_process(main)

print("run with no warnings")
logging.log_level = LogLevel.ERROR
fork_process(main)
logging.log_level = LogLevel.WARNING

print("run with no framework warnings")
logging.package("framework").log_level = LogLevel.ERROR
fork_process(main)
logging.package("framework").log_level = LogLevel.WARNING

print("run with no module warnings")
module.set_log_level(LogLevel.ERROR)
fork_process(main)
module.set_log_level(LogLevel.WARNING)

print("run without summary")
logging.enable_summary(False)
fork_process(main)

# @endcond
