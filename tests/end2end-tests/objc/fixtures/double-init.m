#include <Foundation/Foundation.h>

@interface FooClass : NSObject

- (instancetype)initWithBytes:(const void *)bytes length:(int)l;

@end

@implementation FooClass

- (instancetype)initWithBytes:(const void *)bytes length:(int)l {
  self = [super init];

  return self;
}

@end

int main() {
  const void *mem = 0;
  FooClass* obj = [[FooClass alloc] initWithBytes:mem length:0];
  [obj initWithBytes:mem length:0];

  FooClass* obj2 = [[FooClass alloc] initWithBytes:mem length:0];
  obj2 = [FooClass alloc];
  [obj2 initWithBytes:mem length:0];

  return 0;
}

