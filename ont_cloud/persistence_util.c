#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ont/error.h"
#include "ont/platform.h"


#ifdef ONT_PLATFORM_PERSISTENCE_SUPPORT


static const char *devicefilename ="ont_device_ctx";

int ont_platform_save_device_status(const char *buf, size_t size)
{
    size_t wsize=0;
    FILE *fd = fopen(devicefilename, "wb");
    if (fd == NULL)
    {
        return ONT_ERR_STATUS_WRITEFAIL;
    }
    else
    {
        wsize = fwrite (buf, 1, size, fd) ;
        fclose(fd);
    }
    if (wsize != size)
    {
        return ONT_ERR_STATUS_WRITEFAIL;    
    }
    return ONT_ERR_OK;
}


int ont_platform_load_device_status(char *buf, size_t maxbuf)
{
    size_t rsize=0;
    FILE *fd = fopen(devicefilename, "rb");
    if (fd == NULL)
    {
        return ONT_ERR_STATUS_READFAIL;
    }
    else
    {
        rsize = fread (buf, 1, maxbuf, fd) ;
        fclose(fd);
    }
    if (rsize != maxbuf)
    {
        return ONT_ERR_STATUS_READFAIL; 
    }
    return ONT_ERR_OK;
}

#endif
