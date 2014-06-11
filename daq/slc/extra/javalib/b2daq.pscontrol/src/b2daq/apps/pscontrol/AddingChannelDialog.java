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
public class AddingChannelDialog {

    public static String showDialog(Scene pscene, HVParamEditorPaneController editor) {
        try {
            FXMLLoader loader = new FXMLLoader(AddingChannelDialog.class.getResource("AddingChannelDialog.fxml"));
            loader.load();
            Parent root = loader.getRoot();
            AddingChannelDialogController controller = loader.getController();
            controller.setEditor(editor);
            Scene scene = new Scene(root);
            Stage dialog = new Stage();//StageStyle.UTILITY);
            dialog.setScene(scene);
            if (pscene != null) {
                dialog.initOwner(pscene.getWindow());
            } else {
                dialog.centerOnScreen();
            }
            dialog.initModality(Modality.WINDOW_MODAL);
            //dialog.initModality(Modality.APPLICATION_MODAL);
            dialog.setResizable(false);
            dialog.setTitle("Adding new channel");
            dialog.showAndWait();
            //return controller.getText();
        } catch (IOException ex) {
            Logger.getLogger(AddingChannelDialog.class.getName()).log(Level.SEVERE, null, ex);
        }
        return null;
    }
}
