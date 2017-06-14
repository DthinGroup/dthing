/*******************************************************************************
 * Copyright (c) 2009, 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution. 
 *
 * The Eclipse Public License is available at 
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at 
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Dave Locke - initial API and implementation and/or initial documentation
 */
package com.yarlungsoft.iot.mqttv3.persist;

import java.util.Enumeration;

import com.yarlungsoft.iot.mqttv3.MqttClientPersistence;
import com.yarlungsoft.iot.mqttv3.MqttPersistable;
import com.yarlungsoft.iot.mqttv3.MqttPersistenceException;


/**
 * An implementation of the {@link MqttClientPersistence} interface that provides
 * file based persistence.
 * 
 * A directory is specified when the Persistence object is created. When the persistence
 * is then opened (see {@link #open(String, String)}), a sub-directory is made beneath the base
 * for this client ID and connection key. This allows one persistence base directory
 * to be shared by multiple clients.
 * 
 * The sub-directory's name is created from a concatenation of the client ID and connection key
 * with any instance of '/', '\\', ':' or ' ' removed.
 */
public class MqttDefaultFilePersistence implements MqttClientPersistence {
	private static final String MESSAGE_FILE_EXTENSION = ".msg";
	private static final String MESSAGE_BACKUP_FILE_EXTENSION = ".bup";
	private static final String LOCK_FILENAME = ".lck";
	@Override
	public void open(String clientId, String serverURI) throws MqttPersistenceException {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void close() throws MqttPersistenceException {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void put(String key, MqttPersistable persistable) throws MqttPersistenceException {
		// TODO Auto-generated method stub
		
	}
	@Override
	public MqttPersistable get(String key) throws MqttPersistenceException {
		// TODO Auto-generated method stub
		return null;
	}
	@Override
	public void remove(String key) throws MqttPersistenceException {
		// TODO Auto-generated method stub
		
	}
	@Override
	public Enumeration keys() throws MqttPersistenceException {
		// TODO Auto-generated method stub
		return null;
	}
	@Override
	public void clear() throws MqttPersistenceException {
		// TODO Auto-generated method stub
		
	}
	@Override
	public boolean containsKey(String key) throws MqttPersistenceException {
		// TODO Auto-generated method stub
		return false;
	} 
	
}
