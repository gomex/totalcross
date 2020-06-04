// Copyright (C) 2000-2013 SuperWaba Ltda.
// Copyright (C) 2014-2020 TotalCross Global Mobile Platform Ltda.
//
// SPDX-License-Identifier: LGPL-2.1-only
package tc.tools.converter.bytecode;

public class BC125_lushr extends Logical {
  public BC125_lushr() {
    super(-1, -2, -1, LONG);
  }

  @Override
  public void exec() {
    stack[-2].asLong >>>= stack[-1].asLong & 0x1F;
  }
}