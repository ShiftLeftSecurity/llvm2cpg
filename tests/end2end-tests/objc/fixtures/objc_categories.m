__attribute__((__objc_root_class__)) @interface RootClass

+ (instancetype)alloc;
- (instancetype)init;

@end

@implementation RootClass

+ (instancetype)alloc {
  return 0;
}

- (instancetype)init {
  return self;
}

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