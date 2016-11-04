#include <utfstring.h>
#include <vm_common.h>
#include <crtl.h>
#include "nativeSimpleMqttOps.h"
#include "MQTTClient.h"

/**
 * Class:     com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps
 * Method:    subscribe0
 * Signature: (Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IIZ)I
 */
void Java_com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps_connect0(const u4* args, JValue* pResult) {
	char * host = dvmGetStringDataInUtf8((StringObject *) args[1]);	
	char * client = dvmGetStringDataInUtf8((StringObject *) args[3]);
	char * name = dvmGetStringDataInUtf8((StringObject *) args[4]);
	char * pswd = dvmGetStringDataInUtf8((StringObject *) args[5]);
	char * will = dvmGetStringDataInUtf8((StringObject *) args[6]);
	jint port = (jint) args[2];
	jint mqttVersion = (jint) args[7];
	jint aliveInterval = (jint) args[8];
	jint cleanSession = (jboolean) args[9];    

	jint ret = mqtt_connect(host, port, client, name, pswd, will, mqttVersion, aliveInterval, cleanSession);

	CRTL_freeif(host);
	CRTL_freeif(client);
	CRTL_freeif(name);
	CRTL_freeif(pswd);
	CRTL_freeif(will);

    RETURN_INT(ret);
}

/**
 * Class:     com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps
 * Method:    subscribe0
 * Signature: (Ljava/lang/String;I)I
 */
void Java_com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps_subscribe0(const u4* args, JValue* pResult) {
	char * topic = dvmGetStringDataInUtf8((StringObject *) args[1]);    
    jint qos = (jint) args[2];
    jint ret = 0;

    // TODO: implementation
	ret = mqtt_subscribe(topic, qos);	
	CRTL_freeif(topic);

    RETURN_INT(ret);
}


/**
 * Class:     com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps
 * Method:    unsubscribe0
 * Signature: (Ljava/lang/String;)I
 */
void Java_com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps_unsubscribe0(const u4* args, JValue* pResult) {
    char * topic = dvmGetStringDataInUtf8((StringObject *) args[1]); 
    jint ret = 0;

    ret = mqtt_unsubscribe(topic);

	CRTL_freeif(topic);

    RETURN_INT(ret);
}


//TODO: just WORKAROUND ,MAYBE bugs
static char s_handle_data[1024] = {0};
char * handle_data(u2 * data, jint length){
	int i=0;
	CRTL_memset(s_handle_data,0,1024);
	assert(length <= 1024);
	for(i=0; i< length; i++){
		s_handle_data[i] = (char)(data[i] & 0xff);
	}
	return &s_handle_data[0];
}

/**
 * Class:     com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps
 * Method:    publish0
 * Signature: (Ljava/lang/String;Ljava/lang/String;IIZZ)I
 */
void Java_com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps_publish0(const u4* args, JValue* pResult) {
	//TODO: BUGs fix in getData (Encode error)
	char * topic   = dvmGetStringDataInUtf8((StringObject *) args[1]); 
	jint payloadlen = dvmGetStringLength((StringObject *) args[2]);  
	char * payload = handle_data(dvmGetStringData((StringObject *) args[2]), payloadlen);//dvmGetStringDataInUtf8((StringObject *) args[2]); 

	jint msgId	   = (jint) args[3];
    jint qos       = (jint) args[4];
    jboolean dup   = (jboolean) args[5];
    jboolean retain = (jboolean) args[6];
    jint ret = 0;

	ret = mqtt_publish(topic, payload, payloadlen, msgId, qos, dup, retain);
	DVMTraceJava("publish0 : topic- %s, %d\n", topic, ret);

//TODO : memory leak
	//CRTL_freeif(topic);
	//CRTL_freeif(payload);
	DVMTraceJava("publish0 ,ready to return: %d\n", ret);
    RETURN_INT(ret);
}

/**
 * Class:     com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps
 * Method:    disconnect0
 * Signature: ()I
 */
void Java_com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps_disconnect0(const u4* args, JValue* pResult) {
    jint ret = 0;

    ret = mqtt_disconnect();

    RETURN_INT(ret);
}

/**
 * Class:     com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps
 * Method:    close0
 * Signature: ()V
 */
void Java_com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps_close0(const u4* args, JValue* pResult) {

    mqtt_final();

    RETURN_VOID();
}

/**
 * Class:     com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps
 * Method:    recv_message0
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
void Java_com_yarlungsoft_iot_mqttv3_simple_SimpleMqttOps_recv_message0(const u4* args, JValue* pResult) {
    StringObject * topicObj = (StringObject *) args[1];
    const jchar* topic = dvmGetStringData(topicObj);
//    const char* topic = dvmCreateCstrFromString(topicObj);
    int topicLen = dvmGetStringLength(topicObj);
    StringObject * messageObj = (StringObject *) args[2];
    const jchar* message = dvmGetStringData(messageObj);
//    const char* message = dvmCreateCstrFromString(messageObj);
    int messageLen = dvmGetStringLength(messageObj);
    jint ret = 0;

    ret = mqtt_yield();

    RETURN_INT(ret);
}
