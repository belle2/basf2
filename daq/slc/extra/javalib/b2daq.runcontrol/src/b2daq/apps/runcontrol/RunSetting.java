package b2daq.apps.runcontrol;

import javafx.beans.property.SimpleIntegerProperty;
import javafx.beans.property.SimpleStringProperty;

public class RunSetting  {

    private final SimpleIntegerProperty expno = new SimpleIntegerProperty();
    private final SimpleIntegerProperty runno = new SimpleIntegerProperty();
    private final SimpleStringProperty operator1 = new SimpleStringProperty();
    private final SimpleStringProperty operator2 = new SimpleStringProperty();
    private final SimpleStringProperty comment = new SimpleStringProperty();

    public SimpleIntegerProperty expnoProperty() {
        return expno;
    }

    public SimpleIntegerProperty runnoProperty() {
        return runno;
    }

    public SimpleStringProperty operator1Property() {
        return operator1;
    }

    public SimpleStringProperty operator2Property() {
        return operator2;
    }

    public SimpleStringProperty commentProperty() {
        return comment;
    }

    public int expnoPort() {
        return expno.get();
    }

    public int runnoPort() {
        return runno.get();
    }

    public String getOperator1() {
        return operator1.get();
    }

    public String getOperator2() {
        return operator2.get();
    }

    public String getComment() {
        return this.comment.get();
    }

    public void setOperator1(String operator1) {
        this.operator1.set(operator1);
    }

    public void setOperator2(String operator2) {
        this.operator2.set(operator2);
    }

    public void setExpno(int expno) {
        this.expno.set(expno);
    }

    public void setRunno(int runno) {
        this.runno.set(runno);
    }

    public void setComment(String comment) {
        this.comment.set(comment);
    }

}
