/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.database.ConfigObject;
import java.io.IOException;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.control.TableView;

/**
 *
 * @author tkonno
 */
public class HVParamTableView {

    public static TableView 
        create(ArrayList<ConfigObject> channels, ConfigObject valueset, HVParamEditorPaneController editor) {
        try {
            FXMLLoader loader = new FXMLLoader(HVParamTableView.class.getResource("HVParamTableView.fxml"));
            loader.load();
            Parent root = loader.getRoot();
            HVParamTableViewController controller = loader.getController();
            controller.add(channels, valueset);
            controller.setEditor(editor);
            return controller.getTable();
        } catch (IOException ex) {
            Logger.getLogger(HVParamTableView.class.getName()).log(Level.SEVERE, null, ex);
        }
        return null;
    }

}