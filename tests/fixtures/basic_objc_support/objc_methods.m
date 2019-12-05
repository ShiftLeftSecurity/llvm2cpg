__attribute__((__objc_root_class__)) @interface RootClass

+ (instancetype)alloc;
- (instancetype)inherited;

+ (void)overridden;
- (void)overridden;

@end

@implementation RootClass

- (instancetype)inherited {
  return self;
}

+ (void)overridden {
}

- (void)overridden {
}

@end

@interface Child : RootClass

+ (instancetype)newChild;
- (void)doSomething;
- (void)overridden;
+ (void)overridden;

@end

@implementation Child

+ (instancetype)newChild {
  return [[self alloc] init];
}

- (void)doSomething {
}

+ (void)overridden {
}

- (void)overridden {
}

@end

Child *useChild() {
  Child *c = [Child newChild];
  [c doSomething];
  return c;
}