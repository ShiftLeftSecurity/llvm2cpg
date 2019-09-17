#pragma once

#include "CPGProtoNode.h"
#include <cpg.pb.h>
#include <memory>

namespace llvm2cpg {

class CPGProtoBuilder {
public:
  CPGProtoBuilder();

  CPGProtoNode unknownNode();

  CPGProtoNode metadataNode();
  CPGProtoNode fileNode();
  CPGProtoNode namespaceBlockNode();
  CPGProtoNode methodNode();
  CPGProtoNode methodReturnNode();
  CPGProtoNode typeDeclNode();
  CPGProtoNode typeNode();
  CPGProtoNode methodBlockNode();
  CPGProtoNode returnNode();
  CPGProtoNode literalNode();
  CPGProtoNode methodParameterInNode();

  void connectAST(const CPGProtoNode &from, const CPGProtoNode &to);
  void connectCFG(const CPGProtoNode &from, const CPGProtoNode &to);

  cpg::CpgStruct *getGraph() const;

private:
  cpg::CpgStruct cpgBuilder;
  cpg::PropertyValue propertyBuilder;
  int64_t nodeId;
  std::unique_ptr<cpg::CpgStruct> graph;

  CPGProtoNode newNode(cpg::CpgStruct_Node_NodeType type);
  void connect(cpg::CpgStruct_Edge_EdgeType type, const CPGProtoNode &from, const CPGProtoNode &to);
};

} // namespace llvm2cpg
