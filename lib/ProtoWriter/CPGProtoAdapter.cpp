#include "CPGProtoAdapter.h"
#include "CPGEmitter.h"
#include "CPGTypeEmitter.h"
#include <google/protobuf/text_format.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm2cpg/CPG/CPG.h>
#include <llvm2cpg/CPG/CPGFile.h>
#include <sstream>
#include <zip.h>

using namespace llvm2cpg;

CPGProtoAdapter::CPGProtoAdapter(std::string zipPath, bool debug)
    : debug(debug), zipPath(std::move(zipPath)) {}

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

    CPGEmitter emitter(builder, typeEmitter, file);

    for (auto &method : file.getMethods()) {
      emitter.emitMethod(method);
    }
  }

  typeEmitter.emitRecordedTypes();
  saveToArchive();
}

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
