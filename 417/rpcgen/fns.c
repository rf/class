#include <rpc/rpc.h>

char ** 
get_addr_1_svc (long * param, struct svc_req * s) {
  char ** ret = (char **) malloc(sizeof(char **));
  char * thestr = "hi there";
  ret = &thestr;

  return ret;
}

long * 
get_id_1_svc (char ** param, struct svc_req * s) {
  long * thelong = malloc(sizeof(long));
  return thelong;
}
