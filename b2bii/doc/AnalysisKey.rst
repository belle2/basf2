.. _analysiskey:

-------------------------------------
PID corrections and systematic errors
-------------------------------------
Efficiency correction factors and systematic errors for Belle PID
were obtained using corresponding MC and data samples.
Those studies were done for different PID selections, and 
possible selections differ for different hypotheses.

There are three groups of lookup tables in Belle:

* Lepton ID:
   * Possible cuts for electron: [0.01, 0.10, 0.50, 0.60, 0.80, 0.90]
   * Possible cuts for muons: [0.10, 0.80, 0.90, 0.95, 0.97]
* Kaon/:math:`\pi` ID:
   * PID cut can be 0.1 - 0.9
* Proton ID:
   * Possible cuts for proton: [0.6, 0.7, 0.8, 0.9]

Details of these tables and how they were studies can be found at 
`Belle PID Joint Homepage <https://belle.kek.jp/group/pid_joint/>`_.

The weights for LID and KID are available in basf2 and can be retrieved via payloads in ``Belle_PID``;
naming schemes of each PID payloads are slightly different.


.. rubric:: Lepton ID

Naming scheme for LID is ``BelleLID<e|mu>_<LID cut>``.
Using ``eIDBelle>0.9`` as an example:

.. code-block:: python3

   import variables as va
   import basf2 as b2

   b2.conditions.prepend_globaltag("BellePID")

   lid_table = "BelleLIDe_0.90"
   va.variables.addAlias('LID_ratio', f"extraInfo({lid_table}_ratio)")
   va.variables.addAlias('LID_ratio_stat_err', f"extraInfo({lid_table}_ratio_stat_err)")
   va.variables.addAlias('LID_ratio_syst_err', f"extraInfo({lid_table}_ratio_syst_err)")
   va.variables.addAlias('LID_ratio_syst_err_from_2photon', f"extraInfo({lid_table}_ratio_syst_err_from_2photon)")
   va.variables.addAlias('LID_ratio_syst_err_from_JPsi', f"extraInfo({lid_table}_ratio_syst_err_from_JPsi)")
   va.variables.addAlias('LID_run_bin', f"extraInfo({lid_table}_run_bin)")
   va.variables.addAlias('LID_kinematic_bin', f"extraInfo({lid_table}_kinematic_bin)")

   reweighter = b2.register_module('ParticleWeighting')
   reweighter.param('tableName', lid_table)
   reweighter.param('particleList', 'e+:all')
   my_path.add_module(reweighter)


.. note::
   The lepton ID fake-rate tables (lepton ID efficiency for pions and kaons) are not
   prepared at Belle.
   It can be studied using the :math:`D^{*}` sample (see
   ~nishida6/public/kid/00dstar_nt.txt on kekcc for details).


.. rubric:: Kaon/:math:`\pi` ID

Naming scheme for payloads is:
``BelleKID<K|Pi><Eff|Fake><Combined|Plus|Minus>_<PID cut:[1..9]>``
Here,

- K or Pi refers to correction information obtained for kaons or pions, accordingly
- Eff or Fake refers to efficiency or fake rate corrections
- Combined, Plus or Minus refers to the charge of tracks used for the study
- PID cut is an integer number from 1 to 9 that corresponds to "cut > 0.number"

To append PID correction information for pion efficiency selected as kaons with
``atcPIDBelle(3, 2)>0.6`` cut:

.. code-block:: python3

   kid_table = "BelleKIDPiFakePlus_6"

   va.variables.addAlias('KID_eff_data', f"extraInfo({kid_table}_eff_data)")
   va.variables.addAlias('KID_eff_data_stat_err', f"extraInfo({kid_table}_eff_data_stat_err)")
   va.variables.addAlias('KID_eff_data_syst_err', f"extraInfo({kid_table}_eff_data_syst_err)")
   va.variables.addAlias('KID_eff_mc', f"extraInfo({kid_table}_eff_mc)")
   va.variables.addAlias('KID_eff_mc_stat_err', f"extraInfo({kid_table}_eff_mc_stat_err)")
   va.variables.addAlias('KID_fit_flag', f"extraInfo({kid_table}_fit_flag)")
   va.variables.addAlias('KID_kinematic_bin', f"extraInfo({kid_table}_kinematic_bin)")
   va.variables.addAlias('KID_ratio', f"extraInfo({kid_table}_ratio)")
   va.variables.addAlias('KID_ratio_stat_err', f"extraInfo({kid_table}_ratio_stat_err)")
   va.variables.addAlias('KID_ratio_syst_err', f"extraInfo({kid_table}_ratio_syst_err)")
   va.variables.addAlias('KID_run_bin', f"extraInfo({kid_table}_run_bin)")

   kid_weights = ['KID_eff_data',
                  'KID_eff_data_stat_err',
                  'KID_eff_data_syst_err',
                  'KID_eff_mc',
                  'KID_eff_mc_stat_err',
                  'KID_fit_flag',
                  'KID_ratio',
                  'KID_ratio_stat_err',
                  'KID_ratio_syst_err',
                  'KID_run_bin',
                  'KID_kinematic_bin']

   reweighter2 = b2.register_module('ParticleWeighting')
   reweighter2.param('tableName', kid_table)
   reweighter2.param('particleList', 'pi+:all')
   my_path.add_module(reweighter2)

.. note::
   ``fit_flag`` indicates if the fits failed; ``fit_flag = 0`` is for successful fit, while 
   any non-zero values of fit_flag are for failed fit. Please only use ``fit_flag==0``
   when evaluating PID contributions.


.. rubric:: Proton ID

Naming scheme for proton ID payloads is:
``BellePID<Plus|Minus>_<PID cut>``

Here,

- Plus or Minus refers to the charge of tracks used for the study
- PID cut can be 0.6, 0.7, 0.8 or 0.9

If you'd like to add the information for proton efficiency selected
with ``atcPIDBelle(4, 2)>0.6`` and ``atcPIDBelle(4, 3)>0.6``.

.. code-block:: python3

   pid_table = "BellePIDPlus_0.6"

   va.variables.addAlias("PID_eff_data", f"extraInfo({pid_table}_eff_data)")
   va.variables.addAlias("PID_eff_data_stat_err", f"extraInfo({pid_table}_eff_data_stat_err)")
   va.variables.addAlias("PID_eff_mc", f"extraInfo({pid_table}_eff_mc)")
   va.variables.addAlias("PID_eff_mc_stat_err", f"extraInfo({pid_table}_eff_mc_stat_err)")
   va.variables.addAlias("PID_ratio", f"extraInfo({pid_table}_ratio)")
   va.variables.addAlias("PID_ratio_stat_err", f"extraInfo({pid_table}_ratio_stat_err)")
   va.variables.addAlias("PID_ratio_syst_err", f"extraInfo({pid_table}_ratio_syst_err)")
   va.variables.addAlias("PID_kinematic_bin", f"extraInfo({pid_table}_kinematic_bin)")
   va.variables.addAlias("PID_run_bin", f"extraInfo({pid_table}_run_bin)")

   reweighter3 = b2.register_module('ParticleWeighting')
   reweighter3.param('tableName', pid_table)
   reweighter3.param('particleList', 'pi+:all')
   my_path.add_module(reweighter3)


-------------------------
Full Event Interpretation
-------------------------

To utilize FEI, the correct prefix of FEI payloads needs to be set:

.. code-block:: python3

   import fei
   configuration = fei.config.FeiConfiguration(prefix='FEI_B2BII_light-2012-minos', cache=0)

   feistate = fei.get_path(particles, configuration)

   path.add_path(feistate.path)


For more details please see :code:`analysis/examples/FEI/B_converted_apply.py`


--------------
Flavour Tagger
--------------

To apply flavour tagger in a b2bii analysis, one will need to append the
correct global tag.
FlavorTagger will call the corresponding payloads in the module.

.. code-block:: python3

   import flavorTagger as ft

   # Flavour Tagger
   weightfiles = 'B2nunubarBGx1'
   basf2.conditions.append_globaltag("analysis_tools_light-2012-minos")
   ft.flavorTagger(
       particleLists=['B+:sig'],
       weightFiles=weightfiles,
       path=my_path)


