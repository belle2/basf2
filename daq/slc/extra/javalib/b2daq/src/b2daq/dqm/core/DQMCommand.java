/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.dqm.core;

import b2daq.nsm.NSMCommand;

/**
 *
 * @author tkonno
 */
public class DQMCommand extends NSMCommand {

    static public final DQMCommand UNKNOWN = new DQMCommand(0, "UNKNOWN");
    static public final DQMCommand DQMDIRGET = new DQMCommand(701, "DQMDIRGET");
    static public final DQMCommand DQMDIRSET = new DQMCommand(702, "DQMDIRSET");
    static public final DQMCommand DQMHISTGET = new DQMCommand(703, "DQMHISTGET");
    static public final DQMCommand DQMHISTSET = new DQMCommand(704, "DQMHISTSET");
    static public final DQMCommand DQMLISTGET = new DQMCommand(705, "DQMLISTGET ");
    static public final DQMCommand DQMLISTSET = new DQMCommand(706, "DQMLISTSET ");
    static public final DQMCommand DQMFILEGET = new DQMCommand(707, "DQMFILEGET");
    static public final DQMCommand DQMFILESET = new DQMCommand(708, "DQMFILESET");
    static public final DQMCommand DQMRESET = new DQMCommand(709, "DQMRESET");

    static private final DQMCommand[] CMD_LIST = new DQMCommand[]{
        DQMDIRGET, DQMDIRSET, DQMHISTGET, DQMHISTSET,
        DQMLISTGET, DQMLISTGET, DQMFILEGET, DQMFILESET, DQMRESET
    };

    static DQMCommand get(int id) {
        for (DQMCommand cmd : CMD_LIST) {
            if (id == cmd.getId()) {
                return cmd;
            }
        }
        return UNKNOWN;
    }

    static DQMCommand get(String label) {
        for (DQMCommand cmd : CMD_LIST) {
            if (label.matches(cmd.getLabel())) {
                return cmd;
            }
        }
        return UNKNOWN;
    }

    public DQMCommand() {
        super(UNKNOWN);
    }

    private DQMCommand(int id, String label) {
        super(id, label);
    }
}
