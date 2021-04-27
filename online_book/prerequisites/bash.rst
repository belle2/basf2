.. _onlinebook_bash:

Command Line Tutorial (Bash)
============================

.. sidebar:: Overview
    :class: overview

    **External training**: 4h30

    **Teaching**: 30 min

    **Exercises**: 10 min

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

For this tutorial we want to focus on the Bash shell. This is the most popular
command line interpreter and can be considered the standard so we stick with it.
There is also ``zsh`` which behaves almost the same but has some advanced features
so you are welcome to try it if you want after this tutorial.

There's another type of command line interpreter called the C shell (``csh``, or
the improved version ``tcsh``) which tended to be popular among scientists in
the nineties. However, in contrast to the Bash it has severe drawbacks when
writing scripts (for more details you can look `here
<https://en.wikipedia.org/wiki/C_shell#Reception>`_).

The C shell is still around in High Energy Physics but support for it fading
out: Many experiments start removing support to use their software with it. Also
Belle II is planning remove support in the near future.

So if you already know C shell you probably should still continue with this
tutorial. And If you're new to command lines and shells you **should definitely
not learn C shell**. If your supervisor uses it that should not be your problem
😉.


The only thing you need to be able to follow this lecture is to have a Bash
available on your system. For macOS and Linux this is basically always the case
but for Windows you need to install it first.

.. admonition:: Installation on Windows
    :class: toggle

    Luckily with recent Windows versions it has become exceedingly easy to
    install a bash and use it. We recommend to follow the `Ubuntu Instructions
    <https://ubuntu.com/wsl>`_ and install the latest long term supported Linux
    (Ubuntu 20.04). You might also want to follow the tutorial to run graphical
    applications on that page but that is optional for now.

    If you're interested in a more technical description of the Windows
    Subsystem for Linux (WSL) please refer to the `Microsoft documentation
    <https://docs.microsoft.com/en-us/windows/wsl/>`_

    We also *strongly* recommend that you install the `Windows Terminal
    <https://www.microsoft.com/en-us/p/windows-terminal/9n0dx20hk701>`_ as it
    makes working with the terminal *much* easier on Windows and gives you
    basically the same features you would get on macOS or Linux.

And since the folks at Software Carpentry have already prepared a very nice
introduction to the `The Unix Shell <https://swcarpentry.github.io/shell-novice/>`_.
We would like you to go there and go through the introduction and then come back
here when you are done.

.. image:: swcarpentry_logo-blue.svg
    :target: https://swcarpentry.github.io/shell-novice/
    :alt: The Unix Shell


After this introduction you know should have a basic understanding of the shell.
One thing we need that was not covered in the introduction above and that is the
use of environment variables. So let's expand a bit on variables in general.

Shell Variables
---------------

You already learned about normal variables when learning about loops: values
can be assigned to names and we can obtain the value by putting a ``$`` in front
of the name. In the previous tutorial this was only used for loop variables and
command line arguments (``$1``, ``$2``, ...).

This concept can also be extend to user defined variables: you can very easily
define your own variables:

.. code-block:: bash

    myvariable="Some value"
    echo "I defined myvariable to ${myvariable}"

.. warning::

    You cannot have any spaces between the name, the equal sign, and the value.

This can be very helpful when writing scripts as you can assign repeated values
or command line arguments to readable names. Now the values of these variables
are "local" to the current shell: if you run a program it will not see variables
defined in this way.

.. admonition:: Exercise
    :class: exercise stacked

    Run the two lines above. Then write a shell script to just print the name
    of the variable ``$myvariable``.

.. admonition:: Solution
    :class: toggle solution

    Create a file ``print_myvariable.sh`` that just contains one line:

    .. code-block:: bash

        echo "The value of myvariable is ${myvariable}"

    and run it with ``bash print_myvariable.sh``.

    The variable should be empty so the output should just be

    .. code-block:: bash

        The value of myvariable is

You can tell the shell to export your variables to all programs you call with
the ``export`` statement. It looks basically the same as the normal variable
definition.

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

Exported variables are called environment variables and by convention they
should be always be in capital letters, so in the example above we should have
called it ``MYVARIABLE`` or maybe ``MY_VARIABLE``.

.. note::

    It is technically impossible for sub process to modify the environment
    variables of its parent process. So if you execute a script or run a program
    it cannot modify the environment variables in your shell.

    In the example above, if the script would modify
    the value of ``$myvariable`` to something else this would not have any
    affect on your the value in your current shell.


Environment Variables
---------------------

As we said, any exported variable is technically an environment variable but
some names have special meaning. The most important one is the variable
``$PATH``

.. admonition:: Exercise
    :class: exercise stacked

    Print the value of ``$PATH`` in your shell.

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

    Find out in which directory the ``ls`` program is located.

.. admonition:: Solution
    :class: solution toggle

    Running ``which ls`` should produce

    .. code-block:: bash

        /bin/ls

You can modify this ``$PATH`` to look for programs in additional directories,
for example to first look for programs in ``bin`` in your home directory you
could use:

.. code-block:: bash

    export PATH=~/bin:$PATH

.. admonition:: Question
    :class: exercise stacked

    Why do we have ``$PATH`` in the value of the variable assignment?

.. admonition:: Solution
    :class: solution toggle

    We want to add a directory to the existing ``$PATH``, not fully replace the
    value. Otherwise the shell would **only** look in our home directory for
    programs.

There are other important variables that affect the behavior of the shell. The
most important ones are:

.. envvar:: PATH

    Determines where to look for executables.

.. envvar:: LD_LIBRARY_PATH

    Similar to :envvar:`PATH` this determines where to look for shared libraries
    which might be needed by the executables.

.. envvar:: PYTHONPATH

    Similar to :envvar:`PATH` this determines where the Python scripting
    language will look for additional modules.

.. envvar:: LC_ALL

    Change the language settings in your shell. This goes together with a large
    list of "locale" variables all starting with ``LC_`` to change how numbers,
    dates, or times are formatted and how letters are sorted. :envvar:`LC_ALL`
    allows to set all of them at once.

    For example to change everything to German we could use

    .. code-block:: bash

        export LC_ALL=de_DE.utf8

    You can find out which locales are available on your system by running
    ``locale -a`` and you can see your current settings by running just ``locale``.

    .. note::

        These days you should **always** choose a locale ending in ``utf8`` to
        have support for all characters.

.. envvar:: EDITOR

    Lets you set your preferred editor to start when a program needs a text editor.
    Can be set to the executable of any editor you would like to use by default.

.. rubric:: Modifying Environment Variables in the Shell

Now to be able to use software not installed in the default locations, like for
example the Belle II software, we need to change at least :envvar:`PATH` and
:envvar:`LD_LIBRARY_PATH` but usually also set a few others.

Now we already discussed above that executing a script cannot modify the
environment variables of our current shell but it would be very inconvenient if
everyone had to copy paste instructions what to set there all the time.

Luckily there is a way to modify the environment in our shell, it's called
"sourcing" a script. It behaves almost like executing a script but all the
commands affect the current shell:

.. code-block:: bash

    source myenvvars.sh

This will read the script ``myenvvars.sh`` and execute all the commands it finds
in there in the current terminal. It is exactly equivalent to copy pasting every
single line into the terminal one by one and hitting return.

.. warning::

    While in many cases this looks almost identical to executing a script there
    can be very big differences. For example if the script contains an ``exit``
    command it will close your current terminal and not just stop executing the
    script itself.

    You should only use sourcing if you really need to modify the current shell.

There's also a short version which gets used very often but can be very
misleading: The source command can be replaced by a single ``.``, so the above
could also be written as:

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


Further reading
---------------

Bash has a lot of features and it might take a long time before you feel fully
"at home" in the command line. As with many other tools, it might feel very
clumsy at first and it takes some effort to get a feeling for its true power.
We could only show you some very basic features, but there is much more to be
discovered.
The more you know, the more the command line will become an integral part of life
for you (after some time you might be surprised with how many commands you remember).

.. admonition:: Exercise
    :class: exercise

    Search for "most useful bash commands" in your browser. Write down a
    couple of commands that you might need in the future.

.. danger::

    NEVER use a bash command you do not understand.
    As you might have seen, bash commands are a very *raw* way of interacting
    with your machine. You cannot count on being prompted for confirmation if you
    do something dangerous (the ``rm`` command just deletes, it doesn't ask for
    confirmation and there is no trash).
    So always make sure that you know what you're doing.

.. seealso::

  We have started to compile a reading list for git `on confluence <https://confluence.desy.de/x/pRG3Cg>`_.
  Please take a look (and help us extend it if you can recommend other tutorials)!

.. include:: ../lesson_footer.rstinclude

.. topic:: Author of this lesson

     Martin Ritter
