/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.belle2.daq.nsm;

import java.io.IOException;

import org.belle2.daq.base.Reader;
import org.belle2.daq.base.Serializable;
import org.belle2.daq.base.Writer;

/**
 * 
 * @author tkonno
 */
public class NSMVar implements Serializable {

	public static final int NONE = 0;
	public static final int INT = 1;
	public static final int FLOAT = 2;
	public static final int TEXT = 3;

	private String m_node = "";
	private String m_name = "";
	private int m_type = NONE;
	private int[] m_var_i = null;
	private float[] m_var_f = null;
	private String m_var_t = null;
	private int m_len = 0;
	private int m_id = 0;
	private int m_revision = 0;

	public NSMVar() {
	}

	public NSMVar(String name, String value) {
		m_type = TEXT;
		m_name = name;
		m_var_t = value;
		m_len = value.length();
	}

	public NSMVar(String name, int value) {
		m_type = INT;
		m_name = name;
		m_var_i = new int[1];
		m_var_i[0] = value;
		m_len = 0;
	}

	public NSMVar(String name, float value) {
		m_type = FLOAT;
		m_name = name;
		m_var_f = new float[1];
		m_var_f[0] = value;
		m_len = 0;
	}

	public NSMVar(String name, int[] value) {
		m_type = INT;
		m_name = name;
		m_var_i = value;
		m_len = value.length;
	}

	public NSMVar(String name, float[] value) {
		m_type = FLOAT;
		m_name = name;
		m_var_f = value;
		m_len = value.length;
	}

	public NSMVar(String value) {
		this("", value);
	}

	public NSMVar(int value) {
		this("", value);
	}

	public NSMVar(float value) {
		this("", value);
	}

	public NSMVar(int[] value) {
		this("", value);
	}

	public NSMVar(float[] value) {
		this("", value);
	}

	public void setName(String name) {
		m_name = name;
	}

	public void setNode(String node) {
		m_node = node;
	}

	public int size() {
		switch (m_type) {
		case NONE:
			return 0;
		case INT:
			return m_var_f.length * 4;
		case FLOAT:
			return m_var_f.length * 4;
		case TEXT:
			return m_var_t.length();
		}
		return 0;
	}

	public String getNode() {
		return m_node;
	}

	public String getName() {
		return m_name;
	}

	public int getType() {
		return m_type;
	}

	public int getLength() {
		switch (m_type) {
		case INT:
			return (m_var_i != null) ? m_var_i.length : 0;
		case FLOAT:
			return (m_var_f != null) ? m_var_f.length : 0;
		case TEXT:
			return (m_var_t != null) ? m_var_t.length() : 0;
		}
		return 0;
	}

	public int getInt() {
		return (m_type == INT && m_var_i != null && m_len == 0) ? m_var_i[0]
				: 0;
	}

	public float getFloat() {
		return (m_type == FLOAT && m_var_f != null && m_len == 0) ? m_var_f[0]
				: 0;
	}

	public String getText() {
		return (m_type == TEXT && m_var_t != null) ? m_var_t : "";
	}

	public int getInt(int index) {
		return (m_type == INT && m_var_i != null && m_len > index) ? m_var_i[index]
				: 0;
	}

	public float getFloat(int index) {
		return (m_type == FLOAT && m_var_f != null && m_len > index) ? m_var_f[index]
				: 0;
	}

	@Override
	public void readObject(Reader reader) throws IOException {
		m_node = reader.readString();
		m_name = reader.readString();
		m_type = reader.readInt();
		m_len = reader.readInt();
		m_id = reader.readInt();
		m_revision = reader.readInt();
		int length = (m_len > 0) ? m_len : 1;
		m_var_i = null;
		m_var_f = null;
		m_var_t = null;
		switch (m_type) {
		case INT:
			m_var_i = new int[length];
			for (int i = 0; i < length; i++) {
				m_var_i[i] = reader.readInt();
			}
			return;
		case FLOAT:
			m_var_f = new float[length];
			for (int i = 0; i < length; i++) {
				m_var_f[i] = reader.readFloat();
			}
			return;
		case TEXT:
			StringBuilder s = new StringBuilder();
			for (int i = 0; i < length; i++) {
				char c = reader.readChar();
				if (c == '\n' || c == '\r' || (c >= ' ' && c <= '~')) {
					s.append(c);
				}
			}
			m_var_t = s.toString();
		}
	}

	@Override
	public void writeObject(Writer writer) throws IOException {
		writer.writeString(m_node);
		writer.writeString(m_name);
		writer.writeInt(m_type);
		writer.writeInt(m_len);
		writer.writeInt(m_id);
		writer.writeInt(m_revision);
		switch (m_type) {
		case INT:
			for (int v : m_var_i) {
				writer.writeInt(v);
			}
			return;
		case FLOAT:
			for (float v : m_var_f) {
				writer.writeFloat(v);
			}
			return;
		case TEXT:
			for (char v : m_var_t.toCharArray()) {
				writer.writeChar(v);
			}
		}
	}

}
