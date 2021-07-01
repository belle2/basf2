/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
# Tagging

Upsilon4S decays into two B mesons. The kinematics of the Upsilon4S are well known (by using the energies/momenta of the electron and positron), so if reconstruct one of the two B mesons, we can infer the kinematics of the other. Therefore, one often reconstructs one of the B mesons in a well understood channel (called the tag B) and uses it to constrain the properties of the other B (the signal B), which is reconstructed in a more interesting channel.

A very powerful way to tag your event (i.e. reconstruct a tag B) is with the use of the Full Event Interpretation (FEI).
