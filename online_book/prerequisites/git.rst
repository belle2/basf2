.. _onlinebook_git:

Version Control with Git
========================

.. sidebar:: Overview
    :class: overview

    **External Training**: about 3 hours.

    **Teaching**: 10 min

    **Exercises**: 0 min

    **Prerequisites**:

    	* Basic bash knowledge

    **Questions**:

        * How to collaboratively develop software?
        * How to keep different versions of my code?
        * How to synchronize my code between servers?

    **Objectives**:

        * Learn to use the world's most successful version control system
        * Get ready to contribute to Belle 2 software

Creating a physics analysis or developing a piece of software is not a straight
forward process without errors. There will be a lot of changes to the scripts,
notes, documents, code over time.

Sometimes the changes will be wrong and we need to fix these errors. Sometimes
we might get feedback to a specific version of our paper but we might have
changed this part already and we need to figure out what still applies. And
sometimes we might get an email like this

    Hi,

    2 years ago you presented This Thing™ at That Meeting™ and I'm trying to do
    the same, could you send me the script you used back then?

    Best Regards,

    -- Poor Student

Now one way to achieve this is would maybe be to keep all old versions manually,
either in the same file but commented out like this:

.. code-block:: python

    a = 5
    # a += 6  wrong, changed April 2019
    # a += 7  still wrong, changed May 2019
    a += 8
    # print(f"value is {a/2}")  no longer needed, May 2019

Or by keeping old versions of the file manually and create ``thesis.tex``,
``thesis-v2.tex``, ``thesis-v2.1.tex``, ``thesis-v2.2-final.tex``,
``thesis-v2.2-final-really.tex`` ...

Or you could use a version control software to do this. And *the* version
control system to use these days is called Git. It helps you keep track of the
changes over time and more importantly, is basically essential to be able to
effectively work together with colleagues on the same project.

Git is everywhere in the software industry but also in science it is by now the
de-facto standard. It can and will make your life much, much easier but first
you will learn how to use it.

Luckily there is a very large amount of good git tutorials out there. We'll
stick with Software Carpentry and their `Version Control with Git
<https://swcarpentry.github.io/git-novice/>`_  introduction. We would like you
to go there and go through the introduction and then come back here when you are
done.

.. image:: swcarpentry_logo-blue.svg
    :target: https://swcarpentry.github.io/git-novice/
    :alt: Version Control with Git

Afterwards we can quickly go to the Belle II specific things you might need to
be aware of.

Belle II Specifics
------------------

In Belle II we use Bitbucket Server by Atlassian for our git repositories. While
this is rather similar to github in some regards. It is strongly advised to use
``ssh`` as access protocol and for that you need to upload your public key to
https://stash.desy.de.

You should already have a ssh key and you should copy the the contents of the
public key file and upload it `here
<https://stash.desy.de/plugins/servlet/ssh/account/keys>`_.

You should also make sure that you configure your git to username and email to
something your Collaborators can recognize. The best is to use the email address
you used when registering your DESY account or alternatively the DESY email
address you got with your account.

.. note::

    If you want to commit to the main Belle II software repository the email is
    actually validated to be one of the two options above.

We also have a `confluence page <https://confluence.desy.de/x/2o4iAg>`_ with
more information and help in case of problems.


.. topic:: Author of this lesson

     Martin Ritter
