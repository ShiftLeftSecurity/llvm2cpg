#include "CPGProtoAdapter.h"
#include <google/protobuf/text_format.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm2cpg/CPG/CPG.h>
#include <llvm2cpg/CPG/CPGFile.h>
#include <sstream>
#include <utility>
#include <zip.h>

using namespace llvm2cpg;

static std::string typeToString(llvm::Type *type) {
  std::string typeName;
  llvm::raw_string_ostream stream(typeName);
  type->print(stream);
  stream.flush();
  return typeName;
}

CPGProtoAdapter::CPGProtoAdapter(std::string zipPath, bool debug)
    : debug(debug), zipPath(std::move(zipPath)) {}

void CPGProtoAdapter::writeCpg(const llvm2cpg::CPG &cpg) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  auto metadata = builder.metadataNode();
  metadata //
      .setLanguage(cpg::LANGUAGES::C)
      .setVersion("0");

  std::set<llvm::Type *> types;
  for (auto &file : cpg.getFiles()) {
    std::copy(std::begin(file.getTypes()),
              std::end(file.getTypes()),
              std::inserter(types, types.begin()));
  }

  for (auto type : types) {
    auto typeName = typeToString(type);

    auto typeDeclNode = builder.typeDeclNode();
    typeDeclNode //
        .setName(typeName)
        .setFullName(typeName)
        .setIsExternal(false)
        .setOrder(0)
        .setASTParentType("DANGLING_AREA")
        .setASTParentFullName("dangling_area");

    auto typeNode = builder.typeNode();
    typeNode //
        .setName(typeName)
        .setFullName(typeName)
        .setTypeDeclFullName(typeName);
  }

  for (auto &file : cpg.getFiles()) {

    auto fileNode = builder.fileNode();
    fileNode //
        .setName(file.getName())
        .setOrder(0);

    auto namespaceBlockName = file.getName() + "_global";
    auto namespaceBlockNode = builder.namespaceBlockNode();
    namespaceBlockNode //
        .setName(namespaceBlockName)
        .setFullName(namespaceBlockName)
        .setOrder(0);

    builder.connectAST(fileNode, namespaceBlockNode);

    for (auto &method : file.getMethods()) {
      auto methodReturnType = typeToString(method.getReturnType());

      auto methodNode = builder.methodNode();
      methodNode //
          .setName(method.getName())
          .setFullName(method.getName())
          .setASTParentType("NAMESPACE_BLOCK")
          .setASTParentFullName(namespaceBlockName)
          .setIsExternal(method.isExternal())
          .setSignature(method.getSignature())
          .setOrder(0);

      const llvm::Function &function = method.getFunction();
      size_t argIndex = 0;
      for (const auto &arg : function.args()) {
        auto parameterInNode = builder.methodParameterInNode();
        parameterInNode //
            .setName(arg.getName())
            .setTypeFullName(typeToString(arg.getType()))
            .setCode(arg.getName())
            .setEvaluationStrategy("EVAL")
            .setOrder(argIndex);

        builder.connectAST(methodNode, parameterInNode);
      }

      auto methodReturnNode = builder.methodReturnNode();
      methodReturnNode //
          .setOrder(0)
          .setTypeFullName(methodReturnType)
          .setCode(methodReturnType)
          .setEvaluationStrategy("EVAL");
      builder.connectAST(methodNode, methodReturnNode);

      auto blockNode = builder.methodBlockNode();
      blockNode //
          .setOrder(0)
          .setTypeFullName(methodReturnType)
          .setCode("x + 42")
          .setArgumentIndex(0);
      builder.connectAST(methodNode, blockNode);

      for (auto &inst : llvm::instructions(function)) {
        auto node = emitValue(&inst);
        builder.connectAST(blockNode, node);

        if (node.isReturn()) {
          builder.connectCFG(node, methodReturnNode);
        }
      }
    }
  }

  saveToArchive();
}

#pragma mark -

void CPGProtoAdapter::saveToArchive() {
  cpg::CpgStruct *graph = builder.getGraph();
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

CPGProtoNode CPGProtoAdapter::emitValue(const llvm::Value *value) {
  if (auto retInst = llvm::dyn_cast<llvm::ReturnInst>(value)) {
    auto returnNode = builder.returnNode();
    returnNode //
        .setOrder(0)
        .setArgumentIndex(0)
        .setCode("return");

    if (auto retValue = retInst->getReturnValue()) {
      auto node = emitValue(retValue);
      builder.connectAST(returnNode, node);
      builder.connectCFG(node, returnNode);
    }

    return returnNode;
  }

  if (auto constantInt = llvm::dyn_cast<llvm::ConstantInt>(value)) {
    auto literalNode = builder.literalNode();
    literalNode //
        .setOrder(0)
        .setArgumentIndex(0)
        .setTypeFullName(typeToString(constantInt->getType()))
        .setCode(constantInt->getValue().toString(10, true));
    return literalNode;
  }

  llvm::errs() << *value << "\n";

  llvm_unreachable("Cannot handle this value");

  return builder.unknownNode();
}
