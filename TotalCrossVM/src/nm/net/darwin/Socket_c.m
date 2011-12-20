/*********************************************************************************
 *  TotalCross Virtual Machine, version 1                                        *
 *  Copyright (C) 2007-2012 SuperWaba Ltda.                                      *
 *  All Rights Reserved                                                          *
 *********************************************************************************/

#import <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/nameser.h>   // NS_MAXDNAME
#include <netdb.h>          // getaddrinfo, struct addrinfo, AI_NUMERICHOST
#include <unistd.h>         // getopt

int iphoneSocket(char* hostname, struct sockaddr *in_addr)
{
   CFStringRef hostnameStr;
   CFHostRef host;
   CFStreamError error;
   Boolean success;
   CFArrayRef addresses;
   CFIndex index, count;
   struct sockaddr *addr;
   char             ipAddress[INET6_ADDRSTRLEN];
   int err;
   
   hostnameStr = CFStringCreateWithCString(NULL, hostname, kCFStringEncodingASCII);
   if (hostnameStr == NULL)
      return -2;
   
   host = CFHostCreateWithName(NULL, hostnameStr);
   if (host == NULL)
      return -3;
   
   success = CFHostStartInfoResolution(host, kCFHostAddresses, &error);
   if (!success)
      { debug("error in CFHostStartInfoResolution: %d %X",(int)error.error,(int)error.error);
   return -4;}
   
   addresses = CFHostGetAddressing(host, &success);
   if (!success)
      return -5;
   
   if (addresses != NULL)
   {
      count = CFArrayGetCount(addresses);
      for (index = 0; index < count; index++)
      {
         addr = (struct sockaddr *)CFDataGetBytePtr(CFArrayGetValueAtIndex(addresses, index));
         if (addr != NULL)
         {
            memcpy(in_addr, addr, sizeof(struct sockaddr));
            /* getnameinfo coverts an IPv4 or IPv6 address into a text string. */
            err = getnameinfo(addr, addr->sa_len, ipAddress, INET6_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);
            if (err != 0) 
               debug("getnameinfo returned %d\n", err);
            //break;
         }
      }
   }
   
   CFRelease(host);
      
   
//   CFSocketRef socketRef = CFSocketCreate(
//                              NULL,       // default allocator
//                              PF_INET,
//                              SOCK_STREAM,
//                              IPPROTO_TCP,
//                              0,          // kCFSocketNoCallBack
//                              NULL,       // no callback function
//                              NULL);         // CFSocketContext
//   
//   if (socketRef == NULL)
//      return -1;
//   
//   int handle = CFSocketGetNative(socketRef);
//   CFRelease(socketRef);
   
   return 0;
}
