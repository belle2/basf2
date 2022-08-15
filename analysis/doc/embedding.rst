.. _Embedding:

Signal Embedding
================

Most Belle II analyses use machine learning methods that increase signal purity.  The classifiers are typically trained using simulated samples and require validation with data. This validation is arranged using a control channel, classifier efficiency for which is compared between data and simulation.  However, kinematic differences between the signal and control channel lead to different classifier output. The high-purity and low-efficiency region that is usually employed in analyses is hard to model in particular making efficiency correction based on a control channel unreliable.

Signal embeddind is designed to address this issue. It is assumed that signal decay reconstruction efficiency before the classifier can be studied by other means. The key question is to test that the rest-of-event (ROE) information that is used by the classifier is modelled adequatelly and provide a correction, if not. 


Signal emedding uses two inputs. The first input consists of the control channel, reconstructed in data and simulation and stored in a dediscated udst skim. The control channel decay list must be stored in the file. The control channel should contain full kinematic information of a B decay, i.e. it should be a hadronic tag such as code: `B+ -> K+ J/psi (mu+ mu-)`.  The second input is a dedicated skim of the signal simulation. The signal decay list should be truth-matched and this list should be also stored in the file. 

The embedding software takes the first file and removes the object associated with the control channel decay list, leaving ROE. For the second file, objects that are not associated with the signal channel decay list are removed.  The files are then merged together and the kinematics of the signal decay is adjusted such that it matches the control channel tag. 

Assuming the skims are prepared, the removal of the control channel/ROE is arranged by code: `udst_purge_list.py` script. E.g.

.. code-block:: shell
	python3 analysis/scripts/udst_purge_list.py --listName "B+:BplusToKplusJpsiMuMu"  --charge 'pos' --fileIn /home/glazov/belle/data/skims_moriond/skim_BplusToKplusJpsiMuMu_bucket17.root
	python3 analysis/scripts/udst_purge_list.py --isSignal --listName "B+:BplusToKplusNuNu"  --charge 'pos' --fileIn /home/glazov/belle/data/skims_moriond/sig/skim_sig_71_Bplus2Kplus.udst.root

..
