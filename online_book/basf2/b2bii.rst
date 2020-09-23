.. _onlinebook_b2bii:

B2BII
=====

.. sidebar:: Overview
   :class: overview

   **Teaching**: ?? min

   **Exercises**: ?? min

   **Prerequisites**: 
    	
   	* None

   **Questions**:

       * Can I use basf2 to analysis Belle data?
       * Why can I use the same basf2 particle lists and variables in my b2bii analysis?
       * How to apply PID efficiency weights in my ntuple?

   **Objectives**:

       * Process Belle data with basf2

The ``b2bii`` package in basf2 converts Belle MDST files (basf data format) to
Belle II MDST (basf2 data format). This enables performing physics analysis
using data collected with Belle detector with the analysis software and
algorithms developed for the analysis of data collected with the Belle II
detector. It allows for estimation and validation of
performances of various advanced algorithms being developed for
Belle II
(such as Full Event Interpretation, Flavour Tagger, Tag Vertex, ...).


B2BII converter
---------------
The b2bii convert reads and converts Belle MDST within basf2, and
the converted data can be then analysed within the same job, without any
intermediate steps.

The workflow is illustrated in the figure:
.. image:: b2bii/workflow.png
   :width: 300px

* Reads Belle MDST file.
* Applies momentum or energy corrections in the objects.
* Save objects in basf2 format
* Time for basf2 analysis!


MC samples
----------
As there is no Belle detector description inside basf2, one needs to either
use the official Belle MC samples or generate own signal MC using basf.
The information of generating Belle MC can be found
`here <https://belle.kek.jp/secured/wiki/doku.php?id=software%3Amcprod_scripts>__`

Generic MC
~~~~~~~~~~
Generic MC is the official MC samples at Belle, which was generated with
run-dependent beam energy. There are multiple ``streams`` of these samples,
and each ``stream`` corresponds to the amount of Belle data.

But what exactly the types of MC samples are there?
Similar to Belle II MC, there are also different categories of Belle MC:
* Generic :math:`B` samples : charged (:math:`B^+ B^-`) and mixed (:math:`B^{0}\overline{B}^{0}`)
* Continuum samples  : uds, charm
* Y(5S) samples      : bsbs, nonbsbs

Generic :math:`B` samples only contain decay modes with :math:`b \to c` quark
transition, and generated based on the decay tables at
/sw/belle/belle/b20090127_0910/share/data-files/evtgenutil/DECAY.DEC.

.. rubric:: How to find Generic MC samples?

You can find the sample(s) you want through
`Belle File Search Engine <http://bweb3.cc.kek.jp/>__`

.. image:: b2bii/bweb3.png
   :width: 300px

By specifying ``Exp No``, ``Event Type``, ``Data Type``, and ``Stream No``,
``Event Type`` means different MC types (charged, mixed, uds, .. ).
``Data Type`` is for different energy runs (on-resonance, off-resonance, ...).
In total there are 10 streams of Generic :math:`B` samples and 6 streams of
continuum samples.

You can either use the file list (physical path) or URL as input file list for
b2bii jobs.

.. note::

   `Belle File Search Engine <http://bweb3.cc.kek.jp/>__` is also
   for data files.

**`Belle File Search Engine <http://bweb3.cc.kek.jp/>__` is only
   accessible within KEK domain or via VPN.**

**More information about official MC and data can be found
`here <https://belle.kek.jp/secured/wiki/doku.php?id=software:data_search>__`**

Rare MC
~~~~~~~
Just from this name you can guess that this type of MC aims for
rarer processes, such as :math:`b \to u \ell \nu`, :math:`e^+ e^- \to \tau^+ \tau^-`...

Rare :math:`B` MC samples was generated with the experiment-dependent beam
energy, but not run-dependent (i.e. The same beam energy and IP profile in
the same experiment).

Location of those special MC files can be found at
`here <https://belle.kek.jp/secured/wiki/doku.php?id=software:rare_mc_search>__`
 
Signal MC
~~~~~~~~~
As there is no Belle detector description, you can only use basf to produce
signal MC samples.

Now we will learn how to use ``mcproduzh`` package to generate signal MC in Belle.
This package was developed by "U"shiroda-san, A. "Z"upanc, and "H"orii-san, and 
it consists of generation, simulation, and reconstuction based on ``evtgen`` and
``gsim`` scripts.
It will create MC samples for a list of experiments, normalized by their
:math:`N(B\overline{B})` or integrated luminosity.

The beam energy, IP profile, and detector configuration of this MC will be
experiment-dependent, but not run-dependent.
Moreover, RunNo for these events will be set to 0, hence it doesn't
work for off-resonance or :math:`\Upsilon(nS)`.

.. rubric:: First step: copy the file are unzip it

.. code-block:: bash

   cp /home/belle/capid/public/B2SKW/mc/mcproduzh.tar.gz your_working_directory
   tar -zxvf mcproduzh.tar.gz

There will be two directories ``evtgen`` and ``gsim``, and one file ``READER``.

.. rubric:: Second step: generate events according to a decay table
Go to evtgen directory

.. code-block:: bash

   cd mcproduzh/evtgen

   ./runEvtgen nBB.txt [user-decay-table].dec [module-param-config].conf
   [TotalNomberOfEvents] [EventsPerJob]

[module-param-config].conf is for evtgen module configuration setting.
There are config setting examples in the package. For B analysis,
just choose ``Y4S.conf`` for you jobs.

In this step, you will get \*.gen files stored under 
``mcproduzh/evtgen/gen`` directory.

.. rubric:: Finally, run simulation and produce mdst file
Go to gsim directory

.. code-block:: bash

   cd mcproduzh/gsim/

   ./runGsimReco.csh [absolutePathToEvtgenGeneratorFiles/]"

**The path of evtgen files has to be absolute path.**

Now you have mdst files produced in ``mcproduzh/gsim/mdst/`` directory.


.. admonition:: Exercise
   :class: exercise stacked

      Try to generate a MC sample with 1000
      :math:`B^{+} \to \overline{D}^{0}(\to K^{+} \pi^{-}) \pi^{+}` events.


.. admonition:: Solution
   :class: toggle solution

   Generation:

   .. code-block:: bash

      cd <your_working_directory>/mcproduzh/evtgen
      ./runEvtgen nBB-Y4S.txt BptoD0pip-D0toKpi.dec Y4S.conf 1000 1000

   Simulation:

   .. code-block:: bash

      cd ../gsim
      ./runGsimReco.csh <your_working_directory>/mcproduzh/evtgen/gen/ 

More information about MC can be found
`here <https://belle.kek.jp/secured/wiki/doku.php?id=software%3Amcprod_scripts>__`


First b2bii Analysis
--------------------
Now with Belle MDST in hand, you can use it for your first b2bii analysis.
It is very simple, just add two lines in your script:

.. code-block:: python

   from b2biiConversion import convertBelleMdstToBelleIIMdst
   convertBelleMdstToBelleIIMdst(inputfile, path=mypath)

However, there are many difference between Belle detector and Belle II detector,
as well as basf and basf2.






.. admonition:: Key points
   :class: key-points

   * Making basf2 process Belle data is as easy as adding 
     ``convertBelleMdstToBelle2Mdst()`` to the top of your steering file.
   * Be careful with: particle lists and variables in your analysis.


.. topic:: Author of this lesson

   Chia-Ling Hsu

