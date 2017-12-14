/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.belle2.daq.nsm;

import java.io.IOException;

import org.belle2.daq.database.FieldInfo;


/**
 * 
 * @author Tomoyuki Konno
 */
public abstract class NSMDATASetParamHandler extends NSMRequestHandler {

	private String m_name = "";
	private String m_format = "";
	private String m_pname = "";
	private int m_type = FieldInfo.NONE;

	String getDataName() {
		return m_name;
	}

	String getParName() {
		return m_pname;
	}

	int getType() {
		return m_type;
	}

	public NSMDATASetParamHandler(boolean once, String name, String format, String pname) {
		super(once);
		m_name = name;
		m_format = format;
		m_pname = pname;
	}

	public NSMDATASetParamHandler(String name, String format, String pname) {
		this(false, name, format, pname);
	}

	@Override
	public boolean handle(NSMCommand cmd, NSMMessage msg, NSMCommunicator com) {
		if (cmd.equals(NSMCommand.DATASET)) {
			NSMData data = (NSMData) msg.getObject();
			if (!m_name.equals(data.getName()))
				return false;
			try {
				Object obj = data.find(m_pname);
				if (obj == null) {
					System.out.println("No data for " + m_pname);
					return false;
				}
				FieldInfo.Property pro = data.getProperty(m_pname);
				m_type = pro.getType();
				switch (m_type) {
				case FieldInfo.CHAR:
					return handleDataSetParam((Character) obj, com);
				case FieldInfo.SHORT:
					return handleDataSetParam((Short) obj, com);
				case FieldInfo.INT:
					return handleDataSetParam((Integer) obj, com);
				case FieldInfo.LONG:
					return handleDataSetParam((Long) obj, com);
				case FieldInfo.FLOAT:
					return handleDataSetParam((Float) obj,com);
				case FieldInfo.DOUBLE:
					return handleDataSetParam((Double) obj,com);
				default:
					if (obj instanceof Character) 
						return handleDataSetParam((Character) obj, com);
					else if (obj instanceof Short) 
						return handleDataSetParam((Short) obj, com);
					else if (obj instanceof Integer) 
						return handleDataSetParam((Integer) obj, com);
					else if (obj instanceof Long) 
						return handleDataSetParam((Long) obj, com);
					else if (obj instanceof Float) 
						return handleDataSetParam((Float) obj, com);
					else if (obj instanceof Double) 
						return handleDataSetParam((Double) obj, com);
					break;
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
			if (m_com != null)
        		m_com.remove(this);
		}
		return false;
	}

	@Override
	public boolean connected(NSMCommunicator com) {
		try {
			if (com != null)
        		com.requestDATAGet(m_name, m_format);
		} catch (IOException e) {
			e.printStackTrace();
		}
		return true;
	}

	public boolean handleDataSetParam(char val, NSMCommunicator com) {
		return true;
	}

	public boolean handleDataSetParam(short val, NSMCommunicator com) {
		return true;
	}

	public boolean handleDataSetParam(int val, NSMCommunicator com) {
		return true;
	}

	public boolean handleDataSetParam(float val, NSMCommunicator com) {
		return true;
	}

	public boolean handleDataSetParam(double val, NSMCommunicator com) {
		return true;
	}

}
