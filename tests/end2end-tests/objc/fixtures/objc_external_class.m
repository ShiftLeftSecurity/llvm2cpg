__attribute__((__objc_root_class__)) @interface RootClass

+ (instancetype)new;

@end

@interface Child : RootClass

@end

@implementation Child

@end

Child *newChild() {
  return [Child new];
}