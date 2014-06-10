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
public class CreatingNewConfigDialog {

    public static String [] showDialog(Scene pscene, String configname, String... namelist) {
        try {
            FXMLLoader loader = new FXMLLoader(CreatingNewConfigDialog.class.getResource("CreatingNewConfigDialog.fxml"));
            loader.load();
            Parent root = loader.getRoot();
            CreatingNewConfigDialogController controller = loader.getController();
            controller.init(configname, namelist);
            Scene scene = new Scene(root);
            Stage dialog = new Stage(StageStyle.UTILITY);
            dialog.setScene(scene);
            if (pscene == null) dialog.initOwner(pscene.getWindow());
            dialog.initModality(Modality.WINDOW_MODAL);
            dialog.setResizable(false);
            dialog.showAndWait();
            return controller.getTexts();
        } catch (IOException ex) {
            Logger.getLogger(CreatingNewConfigDialog.class.getName()).log(Level.SEVERE, null, ex);
        }
        return null;
    }
}
