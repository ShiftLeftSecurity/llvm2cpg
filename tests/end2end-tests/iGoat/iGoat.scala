// REQUIRES: OCULAR
// RUN: %llvm2cpg --output=%t.cpg.bin.zip %DATA_DIR/llvm2cpg-samples/iGoat/x86_64/*.bc

// RUN: cd %ANALYZER_DIR
// RUN: %analyzer -J-Xmx4g --script %s --params cpgFilePath=%t.cpg.bin.zip | %filecheck %s --match-full-lines

def getArgument(call: Call, n: Int): String = {
  return call.start.argument.order(n).code.head
}

def getLocation(call: Call): String = {
  return call.start.file.name.head.split("/").last + ":" + call.lineNumber.get.toString
}

def concatArgs(call: NodeSteps[Call], first: Int, second: Int) : String = {
  return call.map(c =>
    getArgument(c, first) + " = " +
    getArgument(c, second) + " at " +
    getLocation(c)
  ).toList.sorted.mkString("\n")
}

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  yapDatabaseExercise(cpg)
  userDefaultsExercise(cpg)
  keychainExercise(cpg)
  realmExercise(cpg)
  coreDataExercise(cpg)
}

def yapDatabaseExercise(cpg: Cpg) = {
  var call = cpg.method.fullNameExact("-[YapDatabaseReadWriteTransaction setObject:forKey:inCollection:]").callIn
  println(concatArgs(call, 4, 3))

  // CHECK: YapKeyEmail = JohnDoe@yap.com at YAPExcersizeViewController.m:28
  // CHECK-NEXT: YapKeyPassword = TheUnknown at YAPExcersizeViewController.m:29
}

def userDefaultsExercise(cpg: Cpg) = {
  val call = cpg.method.fullNameExact("-[NSUserDefaults setObject:forKey:]").callIn.where(c => c.start.file.name.head.contains("NSUserDefaultsStorageExerciseViewController"))
  println(concatArgs(call, 4, 3))
  // CHECK: PIN = 53cr3tP at NSUserDefaultsStorageExerciseViewController.m:22
}

def keychainExercise(cpg: Cpg) = {
  val call = cpg.method.fullNameExact("-[NSUserDefaults setObject:forKey:]").callIn.where(c => c.start.file.name.head.contains("KeychainExerciseViewController"))
  val callArgs = call.map(c => "Storing " + getArgument(c, 4) + " in NSUserDefaults at " + getLocation(c)).toList.sorted.mkString("\n")
  println(callArgs)
  // CHECK: Storing password in NSUserDefaults at KeychainExerciseViewController.m:28
  // CHECK-NEXT: Storing username in NSUserDefaults at KeychainExerciseViewController.m:27
}

def realmExercise(cpg: Cpg) = {
  println(cpg.typeDecl.nameExact("RLMObject").derivedTypeDecl.name.toList.mkString("\n"))
  // CHECK: RCreditInfo

  println(cpg.typeDecl.name("RLMObject").derivedTypeDecl.method.fullName.toList.sorted.mkString("\n"))
  // CHECK: -[RCreditInfo .cxx_destruct]
  // CHECK-NEXT: -[RCreditInfo cardNumber]
  // CHECK-NEXT: -[RCreditInfo cvv]
  // CHECK-NEXT: -[RCreditInfo name]
  // CHECK-NEXT: -[RCreditInfo setCardNumber:]
  // CHECK-NEXT: -[RCreditInfo setCvv:]
  // CHECK-NEXT: -[RCreditInfo setName:]

  val setName = cpg.method.fullNameExact("-[RCreditInfo setName:]").callIn.head
  val setCard = cpg.method.fullNameExact("-[RCreditInfo setCardNumber:]").callIn.head
  val setCVV = cpg.method.fullNameExact("-[RCreditInfo setCvv:]").callIn.head

  println("Setting Name to " + getArgument(setName, 3) + " at " + getLocation(setName))
  println("Setting Card Number to " + getArgument(setCard, 3) + " at " + getLocation(setCard))
  println("Setting CVV to " + getArgument(setCVV, 3) + " at " + getLocation(setCVV))

  // CHECK: Setting Name to John Doe at RealmExerciseViewController.m:30
  // CHECK-NEXT: Setting Card Number to 4444 5555 8888 1111 at RealmExerciseViewController.m:31
  // CHECK-NEXT: Setting CVV to 911 at RealmExerciseViewController.m:32
}

def coreDataExercise(cpg: Cpg) = {
  println(cpg.typeDecl.name("NSManagedObject").derivedTypeDecl.name.l.mkString("\n"))
  // CHECK: User

  println(cpg.call.where(c => c.start.typ.name.head == "User*").name.toSet.toList.sorted.mkString("\n"))
  // CHECK: <operator>.assignment
  // CHECK-NEXT: <operator>.cast
  // CHECK-NEXT: <operator>.indirection
  // CHECK-NEXT: email
  // CHECK-NEXT: password
  // CHECK-NEXT: setEmail:
  // CHECK-NEXT: setPassword:

  /// TODO: Rewrite when we support dynamic properties
  val setEmail = cpg.call.where(c => c.start.typ.name.head == "User*").name("setEmail:").head
  val setPassword = cpg.call.where(c => c.start.typ.name.head == "User*").name("setPassword:").head

  println("Setting email to" + getArgument(setEmail, 3) + " at " + getLocation(setEmail))
  println("Setting password to " + getArgument(setPassword, 3) + " at " + getLocation(setPassword))

  // CHECK: Setting email tojohn@test.com at CoreDataExerciseViewController.m:80
  // CHECK-NEXT: Setting password to coredbpassword at CoreDataExerciseViewController.m:81
}

// CHECK: script finished successfully

