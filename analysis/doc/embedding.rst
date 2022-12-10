.. _Embedding:

Signal Embedding
================

Most Belle II analyses use machine learning methods that increase signal purity.  The classifiers are typically trained using simulated samples and require validation with data. This validation is arranged using a control channel, for which the classifier efficiency is compared between data and simulation.  However, kinematic differences between the signal and control channel lead to different classifier output. The high-purity and low-efficiency region that is usually employed in analyses is hard to model in particular making efficiency correction based on a control channel unreliable.

Signal embedding is designed to address this issue. It is assumed that signal decay reconstruction efficiency before the classifier can be studied by other means. The key question is to test that the rest-of-event (ROE) information that is used by the classifier is modelled adequately and provide a correction, if not. 

Signal embedding uses two inputs. The first input consists of the control channel, reconstructed in data and simulation and stored in a dedicated udst skim. The control channel decay list must be stored in the file. The control channel should contain full kinematic information of a B decay, i.e. it should be a hadronic tag such as :math:`B^+ \to K^+ J/\psi (\mu^+ \mu^-)`.  The second input is a dedicated skim of the signal simulation. The signal decay list should be truth-matched and this list should be also stored in the file. 

The embedding software takes the first file and removes the object associated with the control channel decay list, leaving ROE. For the second file, objects that are not associated with the signal channel decay list are removed.  The files are then merged and the kinematics of the signal decay is adjusted such that it matches the control channel tag. 

Assuming the skims are prepared, the removal of the control channel/ROE is arranged by :code:`udst_purge_list.py` script. E.g.

.. code-block::

   python3 analysis/scripts/udst_purge_list.py --listName "B+:BplusToKplusJpsiMuMu"  --charge 'pos' --fileIn $BELLE2_EXAMPLES_DATA_DIR/embedding_skim_BplusToKplusJpsiMuMu_bucket17.udst.root --fileOut data.root
   python3 analysis/scripts/udst_purge_list.py --isSignal --listName "B+:BplusToKplusNuNu"  --charge 'pos' --fileIn $BELLE2_EXAMPLES_DATA_DIR/embedding_skim_sig_71_Bplus2Kplus.udst.root --fileOut sigmc.root

..

Here  :code:`B+:BplusToKplusJpsiMuM` and  :code:`B+:BplusToKplusNuNu` are the list names for the reconstructed :math:`B^+ \to K^+ J/\psi (\mu^+ \mu^-)` decay and truth-matched :math:`B^+ \to K^+ \nu \bar{\nu}` decay, respectively. The two output files are :code:`data.root` and :code:`sigmc.root`. Note that apart from selecting objects, the script performs vertex fits of :code:`listName` and stores the fit results. Events with multiple B candidates and failed vertex fits are removed. 

The two output files can be then merged using

.. code-block::

   python3 analysis/scripts/embedding_merge.py --filePrimary data.root --fileSecondary sigmc.root --fileOut embedded_data.root

..

Apart from merging the input files, the script performs kinematic corrections for the signal decay, to match it with the tag kinematics. The results are stored in :code:`fileOut` file. This file is ready to be processed by the standard analysis scripts.

Documentation of the scripts:
-----------------------------

.. argparse::
    :filename: analysis/scripts/udst_purge_list.py
    :func: get_parser
    :prog: udst_purge_list.py
    :nodefault:

.. argparse::
    :filename: analysis/scripts/embedding_merge.py
    :func: get_parser
    :prog: embedding_merge.py
    :nodefault:
