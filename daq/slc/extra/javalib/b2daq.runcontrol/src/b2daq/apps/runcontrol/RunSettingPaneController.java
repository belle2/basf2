package b2daq.apps.runcontrol;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
import b2daq.database.ConfigObject;
import b2daq.nsm.NSMData;
import b2daq.runcontrol.core.RCState;
import b2daq.ui.InputDialog;
import java.net.URL;
import java.util.ResourceBundle;
import javafx.beans.binding.ObjectBinding;
import javafx.beans.property.BooleanProperty;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.event.Event;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextArea;
import javafx.scene.control.TextField;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class RunSettingPaneController implements Initializable {

    @FXML
    private TextField field_operator1;
    @FXML
    private TextField field_operator2;
    @FXML
    private TextArea area_comments;
    @FXML
    private Label label_expno;
    @FXML
    private Button button_expno;

    private final BooleanProperty ready = new SimpleBooleanProperty(false);
    private final RCState state = new RCState();
    private boolean edited = false;

    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
    }

    @FXML
    public void handleIncrementButton() {
        handleIncrementButtonImpl("Input new experiment number");
    }

    private void handleIncrementButtonImpl(String title) {
        int expno_org = Integer.parseInt(label_expno.getText());
        String str = InputDialog.showDialog(button_expno.getScene(),
                "Edit experiment number", title,
                "" + expno_org);
        if (str != null) {
            int expno = 0;
            try {
                expno = Integer.parseInt(str);
            } catch (Exception e) {
            }
            if (expno > expno_org) {
                edited = true;
                label_expno.setText(String.format("%04d", expno));
            } else {
                handleIncrementButtonImpl("Inter lager number");
            }
        }
    }

    public TextField getFieldOperator1() {
        return field_operator1;
    }

    public TextField getFieldOperator2() {
        return field_operator2;
    }

    public Label getLabelExpNo() {
        return label_expno;
    }

    public Label getButtonExpNo() {
        return label_expno;
    }

    public TextArea getAreatComments() {
        return area_comments;
    }

    void clear() {
        edited = false;
    }

    void update(ConfigObject cobj, NSMData ndata) {
        if (!edited) {
            label_expno.setText(String.format("%04d",
                    ndata.getInt("expno")));
        }
    }

    public boolean isReady() {
        return field_operator1.getText().length() > 0
                && area_comments.getText().length() > 0;
    }

    public void bind(RunControlMainPaneController rcmain) {
        final ObjectBinding binding = new ObjectBinding() {
            {
                super.bind(rcmain.getTextRCState().textProperty());
            }

            @Override
            protected Boolean computeValue() {
                state.copy(rcmain.getTextRCState().getText());
                if (state.equals(RCState.RUNNING_S)) {
                    return false;
                } else if (state.equals(RCState.READY_S)) {
                    return true;
                } else if (state.equals(RCState.CONFIGURED_S)) {
                    return true;
                } else if (state.equals(RCState.INITIAL_S)) {
                    return true;
                }
                return false;
            }
        };
        field_operator1.editableProperty().bind(binding);
        field_operator2.editableProperty().bind(binding);
        area_comments.editableProperty().bind(binding);
        final ObjectBinding buttonbinding = new ObjectBinding() {
            {
                super.bind(rcmain.getTextRCState().textProperty());
            }

            @Override
            protected Boolean computeValue() {
                state.copy(rcmain.getTextRCState().getText());
                if (state.equals(RCState.RUNNING_S)) {
                    return true;
                } else if (state.equals(RCState.READY_S)) {
                    return false;
                } else if (state.equals(RCState.CONFIGURED_S)) {
                    return true;
                } else if (state.equals(RCState.INITIAL_S)) {
                    return true;
                }
                return true;
            }
        };
        button_expno.disableProperty().bind(buttonbinding);
        EventHandler handler = new EventHandler() {
            @Override
            public void handle(Event event) {
                ready.set(field_operator1.getText().length() > 0
                        && area_comments.getText().length() > 0);
            }
        };
        field_operator1.setOnKeyTyped(handler);
        area_comments.setOnKeyTyped(handler);
    }

    public boolean isEdited() {
        return edited;
    }

    public BooleanProperty readyProperty() {
        return ready;
    }

}
