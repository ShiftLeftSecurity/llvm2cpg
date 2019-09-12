#include "CPGProtoAdapter.h"
#include <google/protobuf/text_format.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm2cpg/CPG/CPG.h>
#include <llvm2cpg/CPG/CPGFile.h>
#include <sstream>
#include <utility>
#include <zip.h>

using namespace llvm2cpg;

CPGProtoAdapter::CPGProtoAdapter(std::string zipPath, bool debug)
    : debug(debug), zipPath(std::move(zipPath)), nodeId(0), graph(cpgBuilder.New()) {}

void CPGProtoAdapter::writeCpg(const llvm2cpg::CPG &cpg) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  auto metadata = newMetadataNode();
  setLanguage(metadata, cpg::LANGUAGES::C);

  std::set<llvm::Type *> types;
  for (auto &file : cpg.getFiles()) {
    std::copy(std::begin(file.getTypes()),
              std::end(file.getTypes()),
              std::inserter(types, types.begin()));
  }

  for (auto type : types) {
    auto typeDeclNode = newTypeDeclNode();
    std::string typeName;
    llvm::raw_string_ostream stream(typeName);
    type->print(stream);
    stream.flush();
    setName(typeDeclNode, typeName);
    setFullName(typeDeclNode, typeName);
  }

  for (auto &file : cpg.getFiles()) {
    auto fileNode = newFileNode(file.getName());
    auto ns = newNamespaceNodeBlock(file.getName() + "_global");
    setFullName(ns, file.getName() + "_global");
    connectASTNodes(fileNode, ns);

    for (auto &method : file.getMethods()) {
      auto methodNode = newMethodNode(method.getName());
      connectASTNodes(ns, methodNode);
      setFullName(methodNode, method.getName());
      setStringProperty(methodNode, cpg::NodePropertyName::AST_PARENT_TYPE, "NAMESPACE_BLOCK");
      setStringProperty(
          methodNode, cpg::NodePropertyName::AST_PARENT_FULL_NAME, file.getName() + "_global");
      setIsExternal(methodNode, method.isExternal());

      auto methodReturnNode = newMethodReturnNode();
      connectASTNodes(methodNode, methodReturnNode);
    }
  }

  saveToArchive();
}

#pragma mark - Nodes

cpg::CpgStruct_Node *CPGProtoAdapter::newNode() {
  auto node = graph->add_node();
  node->set_key(nodeId++);
  return node;
}

cpg::CpgStruct_Node *CPGProtoAdapter::newMetadataNode() {
  auto node = newNode();
  node->set_type(cpg::CpgStruct_Node_NodeType_META_DATA);
  return node;
}

cpg::CpgStruct_Node *CPGProtoAdapter::newFileNode(const std::string &name) {
  auto node = newNode();
  node->set_type(cpg::CpgStruct_Node_NodeType_FILE);
  setName(node, name);
  return node;
}

cpg::CpgStruct_Node *CPGProtoAdapter::newNamespaceNodeBlock(const std::string &name) {
  auto node = newNode();
  node->set_type(cpg::CpgStruct_Node_NodeType_NAMESPACE_BLOCK);
  setName(node, name);
  return node;
}

cpg::CpgStruct_Node *CPGProtoAdapter::newMethodNode(const std::string &name) {
  auto node = newNode();
  node->set_type(cpg::CpgStruct_Node_NodeType_METHOD);
  setName(node, name);
  return node;
}

void CPGProtoAdapter::connectASTNodes(cpg::CpgStruct_Node *from, cpg::CpgStruct_Node *to) {
  auto edge = graph->add_edge();
  edge->set_type(cpg::CpgStruct_Edge_EdgeType_AST);
  edge->set_src(from->key());
  edge->set_dst(to->key());
}

cpg::CpgStruct_Node *CPGProtoAdapter::newMethodReturnNode() {
  auto node = newNode();
  node->set_type(cpg::CpgStruct_Node_NodeType_METHOD_RETURN);
  return node;
}

cpg::CpgStruct_Node *CPGProtoAdapter::newTypeDeclNode() {
  auto node = newNode();
  node->set_type(cpg::CpgStruct_Node_NodeType_TYPE_DECL);
  return node;
}

#pragma mark - Properties

void CPGProtoAdapter::setStringProperty(cpg::CpgStruct_Node *node,
                                        cpg::NodePropertyName propertyName,
                                        const std::string &value) {
  auto property = node->add_property();
  auto propertyValue = propertyBuilder.New();

  property->set_name(propertyName);
  propertyValue->set_string_value(value);
  property->set_allocated_value(propertyValue);
}

void CPGProtoAdapter::setIntProperty(cpg::CpgStruct_Node *node, cpg::NodePropertyName propertyName,
                                     int value) {
  auto property = node->add_property();
  auto propertyValue = propertyBuilder.New();

  property->set_name(propertyName);
  propertyValue->set_int_value(value);
  property->set_allocated_value(propertyValue);
}

void CPGProtoAdapter::setBooleanProperty(cpg::CpgStruct_Node *node,
                                         cpg::NodePropertyName propertyName, bool value) {
  auto property = node->add_property();
  auto propertyValue = propertyBuilder.New();

  property->set_name(propertyName);
  propertyValue->set_bool_value(value);
  property->set_allocated_value(propertyValue);
}

void CPGProtoAdapter::setLanguage(cpg::CpgStruct_Node *metadata, cpg::LANGUAGES language) {
  setStringProperty(metadata, cpg::NodePropertyName::LANGUAGE, cpg::LANGUAGES_Name(language));
}

void CPGProtoAdapter::setName(cpg::CpgStruct_Node *node, const std::string &name) {
  setStringProperty(node, cpg::NodePropertyName::NAME, name);
}

void CPGProtoAdapter::setFullName(cpg::CpgStruct_Node *node, const std::string &name) {
  setStringProperty(node, cpg::NodePropertyName::FULL_NAME, name);
}

void CPGProtoAdapter::setIsExternal(cpg::CpgStruct_Node *node, bool isExternal) {
  setBooleanProperty(node, cpg::NodePropertyName::IS_EXTERNAL, isExternal);
}

#pragma mark -

void CPGProtoAdapter::saveToArchive() {
  int zipError = 0;
  auto archive = zip_open(zipPath.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &zipError);
  if (!archive) {
    zip_error_t error;
    zip_error_init_with_code(&error, zipError);
    std::cerr << "zip_open: " << zip_error_strerror(&error) << "\n";
    return;
  }

  std::stringstream stream;
  if (!graph->SerializeToOstream(&stream)) {
    std::cerr << "Cannot serialize CPG into protobuf\n";
    return;
  }

  if (debug) {
    std::string content;
    google::protobuf::TextFormat::PrintToString(*graph, &content);
    std::cout << content;
  }

  auto str = stream.str();
  auto source = zip_source_buffer(archive, str.c_str(), str.size(), 0);
  if (!source) {
    auto error = zip_get_error(archive);
    std::cerr << "zip_source_buffer_create: " << zip_error_strerror(error) << "\n";
    zip_discard(archive);
    return;
  }

  auto fileIndex = zip_file_add(archive, "cpg.bin", source, 0);

  if (fileIndex == -1) {
    auto error = zip_get_error(archive);
    std::cerr << "zip_file_add: " << zip_error_strerror(error) << "\n";
    zip_source_free(source);
    zip_discard(archive);
    return;
  }

  if (zip_close(archive) == -1) {
    auto error = zip_get_error(archive);
    std::cerr << "zip_close: " << zip_error_strerror(error) << "\n";
    zip_discard(archive);
    return;
  }

  std::cout << "CPG is successfully save on disk: " << zipPath << "\n";
}