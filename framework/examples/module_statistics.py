#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

######################################################
# This steering file shows how to use the statistics
# object exposed by the framework
######################################################

from basf2 import Path, process, statistics

# Create main path
main = Path()

# EventInfoSetter - generate event meta data
eventinfosetter = main.add_module('EventInfoSetter',
                                  expList=[71, 71, 73, 73, 73],
                                  runList=[3, 4, 10, 20, 30],
                                  evtNumList=[40, 600, 20, 500, 301])
# eventinfo - show event meta info
eventinfo = main.add_module('EventInfoPrinter')

# Process all events
process(main)

# Print basic event statistics to stdout
print('Event Statistics:')
print(statistics)

# Print basic event statistics for specific modules
print('Event Statistics for selected modules:')
print(statistics(modules=[eventinfosetter]))

# Print statistics adding all counters
print('Total processing times:')
print(statistics(statistics.TOTAL))

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
    print(f'Module {stats.name}:')
    for stat_counter, stat_name in statistic_counters:
        print(f' -> {stat_name:12}: {stats.time_sum(stat_counter) / 1000000.0:10.3f} ms, {int(stats.calls(stat_counter)):4} ' +
              f'calls, {stats.time_mean(stat_counter) / 1000000.0:10.3f} ' +
              f'+-{stats.time_stddev(stat_counter) / 1000000.0:10.3f} ms/call')
        # Time is in the default time unit which happens to be ns
    print()

print('Memory statistics')
for stats in statistics.modules:
    print(f'Module {stats.name}:')
    for stat_counter, stat_name in statistic_counters:
        print(f' -> {stat_name:12}: {int(stats.memory_sum(stat_counter)):10} KB, {int(stats.calls(stat_counter)):4} calls, ' +
              f'{int(stats.memory_mean(stat_counter)):10} +-{stats.memory_stddev(stat_counter):10.3f} KB/call')

# Get Statistics for single module
stats = statistics.get(eventinfosetter)
eventinfo_total = stats.time_sum(statistics.TOTAL)
print(f'EventInfoSetter needed {eventinfo_total / 1000000.0:.3f} ms in total')

# Print total processing time
framework_total = statistics.get_global().time_sum(statistics.TOTAL)
print(f'Total processing time: {framework_total / 1000000.0:.3f} ms')

# Calculate estimate for framework overhead
modules_total = sum(e.time_sum(statistics.TOTAL) for e in statistics.modules)
overhead = framework_total - modules_total
print(f'Framework overhead: {overhead / 1000000.0:.3f} ms ({100 * overhead / framework_total:.2f} %)')
print()

# Clear statistics
statistics.clear()
print('Empty statistics')
print(statistics)
