// Copyright (C) 2000-2013 SuperWaba Ltda.
// Copyright (C) 2014-2020 TotalCross Global Mobile Platform Ltda.
//
// SPDX-License-Identifier: LGPL-2.1-only
package tc.tools.converter.bytecode;

public class StackManipulation extends ByteCode {
  public StackManipulation(int stackInc, boolean is64) {
    this.stackInc = stackInc;
    this.targetType = is64 ? LONG : INT;
  }
}
