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
    static public final DQMCommand DQMDATAGET = new DQMCommand(703, "DQMDATAGET");
    static public final DQMCommand DQMDATASET = new DQMCommand(704, "DQMDATASET");
    static public final DQMCommand DQMRESET = new DQMCommand(705, "DQMRESET");
    static public final DQMCommand DQMINIT = new DQMCommand(706, "DQMINIT");
    static public final DQMCommand DQMFILEGET = new DQMCommand(707, "DQMFILEGET");
    static public final DQMCommand DQMFILESET = new DQMCommand(708, "DQMFILESET");

    static final DQMCommand[] list = new DQMCommand[]{
        DQMDIRGET, DQMDIRSET, DQMDATAGET, DQMDATASET, 
        DQMRESET, DQMINIT, DQMFILEGET, DQMFILESET
    };

    static DQMCommand get(int id) {
        id = id % 700 - 1;
        if (id >= 0 && id < list.length) {
            return list[id];
        }
        return UNKNOWN;
    }

    static DQMCommand get(String label) {
        for (DQMCommand cmd : list) {
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
