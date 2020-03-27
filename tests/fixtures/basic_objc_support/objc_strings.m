@class NSString;

__attribute__((objc_root_class))
@interface RootClass

+ (void)createObject;

@end

@implementation RootClass

+ (void)createObject {}

@end

void use() {
  const char *cStringHello = "Hello";
  NSString *objcString = @"Hello";
  const char *cString = "world";
  const char *cStringOffset = ("world") + 2;
}

void send() {
  [RootClass createObject];
}