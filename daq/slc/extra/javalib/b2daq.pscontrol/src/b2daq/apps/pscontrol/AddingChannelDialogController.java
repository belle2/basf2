/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import java.net.URL;
import java.util.ResourceBundle;
import javafx.beans.binding.Binding;
import javafx.beans.binding.ObjectBinding;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.Tab;
import javafx.scene.control.TableView;
import javafx.scene.control.TextField;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class AddingChannelDialogController implements Initializable {

    @FXML
    private Button okButton;
    @FXML
    private Button cancelButton;
    @FXML
    private TextField field_crate;
    @FXML
    private TextField field_slot;
    @FXML
    private TextField field_channel;

    private boolean isOK = false;
    private HVParamEditorPaneController editor;

    @FXML
    private void handleOKButton() {
        try {
            String[] s_crate_v = field_crate.getText().split(",");
            String[] s_slot_v = field_slot.getText().split(",");
            String[] s_channel_v = field_channel.getText().split(",");
            for (Tab tab : editor.getTabs()) {
                TableView table = (TableView) tab.getContent();
                for (String s_crate : s_crate_v) {
                    int crate = Integer.parseInt(s_crate);
                    for (String s_slot : s_slot_v) {
                        int slot = Integer.parseInt(s_slot);
                        for (String s_channel : s_channel_v) {
                            int channel = Integer.parseInt(s_channel);
                            boolean overlapped = false;
                            for (Object obj : table.getItems()) {
                                HVStatusProperty pro = (HVStatusProperty)obj;
                                if (pro.getCrate() == crate && 
                                        pro.getSlot() == slot &&
                                        pro.getChannel() == channel) {
                                    overlapped = true;
                                    break;
                                }
                            }
                            if (!overlapped) {
                                table.getItems().add(new HVStatusProperty(crate, slot, channel));
                            }
                        }
                    }
                }
            }
        } catch (Exception e) {
        }
        handleCloseAction(true);
    }

    @FXML
    private void handleCancelButton() {
        handleCloseAction(false);
    }

    private void handleCloseAction(boolean isOK) {
        this.isOK = isOK;
        okButton.getScene().getWindow().hide();
    }

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        okButton.setOnKeyPressed(new EventHandler<KeyEvent>() {
            @Override
            public void handle(KeyEvent ke) {
                if (ke.getCode().equals(KeyCode.ENTER)) {
                    handleCloseAction(true);
                }
            }
        });
        cancelButton.setOnKeyPressed(new EventHandler<KeyEvent>() {
            @Override
            public void handle(KeyEvent ke) {
                if (ke.getCode().equals(KeyCode.ENTER)) {
                    handleCloseAction(false);
                }
            }
        });
        Binding binding = new ObjectBinding<Boolean>() {
            {
                super.bind(field_crate.textProperty(),
                        field_slot.textProperty(),
                        field_channel.textProperty());
            }

            @Override
            protected Boolean computeValue() {
                return !(field_crate.getText().length() > 0
                        && field_slot.getText().length() > 0
                        && field_channel.getText().length() > 0);
            }
        };
        okButton.disableProperty().bind(binding);
    }

    void setEditor(HVParamEditorPaneController editor) {
        this.editor = editor;
    }

}
