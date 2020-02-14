__attribute__((__objc_root_class__)) @interface RootClass

+ (instancetype)new;

@end

@interface Child2 : RootClass

@end

@implementation Child2

@end

Child2 *newChild() {
  return [Child2 new];
}