#pragma once

#include <cpg.pb.h>
#include <cstdint>

namespace llvm2cpg {

class CPG;
class CPGMethod;

class CPGProtoAdapter {
public:
  CPGProtoAdapter(std::string zipPath, bool debug);
  void writeCpg(const llvm2cpg::CPG &cpg);

private:
  bool debug;
  std::string zipPath;

  int64_t nodeId;
  cpg::CpgStruct cpgBuilder;
  cpg::PropertyValue propertyBuilder;

  std::unique_ptr<cpg::CpgStruct> graph;

#pragma mark - Nodes

  cpg::CpgStruct_Node *newNode();
  cpg::CpgStruct_Node *newMetadataNode();
  cpg::CpgStruct_Node *newFileNode(const std::string &name);
  cpg::CpgStruct_Node *newNamespaceNodeBlock(const std::string &name);
  cpg::CpgStruct_Node *newMethodNode(const std::string &name);
  cpg::CpgStruct_Node *newMethodReturnNode();
  cpg::CpgStruct_Node *newTypeDeclNode();
  cpg::CpgStruct_Node *newMethodBlockNode();

  void connectASTNodes(cpg::CpgStruct_Node *from, cpg::CpgStruct_Node *to);

#pragma mark - Property Setters

  void setStringProperty(cpg::CpgStruct_Node *node, cpg::NodePropertyName propertyName,
                         const std::string &value);
  void setIntProperty(cpg::CpgStruct_Node *node, cpg::NodePropertyName propertyName, int value);
  void setBooleanProperty(cpg::CpgStruct_Node *node, cpg::NodePropertyName propertyName,
                          bool value);

  void setLanguage(cpg::CpgStruct_Node *metadata, cpg::LANGUAGES language);
  void setVersion(cpg::CpgStruct_Node *metadata, const std::string &version);
  void setName(cpg::CpgStruct_Node *node, const std::string &name);
  void setFullName(cpg::CpgStruct_Node *node, const std::string &name);
  void setASTParentType(cpg::CpgStruct_Node *node, const std::string &type);
  void setASTParentFullName(cpg::CpgStruct_Node *node, const std::string &name);
  void setSignature(cpg::CpgStruct_Node *node, const std::string &signature);
  void setTypeFullName(cpg::CpgStruct_Node *node, const std::string &name);
  void setCode(cpg::CpgStruct_Node *node, const std::string &code);
  void setEvaluationStrategy(cpg::CpgStruct_Node *node, const std::string &strategy);
  void setIsExternal(cpg::CpgStruct_Node *node, bool isExternal);
  void setOrder(cpg::CpgStruct_Node *node, int order);
  void setArgumentIndex(cpg::CpgStruct_Node *node, int index);

  void saveToArchive();
};

} // namespace llvm2cpg