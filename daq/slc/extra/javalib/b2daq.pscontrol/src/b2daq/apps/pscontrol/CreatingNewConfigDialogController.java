/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package b2daq.apps.pscontrol;

import java.net.URL;
import java.util.ResourceBundle;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.GridPane;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class CreatingNewConfigDialogController implements Initializable {

    @FXML
    private TextField field;
    @FXML
    private GridPane grid;
    private boolean isOK = false;
    private int nfield;
    
    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        // TODO
    }    
 
    @FXML
    void handleCancelButton() {
        isOK = false;
        grid.getScene().getWindow().hide();
    }
    
    @FXML
    void handleCreateButton() {
        isOK = true;
        grid.getScene().getWindow().hide();
    }
    
    public void init(String configname, String [] valuesetnames) {
        field.setText(configname);
        nfield = 0;
        for (String name : valuesetnames) {
            TextField f = new TextField();
            f.setText(name);
            grid.add(f, 1, nfield);
            nfield++;
        }
        
    }
    
    public String [] getTexts() {
        if (!isOK) return null;
        String [] namelist = new String [nfield+1];
        if ( namelist.length == 1) return null;
        namelist[0] = field.getText();
        for (int n = 0; n < nfield; n++) {
            namelist [n+1] = ((TextField)grid.getChildren().get(n)).getText();
        }
        return namelist;
    }
    
}
