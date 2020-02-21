#pragma once

#include <cpg.pb.h>
#include <string>

namespace llvm {
class Value;
}

namespace llvm2cpg {

class CPGProtoNode {
public:
  CPGProtoNode(cpg::CpgStruct_Node *cpgNode, cpg::PropertyValue &propertyBuilder);

  CPGProtoNode(CPGProtoNode &&that) = delete;
  CPGProtoNode &operator=(CPGProtoNode &&) = delete;
  CPGProtoNode(const CPGProtoNode &) = delete;
  CPGProtoNode &operator=(const CPGProtoNode &) = delete;

  int64_t getID() const;

  void setEntry(int64_t e);
  int64_t getEntry() const;

  CPGProtoNode &setLanguage(cpg::LANGUAGES language);
  CPGProtoNode &setVersion(const std::string &version);
  CPGProtoNode &setName(const std::string &name);
  CPGProtoNode &setCanonicalName(const std::string &name);
  CPGProtoNode &setFullName(const std::string &name);
  CPGProtoNode &setTypeDeclFullName(const std::string &name);
  CPGProtoNode &setASTParentType(const std::string &type);
  CPGProtoNode &setASTParentFullName(const std::string &name);
  CPGProtoNode &setSignature(const std::string &signature);
  CPGProtoNode &setTypeFullName(const std::string &name);
  CPGProtoNode &setCode(const std::string &code);
  CPGProtoNode &setEvaluationStrategy(const std::string &strategy);
  CPGProtoNode &setDispatchType(const std::string &dispatchType);
  CPGProtoNode &setMethodInstFullName(const std::string &name);
  CPGProtoNode &setMethodFullName(const std::string &name);
  CPGProtoNode &setIsExternal(bool isExternal);
  CPGProtoNode &setOrder(int order);
  CPGProtoNode &setArgumentIndex(int index);
  CPGProtoNode &setOrderAndIndex(int order);
  CPGProtoNode &setLineNumber(int line);
  CPGProtoNode &setColumnNumber(int column);
  CPGProtoNode &setInheritsFromTypeFullName(const std::string &name);
  CPGProtoNode &setDynamicTypeHintFullName(const std::string &name);
  CPGProtoNode &setAliasTypeFullName(const std::string &name);

private:
  cpg::CpgStruct_Node *cpgNode;
  cpg::PropertyValue &propertyBuilder;

  /// id is stored within cpgNode, but it's useful to also have it here for debugging
  int64_t id;
  int64_t entry;

  void setStringProperty(cpg::NodePropertyName propertyName, const std::string &value);
  void setIntProperty(cpg::NodePropertyName propertyName, int value);
  void setBooleanProperty(cpg::NodePropertyName propertyName, bool value);
};

} // namespace llvm2cpg
