#!/usr/bin/env python3

import basf2
import unittest
from b2test_utils import run_in_subprocess, clean_working_directory

# @cond internal_test


class DBInterface(unittest.TestCase):
    def assertDeath(self, function, *args, **kwargs):
        """Run function in child process and check if it died. Only way to check for B2FATAL"""
        exitcode = run_in_subprocess(target=function, *args, **kwargs)
        self.assertNotEqual(exitcode, 0)

    def check_logs(self, target):
        """Check the output of the log messages"""
        with open("logmessages.txt") as f:
            self.assertEqual(target, f.read())

    def setUp(self):
        # disable error summary
        basf2.logging.enable_summary(False)
        # log to a file. We assume this is run in a temporary directory so
        # fixed filename is fine
        basf2.reset_log()
        basf2.log_to_console(True)
        basf2.log_to_file("logmessages.txt")
        # Reset log level
        basf2.logging.log_level = basf2.LogLevel.INFO
        basf2.logging.debug_level = 100
        # modify logging to remove the useless information
        for level in basf2.LogLevel.values.values():
            basf2.logging.set_info(level, basf2.LogInfo.LEVEL | basf2.LogInfo.MESSAGE)

    def test_debug(self):
        # check argument handling
        with self.assertRaises(TypeError):
            basf2.B2DEBUG()
        with self.assertRaises(TypeError):
            basf2.B2DEBUG(1)
        with self.assertRaises(TypeError):
            basf2.B2DEBUG("foo", "bar")
        with self.assertRaises(TypeError):
            basf2.B2DEBUG(3.14, "bar")
        with self.assertRaises(TypeError):
            basf2.B2DEBUG([34, 324], "bar")

        # check visibility
        basf2.B2DEBUG(101, "should not show")
        basf2.B2DEBUG(100, "this neither")
        basf2.logging.log_level = basf2.LogLevel.DEBUG
        basf2.B2DEBUG(101, "should still not show")
        basf2.B2DEBUG(100, "this should")
        basf2.B2DEBUG(99, "and with variables", foo="bar", bar="foo", int=42, float=3.14)
        self.check_logs("[DEBUG:100] this should\n"
                        "[DEBUG:99] and with variables\n"
                        "\tbar = foo\n"
                        "\tfloat = 3.14\n"
                        "\tfoo = bar\n"
                        "\tint = 42\n")

    def test_fatal(self):
        # check argument handling
        with self.assertRaises(TypeError):
            basf2.B2FATAL()

        # check that fatal actually kills the process
        def checkfatal():
            basf2.B2FATAL("exit")
            basf2.B2ERROR("should not show")
        self.assertDeath(checkfatal)

        self.check_logs("[FATAL] exit\n")

    def test_others(self):
        # check argument handling
        for i, f in enumerate([basf2.B2INFO, basf2.B2WARNING, basf2.B2ERROR]):
            with self.assertRaises(TypeError):
                f()

            f("Show me", " a message with index ", i, index=i, dictvar={"i": i})

        # and visibility
        basf2.logging.log_level = basf2.LogLevel.ERROR
        basf2.B2INFO("No output here")
        basf2.B2WARNING("Nor here")
        basf2.B2ERROR("But here")

        self.check_logs("[INFO] Show me a message with index 0\n"
                        "\tdictvar = {'i': 0}\n"
                        "\tindex = 0\n"
                        "[WARNING] Show me a message with index 1\n"
                        "\tdictvar = {'i': 1}\n"
                        "\tindex = 1\n"
                        "[ERROR] Show me a message with index 2\n"
                        "\tdictvar = {'i': 2}\n"
                        "\tindex = 2\n"
                        "[ERROR] But here\n")

    def test_inspect(self):
        # sometimes the path to the file is absolute so make sure the filename
        # is the same as reported for this frame
        import inspect
        filename = inspect.currentframe().f_code.co_filename
        # no change log info to show everything except time
        li = basf2.LogInfo
        basf2.logging.set_info(basf2.LogLevel.INFO, li.MESSAGE | li.LEVEL | li.PACKAGE | li.FUNCTION | li.FILE | li.LINE)
        # and print a message
        basf2.B2INFO("show current frame info", why="because we can")
        self.check_logs("[INFO] show current frame info\n"
                        "\twhy = because we can  { package: steering function: test_inspect @%s:110 }\n" % filename)


if __name__ == "__main__":
    with clean_working_directory():
        unittest.main(verbosity=0)

# @endcond
