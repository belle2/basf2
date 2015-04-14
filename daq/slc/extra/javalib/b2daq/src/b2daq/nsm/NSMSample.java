/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm;

import b2daq.database.FieldInfo;
import b2daq.runcontrol.core.RCCommand;
import java.io.IOException;

/**
 *
 * @author tkonno
 */
public class NSMSample {

    static public void main(String[] argv) throws IOException {
        // configurations to connect nsm2socket
        String hostname = "ropc01";
        int port = 9090;
        String nsmhost = "ropc01";
        int nsmport = 9122;
        String nsmuiname = "nsmui";
        
        // configuration to read NSM data 
        String dataname = "ROPC01_STATUS";
        String dataformat = "ronode_status";
        
        // configuration to read NSM var
        String nsmnode = "ROPC01";
        String varname = "rcstate";
        
        // connecting to nsm2socket
        // hostname and port : hostname and port for nsm2socket 
        // nsmuiname : nsm node name for the connecting client
        // nsmhost : hostname or ip address for nsmd2
        // nsmport : hostname or ip address for nsmd2
        NSMCommunicator.get().reconnect(hostname, port, nsmuiname, nsmhost, nsmport);

        // add handler of NSM data
        // send request of NSM data to nsm2socket
        NSMCommunicator.get().add(new NSMDATASetHandler(false, dataname, dataformat));

        // add handler of a parameter in a NSM data
        // NSMCommunicator thread performs handleDataSet when an update of the NSM data is delivered
        NSMCommunicator.get().add(new NSMDATASetParamHandler(dataname, "ctime", FieldInfo.INT) {
            @Override
            public boolean handleDataSetParam(int val) {
                System.out.println(""+getDataName() + ":"+ getParName() + " = " + val);
                return true;
            }
        });

        // add handler of a parameter in a NSM var
        // NSMCommunicator thread performs handleVSet when an update of the NSM var is delivered
        NSMCommunicator.get().add(new NSMVSetHandler(false, nsmnode, varname, NSMVar.TEXT) {
            @Override
            public boolean handleVSet(NSMVar var) {
                System.out.println(""+var.getNode() + ":"+ var.getName() + " = " + var.getText());
                return true;
            }
        });

        // send NSM var with integer
        NSMCommunicator.get().requestVSet(nsmnode, new NSMVar("eb0.port", 5101));

        // send NSM message to a NSM node of "ROPC01"
        // request contains a command to perform refresh of configuration parameters
        NSMCommunicator.get().request(new NSMMessage(nsmnode, RCCommand.CONFIGURE));
    }
}
