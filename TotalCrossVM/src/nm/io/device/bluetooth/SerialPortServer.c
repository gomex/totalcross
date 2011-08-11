/*********************************************************************************
 *  TotalCross Software Development Kit                                          *
 *  Copyright (C) 2000-2011 SuperWaba Ltda.                                      *
 *  All Rights Reserved                                                          *
 *                                                                               *
 *  This library and virtual machine is distributed in the hope that it will     *
 *  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                         *
 *                                                                               *
 *********************************************************************************/



#include "tcvm.h"
#include "guid.h"

#if defined (PALMOS)
// #include "palm/SerialPortServer_c.h"
#elif defined (WIN32) || defined (WINCE)
 #include "win/SerialPortServer_c.h"
#elif defined ANDROID
 #include "android/SerialPortServer_c.h"
#endif

//////////////////////////////////////////////////////////////////////////
TC_API void tidbSPS_createSerialPortServer_s(NMParams p) // totalcross/io/device/bluetooth/SerialPortServer native private void createSerialPortServer(String uuid, String []params) throws totalcross.io.IOException;
{
#if defined (WIN32) || defined (WINCE)
   Object serialPortServerObj = p->obj[0];
   Object uuidObj = p->obj[1];
   Object paramsArray = p->obj[2];
   Object nativeHandleObj;
   NATIVE_HANDLE* nativeHandle; 
   GUID guid;
   Err err;

   if (!String2GUID(uuidObj, &guid))
      throwException(p->currentContext, IllegalArgumentException, "Invalid UUID.");
   else if ((nativeHandleObj = createByteArray(p->currentContext, sizeof(NATIVE_HANDLE))) != null)
   {
      nativeHandle = (NATIVE_HANDLE*) ARRAYOBJ_START(nativeHandleObj);
      if ((err = btsppServerCreate(nativeHandle, guid)) != NO_ERROR)
      {
         setObjectLock(nativeHandleObj, UNLOCKED);
         throwExceptionWithCode(p->currentContext, IOException, err);
      }
      else
         SerialPortServer_nativeHandle(serialPortServerObj) = nativeHandleObj;
   }
#else
   p = 0;
#endif
}
//////////////////////////////////////////////////////////////////////////
TC_API void tidbSPS_accept(NMParams p) // totalcross/io/device/bluetooth/SerialPortServer native public totalcross.io.Stream accept() throws totalcross.io.IOException;
{
#if defined (WIN32) || defined (WINCE)
   Object serialPortServerObj = p->obj[0];
   Object nativeHandleObj = SerialPortServer_nativeHandle(serialPortServerObj);
   NATIVE_HANDLE* nativeHandle = (NATIVE_HANDLE*) ARRAYOBJ_START(nativeHandleObj);
   NATIVE_HANDLE* clientHandle;
   Object serialPortClientObj;
   Object clientHandleObj;
   Err err;

   if ((serialPortClientObj = createObject(p->currentContext, "totalcross.io.device.bluetooth.SerialPortClient")) != null
    && (clientHandleObj = createByteArray(p->currentContext, sizeof(NATIVE_HANDLE))) != null)
   {
      clientHandle = (NATIVE_HANDLE*) ARRAYOBJ_START(clientHandleObj);
      if ((err = btsppServerAccept(nativeHandle, clientHandle)) != NO_ERROR)
      {
         setObjectLock(clientHandleObj, UNLOCKED); // it will be unlocked by the client's close.
         throwExceptionWithCode(p->currentContext, IOException, err);
      }
      else if (*clientHandle == null)
      {
         // the handle is null, that means the socket was closed during this IO blocking operation. we'll throw a different exception in this case.
         setObjectLock(clientHandleObj, UNLOCKED); // it will be unlocked by the client's close.
         throwExceptionWithCode(p->currentContext, SocketTimeoutException, err);
      }
      else
      {
         SerialPortClient_nativeHandle(serialPortClientObj) = clientHandleObj;
         p->retO = serialPortClientObj;
      }
   }
   setObjectLock(serialPortClientObj, UNLOCKED);
#else
   p = 0;
#endif
}
//////////////////////////////////////////////////////////////////////////
TC_API void tidbSPS_close(NMParams p) // totalcross/io/device/bluetooth/SerialPortServer native public void close() throws throws totalcross.io.IOException;
{
#if defined (WIN32) || defined (WINCE)
   Object serialPortServerObj = p->obj[0];
   Object nativeHandleObj = SerialPortServer_nativeHandle(serialPortServerObj);
   NATIVE_HANDLE* nativeHandle;
   Err err;

   if (nativeHandleObj == null)
      throwException(p->currentContext, IOException, "Invalid object");
   else
   {
      nativeHandle = (NATIVE_HANDLE*) ARRAYOBJ_START(nativeHandleObj);
      if ((err = btsppServerClose(nativeHandle)) != NO_ERROR)
         throwExceptionWithCode(p->currentContext, IOException, err);
      setObjectLock(nativeHandleObj, UNLOCKED);
      SerialPortServer_nativeHandle(serialPortServerObj) = null;
   }
#else
   p = 0;
#endif
}

#ifdef ENABLE_TEST_SUITE
//#include "SerialPortServer_c.h"
#endif
