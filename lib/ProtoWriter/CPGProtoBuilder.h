#pragma once

#include "CPGProtoNode.h"
#include <cpg.pb.h>
#include <memory>

namespace llvm2cpg {

class CPGProtoBuilder {
public:
  CPGProtoBuilder();

  CPGProtoNode *unknownNode();

  CPGProtoNode *metadataNode();
  CPGProtoNode *fileNode();
  CPGProtoNode *namespaceBlockNode();
  CPGProtoNode *methodNode();
  CPGProtoNode *methodInstNode();
  CPGProtoNode *methodReturnNode();
  CPGProtoNode *typeDeclNode();
  CPGProtoNode *typeNode();
  CPGProtoNode *methodBlockNode();
  CPGProtoNode *returnNode();
  CPGProtoNode *literalNode();
  CPGProtoNode *methodParameterInNode();
  CPGProtoNode *functionCallNode();
  CPGProtoNode *localVariableNode();
  CPGProtoNode *identifierNode();
  CPGProtoNode *methodRef();

  void connectAST(const CPGProtoNode *from, const CPGProtoNode *to);
  void connectREF(const CPGProtoNode *from, const CPGProtoNode *to);
  void connectReceiver(const CPGProtoNode *from, const CPGProtoNode *to);
  void connectCFG(uint64_t from, uint64_t to);

  cpg::CpgStruct *getGraph() const;

private:
  cpg::CpgStruct cpgBuilder;
  cpg::PropertyValue propertyBuilder;
  int64_t nodeId;
  std::unique_ptr<cpg::CpgStruct> graph;
  std::vector<std::unique_ptr<CPGProtoNode>> nodes;

  CPGProtoNode *newNode(cpg::CpgStruct_Node_NodeType type);
  void connect(cpg::CpgStruct_Edge_EdgeType type, uint64_t from, uint64_t to);
};

} // namespace llvm2cpg
