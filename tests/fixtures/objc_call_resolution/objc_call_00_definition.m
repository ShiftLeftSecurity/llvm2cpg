@class NSString;

__attribute__((objc_root_class))
@interface NSObject

+ (instancetype)new;
- (NSString *)description;

@end

@implementation NSObject

+ (instancetype)new {
  return 0;
}

- (NSString *)description {
  return 0;
}

@end