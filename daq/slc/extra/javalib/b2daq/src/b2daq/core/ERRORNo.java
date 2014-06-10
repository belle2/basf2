/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package b2daq.core;

import static b2daq.core.Enum.UNKNOWN;
import b2daq.nsm.NSMCommand;
import static b2daq.nsm.NSMCommand.OK;

/**
 *
 * @author tkonno
 */
public class ERRORNo extends Enum {

    final static public ERRORNo NSMONLINE = new ERRORNo(11, "NSMONLINE");
    final static public ERRORNo NSMSENDREQ = new ERRORNo(12, "NSMSENDREQ");
    final static public ERRORNo NSMMEMOPEN = new ERRORNo(13, "SENOPEN");
    final static public ERRORNo NSMINIT = new ERRORNo(14, "NSMINIT");
    final static public ERRORNo DATABASE = new ERRORNo(31, "DATABASE");
    
    public ERRORNo() {
        super(UNKNOWN.getId(), UNKNOWN.getLabel());
    }

    protected ERRORNo(int id, String label) {
        super(id, label);
    }
    
    public static ERRORNo get(String msg) {
        if (msg.equals(NSMSENDREQ.getLabel())) {
            return NSMSENDREQ;
        } else if (msg.equals(NSMONLINE.getLabel())) {
            return NSMONLINE;
        } else if (msg.equals(NSMMEMOPEN.getLabel())) {
            return NSMMEMOPEN;
        } else if (msg.equals(NSMINIT.getLabel())) {
            return NSMINIT;
        } else if (msg.equals(DATABASE.getLabel())) {
            return DATABASE;
        } else {
            return (ERRORNo) UNKNOWN;
        }
    }

    public static ERRORNo get(int id) {
        if (id == NSMSENDREQ.getId()) {
            return NSMSENDREQ;
        } else if (id == NSMONLINE.getId()) {
            return NSMONLINE;
        } else if (id == NSMMEMOPEN.getId()) {
            return NSMMEMOPEN;
        } else if (id == NSMINIT.getId()) {
            return NSMINIT;
        } else if (id == DATABASE.getId()) {
            return DATABASE;
        } else {
            return (ERRORNo) UNKNOWN;
        }
    }
}
