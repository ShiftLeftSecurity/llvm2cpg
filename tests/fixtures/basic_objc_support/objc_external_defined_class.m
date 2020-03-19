__attribute__((__objc_root_class__)) @interface RootClass

+ (instancetype)new;

@end

@interface Child : RootClass

@end

@implementation RootClass

+ (instancetype)new {
  return 0;
}

@end
