/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

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
public class RunSettingDialog {

    public static RunSetting showDialog(String nodename, String title, int expno, int runno,
            String operators, String comment) {
        try {
            FXMLLoader loader = new FXMLLoader(RunSettingDialog.class.getResource("RunSettingDialog.fxml"));
            loader.load();
            Parent root = loader.getRoot();
            RunSettingDialogController controller = loader.getController();
            controller.set(nodename, expno, runno, operators, comment);
            Scene scene = new Scene(root);
            Stage dialog = new Stage();//StageStyle.UTILITY);
            dialog.setScene(scene);
            dialog.centerOnScreen();
            dialog.initModality(Modality.APPLICATION_MODAL);
            dialog.setResizable(false);
            dialog.setTitle(title);
            dialog.showAndWait();
            RunSetting setting = controller.getSetting();
            return setting;
        } catch (IOException ex) {
            Logger.getLogger(RunSettingDialog.class.getName()).log(Level.SEVERE, null, ex);
        }
        return null;
    }
}
