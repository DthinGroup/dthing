/*******************************************************************************
 * Copyright (c) 2009, 2015 IBM Corp.
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
 *    Ian Craggs - MQTT 3.1.1 support
 *    Ian Craggs - per subscription message handlers (bug 466579)
 *    Ian Craggs - ack control (bug 472172)
 */

package com.yarlungsoft.iot.mqttv3.simple;

import com.yarlungsoft.iot.mqttv3.IMqttMessageListener;
import com.yarlungsoft.iot.mqttv3.MqttCallback;
import com.yarlungsoft.iot.mqttv3.MqttConnectOptions;
import com.yarlungsoft.iot.mqttv3.MqttException;
import com.yarlungsoft.iot.mqttv3.MqttMessage;
import com.yarlungsoft.iot.mqttv3.MqttPersistenceException;
import com.yarlungsoft.iot.mqttv3.MqttSecurityException;
import com.yarlungsoft.iot.mqttv3.MqttTopic;

/**
 * Enables an application to communicate with an MQTT server using blocking methods.
 * <p>
 * This interface allows applications to utilize all features of the MQTT version 3.1
 * specification including:
 * <ul>
 * <li>connect
 * <li>publish
 * <li>subscribe
 * <li>unsubscribe
 * <li>disconnect
 * </ul>
 * </p>
 * <p>
 * There are two styles of MQTT client, this one and {@link IMqttAsyncClient}.
 * <ul>
 * <li>IMqttClient provides a set of methods that block and return control to the application
 * program once the MQTT action has completed.</li>
 * <li>IMqttAsyncClient provides a set of non-blocking methods that return control to the
 * invoking application after initial validation of parameters and state. The main processing is
 * performed in the background so as not to block the application programs thread. This non
 * blocking approach is handy when the application wants to carry on processing while the
 * MQTT action takes place. For instance connecting to an MQTT server can take time, using
 * the non-blocking connect method allows an application to display a busy indicator while the
 * connect action is occurring. Non-blocking methods are particularly useful in event-oriented
 * programs and graphical programs where issuing methods that take time to complete on the the
 * main or GUI thread can cause problems.</li>
 * </ul>
 * </p>
 * <p>
 * The non-blocking client can also be used in a blocking form by turning a non-blocking
 * method into a blocking invocation using the following pattern:
 *     <code><pre>
 *     IMqttToken token;
 *     token = asyncClient.method(parms).waitForCompletion();
 *     </pre></code>
 * Using the non-blocking client allows an application to use a mixture of blocking and
 * non-blocking styles. Using the blocking client only allows an application to use one
 * style. The blocking client provides compatibility with earlier versions
 * of the MQTT client.</p>
 */
public interface ISimpleMqttClient { //extends IMqttAsyncClient {
	public static final int SUCCESS = 0;
	public static final int FAILURE = -1;
	
	
	/**
	 * Connects to an MQTT server using the default options.
	 * <p>The default options are specified in {@link MqttConnectOptions} class.
	 * </p>
	 *
	 * @throws MqttSecurityException when the server rejects the connect for security
	 * reasons
	 * @throws MqttException  for non security related problems
	 * @see #connect(MqttConnectOptions)
	 */
  public int connect(String scheme, String host, int port) throws MqttException, Exception;

	/**
	 * Connects to an MQTT server using the specified options.
	 * <p>The server to connect to is specified on the constructor.
	 * It is recommended to call {@link #setCallback(MqttCallback)} prior to
	 * connecting in order that messages destined for the client can be accepted
	 * as soon as the client is connected.
	 * </p>
	 * <p>This is a blocking method that returns once connect completes</p>
	 *
	 * @param options a set of connection parameters that override the defaults.
	 * @throws MqttSecurityException when the server rejects the connect for security
	 * reasons
	 * @throws MqttException  for non security related problems including communication errors
	 */
  public int connect(String scheme, String host, int port, MqttConnectOptions options) throws MqttException, Exception;
	
	/**
	 * Subscribe to a topic, which may include wildcards using a QoS of 1.
	 *
	 * @see #subscribe(String[], int[])
	 *
	 * @param topicFilter the topic to subscribe to, which can include wildcards.
	 * @throws MqttException if there was an error registering the subscription.
	 */
  public int subscribe(String topicFilter, IMqttMessageListener messageListener) throws MqttException, MqttSecurityException;
	
	/**
	 * Subscribe to a topic, which may include wildcards.
	 *
	 * @see #subscribe(String[], int[])
	 *
	 * @param topicFilter the topic to subscribe to, which can include wildcards.
	 * @param qos the maximum quality of service at which to subscribe. Messages
	 * published at a lower quality of service will be received at the published
	 * QoS.  Messages published at a higher quality of service will be received using
	 * the QoS specified on the subscribe.
	 * @throws MqttException if there was an error registering the subscription.
	 */
  public int subscribe(String topicFilter, int qos, IMqttMessageListener messageListener) throws MqttException;	

	/**
	 * Requests the server unsubscribe the client from a topic.
	 *
	 * @see #unsubscribe(String[])
	 * @param topicFilter the topic to unsubscribe from. It must match a topicFilter
	 * specified on the subscribe.
	 * @throws MqttException if there was an error unregistering the subscription.
	 */
  public int unsubscribe(String topicFilter) throws MqttException;

  
  public int disconnect() throws MqttException;

	/**
	 * Publishes a message to a topic on the server and return once it is delivered.
	 * <p>This is a convenience method, which will
	 * create a new {@link MqttMessage} object with a byte array payload and the
	 * specified QoS, and then publish it.  All other values in the
	 * message will be set to the defaults.
	 * </p>
	 *
	 * @param topic  to deliver the message to, for example "finance/stock/ibm".
	 * @param payload the byte array to use as the payload
	 * @param qos the Quality of Service to deliver the message at.  Valid values are 0, 1 or 2.
	 * @param retained whether or not this message should be retained by the server.
	 * @throws MqttPersistenceException when a problem with storing the message
	 * @throws IllegalArgumentException if value of QoS is not 0, 1 or 2.
	 * @throws MqttException for other errors encountered while publishing the message.
	 * For instance client not connected.
	 * @see #publish(String, MqttMessage)
	 * @see MqttMessage#setQos(int)
	 * @see MqttMessage#setRetained(boolean)
	 */
	public int publish(String topic, byte[] payload, int qos, boolean retained) throws MqttException;

	/**
	 * Publishes a message to a topic on the server.
	 * <p>
	 * Delivers a message to the server at the requested quality of service and returns control
	 * once the message has been delivered. In the event the connection fails or the client
	 * stops, any messages that are in the process of being delivered will be delivered once
	 * a connection is re-established to the server on condition that:
	 * <ul>
	 * <li>The connection is re-established with the same clientID
	 * <li>The original connection was made with (@link MqttConnectOptions#setCleanSession(boolean)}
	 * set to false
	 * <li>The connection is re-established with (@link MqttConnectOptions#setCleanSession(boolean)}
	 * set to false
	 * </ul>
	 * </p>
	 * <p>In the event that the connection breaks or the client stops it is still possible to determine
	 * when the delivery of the message completes. Prior to re-establishing the connection to the server:
	 * <ul>
	 * <li>Register a {@link #setCallback(MqttCallback)} callback on the client and the delivery complete
	 * callback will be notified once a delivery of a message completes
	 * <li>or call {@link #getPendingDeliveryTokens()} which will return a token for each message that
	 * is in-flight.  The token can be used to wait for delivery to complete.
	 * </ul>
	 * </p>
	 *
	 * <p>When building an application,
	 * the design of the topic tree should take into account the following principles
	 * of topic name syntax and semantics:</p>
	 *
	 * <ul>
	 * 	<li>A topic must be at least one character long.</li>
	 * 	<li>Topic names are case sensitive.  For example, <em>ACCOUNTS</em> and <em>Accounts</em> are
	 * 	two different topics.</li>
	 * 	<li>Topic names can include the space character.  For example, <em>Accounts
	 * 	payable</em> is a valid topic.</li>
	 * 	<li>A leading "/" creates a distinct topic.  For example, <em>/finance</em> is
	 * 	different from <em>finance</em>. <em>/finance</em> matches "+/+" and "/+", but
	 * 	not "+".</li>
	 * 	<li>Do not include the null character (Unicode<samp class="codeph"> \x0000</samp>) in
	 * 	any topic.</li>
	 * </ul>
	 *
	 * <p>The following principles apply to the construction and content of a topic
	 * tree:</p>
	 *
	 * <ul>
	 * 	<li>The length is limited to 64k but within that there are no limits to the
	 * 	number of levels in a topic tree.</li>
	 * 	<li>There can be any number of root nodes; that is, there can be any number
	 * 	of topic trees.</li>
	 * 	</ul>
	 * </p>
	 *
	 * <p>This is a blocking method that returns once publish completes</p>	 *
	 *
	 * @param topic  to deliver the message to, for example "finance/stock/ibm".
	 * @param message to delivery to the server
 	 * @throws MqttPersistenceException when a problem with storing the message
	 * @throws IllegalArgumentException if value of QoS is not 0, 1 or 2.
	 * @throws MqttException for other errors encountered while publishing the message.
	 * For instance client not connected.
	 */
	public int publish(String topic, MqttMessage message) throws MqttException;

	/**
	 * Sets the callback listener to use for events that happen asynchronously.
	 * <p>There are a number of events that listener will be notified about. These include
	 * <ul>
	 * <li>A new message has arrived and is ready to be processed</li>
	 * <li>The connection to the server has been lost</li>
	 * <li>Delivery of a message to the server has completed.</li>
	 * </ul>
	 * </p>
	 * <p>Other events that track the progress of an individual operation such
	 * as connect and subscribe can be tracked using the {@link MqttToken} passed to the
	 * operation<p>
	 * @see MqttCallback
	 * @param callback the class to callback when for events related to the client
	 */
	public void setCallback(MqttCallback callback);


	/**
	 * Determines if this client is currently connected to the server.
	 *
	 * @return <code>true</code> if connected, <code>false</code> otherwise.
	 */
	public boolean isConnected();

	/**
	 * Returns the client ID used by this client.
	 * <p>All clients connected to the
	 * same server or server farm must have a unique ID.
	 * </p>
	 *
	 * @return the client ID used by this client.
	 */
	public String getClientId();
	
	/**
	 * Close the client
	 * Releases all resource associated with the client. After the client has
	 * been closed it cannot be reused. For instance attempts to connect will fail.
	 * @throws MqttException  if the client is not disconnected.
	 */
	public void close() throws MqttException;
}
