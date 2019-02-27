#include "mytar.h"

/*static uint32_t extract_special_int(char *where, int len);*/

/*static uint32_t extract_special_int(char *where, int len) {

  int32_t val= -1;
  if ( (len >= sizeof(val)) && (where[0] & 0x80)) {

    val = *(int32_t *)(where+len-sizeof(val));
    val = ntohl(val);
  }
  return val;
}*/

int insert_special_int(char *where, size_t size, int32_t val) {
    /* For interoperability with GNU tar.  GNU seems to
     * set the high-order bit of the first byte, then
     * treat the rest of the field as a binary integer
     * in network byte order.
     * Insert the given integer into the given field
     * using this technique.  Returns 0 on success, nonzero
     * otherwise
     */
    int err=0;
    extern char options[6];
    
    if (val > 2097151 && options[S_INDEX] == 1) {
        return -1;
    }

    if ( val < 2097151 || options[4] == 1) {
        sprintf(where, "%07o", val);
    } else {
        if ( val < 0 || ( size < sizeof(val))  ) {
            /* if it's negative, bit 31 is set and we can't use the flag
             * if len is too small, we can't write it.  Either way, we're
             * done.
             */
            err++;
        } else {
            /* game on....*/
            memset(where, 0, size);     /*   Clear out the buffer  */
            *(int32_t *)(where+size-sizeof(val)) = htonl(val);
            *where |= 0x80;             /* set that high-order bit */
        }
    }

    return err;
}
