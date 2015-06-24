#include <std_global.h>
#include <properties.h>
#include <opl_file.h>
#include <encoding.h>
#include <vm_common.h>

#define SYSTEM_CONFIG_FILE "sys.config"

/******************************************
  Read Stream definitions.
******************************************/

#define STREAM_CACHE_SIZE 64

typedef int32_t (*ReadByte)(void* self); 
typedef void    (*Close)(void** self);

typedef struct ReadStream_s
{
    uint8_t  data[STREAM_CACHE_SIZE];
    int32_t  dataSize; /* available data size */
    int32_t  handle;
    int32_t  pos;
    ReadByte read;
    Close    close;
}ReadStream;

static ReadStream* createFileReadStream(uint16_t* fn, int32_t len);


/******************************************
  Read Stream definitions.
******************************************/

/**
 * Simple array of built in properties, where the value part comes
 * immediately after the zero terminated key.
 */
static const char * const builtinProperties[] =
{
    #define DEF_PROPERTY(k, v)  (k "\0" v)
#ifdef ARCH_X86	
    //DEF_PROPERTY("appdb.dir", "D:/dvm/appdb/"),
    DEF_PROPERTY("appdb.dir", "D:/nix.long/ReDvmAll/dvm/appdb/"),
#elif defined(ARCH_ARM_SPD)
    DEF_PROPERTY("appdb.dir", "D:/dthing/"),
#endif
    DEF_PROPERTY("manufacture", "helio")
};

typedef struct Property_s
{
    struct Property_s* next;    /* Ptr to next property (NULL ends list) */
    const char         data[1]; /* UTF8 key, followed by UTF8 value */
} Property;

static Property* sysProps;


static void parseProperty(ReadStream* rs)
{
    #define MAX_LINE 512
    uint8_t buf[MAX_LINE+1];
    int32_t i;
    int32_t line = 0;
    int32_t c;
    
    for (i = 0; i < MAX_LINE; i++)
    {
        c = rs->read(rs);

        if (c < 0)
            break; /* EOF */
        buf[i] = (char)c;

        if ((c == '\n') || (i == MAX_LINE))
        {
            line++;
            buf[i] = '\0';

            if (buf[0] != '#')
            {
                char* key = buf;
                char* value;

                for (value = key; *value != '\0'; value++)
                {
                    if (*value == ':') 
                    {
                        *value++ = '\0';

                        while(CRTL_isspace(*value)) 
                            value++;

                        props_setValue(key, value);
                        break;
                    }
                }
            }

            while(i == MAX_LINE && buf[i] != '\n' && c > 0)
                c = rs->read(rs);

            i = -1; //reset to -1;
        }

        if (buf[i] == '\r') {
            i--;
        }
    }
}

static int32_t readNextByte(ReadStream* rs)
{
    if (rs->pos >= rs->dataSize)
    {
        rs->dataSize = file_read(rs->handle, rs->data, STREAM_CACHE_SIZE);
        if (rs->dataSize <= 0)
        {
            if (rs->dataSize != FILE_RES_EOF)
            {
                DVMTraceWar("readNextByte could not reach end of file\n");
            }
            return -1;
        }
        rs->pos = 0;
    }

    return (int)rs->data[rs->pos++];
}

static void closeRs(ReadStream** rs)
{
    ReadStream* tmpRs = NULL;

    if (rs == NULL) return; //nothing to do;
    
    tmpRs = *rs;
    if (tmpRs != NULL && tmpRs->handle > 0)
    {
        file_close(tmpRs->handle);
    }
    CRTL_freeif(tmpRs);
}

static ReadStream* createFileReadStream(uint16_t* fn, int32_t len)
{
    bool_t res = TRUE;
    ReadStream* readStream = NULL;
    
    do
    {
        readStream = (ReadStream*)CRTL_malloc(sizeof(ReadStream));
        if (readStream == NULL)
        {
            DVMTraceErr("createFileReadStream - malloc fail,Not enough memory\n");
            res = FALSE;
            break;
        }
        CRTL_memset(readStream, 0x0, sizeof(ReadStream));

        if (file_open(fn, len, FILE_MODE_RD, &readStream->handle) != FILE_RES_SUCCESS)
        {
            //DVMTraceErr("createFileReadStream - open file fail!\n");
            res = FALSE;
            break;
        }
        readStream->pos = 0;
        readStream->dataSize = 0;
        readStream->read = readNextByte;
        readStream->close = closeRs;
    }
    while(FALSE);

    if (!res)
    {
        if (readStream != NULL )
        {
            CRTL_free(readStream);            
        }
		return NULL;
    }

    return readStream;
}

static void initProperty(const char* homeDir, const char* propsName)
{
    uint16_t fName[MAX_FILE_NAME_LEN];
    int32_t  len = 0;
    int32_t  n = 0;
    if (homeDir == NULL)
    {
        homeDir = props_getValue("appdb.dir");
    }
    if (propsName == NULL)
    {
        propsName = SYSTEM_CONFIG_FILE;
    }

    n = convertAsciiToUcs2(homeDir, -1, fName + len, MAX_FILE_NAME_LEN - n);
    if (n != -1)
    {
        len += n;
        n = convertAsciiToUcs2(propsName, -1, fName + len, MAX_FILE_NAME_LEN - n);
    }

    if (n != -1)
    {
        ReadStream* rs;
        len += n;
        rs = createFileReadStream(fName, len);
        if (rs != NULL)
        {
            parseProperty(rs);
            rs->close(&rs);
        }
    }
}


void props_startup()
{
    sysProps = NULL;
    initProperty(NULL, NULL);
}

/**
 * Free off some properties
 * @param pp    Ptr to properties to free off.
 */
static void freeProperties(Property** pp)
{
    Property* p = *pp;
    *pp = NULL;

    while (p != NULL)
    {
        Property* tmp = p;
        p = p->next;
        CRTL_freeif(tmp);
    }
}

void props_shutdown()
{
    freeProperties(&sysProps);
    sysProps = NULL;
}


const char* props_getValue(const char* key)
{
    const char* const* bin;
    Property* p = NULL;
	int i=0;

    /* loop defined properties to find. */
    for (p = sysProps; p != NULL; p = p->next)
    {
        if (CRTL_strcmp(key, p->data) == 0)
        {
            return p->data + CRTL_strlen(key) + 1;  /* skip '\0' */
        }
    }
    
    /* loop builtin properties. */
    for (bin = builtinProperties; *bin != NULL && i++ < sizeof(builtinProperties)/sizeof(const char *); bin++)
    {
        if (CRTL_strcmp(key, *bin) == 0)
        {
            return *bin + CRTL_strlen(*bin) + 1;
        }
    }

    return NULL;
}

const char* props_setValue(const char* key, const char* val)
{
    Property* p;
    Property* pp;
    int len = CRTL_strlen(key) + 1;

    /* loop defined properties to avoid re-definitions. */
    for (p = sysProps; p != NULL; pp = p, p = p->next)
    {
        if (CRTL_strcmp(key, p->data) == 0)
        {
            if (p == sysProps)
            {
                sysProps = p->next;
                CRTL_freeif(p);
            }
            else
            {
                pp->next = p->next;
                CRTL_freeif(p);
            }
            break;
        }
    }

    p = CRTL_malloc(sizeof(Property) + len + CRTL_strlen(val));
    if (p == NULL)
    {
        DVMTraceErr("props_setValue not enough memory \n");
        return NULL;
    }
    CRTL_strcpy((char*)&p->data[0], key);
    CRTL_strcpy((char*)&p->data[len], val);
    p->next = sysProps;
    sysProps = p;

    return &p->data[len];
}

