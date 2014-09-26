/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm;

/**
 *
 * @author tkonno
 */
public class NSMClientSample {

    static public void main(String[] argv) {
        NSMConfig config = new NSMConfig();
        config.setHostname("ropc02.kek.jp");
        config.setPort(9090);
        config.setNsmHost("172.22.32.11");
        config.setNsmNode("CSS_NSM_DUMMY");
        config.setNsmPort(8122);
        config.setNsmTarget("ECL01");
        NSMListenerService.setNSMConfig(config);
        NSMListenerService.restart();
        NSMListenerService.requestNSMGet("ECL01_STATUS", "rorc_status", 1);
        while (true) {
            try {
                Thread.sleep(10000);
            } catch (InterruptedException ex) {
            }
            NSMData data = NSMListenerService.getData("ECL01_STATUS");
            if (data != null) {
                data.print();
            }
        }
    }
}
