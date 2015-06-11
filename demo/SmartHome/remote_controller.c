#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <error.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/select.h>
#include <stdbool.h>
#include <stdlib.h>

#define AS_DEVICE_SERVER_PORT  7777
#define AS_APP_SERVER_PORT  7778
#define DEBUG 1
#define DEFAULT_HEADER_LENGTH (DEFAULT_SIZE_LENGTH * 2)
#define DEFAULT_SIZE_LENGTH 4
#define DEFAULT_CONTENT_START_POSITION (DEFAULT_HEADER_LENGTH + DEFAULT_SIZE_LENGTH)
#define MIN_CMD_RESP_LEN DEFAULT_HEADER_LENGTH
#define AIR_CONDITIONER "air-conditioner"
#define FAN "fan"

typedef enum
{
	CMD_NULL = 0,
	CMD_AIR_CONDITIONER,
	CMD_FAN,
	CMD_COUNT
} CMD;

const char * CMD_NAME[] = 
{
    "CMD+NULL",
    AIR_CONDITIONER,
    FAN,
};

enum
{
    STATUS_REFRIGERATION = 0,
    STATUS_HEATING = 1,
    STATUS_AERATION = 2,
    STATUS_AREFACTION = 3,
    STATUS_WIND_SWING = 4,
    STATUS_AUTO = 5,
    STATUS_POWER = 6,
    STATUS_TEMPERATURE = 7,
    STATUS_WIND_FORCE = 8,
    STATUS_MAX
};

//typedef bool;

int g_AirConditionerStatus[STATUS_MAX] = {0};

enum
{
    BUTTON_WIND_FORCE = 0,
    BUTTON_POWER = 1,
    BUTTON_WIND_SWING = 2,
    BUTTON_SCHEDULE = 3,
    BUTTON_MAX
};

//typedef bool;
int g_FanButtonPressed = -1;

unsigned char * intToByteArray(int val)
{
    static unsigned char buff[4] = {0};

    buff[0] = (val >> 24 ) & 0xff;
    buff[1] = (val >> 16 ) & 0xff;
    buff[2] = (val >>  8 ) & 0xff;
    buff[3] = (val >>  0 ) & 0xff;

    return buff;
}

int byteArrayToInt(unsigned char * buff)
{
	int part1 = (buff[0] << 24) & 0xff000000;
	int part2 = (buff[1] << 16) & 0xff0000;
	int part3 = (buff[2] << 8) & 0xff00;
	int part4 = buff[3] & 0xff;
    int val = part1 + part2 + part3 + part4;

    return val;
}

int ReceiveCmd(int fd, unsigned char *buff, int * len)
{
    int length = 0;
    int total = 0;

    //2 bytes : stream length
    length = recv(fd, (unsigned char*)buff, DEFAULT_SIZE_LENGTH, MSG_WAITALL);
    if (DEBUG) printf("1.recv bytes:%d from %d \n", length, fd);

    if (length != DEFAULT_SIZE_LENGTH)
    {
        printf("[exception] net error,please retry!\n");
        return 0;
    }

    total = byteArrayToInt(buff);
    printf("total:%d \n",total);

    length = recv(fd, (unsigned char*)&buff[DEFAULT_SIZE_LENGTH], total - DEFAULT_SIZE_LENGTH, MSG_WAITALL);
    if(DEBUG) printf("2.recv bytes:%d from %d\n", length, fd);
    if (length != total - DEFAULT_SIZE_LENGTH)
    {
        printf("  [exception] net error,please retry!\n");
        return 0;
    }

    *len = total;

    return 1;
}

int SendToDevice(int fd, int cmd, char *buff, int len, char *buff1, int len1)
{
    int ret =0;
    int gramlen = 0;
    char grambuf[1024] = {0};
    const char *cmdName = CMD_NAME[cmd];
    int i = 0;
    
    printf("send %s info to device\n", cmdName);

    gramlen = DEFAULT_HEADER_LENGTH + len + ((len > 0)? 4 : 0) + len1 + ((len1 > 0)? 4 : 0);

    printf("SendToDevice,gramlen:%d, len = %d, buff = %s, buff1 = %s \n", gramlen, len, buff, (len1>0 ? buff1 : "NULL"));
    if (DEBUG)
    {
        for (i = 0; i < len; i++)
        {
            printf("buff[%d] = %d\n", i, buff[i]);
        }
    }

    memcpy(&grambuf[0], intToByteArray(gramlen), DEFAULT_SIZE_LENGTH);  //data_len
    memcpy(&grambuf[DEFAULT_SIZE_LENGTH], intToByteArray(cmd), DEFAULT_SIZE_LENGTH);  //CMD_Type
    if (len > 0)
    {
		memcpy(&grambuf[DEFAULT_HEADER_LENGTH], intToByteArray(len), DEFAULT_SIZE_LENGTH); //param len
		memcpy(&grambuf[DEFAULT_CONTENT_START_POSITION], buff, len);  //param string

        if (len1 > 0)
        {
            memcpy(&grambuf[DEFAULT_CONTENT_START_POSITION + len], intToByteArray(len1), DEFAULT_SIZE_LENGTH); //param len
            memcpy(&grambuf[DEFAULT_CONTENT_START_POSITION + DEFAULT_SIZE_LENGTH + len], buff1, len1);  //param string
        }
    }

    ret = send(fd, grambuf, gramlen, 0);

    if(ret < 0)
    {
        printf("[exception] send to device msg error! ret = %d\n", ret);
        return 0;
    }
    else
    {
        //reset the button pressed since send to device successfully
        if (cmd == CMD_FAN && buff[0] != -1)
        {
            g_FanButtonPressed = -1;
        }
    }
    if(DEBUG) printf("send to device byte:%d \n", ret);

    return 1;
}

int getSendGramBuff(unsigned char *buff, unsigned char * data, CMD cmdType)
{
	char value[20] = {0};
	int dataLength = strlen(data);
	if (!buff || !data || strlen(data) < MIN_CMD_RESP_LEN)
	{
		return -1;
	}
	memcpy(&buff[0], data, dataLength);
	if (strlen(value) > 0)
	{
		memcpy(&buff[dataLength], intToByteArray(dataLength), DEFAULT_SIZE_LENGTH);
		memcpy(&buff[dataLength + DEFAULT_SIZE_LENGTH], value, strlen(value));
	}
	return 0;
}

int GetAirConditionerSendBuff(unsigned char *buff)
{
    int dataLength = 0;
    if (g_AirConditionerStatus[STATUS_POWER])
    {
        int i = 0;
        unsigned char data[STATUS_MAX * 2] = {0};
        for (i = 0; i < STATUS_MAX; i++)
        {
            if (g_AirConditionerStatus[i])
            {
                data[dataLength] = i;
                data[dataLength + 1] = g_AirConditionerStatus[i];
                dataLength += 2;
            }
        }
        memcpy(buff, data, sizeof(data));
    }
    else
    {
        unsigned char data[2];
        data[0] = STATUS_POWER;
        data[1] = 0;
        memcpy(buff, data, sizeof(data));
        dataLength = 2;
    }
    return dataLength;
}

int GetFanSendBuff(unsigned char *buff)
{
    if (DEBUG)
    {
        printf("g_FanButtonPressed = %d\n", g_FanButtonPressed);
    }
    buff[0] = g_FanButtonPressed;
    return 1;
}

int PraseReceivedDeviceData(int fd, unsigned char * buff, int len)
{
    int ret = 0;
    int bufLen = 0;
    int cmdType = 0;
    int cmdResult = -1;

    unsigned char buff0[100] = {0};
    unsigned char sendContent[100] = {0};

    /*gram head*/
    bufLen = byteArrayToInt(buff);
    if (len < MIN_CMD_RESP_LEN || bufLen < MIN_CMD_RESP_LEN)
    {
        if (DEBUG)
        {
            printf("Error: The response data is less than %d!", MIN_CMD_RESP_LEN);
        }
        return ret;
    }
    cmdType = byteArrayToInt(buff + DEFAULT_SIZE_LENGTH);
    strcat(buff0,"> Recv cmd result of ");
    switch (cmdType)
    {
    case CMD_AIR_CONDITIONER:
    	cmdResult = GetAirConditionerSendBuff(sendContent);
    	break;
    	
    case CMD_FAN:
        cmdResult = GetFanSendBuff(sendContent);
        break;
    	
    default:
    	printf("PraseReceivedDeviceData undefined command:%d\n", cmdType);
    	return ret;
    }

	sprintf(buff0, "%s[%s]: cmdResult = %d, ", buff0, CMD_NAME[cmdType], cmdResult);
    if (cmdResult >= 0)
    {
		strcat(buff0, "success! ");
		SendToDevice(fd, cmdType, sendContent, cmdResult, NULL, 0);
		ret = 1;
    }
    else
    {
        strcat(buff0, "fail! ");
    }

    printf("-----------------------------------------\n");
    printf("%s\n",buff0);
    printf("-----------------------------------------\n");

    return ret;
}

int StoreAirConditionerStatus(unsigned char * buff, int len)
{
    unsigned char *content = buff;
    int type = -1;
    int value = -1;
    while (len != 0)
    {
        type = *content++;
        value = *content++;
        len -= 2;
        if (type >= STATUS_REFRIGERATION && type < STATUS_MAX)
        {
            g_AirConditionerStatus[type] = value;
        }
        else
        {
            printf("Error: unknown air conditioner command\n");
            return 0;
        }
    }
    return 1;
}

int StoreFanButtonPressTime(unsigned char * buff, int len)
{
    if (len != 1)
    {
        printf("Error: fan only handle one button one time\n");
        return 0;
    }
    unsigned char *content = buff;
    int type = -1;
    type = *content++;
    len -= 2;
    printf("%d pressed\n", type);
    if (type >= BUTTON_WIND_FORCE && type < BUTTON_MAX)
    {
        g_FanButtonPressed = type;
    }
    else
    {
        printf("Error: unknown fan button\n");
        return 0;
    }
    return 1;
}

int PraseReceivedAppData(unsigned char * buff, int len)
{
    int ret = 0;
    int bufLen = 0;
    int cmdType = 0;
    int dataLength = 0;
    unsigned char content[1024] = {0};

    unsigned char buff0[100] = {0};
    /*gram head*/
    bufLen = byteArrayToInt(buff);
    printf("bufLen = %d\n", bufLen);
    if (len < MIN_CMD_RESP_LEN || bufLen < MIN_CMD_RESP_LEN)
    {
        if (DEBUG)
        {
            printf("Error: The response data is less than %d!", MIN_CMD_RESP_LEN);
        }
        return ret;
    }
    cmdType = byteArrayToInt(buff + DEFAULT_SIZE_LENGTH);
    printf("cmdType = %s\n", CMD_NAME[cmdType]);
    dataLength = byteArrayToInt(buff + DEFAULT_HEADER_LENGTH);
    printf("dataLength = %d\n", dataLength);
    memcpy(content, &buff[DEFAULT_CONTENT_START_POSITION], dataLength);
    switch (cmdType)
    {
    case CMD_AIR_CONDITIONER:
    	ret = StoreAirConditionerStatus(content, dataLength);
    	break;
    case CMD_FAN:
        ret = StoreFanButtonPressTime(content, dataLength);
        break;
    	
    default:
    	printf("PraseReceivedAppData undefined command:%d\n", cmdType);
    	return ret;
    }

	sprintf(buff0, "> Recv cmd result of [%s]: %d", CMD_NAME[cmdType], ret);

    printf("-----------------------------------------\n");
    printf("%s\n",buff0);
    printf("-----------------------------------------\n");

    return ret;
}

/*=============================================================
 *  remote controller entry
 * ============================================================*/
char connAddr[32]={0};
int main(int argc,char ** argv)
{
    struct sockaddr_in cli_addr_device, serv_addr_device;
    struct sockaddr_in cli_addr_app, serv_addr_app;
    int listenfd_device = -1, listenfd_app = -1, connfd_device = -1, connfd_app = -1;
    socklen_t clientlen =0;
    pid_t chdpid = 0;
    unsigned char buff[1024] = {0};
    ////
    fd_set ln_fds;
    int max_lnfd = 0;
    int fd_lnret =0;

    printf("=====================================\n");
    printf("==  YarlungSoft Remote Controller  ==\n");
    printf("==     <www.yarlungsoft.com>       ==\n");
    printf("=====================================\n");

    listenfd_device = socket(AF_INET,SOCK_STREAM,0);
    if (listenfd_device < 0)
    {
        printf("Create socket listenfd_device fail!\n");
        exit(0);
    }

    bzero(&serv_addr_device,sizeof(struct sockaddr_in));

    serv_addr_device.sin_family = AF_INET;
    serv_addr_device.sin_addr.s_addr = htons(INADDR_ANY);
    serv_addr_device.sin_port = htons(AS_DEVICE_SERVER_PORT);

    if (bind(listenfd_device, (const struct sockaddr *)&serv_addr_device,sizeof(serv_addr_device)))
    {
        printf("bind listenfd_device fail!\n");
        exit(0);
    }

    if (listen(listenfd_device, 3))
    {
        printf("listen listenfd_device error!\n");
        exit(0);
    }

    /*-----init server of android-----*/
    listenfd_app = socket(AF_INET,SOCK_STREAM,0);
    if (listenfd_app < 0)
    {
        printf("Create socket listenfd_app fail!\n");
        exit(0);
    }

    bzero(&serv_addr_app, sizeof(struct sockaddr_in));

    serv_addr_app.sin_family = AF_INET;
    serv_addr_app.sin_addr.s_addr = htons(INADDR_ANY);
    serv_addr_app.sin_port = htons(AS_APP_SERVER_PORT);

    if (bind(listenfd_app, (const struct sockaddr *)&serv_addr_app, sizeof(serv_addr_app)))
    {
        printf("bind listenfd_app fail!\n");
        exit(0);
    }

    if (listen(listenfd_app, 3))
    {
        printf("listen listenfd_app error!\n");
        exit(0);
    }
    printf("listenfd_device = %d, listenfd_app = %d \n", listenfd_device, listenfd_app);
    printf("Listening....\n");

    int max1, max2;
    int cmd =0;
    int lastCmd =0;
    int parm_len = 0;
    char paras[1024] = {0};
    bool bPrintTips = true;

    while (true)
    {

        FD_ZERO(&ln_fds);
        if (bPrintTips)
        {
            bPrintTips = false;
            printf("--------------------------------------------------------------------------------\n");
            printf("Remote cmd:\n");
            printf(" air-conditioner <temprature>              Air conditioner temprature\n");
            printf(" fan <button>              fan button\n");
            printf("--------------------------------------------------------------------------------\n");
        }
        FD_SET(listenfd_device, &ln_fds);
        FD_SET(listenfd_app, &ln_fds);

        if (connfd_device > 0)
        {
            printf("--fd_set connfd_device:%d \n", connfd_device);
            FD_SET(connfd_device, &ln_fds);
        }
        if (connfd_app > 0)
        {
            printf("--fd_set connfd_app:%d \n", connfd_app);
            FD_SET(connfd_app,&ln_fds);
        }
        max1 = listenfd_device > listenfd_app ? listenfd_device : listenfd_app ;
        max2 = connfd_device > connfd_app ? connfd_device : connfd_app ;
        max_lnfd = max1 > max2 ? max1 : max2;
        printf("listenfd_device:%d, listenfd_app:%d \n", listenfd_device, listenfd_app);
        printf("connfd_device:%d, connfd_app:%d \n", connfd_device, connfd_app);
 
        printf("max fd:%d \n", max_lnfd);

        fd_lnret = select(max_lnfd+1, &ln_fds, NULL, NULL, NULL);

        if (fd_lnret < 0)
        {
            if (DEBUG) printf("[exception] select error!\n");
            continue;
        }

        if (FD_ISSET(listenfd_device, &ln_fds)) /* device connection request handler */
        {
            printf("Receiving device connection request!\n");
            if (connfd_device > 0)
            {
                close(connfd_device);
                connfd_device = -1;
            }
            clientlen = sizeof(cli_addr_device);
            connfd_device = accept(listenfd_device, (struct sockaddr *)&cli_addr_device, &clientlen);
            if (connfd_device <=0)
            {
                printf("accept device port error!\n");
                continue;
            }
        }
        else if(FD_ISSET(listenfd_app, &ln_fds)) /* Receive Android App connection request */
        {
            if(connfd_app > 0)
            {
                close(connfd_app);
                connfd_app = -1;
            }
            clientlen = sizeof(cli_addr_app);
            connfd_app = accept(listenfd_app, (struct sockaddr *)&cli_addr_app, &clientlen);
            if (connfd_app <=0)
            {
                printf("accept app port error!\n");
                continue;
            }
        }
        else if (FD_ISSET(connfd_device, &ln_fds)) /* Handle messages from device */
        {
            if (!ReceiveCmd(connfd_device, paras, &parm_len))
            {
                /* set connfd_device to disconnected status */
                close(connfd_device);
                connfd_device = -1;
                printf("  recv device msg error! \n");
                printf("Listening....\n");
                continue;
            }
            if (!PraseReceivedDeviceData(connfd_device, paras, parm_len))
            {
                /* set connfd_device to disconnected status */
                printf("client closed! \n");
                close(connfd_device);
                connfd_device = -1;
                printf("Listening....\n");
                continue;
            }
        }
        else if(FD_ISSET(connfd_app,&ln_fds)) /* Handle Message from Android APP client */
        {
            char rcvBuf[1024] = {0};
            int  rcvBufLen = 0;
            int cmdb =0;
            if(!ReceiveCmd(connfd_app, paras, &parm_len))
            {
                close(connfd_app);
                connfd_app =-1;
                printf("  recv app msg error! \n");
                printf("Listening....\n");
                continue;
            }

            if(!PraseReceivedAppData(paras, parm_len))
            {
                continue;
            }
        }
    }
    exit(0);
}
