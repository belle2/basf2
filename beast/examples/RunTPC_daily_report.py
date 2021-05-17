#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import datetime
import basf2 as b2

d = datetime.datetime.today()
print((d.strftime('job start: %Y-%m-%d %H:%M:%S\n')))

# 1/ mkdir output_directory
# 2/ basf2 ./RunTPC_daily_report.py yyyy mm dd TPCposition output_directory

location = ""
if len(sys.argv) == 6:
    # parse input
    inputs = '/home/belle/igal/beast/data/NTP/TPC/' + \
        str(sys.argv[1]) + "-" + str(sys.argv[2]) + "-" + str(sys.argv[3]) + "/tpc" + str(sys.argv[4]) + "*.root"
    # parse output
    outfile = str(sys.argv[5]) + "/DailyReportOfTPC" + str(sys.argv[4]) + "_" + \
        str(sys.argv[1]) + str(sys.argv[2]) + str(sys.argv[3]) + ".root"
    # date yyyymmdd
    date = str(sys.argv[1]) + str(sys.argv[2]) + str(sys.argv[3])
    # TPC position
    TPCnb = str(sys.argv[4])
else:
    print
    print("Usage:")
    print(" 1/ Create an output directory ")
    print("  mkdir output_directory")
    print(" 2/ basf2 ./RunTPC_daily_report.py yyyy mm dd TPCposition output_directory")
    print("For example: ")
    print(" basf2 ./RunTPC_daily_report.py 2016 05 30 3 output_directory")
    print
    quit()

print(inputs)
print(outfile)
print(date)
print(TPCnb)

# Register necessary modules
main = b2.create_path()

# Register RootInput
simpleinput = b2.register_module('RootInput')
simpleinput.param('inputFileNames', inputs)
main.add_module(simpleinput)

# Register HistoManager
histo = b2.register_module("HistoManager")  # Histogram Manager
histo.param('histoFileName', outfile)
main.add_module(histo)

# Register Gearbox
gearbox = b2.register_module('Gearbox')
gearbox.param('fileName', '/geometry/Beast2_phase1.xml')
main.add_module(gearbox)

# Register MicrotpcDailyReport
daily = b2.register_module('MicrotpcDailyReport')
daily.param('inputTPCNumber', int(TPCnb))
daily.param('inputReportDate', int(date))
main.add_module(daily)


b2.process(main)

print('Event Statistics:')
print(b2.statistics)

d = datetime.datetime.today()
print(d.strftime('job finish: %Y-%m-%d %H:%M:%S\n'))
