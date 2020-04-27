__attribute__((__objc_root_class__))
@interface RootClass

+ (instancetype)alloc;
- (instancetype)init;

@end

@implementation RootClass

- (instancetype)init {
  return self;
}

@end

@interface Child : RootClass

+ (instancetype)newChild;
- (void)doSomething;

@end

@implementation Child

+ (instancetype)newChild {
  return [[self alloc] init];
}

- (void)doSomething {

}

@end

Child *useChild() {
  Child *c = [Child newChild];
  [c doSomething];
  return c;
}