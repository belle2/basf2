package b2daq.java.ui;

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.EventListener;
import java.util.EventObject;

import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.event.EventListenerList;

import b2daq.ui.Updatable;

public abstract class ExpansionPanel extends JPanel implements Updatable {

	private static final long serialVersionUID = 1L;

	abstract public Container makePanel();
    private JButton _button;
    private Container _panel;
    private JScrollPane _scroll;
    private boolean _open_flag = false;
    private final EventListenerList _listener_List = new EventListenerList();
    private ExpansionEvent _event = null;

    public ExpansionPanel() {
        super(new BorderLayout());
    }
    
    public void init(String label) {
        _button = new JButton(label);
        _button.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                setSelected(!isSelected());
                fireExpansionEvent();
            }
        });
        _panel  = makePanel();
        _scroll = new JScrollPane(_panel);
        _scroll.getVerticalScrollBar().setUnitIncrement(25);
        add(_button, BorderLayout.NORTH);
    }

    public boolean isSelected() {
        return _open_flag;
    }
    public void setSelected(boolean flg) {
        _open_flag = flg;
        if(_open_flag) {
            add(_scroll);
        }else{
            remove(_scroll);
        }
    }

    public void addExpansionListener(ExpansionListener l) {
        _listener_List.add(ExpansionListener.class, l);
    }
    public void removeExpansionListener(ExpansionListener l) {
        _listener_List.remove(ExpansionListener.class, l);
    }
    protected void fireExpansionEvent() {
        Object[] listeners = _listener_List.getListenerList();
        for(int i = listeners.length-2; i>=0; i-=2) {
            if(listeners[i]==ExpansionListener.class) {
                if(_event == null) {
                	_event = new ExpansionEvent(this);
                }
                ((ExpansionListener)listeners[i+1]).expansionStateChanged(_event);
            }
        }
    }
}

class ExpansionEvent extends EventObject {
	private static final long serialVersionUID = 1L;
	public ExpansionEvent(Object source) {
        super(source);
    }
}

interface ExpansionListener extends EventListener {
    public void expansionStateChanged(ExpansionEvent e);
}
