The most up to date documentation for gBasf2 can be found at:
http://b2comp.kek.jp/~twiki/bin/view/Computing/GBasf2

gBasf2 is the commandline client for submitting grid-based basf2 jobs. It uses the power of the
DIRAC Distributed Computing Framework to control the jobs.

Please note: While basf2 can run anywhere, gBasf2 requires a DIRAC UI to function. Since the DIRAC
UI relies on some gLite middleware components, this limits the operating environments in which it
can function. As grid sites universally run Scientific Linux 5, this is the recommended environment
for preparing your grid jobs.
Basf2 Steering File options

The default configuration option for gBasf2 is to set a number of variables in your normal basf2
steering file:

############################
# gBasf2 configuration     #
############################
#Name for project
project="e055-test"
#Job priority
priority="1"
#Experiments (comma separated list)
experiments="13,57"
#Metadata query
query="id > 10 and id < 15"
#Type of Data ('data' or 'MC')
type="data"
#estimated Maximum CPU Time (in seconds)
cputime="864200"

You can then invoke gBasf2 using the steering file and it will do the rest:

./gbasf2.py -s steering_file.py

Commandline options

If you prefer to work with the commandline - eg for scripting, you can also use the following
options:

Usage:
 ./gbasf2.py (<options>|<cfgFile>)*
 Options:
 -h  --help  :  Shows this help
 -s:  --steering_file=  :  basf2 steering file
 -p:  --project=  :  Name for project
 -c:  --CPUTime=  :  estimated CPUTime (in seconds)
 -w:  --priority=  :  Job priority: 0 is default
 -m:  --query=  :  Metadata Query
 -t:  --type=  :  Type of Data ('data' or 'MC')
 -e:  --experiments=  :  Experiments (comma separated list)
 -l:  --swver=  :  Software Version

For example:

./gbasf2-diracnative.py -s steering-simpleexample.py -p Project0 -m 'id >10 and id <15' -t data -e 13,57
