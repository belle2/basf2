/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.ui;

import b2daq.io.ConfigFile;
import b2daq.nsm.NSMConfig;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Modality;
import javafx.stage.Stage;

/**
 *
 * @author tkonno
 */
public class LoginDialog {

    public static NSMConfig showDialog(String filename,
            String title, String hostname, int port,
            String nsmnode, String nsmhost, int nsmport, String guinode) {
        try {
            ConfigFile file = new ConfigFile();
            try {
                file.read(filename);
            } catch (Exception e) {
            }
            if (!file.hasKey("hostname")) {
                if (hostname == null) {
                    try {
                        hostname = InetAddress.getLocalHost().getHostName();
                    } catch (UnknownHostException e) {
                        hostname = "localhost";
                    }
                }
                if (nsmhost == null) {
                    nsmhost = hostname;
                }
                file.add("hostname", hostname);
                file.add("port", "" + port);
                file.add("nsmnode", nsmnode);
                file.add("nsmhost", nsmhost);
                file.add("nsmport", "" + nsmport);
                file.add("guinode", guinode);
            }

            FXMLLoader loader = new FXMLLoader(LoginDialog.class.getResource("LoginDialog.fxml"));
            loader.load();
            Parent root = loader.getRoot();
            LoginDialogController controller = loader.getController();
            controller.set(file.getString("hostname"), file.getInt("port"), file.getString("nsmhost"),
                    file.getString("nsmnode"), file.getInt("nsmport"), file.getString("guinode"));
            Scene scene = new Scene(root);
            Stage dialog = new Stage();//StageStyle.UTILITY);
            dialog.setScene(scene);
            dialog.centerOnScreen();
            dialog.initModality(Modality.APPLICATION_MODAL);
            dialog.setResizable(false);
            dialog.setTitle(title);
            dialog.showAndWait();
            NSMConfig config = controller.getNSMConfig();
            if (config == null) {
                System.exit(0);
            }
            file = new ConfigFile();
            file.setPromptMessage(title);
            file.add("hostname", config.getHostname());
            file.add("port", "" + config.getPort());
            file.add("nsmnode", config.getNsmNode());
            file.add("nsmhost", config.getNsmHost());
            file.add("nsmport", "" + config.getNsmPort());
            file.add("guinode", config.getGuiNode());
            new DataOutputStream(new FileOutputStream(new File(filename))).writeBytes(file.toString());
            return config;
        } catch (IOException ex) {
            Logger.getLogger(LoginDialog.class.getName()).log(Level.SEVERE, null, ex);
        }
        return null;
    }
}
