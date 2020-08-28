.. _onlinebook_basf2_introduction:

The basics.
===========

.. sidebar:: Overview
    :class: overview

    **Teaching**: 20 min

    **Exercises**: 10-20 min

    **Prerequisites**: 
    	
        * An internet browser
        * Collaborative services tutorial
        * DESY accounts
        * An ``ssh`` client
    	* Basic bash

    **Questions**:

        * What is basf2 and what does it do?
        * What is a basf2 module, path, steering file?
        * How can I get help?

    **Objectives**:

        * Correctly set up the Belle II environment.
        * Find the right chapters in this documentation.
        * Run basf2 and display a list of variables
        * Run basf2 in interactive ipython shell mode.
        * Access the help in 3 different ways.


The Belle II software is called basf2.
It is an abbreviation for "Belle II Analysis Software Framework".
There is no set way to write it, you will see BASF2, Basf2 and basf2.
You might wonder why we didn't choose "b2asf", and when you get a bit further
you will probably wonder why it has "analysis" in the name (it does much more
than analysis)? 
Well historic reasons: Belle had BASF, we have basf2.

Basf2 is used in all aspects of the data-processing chain except, perhaps, for
the very final "offline" analysis.
There is a citeable reference:

Kuhr, T. *et al*. *Comput Softw Big Sci* **3**, 1 (2019)
https://doi.org/10.1007/s41781-018-0017-9

... and a logo.

.. figure:: ../../../../_static/b2logo.svg
    :align: center
    :width: 300px
    :alt: The basf2 logo.

    The basf2 logo.

Pragmatically, you will encounter two separate objects named ``basf2``.
It is both a **command-line executable which you can envoke**, and a **python
module** from which you import functions.

You will soon be running commands that look like:

.. code:: bash

        basf2 myScript -i myInputFile.root

... and *inside* the scripts you might see code like:

.. code:: python

        from basf2 import Path
        mypath = Path()


Core concepts
-------------

There are some concepts we use in basf2, which you will definitely need to 
understand.
These are:

        * basf2 module,
        * path,
        * package,
        * steering script / steering file.

Most of the other jargon terms we use are generic software development terms
(so you can search the internet).
A good place to look for Belle II-specific jargon is the **Belle II Glossary**.

.. admonition:: Exercise
     :class: exercise stacked

     Find the Belle II Glossary.

.. admonition:: Hint
     :class: toggle xhint stacked

     You might need to revisit the tutorial pages about Collaborative services.

.. admonition:: Solution
     :class: toggle solution

     It's on confluence! 
     `Belle II Glossary <https://confluence.desy.de/display/BI/Main+Glossary>`_

Basf2 modules
~~~~~~~~~~~~~

A basf2 module is piece of (usually) C++ code that does a specific "unit" of
data processing.
The full documentation for module can be found here in this website under
the section :ref:`general_modpath`.

.. warning:: 

    It is an unfortunate clash of naming that `python uses the word
    "module" <https://docs.python.org/3/tutorial/modules.html>`_
    for a separate concept.
    In these tutorials we will always specify **python module** (and **basf2
    module**) if there is ambiguity.

Path
~~~~

A basf2 path is an ordered list of modules that will be used to process the
data.
You can think of building a path by adding modules in a chain.
It is a python object: `basf2.Path`.

.. warning::

    A common misconception is that adding modules to a path is processing
    data.
    This is not true, you will **prepare** your path for data-processing by
    adding modules.
    The event-loop starts when you process your path.

.. admonition:: Exercise
     :class: exercise stacked

     Find a diagram of a path with modules in this documentation.

.. admonition:: Hint
     :class: toggle xhint stacked

     I have already given you a link to the relevant page.

.. admonition:: Solution
     :class: toggle solution

     Take a look at the :doc:`Belle II Python Interface` page.
     The diagram is `here <framework_modpath_diagram>`.


Package
~~~~~~~

A package is a logical collection of code in basf2.
A typical package has several modules and some python scripts which configure
paths to do common things.

You will encounter some basf2 packages in these lessons.
We try to give them meaningful names (tracking, reconstruction, ...) 
or name the package after the subdetector that they are related to (ecl, klm,
cdc, top, ...).

.. admonition:: Exercise
     :class: exercise stacked

     Find the source code and find a list of all packages.

.. admonition:: Hint
     :class: toggle xhint stacked

     You might need to revisit the tutorial pages about collaborative services.
     

.. admonition:: Solution
     :class: toggle solution

     The source code is online at https://stash.desy.de/projects/B2/software
     The list of **packages** is simply the list of directories in the software
     director.

Steering
~~~~~~~~

A steering file or a steering script is some python code that sets up some
analysis or data-processing task.
A typical steering file will declare a path, configure basf2 modules, and then
add them to the path.
Then it will call `basf2.process` and maybe print some information.
We use the word "steering" since no real data **processing** is done in python.

Generally speaking, the heavy data processing tasks are done in C++, and python
is used as a userfriendly and readable language for configuration.
There are some exceptions, some modules are written in python for instance, 
but they are not very common.

The conditions database / CondDB
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is one more concept that you might hear people talk about.
You probably do not need to know this right now, but be aware of this jargon.
Ideally you, as a user, should not need to worry about this and definitely not
at the start.

But! You now have the skills to find and read the documentation for this.

.. admonition:: Key points
    :class: key-points

    * basf2 is the name of the Belle II software.
    * You work in basf2 by adding modules to a path.
    * Most basf2 modules are written in C++.
    * Data processing happens when you process the path.
    * You do this configuration of the path, and then call `basf2.process` in 
      a *steering file*.
    * You can navigate this online documentation and search for things the
      confluence pages.

.. seealso:: 

    After you've progressed a bit more through these lessons, you should
    revisit the :ref:`general_modpath` documentation page and
    reread the opening paragraphs.

    By that stage everything should be clear.


Getting started, and getting help interactively
-----------------------------------------------

Now let's setup the environment, actually execute ``basf2``, and navigate the
command line help.

Please ``ssh`` onto your favourite site.
If you do not have a preference, you should connect to ``login.cc.kek.jp``.

Before we start though...

You shouldn't need to install anything
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A common misconception by newcommers (and even by senior people in the 
collaboration), is that you need to "install" basf2 or "install a release".

You almost certainly **do not** want or need to do this.
If you are working at KEK (for certain) and at many many other sites, basf2
is available preinstalled.
It is distributed by something called `/cvmfs <https://cernvm.cern.ch/fs/>`_.

b2setup
~~~~~~~

To set up your environment to work with ``basf2`` you first have to source the
setup script...

.. code:: bash

        source /cvmfs/belle.cern.ch/tools/b2setup

Some people like to put an alias to the setup script in their .bashrc file.
You are welcome to do this if you like.

So now you have a Belle II environment.
You might have noticed that you still don't have the ``basf2`` executable:

.. code:: bash

        $> source /cvmfs/belle.cern.ch/tools/b2setup
        Belle II software tools set up at: /cvmfs/belle.cern.ch/tools
        $> basf2
        command not found: basf2

You need to choose a release (a specific version of of the software).
If you don't know what release you want, you should take the latest stable
full release or the latest light release.

There is a command-line tool to help with this.
Try:

.. code:: bash

        b2help-releases --help

or just:

.. code:: bash

        b2help-releases

To setup the release of your choice simply call :doc:`b2setup` again.
Once you've called it once, the :doc:`b2setup` executable will be in your
``PATH``:

.. code:: bash

        b2setup <your choice of release>

If you already know what release you want, you can do it all at once:

.. code:: bash

        source /cvmfs/belle.cern.ch/tools/b2setup <your choice of release>

Note that if you setup an unsupported, old, or strange release you should see a
warning:

.. code:: bash

        $> b2setup release-01-02-09
        Environment setup for release: release-01-02-09
        Central release directory    : /cvmfs/belle.cern.ch/el7/releases/release-01-02-09
        Warning: The release release-01-02-09 is not supported any more. Please update to ...
        
Sometimes people have good reason to use old releases but you should know that
you will get limited help and support if you are using a very old version.
And you expose yourself to strange bugs that will not be fixed (because they
are fixed in some later release).

It is also true that using the latest supported release makes you cool.

.. admonition:: Exercise
     :class: exercise stacked

     There is a detailed page in this documentation describing the differences
     between a full releasee and a light release and also a Belle II question.

.. admonition:: Hint
     :class: toggle xhint stacked

     There is no hint. 
     You've got this.

.. admonition:: Solution
     :class: toggle solution

     Choosing a release under the section :ref:`cvmfs-setup`.

.. admonition:: Question
     :class: exercise stacked

     What is semantic versioning?

.. admonition:: Hint
     :class: toggle xhint stacked

     This is jargon but it is not specific to Belle II.

.. admonition:: Solution
     :class: toggle solution

     A rule for version numbers.
     See the summary at https://semver.org

.. admonition:: Question
     :class: exercise stacked

     If you have code that worked in ``release-AA-00-00`` will it work in
     ``release-AA-01-00`` ?

.. admonition:: Solution
     :class: toggle solution

     **Yes**.
     There should not be anything that breaks backward compativility between minor versions.

.. admonition:: Question
     :class: exercise stacked

     If you have code that worked in ``release-AA-00-00`` will it work in
     ``release-BB-00-00`` ?

.. admonition:: Solution
     :class: toggle solution

     **No, it is not guaranteed**.
     Unfortunately there is no guarantee of backward compatibility between major versions.
     And for good reason: sometimes things need to be changed to introduce new features.

.. admonition:: Question
     :class: exercise stacked

     If you have code that worked in ``light-5501-future`` will it work in
     ``light-5602-reallyfarfuture`` ?

.. admonition:: Solution
     :class: toggle solution

     **No, it is not guaranteed**.
     Unfortunately there is no guarantee of backward compatibility between light releases.
     And for good reason: sometimes things need to be changed to introduce new features.
     For more information:
     `Do light releases break backward compatibility? <https://questions.belle2.org/question/2841/do-light-releases-break-backward-compatibility/>`_.

A useful command
~~~~~~~~~~~~~~~~

If you're ever stuck and you are writing in a B2Questions or an email to an
expert they will always want to know what version you are using.

Try 

.. code:: bash

    basf2 --info

to check everything was set up corrctly.
If that worked, then paste the information at the bottom (after the ascii art)
into any correspondence with experts.

Help at the command line
~~~~~~~~~~~~~~~~~~~~~~~~

There are quite a lot of standard python tools/ways to get you help at the
command line or in an interactive environment.
The Belle II environment supports
`pydoc3 <https://docs.python.org/3/library/pydoc.html>`_.

Try:

.. code:: bash

    pydoc3 basf2.Path

You should notice that this is the same documentation that you will find by
clicking on: `basf2.Path` here in this online documentation.

There are some basf2-specific command-line tools.

Listing the basf2 modules
^^^^^^^^^^^^^^^^^^^^^^^^^

To find information about a basf2 module, try:

.. code:: bash

    basf2 -m # this lists all of them
    basf2 -m | grep "Particle"
    basf2 -m ParticleCombiner

Listing the basf2 variables
^^^^^^^^^^^^^^^^^^^^^^^^^^^

To check the list of basf2 variables known to the :doc:`VariableManager`, run

.. code:: bash

    basf2 variables.py
    basf2 variables.py | grep "invariant"

There is a :doc:`variables` section in this documentation which you might find
more helpful than the big dump.

Listing the modular analysis convenience functions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

We have a python module full of useful shorthand functions which configure
basf2 modules in the recommended way.
It is called `modularAnalysis`.
In a later lesson you will *use* these convenience functions.

For now, you can list them all with:

.. code:: bash
    
    basf2 modularAnalysis.py

Basf2 particles
^^^^^^^^^^^^^^^

To show information about all the particles and properties known to basf2,
there is a tool `b2help-particles`.

.. code:: bash

    b2help-particles B_s      # what was the pdg cod of the B-sub-s meson again?
    b2help-particles Sigma_b- # I've forgotten the mass of the Sigma_b- !
    #b2help-particles         # lists them all

Other useful features
^^^^^^^^^^^^^^^^^^^^^

If you just execute basf2 without any arguments, you will start an 
`IPython <https://ipython.org>`_ session with many basf2 functions imported.
Try just:

.. code:: bash

    basf2

You can also try the basf2 python interface to the `PDG <https://pdg.lbl.gov>`_
database:

.. code:: python

   import pdg
   whatisthis = pdg.get(11)
   print(whatisthis.GetName(), whatisthis.Mass())

You should also make use of IPython's built-in documentation features.

.. code:: python

   import modularAnalysis 
   modularAnalysis.inputMdst?
   # the question mark brings up the function documentation

   print(dir(modularAnalysis)) # the python dir() function will also show you all functions' names

You can remind yourself of the documentation for a `basf2.Path` in yet another way:

.. code:: python

   Path?
   # the question mark brings up the function documentation

   print(help(Path))

To leave interactive basf2 / IPython, simply:

.. code:: python

   exit()
   # or just
   exit 

.. admonition:: Question
     :class: exercise stacked

     What was the luminosity collected in experiment 8?

.. admonition:: Hint
     :class: toggle xhint stacked

     There is a command line tool for that.
     Try tabcompletion ``b2<tab>``.

.. admonition:: Another hint
     :class: toggle xhint stacked

     .. code:: bash 
         
         b2info-<tab>

.. admonition:: Are you sure you really need another hint?
     :class: toggle xhint stacked

     .. code:: bash 
     
          b2info-luminosity --help

.. admonition:: Solution
     :class: toggle solution

     .. code:: bash

          $> b2info-luminosity  --exp 8 --what offline
          Read 697 runs for experiment 8
          TOTAL offline   : L = 5464553.60 /nb =  5464.55 /pb =    5.465 /fb =   0.0055 /ab

     So the answer is :math:`\sim 5.5\textrm{ fb}^{-1}`.

.. admonition:: Key points
    :class: key-points

    * ``b2setup`` sets up the environment.
    * You need to setup a specific release and you should try and keep up-to-date.
    * ``b2help-releases``
    * ``b2setup <choose a release>``
    * ``b2help-particles``
    * Basf2 has a python interface. You can use python tools to find help.
    * ``basf2`` without any tools gets you into a basf2-flavoured IPython shell.


Congratulations!
You are now ready to write your first steering file.
Good luck.

.. topic:: Author of this lesson

    Sam Cunliffe (sam.cunliffe@desy.de)

