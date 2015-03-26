/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

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
public class MonitorOpenDialog {

    public static void showDialog(Scene pscene, PSSettingMainPanelController pmain) {
        try {
            FXMLLoader loader = new FXMLLoader(MonitorOpenDialog.class.getResource("MonitorOpenDialog.fxml"));
            loader.load();
            Parent root = loader.getRoot();
            MonitorOpenDialogController controller = loader.getController();
            controller.setSettingMainPanel(pmain);
            Scene scene = new Scene(root);
            Stage dialog = new Stage();//StageStyle.UTILITY);
            dialog.setScene(scene);
            if (pscene != null) {
                dialog.initOwner(pscene.getWindow());
            } else {
                dialog.centerOnScreen();
            }
            dialog.initModality(Modality.WINDOW_MODAL);
            dialog.setResizable(false);
            dialog.setTitle("Open crate monitor");
            dialog.showAndWait();
        } catch (IOException ex) {
            Logger.getLogger(MonitorOpenDialog.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
}
