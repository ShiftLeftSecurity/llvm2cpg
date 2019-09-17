#include "CPGProtoBuilder.h"

using namespace llvm2cpg;

CPGProtoBuilder::CPGProtoBuilder()
    : cpgBuilder(), propertyBuilder(), nodeId(0), graph(cpgBuilder.New()) {}

CPGProtoNode CPGProtoBuilder::metadataNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_META_DATA);
}

CPGProtoNode CPGProtoBuilder::fileNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_FILE);
}

CPGProtoNode CPGProtoBuilder::namespaceBlockNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_NAMESPACE_BLOCK);
}

CPGProtoNode CPGProtoBuilder::methodNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_METHOD);
}

CPGProtoNode CPGProtoBuilder::methodReturnNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_METHOD_RETURN);
}

CPGProtoNode CPGProtoBuilder::typeDeclNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_TYPE_DECL);
}

CPGProtoNode CPGProtoBuilder::typeNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_TYPE);
}

CPGProtoNode CPGProtoBuilder::methodBlockNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_BLOCK);
}

CPGProtoNode CPGProtoBuilder::returnNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_RETURN);
}

CPGProtoNode CPGProtoBuilder::literalNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_LITERAL);
}

CPGProtoNode CPGProtoBuilder::unknownNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_UNKNOWN_NODE_TYPE);
}

CPGProtoNode CPGProtoBuilder::methodParameterInNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_METHOD_PARAMETER_IN);
}

void CPGProtoBuilder::connectAST(const CPGProtoNode &from, const CPGProtoNode &to) {
  connect(cpg::CpgStruct_Edge_EdgeType_AST, from, to);
}

void CPGProtoBuilder::connectCFG(const CPGProtoNode &from, const CPGProtoNode &to) {
  connect(cpg::CpgStruct_Edge_EdgeType_CFG, from, to);
}

cpg::CpgStruct *CPGProtoBuilder::getGraph() const {
  return graph.get();
}

#pragma mark - Private

CPGProtoNode CPGProtoBuilder::newNode(cpg::CpgStruct_Node_NodeType type) {
  /// the graph holds the ownership over the allocated nodes
  /// https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.repeated_field
  ///
  ///   RepeatedPtrField is particularly different from STL vector as it manages ownership of the
  ///   pointers that it contains.
  ///
  auto node = graph->add_node();
  node->set_key(nodeId++);
  node->set_type(type);
  return CPGProtoNode(node, propertyBuilder);
}

void CPGProtoBuilder::connect(cpg::CpgStruct_Edge_EdgeType type, const CPGProtoNode &from,
                              const CPGProtoNode &to) {
  auto edge = graph->add_edge();
  edge->set_type(type);
  edge->set_src(from.getID());
  edge->set_dst(to.getID());
}
