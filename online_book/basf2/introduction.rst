.. _onlinebook_basf2_introduction:

What is basf2
=============

.. include:: ../todo.rst

.. sidebar:: Overview
    :class: overview

    **Teaching**: 

    **Exercises**: 

    **Prerequisites**: 
    	
    	* Basic bash
    	* Basic python

    **Questions**:

        * What is basf2 and what does it do?
        * When does basf2 *not* do?
        * How do I install basf2?
        * What is a basf2 module?
        * What is this "path"?
        * What is a steering file?
        * What is a ROOT file?
        * Where does basf2 "live"?
        * How can I find out what a module is doing?
        * 

    **Objectives**:

        * Find the right chapters in this documentation
        * Run basf2 and display a list of variables


BASF2 is an abbreviation of the Belle II Analysis Software Framework. The collaboration uses this framework at all steps of data processing - from the collection of raw hits in the detector to providing user ntuples. You can use this link_ to cite the framework in your Belle II paper.

.. _link: https://arxiv.org/abs/1809.04299

.. admonition:: Key points
    :class: key-points

    * basf2 is used to reconstruct particles from raw data
    * basf2 works by adding modules to a path
    * most modules are written in C++
    * There exist many "convenience functions" that add modules to the path for you
    * the output ``*.root`` files can be analyzed by any tool of your choosing

