/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
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
 *    Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/
#include "std_global.h"
#include "MQTTClient.h"


#undef MQTT_TRACE
#define MQTT_TRACE
#if defined(MQTT_TRACE)
	#define MQTT_Trace DVMTraceJava
#else
	#define MQTT_Trace
#endif


static void NewMessageData(MessageData* md, MQTTString* aTopicName, MQTTMessage* aMessage) {
    md->topicName = aTopicName;
    md->message = aMessage;
}


static int getNextPacketId(MQTTClient *c) {
    return c->next_packetid = (c->next_packetid == MAX_PACKET_ID) ? 1 : c->next_packetid + 1;
}


static int sendPacket(MQTTClient* c, int length, Timer* timer)
{
    int rc = FAILURE, 
        sent = 0;
    
    while (sent < length && !TimerIsExpired(timer))
    {
        rc = c->ipstack->mqttwrite(c->ipstack, &c->buf[sent], length, TimerLeftMS(timer));
        if (rc < 0)  // there was an error writing the data
            break;
        sent += rc;
    }
    if (sent == length)
    {
        TimerCountdown(&c->ping_timer, c->keepAliveInterval); // record the fact that we have successfully sent the packet
        rc = SUCCESS;
    }
    else
        rc = FAILURE;
    return rc;
}



static char * reCallocAndCopy(char * str){
	char * ret ;
	int len = CRTL_strlen(str);
	if(str == NULL)
		return NULL;

	ret = CRTL_malloc(len + 1);
	if(ret == NULL)
		return NULL;

	CRTL_memset(ret, 0, len + 1);
	CRTL_memcpy(ret, str, len);
	return ret;
}

void MQTTClientInit(MQTTClient* c, Network* network, unsigned int command_timeout_ms,
		unsigned char* sendbuf, size_t sendbuf_size, unsigned char* readbuf, size_t readbuf_size)
{
    int i;
    c->ipstack = network;
    
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
        c->messageHandlers[i].topicFilter = 0;
    c->command_timeout_ms = command_timeout_ms;
    c->buf = sendbuf;
    c->buf_size = sendbuf_size;
    c->readbuf = readbuf;
    c->readbuf_size = readbuf_size;
    c->isconnected = 0;
    c->ping_outstanding = 0;
    c->defaultMessageHandler = NULL;
	c->next_packetid = 1;
    TimerInit(&c->ping_timer);
#if defined(MQTT_TASK)
	MutexInit(&c->mutex);
#endif
}

void MQTTClientFinal(MQTTClient* c){
	int i;
	for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
        CRTL_freeif(c->messageHandlers[i].topicFilter);
}


static int decodePacket(MQTTClient* c, int* value, int timeout)
{
    unsigned char i;
    int multiplier = 1;
    int len = 0;
    const int MAX_NO_OF_REMAINING_LENGTH_BYTES = 4;

    *value = 0;
    do
    {
        int rc = MQTTPACKET_READ_ERROR;

        if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES)
        {
            rc = MQTTPACKET_READ_ERROR; /* bad data */
            goto exit;
        }
        rc = c->ipstack->mqttread(c->ipstack, &i, 1, timeout);
        if (rc != 1)
            goto exit;
        *value += (i & 127) * multiplier;
        multiplier *= 128;
    } while ((i & 128) != 0);
exit:
    return len;
}


static int readPacket(MQTTClient* c, Timer* timer)
{
    int rc = FAILURE;
    MQTTHeader header = {0};
    int len = 0;
    int rem_len = 0;

    /* 1. read the header byte.  This has the packet type in it */
    if (c->ipstack->mqttread(c->ipstack, c->readbuf, 1, TimerLeftMS(timer)) != 1)
        goto exit;

    len = 1;
    /* 2. read the remaining length.  This is variable in itself */
    decodePacket(c, &rem_len, TimerLeftMS(timer));
    len += MQTTPacket_encode(c->readbuf + 1, rem_len); /* put the original remaining length back into the buffer */

    /* 3. read the rest of the buffer using a callback to supply the rest of the data */
    if (rem_len > 0 && (c->ipstack->mqttread(c->ipstack, c->readbuf + len, rem_len, TimerLeftMS(timer)) != rem_len))
        goto exit;

    header.byte = c->readbuf[0];
    rc = header.bits.type;
exit:
    return rc;
}


// assume topic filter and name is in correct format
// # can only be at end
// + and # can only be next to separator
static char isTopicMatched(char* topicFilter, MQTTString* topicName)
{
    char* curf = topicFilter;
    char* curn = topicName->lenstring.data;
    char* curn_end = curn + topicName->lenstring.len;
    
    while (*curf && curn < curn_end)
    {
        if (*curn == '/' && *curf != '/')
            break;
        if (*curf != '+' && *curf != '#' && *curf != *curn)
            break;
        if (*curf == '+')
        {   // skip until we meet the next separator, or end of string
            char* nextpos = curn + 1;
            while (nextpos < curn_end && *nextpos != '/')
                nextpos = ++curn + 1;
        }
        else if (*curf == '#')
            curn = curn_end - 1;    // skip until end of string
        curf++;
        curn++;
    };
    
    return (curn == curn_end) && (*curf == '\0');
}


int deliverMessage(MQTTClient* c, MQTTString* topicName, MQTTMessage* message)
{
    int i;
    int rc = FAILURE;

    // we have to find the right message handler - indexed by topic
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->messageHandlers[i].topicFilter != 0 && (MQTTPacket_equals(topicName, (char*)c->messageHandlers[i].topicFilter) ||
                isTopicMatched((char*)c->messageHandlers[i].topicFilter, topicName)))
        {
            if (c->messageHandlers[i].fp != NULL)
            {
                MessageData md;
                NewMessageData(&md, topicName, message);
                c->messageHandlers[i].fp(&md);
                rc = SUCCESS;
            }
        }
    }
    
    if (rc == FAILURE && c->defaultMessageHandler != NULL) 
    {
        MessageData md;
        NewMessageData(&md, topicName, message);
        c->defaultMessageHandler(&md);
        rc = SUCCESS;
    }   
    
    return rc;
}


int keepalive(MQTTClient* c)
{
    int rc = FAILURE;

    if (c->keepAliveInterval == 0)
    {
        rc = SUCCESS;
        goto exit;
    }

    if (TimerIsExpired(&c->ping_timer))
    {
        if (!c->ping_outstanding)
        {
			int len;
            Timer timer;
            TimerInit(&timer);
            TimerCountdownMS(&timer, 1000);
            len = MQTTSerialize_pingreq(c->buf, c->buf_size);
            if (len > 0 && (rc = sendPacket(c, len, &timer)) == SUCCESS) // send the ping packet
                c->ping_outstanding = 1;
        }
    }

exit:
    return rc;
}


int cycle(MQTTClient* c, Timer* timer)
{
    // read the socket, see what work is due
    unsigned short packet_type = readPacket(c, timer);
    
    int len = 0,
        rc = SUCCESS;

    switch (packet_type)
    {
        case CONNACK:
        case PUBACK:
        case SUBACK:
            break;
        case PUBLISH:
        {
            MQTTString topicName;
            MQTTMessage msg;
            int intQoS;
            if (MQTTDeserialize_publish(&msg.dup, &intQoS, &msg.retained, &msg.id, &topicName,
               (unsigned char**)&msg.payload, (int*)&msg.payloadlen, c->readbuf, c->readbuf_size) != 1)
                goto exit;
            msg.qos = (enum QoS)intQoS;
            deliverMessage(c, &topicName, &msg);
            if (msg.qos != QOS0)
            {
                if (msg.qos == QOS1)
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBACK, 0, msg.id);
                else if (msg.qos == QOS2)
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREC, 0, msg.id);
                if (len <= 0)
                    rc = FAILURE;
                else
                    rc = sendPacket(c, len, timer);
                if (rc == FAILURE)
                    goto exit; // there was a problem
            }
            break;
        }
        case PUBREC:
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = FAILURE;
            else if ((len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREL, 0, mypacketid)) <= 0)
                rc = FAILURE;
            else if ((rc = sendPacket(c, len, timer)) != SUCCESS) // send the PUBREL packet
                rc = FAILURE; // there was a problem
            if (rc == FAILURE)
                goto exit; // there was a problem
            break;
        }
        case PUBCOMP:
            break;
        case PINGRESP:
            c->ping_outstanding = 0;
            break;
    }
    keepalive(c);
exit:
    if (rc == SUCCESS)
        rc = packet_type;
    return rc;
}


int MQTTYield(MQTTClient* c, int timeout_ms)
{
    int rc = SUCCESS;
    Timer timer;

    TimerInit(&timer);
    TimerCountdownMS(&timer, timeout_ms);

	do
    {
        if (cycle(c, &timer) == FAILURE)
        {
            rc = FAILURE;
            break;
        }
	} while (!TimerIsExpired(&timer));
        
    return rc;
}


void MQTTRun(void* parm)
{
	Timer timer;
	MQTTClient* c = (MQTTClient*)parm;

	TimerInit(&timer);

	while (1)
	{
#if defined(MQTT_TASK)
		MutexLock(&c->mutex);
#endif
		TimerCountdownMS(&timer, 500); /* Don't wait too long if no traffic is incoming */
		cycle(c, &timer);
#if defined(MQTT_TASK)
		MutexUnlock(&c->mutex);
#endif
	} 
}


#if defined(MQTT_TASK)
int MQTTStartTask(MQTTClient* client)
{
	return ThreadStart(&client->thread, &MQTTRun, client);
}
#endif


int waitfor(MQTTClient* c, int packet_type, Timer* timer)
{
    int rc = FAILURE;
    
    do
    {
        if (TimerIsExpired(timer))
            break; // we timed out
    }
    while ((rc = cycle(c, timer)) != packet_type);  
    
    return rc;
}


int MQTTConnect(MQTTClient* c, MQTTPacket_connectData* options)
{
    Timer connect_timer;
    int rc = FAILURE;
    MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
    int len = 0;

#if defined(MQTT_TASK)
	MutexLock(&c->mutex);
#endif
	if (c->isconnected) /* don't send connect packet again if we are already connected */
		goto exit;
    
    TimerInit(&connect_timer);
    TimerCountdownMS(&connect_timer, c->command_timeout_ms);

    if (options == 0)
        options = &default_options; /* set default options if none were supplied */
    
    c->keepAliveInterval = options->keepAliveInterval;
    TimerCountdown(&c->ping_timer, c->keepAliveInterval);
    if ((len = MQTTSerialize_connect(c->buf, c->buf_size, options)) <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &connect_timer)) != SUCCESS)  // send the connect packet
        goto exit; // there was a problem
    
    // this will be a blocking call, wait for the connack
    if (waitfor(c, CONNACK, &connect_timer) == CONNACK)
    {
        unsigned char connack_rc = 255;
        unsigned char sessionPresent = 0;
        if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, c->readbuf, c->readbuf_size) == 1)
            rc = SUCCESS;//connack_rc;
        else
            rc = FAILURE;
    }
    else
        rc = FAILURE;
    
exit:
    if (rc == SUCCESS)
        c->isconnected = 1;

#if defined(MQTT_TASK)
	MutexUnlock(&c->mutex);
#endif

    return rc;
}


int MQTTSubscribe(MQTTClient* c, const char* topicFilter, enum QoS qos, messageHandler messageHandler)
{ 
    int rc = FAILURE;  
    Timer timer;
    int len = 0;
    MQTTString topic = MQTTString_initializer;	
    topic.cstring = (char *)topicFilter;
    
#if defined(MQTT_TASK)
	MutexLock(&c->mutex);
#endif
	if (!c->isconnected)
		goto exit;

    TimerInit(&timer);
    TimerCountdownMS(&timer, c->command_timeout_ms);
    
    len = MQTTSerialize_subscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic, (int*)&qos);
    if (len <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &timer)) != SUCCESS) // send the subscribe packet
        goto exit;             // there was a problem
    
    if (waitfor(c, SUBACK, &timer) == SUBACK)      // wait for suback 
    {
        int count = 0, grantedQoS = -1;
        unsigned short mypacketid;
        if (MQTTDeserialize_suback(&mypacketid, 1, &count, &grantedQoS, c->readbuf, c->readbuf_size) == 1)
            rc = grantedQoS; // 0, 1, 2 or 0x80 
        if (rc != 0x80)
        {
            int i;
            for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
            {
                if (c->messageHandlers[i].topicFilter == 0)
                {
                    c->messageHandlers[i].topicFilter = reCallocAndCopy(topicFilter);   // need to free!!
                    c->messageHandlers[i].fp = messageHandler;
                    rc = 0;
                    break;
                }
            }
        }
    }
    else 
        rc = FAILURE;
        
exit:
#if defined(MQTT_TASK)
	MutexUnlock(&c->mutex);
#endif
    return rc;
}


int MQTTUnsubscribe(MQTTClient* c, const char* topicFilter)
{   
    int rc = FAILURE;
	int len = 0;
    Timer timer;    
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;    

#if defined(MQTT_TASK)
	MutexLock(&c->mutex);
#endif
	if (!c->isconnected)
		goto exit;

    TimerInit(&timer);
    TimerCountdownMS(&timer, c->command_timeout_ms);
    
    if ((len = MQTTSerialize_unsubscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic)) <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &timer)) != SUCCESS) // send the subscribe packet
        goto exit; // there was a problem
    
    if (waitfor(c, UNSUBACK, &timer) == UNSUBACK)
    {
        unsigned short mypacketid;  // should be the same as the packetid above
        if (MQTTDeserialize_unsuback(&mypacketid, c->readbuf, c->readbuf_size) == 1)
            rc = 0; 
    }
    else
        rc = FAILURE;
    
exit:
#if defined(MQTT_TASK)
	MutexUnlock(&c->mutex);
#endif
    return rc;
}


int MQTTPublish(MQTTClient* c, const char* topicName, MQTTMessage* message)
{
    int rc = FAILURE;
	int len = 0;
    Timer timer;   
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;    

#if defined(MQTT_TASK)
	MutexLock(&c->mutex);
#endif
	if (!c->isconnected)
		goto exit;

    TimerInit(&timer);
    TimerCountdownMS(&timer, c->command_timeout_ms);

    if (message->qos == QOS1 || message->qos == QOS2)
        message->id = getNextPacketId(c);
    
    len = MQTTSerialize_publish(c->buf, c->buf_size, 0, message->qos, message->retained, message->id, 
              topic, (unsigned char*)message->payload, message->payloadlen);
    if (len <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &timer)) != SUCCESS) // send the subscribe packet
        goto exit; // there was a problem
    
    if (message->qos == QOS1)
    {
        if (waitfor(c, PUBACK, &timer) == PUBACK)
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = FAILURE;
        }
        else
            rc = FAILURE;
    }
    else if (message->qos == QOS2)
    {
        if (waitfor(c, PUBCOMP, &timer) == PUBCOMP)
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = FAILURE;
        }
        else
            rc = FAILURE;
    }
    
exit:
#if defined(MQTT_TASK)
	MutexUnlock(&c->mutex);
#endif
    return rc;
}


int MQTTDisconnect(MQTTClient* c)
{  
    int rc = FAILURE;
    Timer timer;     // we might wait for incomplete incoming publishes to complete
    int len = 0;

#if defined(MQTT_TASK)
	MutexLock(&c->mutex);
#endif
    TimerInit(&timer);
    TimerCountdownMS(&timer, c->command_timeout_ms);

	len = MQTTSerialize_disconnect(c->buf, c->buf_size);
    if (len > 0)
        rc = sendPacket(c, len, &timer);            // send the disconnect packet
        
    c->isconnected = 0;

#if defined(MQTT_TASK)
	MutexUnlock(&c->mutex);
#endif
    return rc;
}

/*Added APIs*/
//save the unique mqtt client
static MQTTClient g_only_mtqq_client;
//save the unique mqtt network
static Network g_only_mqtt_network;
//recv & send buffer of mqtt
static char g_mqtt_send_buff[512];
static char g_mqtt_recv_buff[512];
GLOBAL void mqtt_init(void)
{
	NetworkInit(&g_only_mqtt_network);	
	MQTTClientInit(&g_only_mtqq_client, &g_only_mqtt_network, 3000, g_mqtt_send_buff, sizeof(g_mqtt_send_buff), g_mqtt_recv_buff, sizeof(g_mqtt_recv_buff));	
}


GLOBAL void mqtt_final(void){
	MQTTClientFinal(&g_only_mtqq_client);
	workaround_alive_task_check_destroy();
}

GLOBAL int mqtt_connect(char * host, int port, char * clientId, char * name, char * pwd, char * will, int mqttVer, int aliveInterval, int cleanSession){
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	int ret;

	mqtt_init();

	ret = NetworkConnect(&g_only_mqtt_network, host, port);
	MQTT_Trace("=== mqtt_connect 1 ret: %d\n", ret);
	if(ret == 0)
	{
		will = will;
		data.willFlag = 0;  // not support now!
		data.MQTTVersion = mqttVer;
		data.clientID.cstring = clientId;
		data.username.cstring = name;
		data.password.cstring = pwd;

		data.keepAliveInterval = aliveInterval;
		data.cleansession = (unsigned char)cleanSession;

		ret = MQTTConnect(&g_only_mtqq_client, &data);
		MQTT_Trace("=== mqtt_connect 2 ret: %d\n", ret);		
	} else {
		MQTT_Trace("connect network fail: %d\n", ret);
	}
	
	if(SUCCESS == ret){
		workaround_alive_task_check_create();
	}

	MQTT_Trace("=== mqtt_connect 3 ret: %d\n", ret);

	return ret;	
}

static void messageArrived(MessageData* md)
{
	MQTTMessage* message = md->message;
	MQTT_Trace("messageArrived: %d.%s \n", (int)message->payloadlen, (char*)message->payload);			
	//fflush(stdout);
}

GLOBAL int mqtt_subscribe(char * topic, int qos){
	int ret ;
	MQTT_Trace("mqtt_subscribe to %s\n", topic);
	ret = MQTTSubscribe(&g_only_mtqq_client, topic, qos, messageArrived);
	MQTT_Trace("mqtt_subscribe %d\n", ret);

	return ret;
}

GLOBAL int mqtt_unsubscribe(char * topic){
	int ret ;
	MQTT_Trace("mqtt_unsubscribe to %s\n", topic);
	ret = MQTTUnsubscribe(&g_only_mtqq_client, topic);
	MQTT_Trace("mqtt_unsubscribe %d\n", ret);

	return ret;
}

GLOBAL int mqtt_publish(char * topic, char * payload, int payloadlen, int msgId, int qos, int dup, int retain){
	int ret;
	MQTTMessage message;
	message.qos = qos;
	message.dup = dup;
	message.id  = msgId;
	message.retained = retain;
	message.payload = payload;
	message.payloadlen = payloadlen;//strlen(payload);

	ret = MQTTPublish(&g_only_mtqq_client, topic, &message);
	MQTT_Trace("mqtt_public payloadlen = %d, ret=%d\n",payloadlen, ret);
	return ret;
}

GLOBAL int mqtt_disconnect(void){
	workaround_alive_task_check_destroy();
	MQTTDisconnect(&g_only_mtqq_client);
	NetworkDisconnect(&g_only_mqtt_network);

	return 0;
}

GLOBAL int mqtt_yield(void){
	return MQTTYield(&g_only_mtqq_client , 1000);
}

GLOBAL int mqtt_keepalive(void){
	return keepalive(&g_only_mtqq_client);
}

