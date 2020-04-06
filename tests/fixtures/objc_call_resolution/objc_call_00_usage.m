@class NSObject;
@class NSString;

extern void NSLog(NSString *fmt, ...);

int main() {
  NSObject *object = [NSObject new];
  NSLog(@"%@", [object description]);
  return 0;
}