/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.database.ConfigObject;
import b2daq.nsm.NSMMessage;

/**
 *
 * @author tkonno
 */
public interface ConfigManager {
    public void reload(NSMMessage msg);
    public ConfigObject update(String nodename, String newconf);
}
