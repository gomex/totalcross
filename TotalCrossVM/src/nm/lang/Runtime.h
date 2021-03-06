// Copyright (C) 2000-2013 SuperWaba Ltda.
// Copyright (C) 2014-2020 TotalCross Global Mobile Platform Ltda.
//
// SPDX-License-Identifier: LGPL-2.1-only

#ifndef Runtime_h
#define Runtime_h
#include "tcvm.h"

#define FILE_STREAM_INPUT 0
#define FILE_STREAM_OUTPUT 1

TCObject createFileStream(Context context, const int streamType, int fd);

#endif