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



#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#if !defined(__SYMBIAN32__)
#include <utime.h>
#endif

#if HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#endif

#if defined(HAVE_STATFS) && defined(darwin)
#include <sys/mount.h>
#endif

#define IS_DEBUG_CONSOLE(path) (xstrstr(path,"DebugConsole") != null)

#ifdef ANDROID
static bool getSDCardPath(char* buf)
{
   JNIEnv *env = getJNIEnv();                                      
   jstring path = (*env)->CallStaticObjectMethod(env, applicationClass, jgetSDCardPath);
   if (path != null)
   {
      jstring2CharP(path, buf);
      (*env)->DeleteLocalRef(env, path); // guich@tc125_1
   }
   else
      buf[0] = 0;
   return buf[0] != 0;               
}
#endif
/*
 *
 * Always return true on POSIX.
 *
 *************************************/

#ifdef ANDROID
static bool fileIsCardInserted(int32 slot)
{
   char buf[64];
   return getSDCardPath(buf);
}
#else
#define fileIsCardInserted(slot) true
#endif

static Err fileGetCardSerialNumber(int32 slot, CharP serialNumber)
{
   if (serialNumber)
      *serialNumber = '\0';
   return NO_ERROR;
}

/*
 *
 * fopen
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/

static Err fileCreate(NATIVE_FILE* fref, TCHARP path, int32 mode, int32* slot)
{
   TCHAR * rwMode;
   struct stat statData;

   switch (mode)
   {
      case READ_WRITE:   rwMode = TEXT("rb+"); break;
      case CREATE:       rwMode = stat(path, &statData) == 0 ? TEXT("rb+") : TEXT("wb+"); break; //flsobral@tc123_19: fixed behaviour of File.CREATE mode on Linux based platforms.
      case CREATE_EMPTY: rwMode = TEXT("wb+"); break;
      case READ_ONLY:    rwMode = TEXT("rb");  break;
   }
   fref->handle = fopen(path, rwMode);
   if (!fref->handle)
      return errno;
#if defined(darwin) // TODO@ app permissions
   else
   if (mode == CREATE || mode == CREATE_EMPTY)
      fchmod(fileno(fref->handle), S_IRWXU | S_IRWXG | S_IRWXO);
#endif

   return NO_ERROR;
}

/*
 *
 * fstat
 * fclose
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/

static Err fileClose(NATIVE_FILE* fref)
{
   FILE *hFile;
   struct stat statData;

   if (fref->handle == INVALID_HANDLE_VALUE)
      return NO_ERROR;

   if (fstat(fileno(fref->handle), &statData))
      return errno;

   if (S_ISDIR(statData.st_mode))
      return NO_ERROR;

   hFile = fref->handle;
   fref->handle = INVALID_HANDLE_VALUE;

   if (fclose(hFile))
      return errno;

   return NO_ERROR;
}

/*
 *
 * mkdir
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/

//#define fileCreateDir(path, slot) CreateDirectory(path, null)
static Err fileCreateDir(TCHARP path, int32 slot)
{
   struct stat statData;
   TCHARP c;

   if (mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO))
   {
      if (path[0] == '/')
         c = path + 1;
      else
         c = path;
      while (*c != 0)
      {
         if (*c == '/')
         {
            *c = 0;
            if (stat(path, &statData))
            {
               if (mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO))
                  goto error;
            }
            *c = '/';
         }
         c++;
      }
      if (mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO))
         goto error;
   }
   return NO_ERROR;

error:
   return errno;
}

/*
 *
 * fstat
 * rmdir
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/

static Err fileDelete(NATIVE_FILE* fref, TCHARP path, int32 slot, bool isOpen)
{
   int ret;
   struct stat statData;

   if (stat(path, &statData))
      return errno;

   if (S_ISDIR(statData.st_mode))
   {
      if (rmdir(path))
         return errno;
      return NO_ERROR;
   }

   if (isOpen)
      fclose(fref->handle);

   if (unlink(path))
      return errno;

   return NO_ERROR;
}

/*
 *
 * struct stat
 *
 * stat
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/

static bool fileExists(TCHARP path, int32 slot)
{
   struct stat statData;
   return stat(path, &statData) == 0;
}

/*
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/

static inline Err fileGetFreeSpace(CharP szPath, int32* freeSpace, int32 slot)
{
#if defined(ANDROID)
   JNIEnv* env = getJNIEnv();
   jmethodID method = (*env)->GetStaticMethodID(env, applicationClass, "fileGetFreeSpace", "(Ljava/lang/String;)I");
   jstring fileName = (*env)->NewStringUTF(env, szPath);
   *freeSpace = (*env)->CallStaticIntMethod(env, applicationClass, method, fileName);
#elif defined(HAVE_STATFS)
   int64 fbytes = 0;
   struct statfs sfs;
   if (statfs(szPath, &sfs))
      return errno;
   fbytes = (int64)sfs.f_bfree * sfs.f_bsize;
   *freeSpace = (fbytes > I64_CONST(0x7FFFFFFF)) ? 0x7FFFFFFF : (int32) fbytes;
#else
   *freeSpace = 100000000;
#endif
   return NO_ERROR;
}

/*
 *
 * struct stat
 * struct statfs
 *
 * fstat
 * statfs
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/

static Err fileGetSize(NATIVE_FILE fref, TCHARP szPath, int32* size)
{
   if (fref.handle != INVALID_HANDLE_VALUE)
   {
      struct stat statData;
      if (!fstat(fileno(fref.handle), &statData))
      {
         *size = statData.st_size;
         return NO_ERROR;
      }
   }

   if( !tcscmp( TEXT("/"), szPath ) )
   {
      int64 fbytes = 0;

#if defined(HAVE_STATFS)
      struct statfs sfs;
      if (statfs(szPath, &sfs))
         return errno;
      fbytes = (int64)sfs.f_blocks * sfs.f_bsize;
#endif

      *size = (fbytes > I64_CONST(0x7FFFFFFF)) ? 0x7FFFFFFF : (int32) fbytes;
   }
   return NO_ERROR;
}

/*
 *
 * fstat
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/

static bool fileIsDir(TCHARP path, int32 slot)
{
   struct stat statData;

   if (stat(path, &statData))
      return false;

   return S_ISDIR(statData.st_mode);
}

static Err fileIsEmpty(NATIVE_FILE* fref, TCHARP path, int32 slot, int32* isEmpty)
{
   struct stat statData;
   Err err = NO_ERROR;
   *isEmpty = true;

   if (stat(path, &statData))
      return NO_ERROR;
   
   if (S_ISDIR(statData.st_mode))
   {
      struct dirent * entry;
      DIR * dir;
      
   #ifdef ANDROID // Android has a bug that result in files being added more than once. so, we track the d_off and break the loop once it restarts
      int32 lastOff = -1;
   #endif
      dir = opendir(path);
      if (dir)
      while ((entry = readdir(dir)))
      {
   #ifdef ANDROID
         if (entry->d_off < lastOff)        
            break;
         lastOff = entry->d_off;
   #endif
         if ((entry->d_name[0] != '.') || ((entry->d_name[1] != '\0') && ((entry->d_name[1] != '.') || (entry->d_name[2] != '\0')))) /* warning: order matters! */
         {
            *isEmpty = false; // at least one file found. stop
            break;
         }
      }
      closedir(dir);
   }
   else
   {
      *isEmpty = statData.st_size;
   }
   return err;
}

/*
 *
 * fread
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/

static inline Err fileReadBytes(NATIVE_FILE fref, CharP bytes, int32 offset, int32 length, int32* bytesRead)
{
   if ((*bytesRead = fread(bytes+offset, 1, length, fref.handle)) <= 0 && !feof(fref.handle)) // flsobral@tc110_1: return 0 and NO_ERROR on EOF.
      return errno;

   return NO_ERROR;
}

/*
 *
 * CloseHandle
 * MoveFile
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/

static inline Err fileRename(NATIVE_FILE fref, int32 slot, TCHARP currPath, TCHARP newPath, bool isOpen)
{
   if (isOpen)
      fclose(fref.handle);
   if (rename(currPath, newPath))
      return errno;

   return NO_ERROR;
}

/*
 *
 * SetFilePointer
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/

static inline Err fileSetPos(NATIVE_FILE fref, int32 position)
{
   if (fseek(fref.handle, position, SEEK_SET))
      return errno;

   return NO_ERROR;
}

/*
 *
 * fwrite
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/

static inline Err fileWriteBytes(NATIVE_FILE fref, CharP bytes, int32 offset, int32 length, int32* bytesWritten)
{
   if ((*bytesWritten = fwrite(bytes+offset, 1, length, fref.handle)) < 0)
      return errno;
   return NO_ERROR;
}

/*
 *
 * fstat
 * chmod
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/

static Err fileSetAttributes(NATIVE_FILE fref, TCHARP path, int32 tcAttributes)
{
   struct stat statData;

   if (!fstat(fileno(fref.handle), &statData))
   {
      // ATTR_HIDDEN is not POSIX
      // ATTR_ARCHIVE is not supported insofar a file type can't be changed
      if( tcAttributes & ATTR_READ_ONLY )
         statData.st_mode &= ~S_IWUSR;
      else
         statData.st_mode |= S_IWUSR;
      if (!chmod(path, statData.st_mode))
         return NO_ERROR;
   }
   return errno;
}

/*
 *
 * fstat
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/

static Err fileGetAttributes(NATIVE_FILE fref, TCHARP path, int32* attributes)
{
   struct stat statData;
   *attributes = ATTR_NORMAL;

   if (fstat(fileno(fref.handle), &statData))
      return errno;

   // ATTR_HIDDEN and ATTR_ARCHIVE are not POSIX
   if ((statData.st_mode & (S_IRUSR|S_IWUSR)) == S_IRUSR)
      *attributes |= ATTR_READ_ONLY;
   if ((statData.st_mode & S_IFREG) != 0)
      *attributes |= ATTR_ARCHIVE;

   return NO_ERROR;
}

/*
 *
 * mktime
 * utime
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 ************************************/

static Err fileSetTime(NATIVE_FILE fref, TCHARP path, int32 which, Object time)
{
#if !defined(__SYMBIAN32__)

   struct tm tm;
   struct utimbuf timbuf;
   struct stat statData;
   time_t t;

   tm.tm_year     = Time_year(time) - 1900;
   tm.tm_mon      = Time_month(time) - 1; //flsobral@tc120_27: POSIX month attribute is 0 based.
   tm.tm_mday     = Time_day(time);
   tm.tm_hour     = Time_hour(time);
   tm.tm_min      = Time_minute(time);
   tm.tm_sec      = Time_second(time);
   //             = Time_millis(time);
   tm.tm_isdst = -1;
   t = mktime(&tm);

   if (stat(path, &statData))
      return errno;

   timbuf.modtime = statData.st_mtime;
   timbuf.actime  = statData.st_atime;

   if (which & TIME_ACCESSED)
      timbuf.actime = t;
   if (which & TIME_MODIFIED)
      timbuf.modtime = t;

   if (utime(path, &timbuf))
      return errno;
#endif

   return NO_ERROR;
}

/*
 *
 * stat
 * localtime
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/

static Err fileGetTime(Context currentContext, NATIVE_FILE fref, TCHARP path, int32 whichTime, Object* time)
{
   struct stat statData;

   if (stat(path, &statData))
      return errno;

   time_t *ft;

   if (whichTime & TIME_CREATED)
      ft = &statData.st_ctime;
   else
   if (whichTime & TIME_MODIFIED)
      ft = &statData.st_mtime;
   else
   if (whichTime & TIME_ACCESSED)
      ft = &statData.st_atime;
   else
      return -1;

   *time = createObject(currentContext, "totalcross.sys.Time");
   if (!(*time))
      return NO_ERROR;

   struct tm * tm = localtime(ft);

   Time_year(*time)   = tm->tm_year + 1900;
   Time_month(*time)  = tm->tm_mon + 1;
   Time_day(*time)    = tm->tm_mday;
   Time_hour(*time)   = tm->tm_hour;
   Time_minute(*time) = tm->tm_min;
   Time_second(*time) = tm->tm_sec;
   Time_millis(*time) = 0;

   return NO_ERROR;
}

/*
 *
 * ftruncate
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/

static inline Err fileSetSize(NATIVE_FILE* fref, int32 newSize)
{
   return ftruncate(fileno(fref->handle), newSize) ? errno : NO_ERROR;
}

/*
 *
 * fflush
 *
 * OS Versions: POSIX compliant systems.
 * Header: stdio.h.
 * Link Library: libc.
 *
 *************************************/
static inline Err fileFlush(NATIVE_FILE fref)
{
   return fflush(fref.handle) ? errno : NO_ERROR;
}

static Err fileChmod(NATIVE_FILE* fref, TCHARP path, int32 slot, int32* mod)
{          
   struct stat statData;

   if (stat(path, &statData)) // get the previous permission
      return errno;

   if (*mod != -1 && chmod(path, toBaseAsDecimal(*mod,8,10))) // set the new permission
      return errno;
      
   *mod = toBaseAsDecimal(statData.st_mode,10,8);
   return NO_ERROR;
}
