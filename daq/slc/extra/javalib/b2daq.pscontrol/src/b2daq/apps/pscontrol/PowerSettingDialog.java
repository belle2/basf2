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
import javafx.stage.StageStyle;

/**
 *
 * @author tkonno
 */
public class PowerSettingDialog {

    public static void showDialog(Scene pscene, HVParamEditorPaneController editor) {
        try {
            FXMLLoader loader = new FXMLLoader(PowerSettingDialog.class.getResource("PowerSettingDialog.fxml"));
            loader.load();
            Parent root = loader.getRoot();
            PowerSettingDialogController controller = loader.getController();
            controller.init(editor);
            Scene scene = new Scene(root);
            Stage dialog = new Stage();//StageStyle.UTILITY);
            dialog.setScene(scene);
            if (pscene != null) dialog.initOwner(pscene.getWindow());
            dialog.initModality(Modality.WINDOW_MODAL);
            dialog.setResizable(false);
            dialog.setTitle("Edit channels");
            dialog.showAndWait();
        } catch (IOException ex) {
            Logger.getLogger(PowerSettingDialog.class.getName()).log(Level.SEVERE, null, ex);
        }
    }
}
