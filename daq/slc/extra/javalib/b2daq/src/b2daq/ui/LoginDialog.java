/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.ui;

import b2daq.nsm.NSMConfig;
import b2daq.nsm.NSMListenerService;
import java.io.IOException;
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

    public static NSMConfig showDialog(Scene pscene, String title,
            String hostname, int port, 
            String nsmhost, String nsmnode, int nsmport) {
        try {
            FXMLLoader loader = new FXMLLoader(LoginDialog.class.getResource("LoginDialog.fxml"));
            loader.load();
            Parent root = loader.getRoot();
            LoginDialogController controller = loader.getController();
            controller.set(hostname, port, nsmhost, nsmnode, nsmport);
            Scene scene = new Scene(root);
            Stage dialog = new Stage();//StageStyle.UTILITY);
            dialog.setScene(scene);
            if (pscene != null) {
                dialog.initOwner(pscene.getWindow());
            } else {
                dialog.centerOnScreen();
            }
            dialog.initModality(Modality.APPLICATION_MODAL);
            dialog.setResizable(false);
            dialog.setTitle(title);
            dialog.showAndWait();
            NSMListenerService.setNSMConfig(controller.getNSMConfig());
            return controller.getNSMConfig();
        } catch (IOException ex) {
            Logger.getLogger(LoginDialog.class.getName()).log(Level.SEVERE, null, ex);
        }
        return null;
    }
}
