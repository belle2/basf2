#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
import unittest
from b2test_utils import run_in_subprocess, clean_working_directory
from contextlib import redirect_stdout
import io
import json

# @cond internal_test


class PythonLogInterface(unittest.TestCase):
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

    def test_fatal_pythonlogging(self):
        # but use sys.stdout for logging
        basf2.logging.enable_python_logging = True

        # check that fatal actually kills the process
        def checkfatal():
            try:
                basf2.B2FATAL("exit")
            except Exception as e:
                basf2.B2ERROR("raised exception: ", e)
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
        # no change log info to show everything except time
        li = basf2.LogInfo
        basf2.logging.set_info(basf2.LogLevel.INFO, li.MESSAGE | li.LEVEL | li.PACKAGE | li.FUNCTION | li.FILE | li.LINE)
        # sometimes the path to the file is absolute so make sure the filename
        # is the same as reported for this frame. Also the line number changes
        # every time we touch this file so determine it automatically
        import inspect
        filename = inspect.currentframe().f_code.co_filename
        lineno = inspect.currentframe().f_lineno + 2
        # and print a message
        basf2.B2INFO("show current frame info", why="because we can")
        self.check_logs(
            "[INFO] show current frame info\n"
            "\twhy = because we can  { package: steering function: test_inspect @%s:%d }\n" %
            (filename, lineno))


class PythonLogJSON(unittest.TestCase):
    def setUp(self):
        # disable error summary
        basf2.logging.enable_summary(False)
        # but use sys.stdout for logging
        basf2.logging.enable_python_logging = True
        # log to a file. We assume this is run in a temporary directory so
        # fixed filename is fine
        basf2.reset_log()
        basf2.logging.add_json()
        # Reset log level
        basf2.logging.log_level = basf2.LogLevel.INFO
        basf2.logging.debug_level = 100
        # modify logging to remove the useless information
        for level in basf2.LogLevel.values.values():
            basf2.logging.set_info(level, basf2.LogInfo.LEVEL | basf2.LogInfo.MESSAGE)

    def getLogMessage(self, message, **argk):
        out = io.StringIO()
        with redirect_stdout(out):
            basf2.B2INFO(message, **argk)
        raw = out.getvalue()
        # make sure string ends in new line
        self.assertEqual(raw[-1], "\n")
        # make sure there are no newlines in the raw string
        self.assertTrue(raw[:-1].find("\n") < 0)
        # make sure we can parse json
        logobject = json.loads(raw)
        return logobject

    def assertLogMessage(self, message, **argk):
        # and that the message and level are there
        logobject = self.getLogMessage(message, **argk)
        if len(argk):
            stringified = {str(key): str(val) for key, val in argk.items()}
            self.assertDictEqual(logobject, {"level": "INFO", "message": message, "variables": stringified})
        else:
            self.assertDictEqual(logobject, {"level": "INFO", "message": message})

    def test_info(self):
        self.assertLogMessage("message")

    def test_multiline(self):
        self.assertLogMessage("message\ncontaining\nnewlines")

    def test_vars(self):
        self.assertLogMessage("message", var1="foo", var2="bar", int=3)

    def test_vars_newline(self):
        self.assertLogMessage("message", var1="foo\nbar", var2="bar\nboo")

    def test_utf8(self):
        zalgo = "h͌̉e̳̞̞͆ͨ̏͋̕ ͍͚̱̰̀͡c͟o͛҉̟̰̫͔̟̪̠m̴̀ͯ̿͌ͨ̃͆e̡̦̦͖̳͉̗ͨͬ̑͌̃ͅt̰̝͈͚͍̳͇͌h̭̜̙̦̣̓̌̃̓̀̉͜!̱̞̻̈̿̒̀͢!̋̽̍̈͐ͫ͏̠̹̺̜̬͍ͅ"
        self.assertLogMessage(zalgo, **{zalgo: zalgo})

    def test_loginfo(self):
        """Make sure all loginfo settings are honored in json output"""
        cum_info = 0
        # level is always added
        cum_keys = {"level"}
        # go through all loginfo values and check if the correct field is in
        # the json, once separately and also cumulative
        for val, key in basf2.LogInfo.values.items():
            cum_info |= val
            key = key.name.lower()
            cum_keys.add(key)
            for info, keys in (val, {"level", key}), (cum_info, cum_keys):
                basf2.logging.set_info(basf2.LogLevel.INFO, info)
                logobject = self.getLogMessage("simple")
                self.assertEqual(set(logobject.keys()), keys)
                varkeys = keys.copy()
                # variables are parts of messages ... but they only show up if
                # message is shown
                if "message" in keys:
                    varkeys.add("variables")
                logobject = self.getLogMessage("with var", var="val")
                self.assertEqual(set(logobject.keys()), varkeys)

    def test_complete(self):
        """Test that we always get all fields when setting output to complete"""
        basf2.reset_log()
        basf2.logging.add_json(True)
        logobject = self.getLogMessage("message")
        self.assertSetEqual(set(logobject.keys()), {"level", "message", "variables", "module", "package",
                                                    "function", "file", "line", "timestamp", "proc",
                                                    "initialize", "count"})


if __name__ == "__main__":
    with clean_working_directory():
        unittest.main(verbosity=0)

# @endcond
