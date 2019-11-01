#include "CPGProtoAdapter.h"
#include "CPGEmitter.h"
#include "CPGTypeEmitter.h"
#include <google/protobuf/text_format.h>
#include <llvm2cpg/CPG/CPG.h>
#include <llvm2cpg/Logger/CPGLogger.h>
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
  for (auto &file : cpg.getFiles()) {
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
      emitter.emitMethod(method);
    }
  }

  typeEmitter.emitRecordedTypes();
  saveToArchive();
}

void CPGProtoAdapter::saveToArchive() {
  logger.info("Serializing CPG");
  cpg::CpgStruct *graph = builder.getGraph();
  int zipError = 0;
  auto archive = zip_open(zipPath.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &zipError);
  if (!archive) {
    zip_error_t error;
    zip_error_init_with_code(&error, zipError);
    logger.error(std::string("zip_open: ") + zip_error_strerror(&error) + "\n");
    return;
  }

  std::stringstream stream;
  if (!graph->SerializeToOstream(&stream)) {
    logger.error("Cannot serialize CPG into protobuf\n");
    return;
  }

  logger.info("Saving CPG on disk");

  auto str = stream.str();
  auto source = zip_source_buffer(archive, str.c_str(), str.size(), 0);
  if (!source) {
    auto error = zip_get_error(archive);
    logger.error(std::string("zip_source_buffer_create: ") + zip_error_strerror(error) + "\n");
    zip_discard(archive);
    return;
  }

  auto fileIndex = zip_file_add(archive, "cpg.bin", source, 0);

  if (fileIndex == -1) {
    auto error = zip_get_error(archive);
    logger.error(std::string("zip_file_add: ") + zip_error_strerror(error) + "\n");
    zip_source_free(source);
    zip_discard(archive);
    return;
  }

  if (zip_close(archive) == -1) {
    auto error = zip_get_error(archive);
    logger.error(std::string("zip_close: ") + zip_error_strerror(error) + "\n");
    zip_discard(archive);
    return;
  }

  logger.info(std::string("CPG is successfully saved on disk: ") + zipPath);
}
