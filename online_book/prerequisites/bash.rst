.. _onlinebook_bash:

Command Line Toturial (Bash)
============================

.. sidebar:: Overview
    :class: overview

    **Teaching**: external tutorial + 30 min

    **Exercises**: external tutorial + 10 min

    **Prerequisites**:

    	* None

    **Questions**:

        * What is a command shell and why would I use one?
        * How to navigate files and directories?
        * How to create, copy, delete, edit files?
        * How to combine commands?
        * How can I find files and contents?
        * What are environment variables?

    **Objectives**:

        * Get familiar with bash

In High Energy Physics we need to work a lot with the command line. This has
multiple reasons, one is that it's a lot of work to create graphical user
interfaces but the most important one is that it is more efficient to work with
once you get used to it.

The good thing is that this is not Belle II specific so there are very good
tutorials out there we can just use.

The only thing you need to be able to follow these tutorials is to have a Bash
available on your system. For MaxOSX and Linux this is basically always the case
but for Windows you need to install it first.

.. admonition:: Installation on Windows
    :class: toggle

    Luckily with recent Windows versions it has become exceedingly easy to
    install a bash and use it. We recommend to follow the `Ubuntu Instructions
    <https://ubuntu.com/wsl>`_ and install the latest long term supported Linux
    (Ubuntu 20.04). You might also want to follow the tutorial to run graphical
    applications on that page but that is optional for now.

    If you're interested in a more technical description of the Windows
    subsystem for Linux please refer to the `Microsoft documentation
    <https://docs.microsoft.com/en-us/windows/wsl/>`_

    We also *strongly* recommend that you install the `Windows Terminal
    <https://www.microsoft.com/en-us/p/windows-terminal/9n0dx20hk701>`_ as it
    makes working with the terminal *much* easier on windows and gives you
    basically the same features you would get on MaxOSX or Linux.

Once you have a bash on your system please follow `this tutorial
<http://swcarpentry.github.io/shell-novice/>`_ first and come back here when
you're done.

You know should have a basic understanding of the shell but one thing we need
that was not covered in the introduction above is the use of environment
variables. So let's expand a bit on variables in general.

Shell Variables
---------------

You already learned about normal variables when learning about loops: Values
can be assigned to names and we can obtain the value by putting a ``$`` in front
of the name. In the previous tutorial this was only used for loop variables and
command line arguments (``$1``, ``$2``, ...).

This concept can also be extend to user defined variables: you can very easily
define your own variables

.. code-block:: bash

    myvariable="Some value"
    echo "I defined myvariable to ${myvariable}"

.. warning::

    You cannot have any spaces between the name, the equal sign, and the value

This can be very helpful when writing scripts as you can assign repeated Values
or command line arguments to readable names. Now the values of these variables
are "local" to the current shell: If you run a program it will not see variables
defined in this way.

.. admonition:: Exercise
    :class: exercise stacked

    Run the two lines above. Then write a shell script to just print the name
    of the variable ``$myvariable``

.. admonition:: Solution
    :class: toggle solution

    Create a file ``print_myvariable.sh`` that just contains

    .. code-block:: bash

        echo "The value of myvariable is ${myvariable}"

    and run it with ``bash print_myvariable.sh``.

    The variable should be empty so the output should just be.

    .. code-block:: bash

        The value of myvariable is

    In fact it should not exist at all but the difference between empty and
    doesn't exist is very small.


You can tell the shell to export your variables to all programs you call with
the ``export`` statement. It looks basically the same as the normal variable
definition

.. code-block:: bash

    export myvariable="Some value"
    echo "I defined myvariable to ${myvariable}"

.. admonition:: Exercise
    :class: exercise stacked

    Run the two lines above. Then execute the shell script from the previous
    exercise again.

.. admonition:: Solution
    :class: toggle solution

    Now the script should show the value you assigned so the output should be

    .. code-block:: bash

        The value of myvariable is Some value

Exported variables are called environment variables any by convention they
should be always be in capital letters, so in the example above we should have
called it ``MYVARIABLE``.

.. note::

    There is no way to modify the environment variables of the parent. A shell
    script cannot have any effect on the shell that started it. In the example
    above, there is no technical way that any script would modify the value of
    ``$myvariable`` in your main shell.


Environment Variables
---------------------

As we said, any exported variable is technically an environment variable but
some names have special meaning. The most important one is the variable
``$PATH``

.. admonition:: Exercise
    :class: exercise stacked

    print the value of ``$PATH`` in your shell

.. admonition:: Solution
    :class: solution toggle

    You should see a number of directory names separated by colon, for example

    .. code-block:: bash

        /usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

    The exact value depends on your operating system and might be different.

The ``$PATH`` variable determines where the shell looks for programs to execute.
If you tell the shell to execute any program it will go through all directories
in this list one by one and look for a program by that name. If it finds one it
will execute it, otherwise it will complain that it cannot find it.

We can find out in which the shell found a certain program by using ``which``.
This command will print the full path to the program that would be called.

.. admonition:: Exercise
    :class: exercise stacked

    Find out in which directory the `ls` program is located.

.. admonition:: Solution
    :class: solution toggle

    Running ``which ls`` should produce

    .. code-block:: bash

        /bin/ls

You can modify this ``$PATH`` to look for programs in additional directories,
for example to first look for programs in ``bin`` in your home directory you
could use

.. code-block:: bash

    export PATH=~/bin:$PATH

.. admonition:: Question
    :class: exercise stacked

    Why do we have ``$PATH`` in the value of the variable assignment

.. admonition:: Solution
    :class: solution toggle

    We want to add a directory to the existing ``$PATH``, not fully replace the
    value. Otherwise the shell would **only** look in our home directory for
    programs.

There are other important variables that affect the behavior of the shell. The
most important ones are

.. envvar:: PATH

    Determines where to look for executables

.. envvar:: LD_LIBRARY_PATH

    Similar to :envvar:`PATH` this determines where to look for shared libraries
    which might be needed by the executables

.. envvar:: PYTHONPATH

    Similar to :envvar:`PATH` this determines where the Python scripting
    language will look for additional modules

.. envvar:: LC_ALL

    Change the language settings in your shell. This goes together with a large
    list of "locale" variables all starting with ``LC_`` to change how numbers,
    dates, or times are formatted and how letters are sorted. :envvar:`LC_ALL`
    allows to set all of them at once.

    For example to change everything to German we could use

    .. code-block:: bash

        export LC_ALL=de_DE.utf8

    You can find out which locales are available on your system by running
    ``locale -a`` and you can see your current settings by running just ``locale``

    .. note::

        These days you should **always** choose a locale ending in ``utf8`` to
        have support for all characters.

.. envvar:: EDITOR

    Lets you set your preferred editor to start when a progam needs a text editor.
    Can be set to the executable of any editor you would like to use by default.

.. rubric:: Modifying Environment Variables in the Shell

Now to be able to use software not installed in the default locations, like for
example the Belle II software, we need to change at least :envvar:`PATH` and
:envvar:`LD_LIBRARY_PATH` but also set a few others.

Now we already discussed that a script cannot modify the environment variables
of our current shell but it would be very inconvenient if everyone had to copy
paste instructions what to set there all the time.

Luckily there is a way to modify the environment in our shell, it's called
"sourcing" a script. It behaves almost like executing a script but all the
commands affect the current shell:

.. code-block:: bash

    source myenvvars.sh

This will read the script ``myenvvars.sh`` and execute all the commands it finds
in there in the current terminal. It is equivalent to copy pasting every single
line into the terminal one by one.

.. warning::

    While this looks almost identical in many cases to executing a script there
    can be very big differences. For example if the script contains an ``exit``
    command it will close your current terminal and not just stop executing the
    script.

    You should only use sourcing if you really need to modify the current shell.

There's also a short version which gets used very often but can be very
misleading: The source command can be replaced by a single ``.``, so the above
would be

.. code-block:: bash

    . myenvvars.sh

.. note::

    There needs to be a space between the ``.`` and the script name. We
    recommend to use ``source`` wherever possible as it is much clearer to
    understand and avoids mistakes.

.. admonition:: Key points
    :class: key-points

    * variables in bash can be created by simply writing ``name=value``
    * to make them available to called programs they need to be exported via
      ``export name=value``
    * executed scripts cannot affect variables in the main shell
    * exported variables are called environment variables
    * there are a few important environment variables like :envvar:`PATH`
    * variables in the current shell can be modified by sourcing a script.
