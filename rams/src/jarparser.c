#include <std_global.h>
#include <opl_file.h>
#include <zlib.h>
#include <jarparser.h>

static uint8_t end[22];
static uint8_t fheader[256];
static long    next;
static long    last;


/**
 * Interpret bytes as a little-endian IU16
 * @param p bytes
 * @return little-endian IU16 read from p
 */
static uint16_t leIU16(const uint8_t *p)
{
    return (uint16_t)((p[1] << 8) | p[0]);
}

/**
 * Interpret bytes as a little-endian IU32
 * @param p bytes
 * @return little-endian IU32 read from p
 */
static uint32_t leIU32(const uint8_t *p)
{
    return
        (((uint32_t)p[3]) << 24) |
        (((uint32_t)p[2]) << 16) |
        (((uint32_t)p[1]) <<  8) |
        (((uint32_t)p[0]));
}

/**
 * Interpret bytes as a big-endian IU16
 * @param p bytes
 * @return big-endian IU16 read from p
 */
static uint16_t beIU16(const uint8_t *p)
{
    return (uint16_t)((p[0] << 8) | p[1]);
}

/**
 * Interpret bytes as a big-endian IU32
 * @param p bytes
 * @return big-endian IU32 read from p
 */
static uint32_t beIU32(const uint8_t *p)
{
    return
        (((uint32_t)p[0]) << 24) |
        (((uint32_t)p[1]) << 16) |
        (((uint32_t)p[2]) <<  8) |
        (((uint32_t)p[3]));
}

/**
 * Allocation routine for zlib.
 */

static voidpf jar_zcalloc(voidpf opaque, uInt items, uInt size)
{
    UNUSED(opaque);
    return CRTL_calloc(items, size);
}

static void jar_zfree(voidpf opaque, voidpf address)
{
    UNUSED(opaque);
    CRTL_freeif(address);
}

int32_t  openJar(const uint16_t* jarfn)
{
    int32_t handle = 0;
    int32_t fnLen = CRTL_wcslen(jarfn);
    if (file_open(jarfn, fnLen, FILE_MODE_RD, &handle) == FILE_RES_SUCCESS)
    {
        file_seekex(handle, -22, FILE_SEEK_END);
        last = file_getLengthByFd(handle);
        file_read(handle, end, 22);

        if (leIU32(end) != 0x06054b50)
        {
            DVMTraceErr("Wrong signature on jar central header\n");
            closeJar(handle);
        }

        next = leIU32(end + 16);

        return handle;
    }

    return -1;//for failure
}

/* refer to jarparser.h */
uint8_t* getJarContentByFileName(int32_t handle, uint8_t* confn, int32_t* dataLen)
{
    uint16_t namelen = 0;
    uint32_t fnLen = (uint32_t)CRTL_strlen(confn);
    bool_t   res = TRUE;
    do
    {
        if (next >= last)
            return NULL;

        /* load in the zip header */
        file_seekex(handle, next, SEEK_SET);
        file_read(handle, fheader, 46);
    
        if (leIU32(fheader) != 0x02014b50)
        {
            DVMTraceErr("Wrong signature of central file header\n");
            return NULL;
        }

        namelen = leIU16(fheader + 28);

        file_read(handle, fheader + 46, namelen);

        fheader[46 + namelen] = 0;

        next += 46 + namelen + leIU16(fheader + 30) + leIU16(fheader + 32);
    }
    while (namelen < 6 || CRTL_strcmp((uint8_t *)fheader + 46 + namelen - fnLen, confn));

    /* parse content and look for data */
    {
        /* Read from file into here */
        uint8_t *rawdata = NULL;
        uint32_t raw_sz = 0;

        /* And uncompress if necessary into here */
        uint8_t *data = NULL;
        uint32_t data_sz = 0;
        do
        {
            /* Annoyingly, the local file header may contain different "extra data"
             * from the central header. So we have to double-check with the local
             * header.
             */

            uint32_t lhdrlen = 30 + namelen;  /* min length of local header */
            uint32_t offs = leIU32(fheader + 42);  /* file offset of local hdr */
            uint32_t compsz = leIU32(fheader + 20);
            uint32_t len = compsz + lhdrlen;
            uint16_t extra;
            uint16_t fmt;

            /* First read fixed local header + namelen + comp data size. Expansion
             * needs a dummy byte, so allocate one extra byte in case we need it.
             */
            rawdata = CRTL_malloc(raw_sz = len+1);
            if (rawdata == NULL)
            {
                DVMTraceErr("Not engouth memory for rawdata\n");
                return NULL;
            }

            file_seekex(handle, offs, FILE_SEEK_BEGIN);
            file_read(handle, rawdata, len);

            if (leIU32(rawdata) != 0x04034b50)
            {
                DVMTraceErr("Wrong local file header signature\n");
                res = FALSE;
                break;
            }

            /* Then check extra bytes from local header */

            extra = leIU16(rawdata + 28);

            if (extra > 0)
            {
                /* Need to read some more...
                 * Who came up with this file format ?
                 */

                if (raw_sz <= len + extra)
                {
                    uint8_t *tmp = CRTL_malloc(raw_sz = len + extra + 1);
                    if (tmp == NULL)
                    {
                        DVMTraceErr("Not engough memory for alloc\n");
                        res = FALSE;
                        break;
                    }
                    CRTL_memcpy(tmp, rawdata, raw_sz);
                    CRTL_free(rawdata);
                    rawdata = tmp;
                    raw_sz = len + extra + 1;
                }

                file_read(handle, rawdata + len, extra);
                lhdrlen += extra;
            }

            fmt = leIU16(rawdata + 8);

            if (fmt == 0)
            {
                /* uncompressed - just return pointer into raw area */
                data = (uint8_t*)CRTL_malloc(raw_sz - lhdrlen + 1);
                if (data == NULL)
                {
                    DVMTraceErr("Not engough memory for alloc\n");
                    res = FALSE;
                    break;
                }
                CRTL_memcpy(data, rawdata + lhdrlen, raw_sz - lhdrlen);
                data[raw_sz - lhdrlen] = '\0';
                *dataLen = (raw_sz - lhdrlen);
                break;
            }
            else if (fmt == 8)
            {
                /* deflated - use zlib to inflate */

                z_stream strm;
                int res;

                uint32_t uncmpsz = leIU32(fheader + 24);

                if (data_sz < uncmpsz)
                {
                    data = (uint8_t*)CRTL_malloc((data_sz = uncmpsz) + 1);
                    if (data == NULL)
                    {
                        DVMTraceErr("Not engough memory for alloc\n");
                        res = FALSE;
                        break;
                    }
                }

                strm.zalloc = jar_zcalloc;
                strm.zfree = jar_zfree;

                strm.next_in = rawdata + lhdrlen;
                strm.avail_in = compsz + 1; /* dummy byte required */
                strm.next_out = data;
                strm.avail_out = data_sz;
                strm.data_type = Z_BINARY;

                /* This doesn't seem to be documented, but -ve window
                 * size tells gzip not to look for a gzip header or trailer.
                 * (Found this in zlib/contrib/minizip/unzip.c)
                 * That comment also mentions the need for an extra dummy byte in
                 * the input stream. (Fixed in newer vsns of zlib, but does no harm
                 * to add it anyway.)
                 */

                if ( (res = inflateInit2(&strm, -15)) != Z_OK ||
                     (res = inflate(&strm, Z_FINISH)) != Z_STREAM_END ||
                     (res = inflateEnd(&strm)) != Z_OK)
                {
                    DVMTraceErr("zlib inflate failed (%d) - %s\n", res, strm.msg ? strm.msg : "???");
                    res = FALSE;
                    break;
                }

                if (crc32(0, data, uncmpsz) != leIU32(fheader + 16))
                {
                    DVMTraceErr("zlib inflate - crc error\n");
                    res = FALSE;
                }
                *dataLen = data_sz;
                data[data_sz] = '\0';
                break;
            }

            DVMTraceErr("unsupported compression style %d\n", (int)fmt);
            res = FALSE;
        } while(FALSE);
        
        CRTL_freeif(rawdata);
        if (!res)
        {
            CRTL_freeif(data);
            return NULL;
        }

        return data;
    }
}

void closeJar(int32_t handle)
{
    file_close(handle);
}