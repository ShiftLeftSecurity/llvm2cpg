#include "CPGProtoBuilder.h"

using namespace llvm2cpg;

CPGProtoBuilder::CPGProtoBuilder()
    : cpgBuilder(), propertyBuilder(), nodeId(1), graph(cpgBuilder.New()) {}

#pragma mark - Node Creation

CPGProtoNode *CPGProtoBuilder::metadataNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_META_DATA);
}

CPGProtoNode *CPGProtoBuilder::fileNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_FILE);
}

CPGProtoNode *CPGProtoBuilder::namespaceBlockNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_NAMESPACE_BLOCK);
}

CPGProtoNode *CPGProtoBuilder::methodNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_METHOD);
}

CPGProtoNode *CPGProtoBuilder::methodInstNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_METHOD_INST);
}

CPGProtoNode *CPGProtoBuilder::methodReturnNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_METHOD_RETURN);
}

CPGProtoNode *CPGProtoBuilder::typeDeclNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_TYPE_DECL);
}

CPGProtoNode *CPGProtoBuilder::typeNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_TYPE);
}

CPGProtoNode *CPGProtoBuilder::methodBlockNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_BLOCK);
}

CPGProtoNode *CPGProtoBuilder::returnNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_RETURN);
}

CPGProtoNode *CPGProtoBuilder::literalNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_LITERAL);
}

CPGProtoNode *CPGProtoBuilder::unknownNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_UNKNOWN);
}

CPGProtoNode *CPGProtoBuilder::methodParameterInNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_METHOD_PARAMETER_IN);
}

CPGProtoNode *CPGProtoBuilder::functionCallNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_CALL);
}

CPGProtoNode *CPGProtoBuilder::localVariableNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_LOCAL);
}

CPGProtoNode *CPGProtoBuilder::identifierNode() {
  return newNode(cpg::CpgStruct_Node_NodeType_IDENTIFIER);
}

CPGProtoNode *CPGProtoBuilder::methodRef() {
  return newNode(cpg::CpgStruct_Node_NodeType_METHOD_REF);
}

#pragma mark - Node Connection

void CPGProtoBuilder::connectAST(const CPGProtoNode *from, const CPGProtoNode *to) {
  connect(cpg::CpgStruct_Edge_EdgeType_AST, from->getID(), to->getID());
}

void CPGProtoBuilder::connectREF(const CPGProtoNode *from, const CPGProtoNode *to) {
  connect(cpg::CpgStruct_Edge_EdgeType_REF, from->getID(), to->getID());
}

void CPGProtoBuilder::connectReceiver(const CPGProtoNode *from, const CPGProtoNode *to) {
  connect(cpg::CpgStruct_Edge_EdgeType_RECEIVER, from->getID(), to->getID());
}

void CPGProtoBuilder::connectArgument(const CPGProtoNode *from, const CPGProtoNode *to) {
  connect(cpg::CpgStruct_Edge_EdgeType_ARGUMENT, from->getID(), to->getID());
}

void CPGProtoBuilder::connectCFG(uint64_t from, uint64_t to) {
  connect(cpg::CpgStruct_Edge_EdgeType_CFG, from, to);
}

#pragma mark - Getters

cpg::CpgStruct *CPGProtoBuilder::getGraph() const {
  return graph.get();
}

#pragma mark - Private

CPGProtoNode *CPGProtoBuilder::newNode(cpg::CpgStruct_Node_NodeType type) {
  /// the graph holds the ownership over the allocated nodes
  /// https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.repeated_field
  ///
  ///   RepeatedPtrField is particularly different from STL vector as it manages ownership of the
  ///   pointers that it contains.
  ///
  auto node = graph->add_node();
  node->set_key(nodeId++);
  node->set_type(type);
  nodes.emplace_back(std::make_unique<CPGProtoNode>(node, propertyBuilder));
  return nodes.back().get();
}

void CPGProtoBuilder::connect(cpg::CpgStruct_Edge_EdgeType type, uint64_t from, uint64_t to) {
  assert(from > 0 && from < nodeId);
  assert(to > 0 && to < nodeId);

  auto edge = graph->add_edge();
  edge->set_type(type);
  edge->set_src(from);
  edge->set_dst(to);
}
