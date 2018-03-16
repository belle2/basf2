#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
# This steering file shows how to use the statistics
# object exposed by the framework
#
# Example steering file - 2011 Belle II Collaboration
######################################################

from basf2 import *

# Create main path
main = create_path()

# EventInfoSetter - generate event meta data
eventinfosetter = main.add_module('EventInfoSetter',
                                  expList=[71, 71, 73, 73, 73],
                                  runList=[3, 4, 10, 20, 30],
                                  evtNumList=[40, 600, 20, 500, 301])
# eventinfo - show event meta info
eventinfo = main.add_module('EventInfoPrinter')
main.add_module('Gearbox')

# Process all events
process(main)

# Print basic event statistics to stdout
print('Event Statistics:')
print(statistics)

# Print basic event statistics for specific modules
print('Event Statistics for selected modules:')
print(statistics([eventinfosetter]))

# Print statistics adding all counters
print('Total processing times:')
print(statistics(statistics.TOTAL))

# Change name of modules
statistics.set_name(eventinfosetter, 'Foo')

# Make a list of all available statistic counters
statistic_counters = [
    (statistics.INIT, "initialize()"),
    (statistics.BEGIN_RUN, "beginRun()"),
    (statistics.EVENT, "event()"),
    (statistics.END_RUN, "endRun()"),
    (statistics.TERM, "terminate()"),
    (statistics.TOTAL, "*total*"),
]

# Print detailed statistics for each module
for stats in statistics.modules:
    print('Module %s:' % stats.name)
    for stat_counter, stat_name in statistic_counters:
        print(' -> %12s: %10.3f ms, %4d calls, %10.3f +-%10.3f ms/call' % (
            stat_name,
            # Time is in the default time unit which happens to be ns
            stats.time_sum(stat_counter) / 1e6,
            stats.calls(stat_counter),
            stats.time_mean(stat_counter) / 1e6,
            stats.time_stddev(stat_counter) / 1e6,
        ))
    print()

print('Memory statistics')
for stats in statistics.modules:
    print('Module %s:' % stats.name)
    for stat_counter, stat_name in statistic_counters:
        print(' -> %12s: %10d KB, %4d calls, %10d +-%10.3f KB/call' % (
            stat_name,
            stats.memory_sum(stat_counter),
            stats.calls(stat_counter),
            stats.memory_mean(stat_counter),
            stats.memory_stddev(stat_counter),
        ))

# Get Statistics for single module
stats = statistics.get(eventinfosetter)
eventinfo_total = stats.time_sum(statistics.TOTAL)
print('EventInfoSetter needed %.3f ms in total' % (eventinfo_total / 1e6))
# Now we can also set the name since we have the correct object
stats.name = "FooBar"

# Print total processing time
framework_total = statistics.get_global().time_sum(statistics.TOTAL)
print('Total processing time: %.3f ms' % (framework_total / 1e6))

# Calculate estimate for framework overhead
modules_total = sum(e.time_sum(statistics.TOTAL) for e in statistics.modules)
overhead = framework_total - modules_total
print('Framework overhead: %.3f ms (%.2f %%)' % (
    overhead / 1e6,
    100 * overhead / framework_total,
))
print()

# Clear statistics
statistics.clear()
print('Empty statistics')
print(statistics)
