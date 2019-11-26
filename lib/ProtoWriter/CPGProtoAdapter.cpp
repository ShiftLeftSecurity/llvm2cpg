#include "CPGProtoAdapter.h"
#include "CPGEmitter.h"
#include "CPGTypeEmitter.h"
#include <google/protobuf/text_format.h>
#include <llvm2cpg/CPG/CPG.h>
#include <llvm2cpg/Logger/CPGLogger.h>
#include <llvm2cpg/Traversals/ObjCTraversal.h>
#include <sstream>
#include <zip.h>

using namespace llvm2cpg;

CPGProtoAdapter::CPGProtoAdapter(CPGLogger &logger, std::string zipPath)
    : logger(logger), zipPath(std::move(zipPath)) {}

void CPGProtoAdapter::writeCpg(const llvm2cpg::CPG &cpg) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  CPGProtoNode *metadata = builder.metadataNode();
  (*metadata) //
      .setLanguage(cpg::LANGUAGES::C)
      .setVersion("0");

  auto globalNamespaceBlockNode = builder.namespaceBlockNode();
  (*globalNamespaceBlockNode) //
      .setName("<global>")
      .setFullName("<global>")
      .setOrder(0);

  CPGTypeEmitter typeEmitter(builder);
  for (size_t index = 0; index < cpg.getFiles().size(); index++) {
    logger.uiInfo(std::string("Emitting CPG ") + std::to_string(index + 1) + "/" +
                  std::to_string(cpg.getFiles().size()));
    std::unordered_map<llvm::Function *, CPGProtoNode *> emittedMethods;
    const CPGFile &file = cpg.getFiles()[index];
    typeEmitter.emitObjCTypes(*file.getModule());
    auto fileNode = builder.fileNode();
    (*fileNode) //
        .setName(file.getName())
        .setOrder(0);

    auto namespaceBlockNode = builder.namespaceBlockNode();
    (*namespaceBlockNode) //
        .setName(file.getGlobalNamespaceName())
        .setFullName(file.getGlobalNamespaceName())
        .setOrder(0);

    builder.connectAST(fileNode, namespaceBlockNode);

    CPGEmitter emitter(logger, builder, typeEmitter, file);

    for (auto &method : file.getMethods()) {
      CPGProtoNode *methodNode = emitter.emitMethod(method);
      emittedMethods.insert(std::make_pair(&method.getFunction(), methodNode));
    }

    ObjCTraversal traversal(file.getModule());
    std::vector<const llvm::ConstantStruct *> objcClassWorklist;
    for (const llvm::ConstantStruct *objcClass : traversal.objcClasses()) {
      objcClassWorklist.push_back(objcClass);
      objcClassWorklist.push_back(traversal.objcMetaclass(objcClass));
    }

    for (const llvm::ConstantStruct *objcClass : objcClassWorklist) {
      const llvm::ConstantStruct *objcClassRO = traversal.objcClassROCounterpart(objcClass);
      std::string className = traversal.objcClassName(objcClassRO);

      for (auto &methodPair : traversal.objcMethods(objcClassRO)) {
        std::string methodName = methodPair.first;
        llvm::Function *method = methodPair.second;
        CPGProtoNode *typeDecl = typeEmitter.objcClassTypeDecl(className);
        CPGProtoNode *methodNode = emittedMethods.at(method);
        (*methodNode)
            .setName(methodName)
            .setASTParentType("TYPE_DECL")
            .setASTParentFullName(className);

        CPGProtoNode *binding = builder.bindingNode();
        (*binding).setName(methodName).setSignature("");
        builder.connectREF(binding, methodNode);
        builder.connectBinding(typeDecl, binding);
      }
    }
  }

  typeEmitter.emitRecordedTypes();
  saveToArchive();
}

void CPGProtoAdapter::saveToArchive() {
  logger.uiInfo("Serializing CPG");
  cpg::CpgStruct *graph = builder.getGraph();
  int zipError = 0;
  auto archive = zip_open(zipPath.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &zipError);
  if (!archive) {
    zip_error_t error;
    zip_error_init_with_code(&error, zipError);
    logger.uiFatal(std::string("zip_open: ") + zip_error_strerror(&error) + "\n");
    return;
  }

  std::stringstream stream;
  if (!graph->SerializeToOstream(&stream)) {
    logger.uiFatal("Cannot serialize CPG into protobuf\n");
    return;
  }

  logger.uiInfo("Saving CPG on disk");

  auto str = stream.str();
  auto source = zip_source_buffer(archive, str.c_str(), str.size(), 0);
  if (!source) {
    auto error = zip_get_error(archive);
    logger.uiFatal(std::string("zip_source_buffer_create: ") + zip_error_strerror(error) + "\n");
    zip_discard(archive);
    return;
  }

  auto fileIndex = zip_file_add(archive, "cpg.bin", source, 0);

  if (fileIndex == -1) {
    auto error = zip_get_error(archive);
    logger.uiFatal(std::string("zip_file_add: ") + zip_error_strerror(error) + "\n");
    zip_source_free(source);
    zip_discard(archive);
    return;
  }

  if (zip_close(archive) == -1) {
    auto error = zip_get_error(archive);
    logger.uiFatal(std::string("zip_close: ") + zip_error_strerror(error) + "\n");
    zip_discard(archive);
    return;
  }

  logger.uiInfo(std::string("CPG is successfully saved on disk: ") + zipPath);
}
