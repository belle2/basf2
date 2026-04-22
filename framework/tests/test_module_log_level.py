#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2

main = b2.Path()

main.add_module(
    'RootInput',
    inputFileName=b2.find_file('framework/tests/root_input.root'),
    branchNames=['EventMetaData'],
    skipNEvents=1)

main.add_module('EventInfoPrinter', logLevel=b2.LogLevel.INFO)

event_info_printer_log_level = b2.logging.module('EventInfoPrinter').log_level
print(f'Log-level for module EventInfoPrinter: {event_info_printer_log_level}')

b2.process(path=main)
