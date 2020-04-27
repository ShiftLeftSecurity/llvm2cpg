__attribute__((__objc_root_class__)) @interface RootClass

+ (instancetype)alloc;
- (instancetype)init;

@end

@interface RootClass (SomeCategory)

- (void)doSomething;
+ (void)doSomethingElse;

@end

@implementation RootClass (SomeCategory)

- (void)doSomething {
}
+ (void)doSomethingElse {
}

@end

RootClass *use() {
  RootClass *c = [[RootClass alloc] init];
  [RootClass doSomethingElse];
  [c doSomething];
  return c;
}