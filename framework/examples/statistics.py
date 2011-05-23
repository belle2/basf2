#!/usr/bin/env python
# -*- coding: utf-8 -*-

######################################################
# This steering file shows how to use the statistics
# object exposed by the framework
#
# Example steering file - 2011 Belle II Collaboration
######################################################

from basf2 import *

# EvtMetaGen - generate event meta data
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param({'ExpList': [71, 71, 73, 73, 73], 'RunList': [3, 4, 10, 20,
                 30], 'EvtNumList': [4, 6, 2, 5, 3]})
# EvtMetaInfo - show event meta info
evtmetainfo = register_module('EvtMetaInfo')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(evtmetagen)
main.add_module(evtmetainfo)

# Process all events
process(main)

# Print basic event statistics to stdout
print 'Event Statistics:'
print statistics

# Print basic event statistics for specific modules
print 'Event Statistics for selected modules:'
print statistics([evtmetagen])

# Print statistics adding all counters
print 'Total processing times:'
print statistics(statistics.TOTAL)

# Change name of modules
statistics.set_name(evtmetagen, 'Foo')

# Print detailed statistics for each module
for stats in statistics.modules:
    print 'Module %s:' % stats.name
    print ' -> initialize(): %10.3f ms, %3d calls, %10.3f ms/call' \
        % (stats.time(statistics.INIT) * 1e3, stats.calls(statistics.INIT),
           stats.time(statistics.INIT) * 1e3 / stats.calls(statistics.INIT))
    print ' -> beginRun():   %10.3f ms, %3d calls, %10.3f ms/call' \
        % (stats.time(statistics.BEGIN_RUN) * 1e3,
           stats.calls(statistics.BEGIN_RUN), stats.time(statistics.BEGIN_RUN)
           * 1e3 / stats.calls(statistics.BEGIN_RUN))
    print ' -> event():      %10.3f ms, %3d calls, %10.3f ms/call' \
        % (stats.time() * 1e3, stats.calls(), stats.time() * 1e3
           / stats.calls())
    print ' -> endRun():     %10.3f ms, %3d calls, %10.3f ms/call' \
        % (stats.time(statistics.END_RUN) * 1e3,
           stats.calls(statistics.END_RUN), stats.time(statistics.END_RUN)
           * 1e3 / stats.calls(statistics.END_RUN))
    print ' -> terminate():  %10.3f ms, %3d calls, %10.3f ms/call' \
        % (stats.time(statistics.TERM) * 1e3, stats.calls(statistics.TERM),
           stats.time(statistics.TERM) * 1e3 / stats.calls(statistics.TERM))
    print

# Get Statistics for single module
stats = statistics.get(evtmetagen)
print 'EvtMetaGen needed %.3f ms in total' % (1e3
        * stats.time(statistics.TOTAL))

# Print total processing time
print 'Total processing time: %.3f ms' % (1e3
        * statistics.framework.time(statistics.TOTAL))

# Calculate estimate for framework overhead
modules_total = sum(e.time(statistics.TOTAL) for e in statistics.modules)
overhead = statistics.framework.time(statistics.TOTAL) - modules_total
print 'Framework overhead: %.3f ms' % (1e3 * overhead)
print

# Clear statistics
statistics.clear()
print 'Empty statistics'
print statistics
