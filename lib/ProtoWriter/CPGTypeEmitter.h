#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace llvm {
class Type;
class Function;
class Module;
class StructType;
class PointerType;
class FunctionType;
class VectorType;
class ArrayType;
} // namespace llvm

namespace llvm2cpg {
class CPGProtoBuilder;
class CPGProtoNode;
class ObjCClassDefinition;
class ObjCTypeHierarchy;
class CPGLogger;

class CPGTypeEmitter {
public:
  CPGTypeEmitter(CPGProtoBuilder &builder, CPGLogger &logger);
  void emitObjCTypes(const llvm::Module &module);
  void emitObjCMethodBindings(const llvm::Module *module,
                              std::unordered_map<llvm::Function *, CPGProtoNode *> &emittedMethods);
  void emitStructMembers(std::vector<const llvm::Module *> &modules);
  void recordCanonicalStructNames(std::vector<const llvm::Module *> &modules);
  std::string recordType(const llvm::Type *type, const std::string &namespaceName);
  void emitRecordedTypes();

  CPGProtoNode *namedTypeDecl(const std::string &typeName);
  std::vector<std::string> &getStructMembers(const llvm::StructType *structType);

private:
  std::string recordType(const std::string &typeName, const std::string &typeLocation);
  void emitStructMembers(const llvm::Module *module,
                         std::unordered_set<std::string> &emittedStructs);
  CPGProtoNode *emitObjCType(ObjCClassDefinition *base, ObjCTypeHierarchy &typeHierarchy);

  CPGProtoNode *emitTypeDecl(const std::string &typeName, const std::string &typeLocation);
  CPGProtoNode *emitType(const std::string &typeName);

  std::string typeToString(const llvm::Type *type);
  std::string typeToString(const llvm::PointerType *type);
  std::string typeToString(const llvm::StructType *type);
  std::string typeToString(const llvm::FunctionType *type);
  std::string typeToString(const llvm::VectorType *type);
  std::string typeToString(const llvm::ArrayType *type);
  static std::string defaultTypeToString(const llvm::Type *type);
  void recordStructInformation(const llvm::Module *module);

  CPGProtoBuilder &builder;
  CPGLogger &logger;
  std::unordered_map<std::string, std::string> recordedTypes;
  std::unordered_map<std::string, CPGProtoNode *> namedTypeDecls;
  std::unordered_map<const llvm::StructType *, std::string> canonicalNames;
  std::unordered_map<std::string, std::vector<std::string>> structMemberNames;
  std::unordered_map<std::string, std::vector<std::string>> typeAliases;
};

} // namespace llvm2cpg
