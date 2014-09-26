/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.nsm.ui;

import java.io.IOException;
import java.util.ArrayList;
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
public class NSMConfigDialog {

    public static NSMConfigDialogController 
        showDialog(Scene pscene, String title,String nodename, ArrayList<NSMDataProperty> properties) {
        try {
            FXMLLoader loader = new FXMLLoader(NSMConfigDialog.class.getResource("NSMConfigDialog.fxml"));
            loader.load();
            Parent root = loader.getRoot();
            NSMConfigDialogController controller = loader.getController();
            controller.init(nodename, properties);
            Scene scene = new Scene(root);
            Stage dialog = new Stage();//StageStyle.UTILITY);
            dialog.setScene(scene);
            if (pscene != null) {
                dialog.initOwner(pscene.getWindow());
            } else {
                dialog.centerOnScreen();
            }
            //dialog.initModality(Modality.WINDOW_MODAL);
            dialog.initModality(Modality.APPLICATION_MODAL);
            dialog.setResizable(false);
            dialog.setTitle(title);
            dialog.showAndWait();
            return controller.isStarted()?controller:null;
        } catch (IOException ex) {
            Logger.getLogger(NSMConfigDialog.class.getName()).log(Level.SEVERE, null, ex);
        }
        return null;
    }
}
