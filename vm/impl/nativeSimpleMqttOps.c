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