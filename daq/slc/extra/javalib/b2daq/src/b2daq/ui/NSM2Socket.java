/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.ui;

import b2daq.io.ConfigFile;
import b2daq.nsm.NSMConfig;
import b2daq.nsm.NSMDataProperty;
import b2daq.nsm.ui.NSMConfigDialog;
import b2daq.nsm.ui.NSMConfigDialogController;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author tkonno
 */
public class NSM2Socket {

    public static NSM2Socket connect(String filename, String hostname, int port,
            String nsmhost, int nsmport, String nsmnode, String nsmtarget, String data[],
            String title, String message) {
        try {
            ConfigFile file = new ConfigFile();
            try {
                file.read(filename);
            } catch (Exception e) {
            }
            if (!file.hasKey("hostname")) {
                file.add("hostname", hostname);
                file.add("port", "" + port);
                file.add("nsmhost", nsmhost);
                file.add("nsmport", "" + nsmport);
                file.add("nsmnode", nsmnode);
                file.add("nsmtarget", nsmtarget);
                for (int i = 0; i < data.length; i++) {
                    String [] str = data[i].split(":");
                    System.out.println(data[i]);
                    if (str.length > 2) {
                        file.add(String.format("data%02d.dataname", i), str[0]);
                        file.add(String.format("data%02d.format", i), str[1]);
                        file.add(String.format("data%02d.revision", i), str[2]);
                    }
                }
            }
            NSMConfig config = LoginDialog.showDialog(null, title,
                    file.getString("hostname"), file.getInt("port"), file.getString("nsmhost"),
                    file.getString("nsmnode"), file.getInt("nsmport"));
            if (config == null) {
                return null;
            }
            ArrayList<NSMDataProperty> datalist = new ArrayList<>();
            for (int i = 0; i < file.getKeyList().size(); i++) {
                String label = String.format("data%02d", i);
                if (file.hasKey(label + ".dataname")) {
                    datalist.add(new NSMDataProperty(file.getString(label + ".dataname"),
                            file.getString(label + ".format"), file.getInt(label + ".revision")));
                }
            }
            NSMConfigDialogController netconfig = NSMConfigDialog.showDialog(null, message,
                    nsmtarget, datalist);
            if (netconfig == null) {
                return null;
            }
            config.setNsmTarget(netconfig.getNSMTarget());

            file = new ConfigFile();
            file.setPromptMessage(title);
            file.add("hostname", config.getHostname());
            file.add("port", "" + config.getPort());
            file.add("nsmhost", config.getNsmHost());
            file.add("nsmport", "" + config.getNsmPort());
            file.add("nsmnode", config.getNsmNode());
            file.add("nsmtarget", config.getNsmTarget());
            int n = 0;
            for (NSMDataProperty property : netconfig.getNSMDataProperties()) {
                file.add(String.format("data%02d.dataname", n), property.getDataname());
                file.add(String.format("data%02d.format", n), "" + property.getFormat());
                file.add(String.format("data%02d.revision", n), "" + property.getRevision());
                n++;
            }
            new DataOutputStream(new FileOutputStream(new File(filename))).writeBytes(file.toString());
            NSM2Socket socket = new NSM2Socket(config, netconfig);
            return new NSM2Socket(config, netconfig);
        } catch (IOException ex) {
            Logger.getLogger(NSM2Socket.class.getName()).log(Level.SEVERE, null, ex);
        }
        return null;
    }

    private final NSMConfig config;
    private final NSMConfigDialogController netconfig;

    private NSM2Socket(NSMConfig config, NSMConfigDialogController netconfig) {
        this.config = config;
        this.netconfig = netconfig;
    }

    public NSMConfig getConfig() { return config; }
    public NSMConfigDialogController getNSMConfig() { return netconfig; }

}
