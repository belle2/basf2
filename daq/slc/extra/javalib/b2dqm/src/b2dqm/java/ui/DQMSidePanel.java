package b2dqm.java.ui;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.EventListener;
import java.util.EventObject;
import java.util.List;

import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JEditorPane;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.event.EventListenerList;
import javax.swing.event.HyperlinkEvent;
import javax.swing.event.HyperlinkListener;
import javax.swing.text.html.HTMLEditorKit;
import javax.swing.text.html.StyleSheet;

import b2daq.ui.Updatable;
import b2dqm.core.Histo;
import b2dqm.core.HistoPackage;
import b2dqm.core.MonObject;

public class DQMSidePanel extends JPanel implements Updatable {
	private static final long serialVersionUID = 1L;

	private Box northBox  = Box.createVerticalBox();
    private Box centerBox = Box.createVerticalBox();
    private Box southBox  = Box.createVerticalBox();
    private List<HistoListPanel> panelList = new ArrayList<HistoListPanel>();
	private DQMMainPanel _main_panel;
    
    public DQMSidePanel() {
        super(new BorderLayout());
    }
    
    public void init(ArrayList<HistoPackage> pack_v) {
		System.out.println("DQMSidePanel::init");
        JPanel panel = new JPanel(new BorderLayout());
        for ( HistoPackage pack : pack_v ) {
        	panelList.add(new HistoListPanel(pack, this));
        }
        ExpansionListener rl = new ExpansionListener() {
            public void expansionStateChanged(ExpansionEvent e) {
                initComps(panelList, e);
            }
        };
        for(HistoListPanel exp: panelList) {
            northBox.add(exp);
            exp.addExpansionListener(rl);
        }
        panel.add(northBox,  BorderLayout.NORTH);
        panel.add(centerBox);
        panel.add(southBox,  BorderLayout.SOUTH);
        panel.setMinimumSize(new Dimension(120, 0));
        add(panel);
        setPreferredSize(new Dimension(420, 240));
		System.out.println("DQMSidePanel::init");
    }
    
    private void initComps(List<HistoListPanel> list, ExpansionEvent e) {
        setVisible(false);
        centerBox.removeAll();
        northBox.removeAll();
        southBox.removeAll();
        HistoListPanel es = (HistoListPanel) e.getSource();
        boolean insertSouth = false;
        for(HistoListPanel exp: list) {
            if(exp==es && exp.isSelected()) {
                centerBox.add(exp);
                insertSouth = true;
            }else if(insertSouth) {
                exp.setSelected(false);
                southBox.add(exp);
            }else{
                exp.setSelected(false);
                northBox.add(exp);
            }
        }
        setVisible(true);
    }
    
	public void setMainPanel(DQMMainPanel main_panel) {
		_main_panel = main_panel;
	}
	public DQMMainPanel getMainPanel() {
		return _main_panel;
	}

	public void update() {
	}

}

class HistoListPanel extends JPanel {

	private static final long serialVersionUID = 1L;

	private HistoPackage _pack;
    private JButton button;
    private JEditorPane panel;
    private JScrollPane scroll;
    private boolean openFlag = false;
    private EventListenerList listenerList = new EventListenerList();
    private ExpansionEvent expansionEvent = null;
	private DQMSidePanel _side_panel;

    public HistoListPanel(HistoPackage pack, DQMSidePanel side_panel) {
        super(new BorderLayout());
        _pack = pack;
        _side_panel = side_panel;
        button = new JButton(pack.getName());
		button.setFont(new Font("Sans", Font.PLAIN, 14));
        button.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                setSelected(!isSelected());
                fireExpansionEvent();
            }
        });
        StyleSheet styleSheet = new StyleSheet();
        styleSheet.addRule("a {font-size: 14; font-family:Sans; color:blue;};");
        HTMLEditorKit htmlEditorKit = new HTMLEditorKit();
        htmlEditorKit.setStyleSheet(styleSheet);
        panel = new JEditorPane("text/html", "");
        panel.setEditorKit(htmlEditorKit);
        StringBuffer buf = new StringBuffer();
        buf.append("<html>");
        for ( MonObject obj: _pack.getMonObjects() ) {
        	try {
        		Histo h = (Histo)obj;
        		buf.append(" - <a href='"+pack.getName()+":"+obj.getName()+"'>" + 
        				h.getTitle() /*+" ("+h.getName() +")"*/+"</a><br/>");
        	} catch (Exception e) {
        	//	e.printStackTrace();
        	}
        }
        buf.append("</html>");
        panel.setText(buf.toString());
        panel.setOpaque(false);
        panel.addHyperlinkListener(new HyperlinkListener() {
          public void hyperlinkUpdate(HyperlinkEvent e) {
            if( e.getEventType() == HyperlinkEvent.EventType.ACTIVATED ) {
            	String [] name_v = e.getDescription().split(":");
            	if ( !DQMMainFrame.get().isSeparated() ) {
            		Component com = _side_panel.getMainPanel().getPanel(name_v[0]);
            		if ( com != null ) {
                		System.out.println(name_v[1]);
            			_side_panel.getMainPanel().setSelectedComponent(com);
            			DQMMainPanel.lookfor(com, name_v[1]);
            		}
            	} else {
            		ArrayList<JFrame> frame_v = DQMMainFrame.get().getChildFrames();
            		for ( JFrame frame : frame_v ) {
            			if ( name_v[0].matches(frame.getTitle()) ) {
            				frame.toFront();
            				DQMMainPanel.lookfor(frame.getComponent(0), name_v[1]);
            				return;
            			}
            		}
            	}
            }
          }
        });
        panel.setEditable(false); 
        scroll = new JScrollPane(panel);
        scroll.getVerticalScrollBar().setUnitIncrement(25);
        add(button, BorderLayout.NORTH);
    }

    public boolean isSelected() {
        return openFlag;
    }
    public void setSelected(boolean flg) {
        openFlag = flg;
        if(openFlag) {
            add(scroll);
        }else{
            remove(scroll);
        }
    }

    public void addExpansionListener(ExpansionListener l) {
        listenerList.add(ExpansionListener.class, l);
    }
    public void removeExpansionListener(ExpansionListener l) {
        listenerList.remove(ExpansionListener.class, l);
    }
    protected void fireExpansionEvent() {
        Object[] listeners = listenerList.getListenerList();
        for(int i = listeners.length-2; i>=0; i-=2) {
            if(listeners[i]==ExpansionListener.class) {
                if(expansionEvent == null) {
                	expansionEvent = new ExpansionEvent(this);
                }
                ((ExpansionListener)listeners[i+1]).expansionStateChanged(expansionEvent);
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
