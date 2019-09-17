#include "CPGProtoNode.h"

using namespace llvm2cpg;

CPGProtoNode::CPGProtoNode(cpg::CpgStruct_Node *cpgNode, cpg::PropertyValue &propertyBuilder)
    : cpgNode(cpgNode), propertyBuilder(propertyBuilder) {}

CPGProtoNode::CPGProtoNode(CPGProtoNode &&that) noexcept
    : cpgNode(that.cpgNode), propertyBuilder(that.propertyBuilder) {
  that.cpgNode = nullptr;
}

int64_t CPGProtoNode::getID() const {
  assert(cpgNode);
  return cpgNode->key();
}

bool CPGProtoNode::isReturn() const {
  assert(cpgNode);
  return cpgNode->type() == cpg::CpgStruct_Node_NodeType_RETURN;
}

CPGProtoNode &CPGProtoNode::setLanguage(cpg::LANGUAGES language) {
  assert(cpgNode->type() == cpg::CpgStruct_Node_NodeType::CpgStruct_Node_NodeType_META_DATA);
  setStringProperty(cpg::NodePropertyName::LANGUAGE, cpg::LANGUAGES_Name(language));
  return *this;
}

CPGProtoNode &CPGProtoNode::setVersion(const std::string &version) {
  assert(cpgNode->type() == cpg::CpgStruct_Node_NodeType::CpgStruct_Node_NodeType_META_DATA);
  setStringProperty(cpg::NodePropertyName::VERSION, version);
  return *this;
}

CPGProtoNode &CPGProtoNode::setName(const std::string &name) {
  setStringProperty(cpg::NodePropertyName::NAME, name);
  return *this;
}

CPGProtoNode &CPGProtoNode::setFullName(const std::string &name) {
  setStringProperty(cpg::NodePropertyName::FULL_NAME, name);
  return *this;
}

CPGProtoNode &CPGProtoNode::setTypeDeclFullName(const std::string &name) {
  setStringProperty(cpg::NodePropertyName::TYPE_DECL_FULL_NAME, name);
  return *this;
}

CPGProtoNode &CPGProtoNode::setASTParentType(const std::string &type) {
  setStringProperty(cpg::NodePropertyName::AST_PARENT_TYPE, type);
  return *this;
}

CPGProtoNode &CPGProtoNode::setASTParentFullName(const std::string &name) {
  setStringProperty(cpg::NodePropertyName::AST_PARENT_FULL_NAME, name);
  return *this;
}

CPGProtoNode &CPGProtoNode::setSignature(const std::string &signature) {
  setStringProperty(cpg::NodePropertyName::SIGNATURE, signature);
  return *this;
}

CPGProtoNode &CPGProtoNode::setTypeFullName(const std::string &name) {
  setStringProperty(cpg::NodePropertyName::TYPE_FULL_NAME, name);
  return *this;
}

CPGProtoNode &CPGProtoNode::setCode(const std::string &code) {
  setStringProperty(cpg::NodePropertyName::CODE, code);
  return *this;
}

CPGProtoNode &CPGProtoNode::setEvaluationStrategy(const std::string &strategy) {
  setStringProperty(cpg::NodePropertyName::EVALUATION_STRATEGY, strategy);
  return *this;
}

CPGProtoNode &CPGProtoNode::setIsExternal(bool isExternal) {
  setBooleanProperty(cpg::NodePropertyName::IS_EXTERNAL, isExternal);
  return *this;
}

CPGProtoNode &CPGProtoNode::setOrder(int order) {
  setIntProperty(cpg::NodePropertyName::ORDER, order);
  return *this;
}

CPGProtoNode &CPGProtoNode::setArgumentIndex(int index) {
  setIntProperty(cpg::NodePropertyName::ARGUMENT_INDEX, index);
  return *this;
}

#pragma mark - Properties

/// Note on memory management and ownership:
/// The caller of the `New` method should take ownership of the returned pointer
/// However, ownership is taken over by `set_allocated_value`, so we don't need to take care of it
/// Search for `set_allocated_` at
/// https://developers.google.com/protocol-buffers/docs/reference/cpp-generated
void CPGProtoNode::setBooleanProperty(cpg::NodePropertyName propertyName, bool value) {
  assert(cpgNode);
  auto property = cpgNode->add_property();
  property->set_name(propertyName);

  auto propertyValue = propertyBuilder.New();
  propertyValue->set_bool_value(value);
  property->set_allocated_value(propertyValue);
}

void CPGProtoNode::setStringProperty(cpg::NodePropertyName propertyName, const std::string &value) {
  assert(cpgNode);
  auto property = cpgNode->add_property();
  property->set_name(propertyName);

  auto propertyValue = propertyBuilder.New();
  propertyValue->set_string_value(value);
  property->set_allocated_value(propertyValue);
}

void CPGProtoNode::setIntProperty(cpg::NodePropertyName propertyName, int value) {
  assert(cpgNode);
  auto property = cpgNode->add_property();
  property->set_name(propertyName);

  auto propertyValue = propertyBuilder.New();
  propertyValue->set_int_value(value);
  property->set_allocated_value(propertyValue);
}
