Tools for luminosity
++++++++++++++++++++

``b2info-luminosity``: Show the luminosity corresponding to a given experiment(s)
---------------------------------------------------------------------------------

This tool shows the luminosity corresponding to a given experiment (or set of
experiments) computed either online or offline. If a single experiment is
selected, also a run range can be defined.
Plot of the collected luminosity per run, per day, or per week can be created.

    usage: b2info-luminosity --exp [E] --runs [Range] --what [online|offline] --plot [DAY|WEEK|RUN] [--verbose] [--help]

.. rubric:: Arguments

--exp E           the experiment number(s) to be selected. Multiple experiment numbers are allowed, also as a range of type --exp 3,7-10 (=3,7,8,9,10). Required

.. rubric:: Optional Arguments

-h, --help        print available options
--runs Range      range of runs to be considered: syntax [AAA,BBB-CCC]. WARNING: available only if a single experiment is selected
--what arg        select onlne or offline (default) luminosity. WARNING, offline might not be available for very recent experiments
--plot type       produce a plot of integrated luminosity for each run, day, or week (type=RUN, DAY, WEEK, respoectively) of data tacking, with total integrated lumi superimposed. Both png and pdf files are created.
--verbose         print lumi information for each run
