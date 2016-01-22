#include <ctype.h>
#include <json.h>
#ifdef __linux__
#include <bsd/string.h> // strlcpy, etc. found in libbsd
#else
#include <string.h>
#endif

#include "e_mod_main.h"
#include "e_share.h"

#define BUFFER_SIZE_MAX 4096
#define DROPNAME "Sourcedrop"
#define SOURCEDROP_URL "http://www.sourcedrop.net/paste"
#define RECOGNITION_STRING "Location: "

static Ecore_Event_Handler *handler = NULL;

static Eina_Bool __upload_completed_cb(void *data, int type, Ecore_Con_Event_Url_Complete *ev);
static json_object *jarray, *jparent;

/* Converts an integer value to its hex character*/
char to_hex(const char code) {
      static char hex[] = "0123456789abcdef";
        return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(const char *str) {
    const char *pstr = str;
    char *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
    while (*pstr) {
        if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
            *pbuf++ = *pstr;
        else if (*pstr == ' ') 
            *pbuf++ = '+';
        else 
            *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
        pstr++;
    }
    *pbuf = '\0';
    return buf;
}

const char *json_encode(const char *title, const char *content)
{
    const char *encoded_string = NULL;

    jarray = json_object_new_array();
    jparent = json_object_new_object();
    json_object_object_add(jparent, "title", json_object_new_string(title));
    json_object_object_add(jparent, "content", json_object_new_string(content));
    json_object_array_add(jarray, jparent);

    encoded_string = json_object_to_json_string(jarray);

    return encoded_string;
}

static Eina_Bool
__upload_completed_cb(void *data, int type, Ecore_Con_Event_Url_Complete *ev)
{
    char buf[BUFFER_SIZE_MAX];
    const Eina_List *headers = NULL, *it;
    const char *header = NULL;
    Share_Data *sd = NULL;

    json_object_put(jarray);
    jarray = NULL;
    json_object_put(jparent);
    jparent = NULL;

    if (ev->status != 200)
    {
        LOG("E_SHARE/" DROPNAME ": This is not supposed to happen. The server returned status code: %d\n", ev->status);
        return ECORE_CALLBACK_RENEW;
    }

    headers = ecore_con_url_response_headers_get(ev->url_con);
    EINA_LIST_FOREACH(headers, it, header)
    {
        if (strncmp(header, RECOGNITION_STRING, (sizeof(RECOGNITION_STRING) - 1)) == 0)
        {
            sd = ecore_con_url_data_get(ev->url_con);
            strlcpy(buf, (char*)(header + sizeof(RECOGNITION_STRING) - 1), sizeof(buf));
            asprintf(&sd->url, "%s", buf);
            e_share_upload_completed(sd);
            ecore_con_url_free(ev->url_con);
            break;
        }
    }

    return ECORE_CALLBACK_CANCEL;
}

void sourcedrop_share(Share_Data *sd)
{
    Ecore_Con_Url *request_url = NULL;
    char post_data[BUFFER_SIZE_MAX];
    const char *json_encoded = NULL;
    char *url_encoded = NULL;

    EINA_SAFETY_ON_NULL_RETURN(sd);

    json_encoded = json_encode(sd->name, sd->content);
    request_url = ecore_con_url_new(SOURCEDROP_URL);
    url_encoded = url_encode(json_encoded);
    snprintf(post_data, (sizeof(post_data) - 1), "data=%s", url_encoded);
    free(url_encoded);
    ecore_con_url_data_set(request_url, (void*)sd);
    ecore_con_url_post(request_url, (void*)post_data, sizeof(post_data), "application/x-www-form-urlencoded"); 
}

void sourcedrop_init(void)
{
    ecore_init();
    ecore_con_init();
    handler = ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, (Ecore_Event_Handler_Cb)__upload_completed_cb, NULL);
}

void sourcedrop_shutdown(void)
{
    ecore_event_handler_del(handler);
    handler = NULL;
    ecore_con_shutdown();
    ecore_shutdown();
}
