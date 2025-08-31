// Copyright 2021 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

// Bit definitions for input type
static NSInteger const TYPE_MASK_CLASS = 0x0000000f;
static NSInteger const TYPE_MASK_FLAGS = 0x00fff000;

// Class for normal text
static NSInteger const TYPE_CLASS_TEXT = 0x00000001;

// Class for numeric text
static NSInteger const TYPE_CLASS_NUMBER = 0x00000002;
static NSInteger const TYPE_NUMBER_FLAG_SIGNED = 0x00001000;
static NSInteger const TYPE_NUMBER_FLAG_DECIMAL = 0x00002000;

// Class for a phone number
static NSInteger const TYPE_CLASS_PHONE = 0x00000003;
