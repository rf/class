/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _TEST_H_RPCGEN
#define _TEST_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif


#define GETNAME 0x31223456
#define GET_VERS 1

#if defined(__STDC__) || defined(__cplusplus)
#define GET_ID 1
extern  long * get_id_1(char **, CLIENT *);
extern  long * get_id_1_svc(char **, struct svc_req *);
#define GET_ADDR 2
extern  char ** get_addr_1(long *, CLIENT *);
extern  char ** get_addr_1_svc(long *, struct svc_req *);
extern int getname_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define GET_ID 1
extern  long * get_id_1();
extern  long * get_id_1_svc();
#define GET_ADDR 2
extern  char ** get_addr_1();
extern  char ** get_addr_1_svc();
extern int getname_1_freeresult ();
#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_TEST_H_RPCGEN */