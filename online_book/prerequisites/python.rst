.. _onlinebook_python:

Python
======

.. sidebar:: Overview
    :class: overview

    **External Training**: about 7 hours.

    **Teaching**: 5 min

    **Exercises**: 0 min

    **Prerequisites**:

    	* None

    **Questions**:

        * What are the key concepts of python?
        * How can I process tabular data?
        * How can I plot data?
        * How can I define functions in python?

    **Objectives**:

        * Get familiar with python

High Energy Physics (HEP) analyses are too complex to be done with pen, paper
and calculator. They usually are not even suited for spreadsheet programs like
Excel. There are multiple reasons for this. For one the data size is usually
larger than paper or spreadsheets can handle. But also the steps we take are
quite complex. You are of course welcome to try but we really don't recommend
it.

So what we need is something more powerful. For many years the HEP community
believed that only very fast and complex programming languages are powerful
enough to handle our data. So most students needed to start with C++ or even
Fortran.

And while there's nothing wrong with those languages once you mastered them, the
learning curve is very long and steep. Issues with the language have been known
to take a major fraction of students time with frustrating issues like:

* Why does it not compile?
* It crashes with an error called "segmentation violation", what's that?
* Somehow it used up all my memory and I didn't even load the data file yet.

And while it's true that once the program was finally done running the analysis
be very fast it is not necessarily efficient. Spending half a year for on a
program for it to finish in an hour instead of one month development and have it
finish in a day is maybe not the best use of students time.

So in recent years HEP has started moving to Python for analysis use: It is very
easy to learn and has very nice scientific libraries to do all kinds of things.
Some people still say python is way too slow and if you misuse it that is
certainly true. But if used correctly python is usually much easier to write and
can achieve comparable if not better speeds when compared to naive C++
implementations. Yes, if you are a master of C++ nothing can beat your execution
speed but the language is very hard to master. In contrast Python offers
sophisticated and optimized libraries for basically all relevant use cases.
Usually these include optimizations that would take years to implement in C++,
like GPU support.

Think about it: almost all of the billion dollar industry that is machine
learning is done in Python and they would not do that if it would not be
efficient.

Consequently in Belle II we make heavy use of Python which means you will need
to be familiar with it. By now you probably know what's coming next.

Luckily there is a very large amount of good python tutorials out there. We'll
stick with Software Carpentry and their `Programming with Python
<https://swcarpentry.github.io/python-novice-inflammation/>`_  introduction. We
would like you to go there and go through the introduction and then come back
here when you are done.

.. image:: swcarpentry_logo-blue.svg
    :target: https://swcarpentry.github.io/python-novice-inflammation/
    :alt: Programming with Python

.. needed from our side: Opening ROOT files; using pandas


.. topic:: Author of this lesson

     Martin Ritter
