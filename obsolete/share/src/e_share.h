#ifndef __E_SHARE_H__
#define __E_SHARE_H__

#undef LOG
#define LOG(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)

typedef struct _Share_Data {
    void *inst;
    char *name;
    char *content;
    char *url;
} Share_Data;

/* API used by submodules */
void e_share_upload_completed(Share_Data *share_data);

/* Submodules' functions */
void sourcedrop_init(void);
void sourcedrop_share(Share_Data *share_data);
void sourcedrop_shutdown(void);

#endif
