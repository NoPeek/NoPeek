// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN
@class LynxMemoryRecord;
/**
 * @brief A block type that constructs and returns a LynxMemoryRecord instance.
 *
 * This block type can be used to create instances of LynxMemoryRecord with specified
 * category, size in kilobytes, and detail information.
 */
typedef LynxMemoryRecord* _Nonnull (^LynxMemoryRecordBuilder)(void);

/**
 * @brief Represents a memory record with a category, size in kilobytes, and detail information.
 */
@interface LynxMemoryRecord : NSObject

/** The category of the memory record. like "image", "vm", "element" etc. */
@property(nonatomic, copy) NSString* category;
/** The size of the memory record in bytes. */
@property(nonatomic, assign) int64_t sizeBytes;
/** The number of instances of the category. */
@property(nonatomic, assign) int instanceCount;
/** The detail information of the memory record. */
@property(nonatomic, strong, nullable) NSDictionary<NSString*, NSString*>* detail;

/**
 * @brief Initializes a new LynxMemoryRecord instance with the specified category, size in
 * kilobytes, and detail information.
 *
 * @param category The category of the memory record.
 * @param sizeBytes The size of the memory record in kilobytes.
 * @param detail The detail information of the memory record.
 *
 * @return A new LynxMemoryRecord instance.
 */
- (instancetype)initWithCategory:(NSString*)category
                       sizeBytes:(int64_t)sizeBytes
                          detail:(NSDictionary<NSString*, NSString*>* _Nullable)detail;

@end
NS_ASSUME_NONNULL_END
