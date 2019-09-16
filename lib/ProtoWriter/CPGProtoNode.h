#pragma once

#include <cpg.pb.h>
#include <string>

namespace llvm2cpg {

class CPGProtoNode {
public:
  CPGProtoNode(cpg::CpgStruct_Node *cpgNode, cpg::PropertyValue &propertyBuilder);
  CPGProtoNode(CPGProtoNode &&that) noexcept;

  CPGProtoNode &operator=(CPGProtoNode &&) = delete;
  CPGProtoNode(const CPGProtoNode &) = delete;
  CPGProtoNode &operator=(const CPGProtoNode &) = delete;

  int64_t getID() const;

  CPGProtoNode &setLanguage(cpg::LANGUAGES language);
  CPGProtoNode &setVersion(const std::string &version);
  CPGProtoNode &setName(const std::string &name);
  CPGProtoNode &setFullName(const std::string &name);
  CPGProtoNode &setTypeDeclFullName(const std::string &name);
  CPGProtoNode &setASTParentType(const std::string &type);
  CPGProtoNode &setASTParentFullName(const std::string &name);
  CPGProtoNode &setSignature(const std::string &signature);
  CPGProtoNode &setTypeFullName(const std::string &name);
  CPGProtoNode &setCode(const std::string &code);
  CPGProtoNode &setEvaluationStrategy(const std::string &strategy);
  CPGProtoNode &setIsExternal(bool isExternal);
  CPGProtoNode &setOrder(int order);
  CPGProtoNode &setArgumentIndex(int index);

private:
  cpg::CpgStruct_Node *cpgNode;
  cpg::PropertyValue &propertyBuilder;

  void setStringProperty(cpg::NodePropertyName propertyName, const std::string &value);
  void setIntProperty(cpg::NodePropertyName propertyName, int value);
  void setBooleanProperty(cpg::NodePropertyName propertyName, bool value);
};

} // namespace llvm2cpg
