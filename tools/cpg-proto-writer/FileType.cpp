#include "FileType.h"
#include "llvm2cpg/Logger/CPGLogger.h"
#include <llvm/BinaryFormat/Magic.h>

using namespace llvm2cpg;

const char *magicName(llvm::file_magic &magic) {
  switch (magic) {

  case llvm::file_magic::unknown:
    return "unknown";
  case llvm::file_magic::bitcode:
    return "bitcode";
  case llvm::file_magic::archive:
    return "archive";
  case llvm::file_magic::elf:
    return "elf";
  case llvm::file_magic::elf_relocatable:
    return "elf_relocatable";
  case llvm::file_magic::elf_executable:
    return "elf_executable";
  case llvm::file_magic::elf_shared_object:
    return "elf_shared_object";
  case llvm::file_magic::elf_core:
    return "elf_core";
  case llvm::file_magic::macho_object:
    return "macho_object";
  case llvm::file_magic::macho_executable:
    return "macho_executable";
  case llvm::file_magic::macho_fixed_virtual_memory_shared_lib:
    return "macho_fixed_virtual_memory_shared_lib";
  case llvm::file_magic::macho_core:
    return "macho_core";
  case llvm::file_magic::macho_preload_executable:
    return "macho_preload_executable";
  case llvm::file_magic::macho_dynamically_linked_shared_lib:
    return "macho_dynamically_linked_shared_lib";
  case llvm::file_magic::macho_dynamic_linker:
    return "macho_dynamic_linker";
  case llvm::file_magic::macho_bundle:
    return "macho_bundle";
  case llvm::file_magic::macho_dynamically_linked_shared_lib_stub:
    return "macho_dynamically_linked_shared_lib_stub";
  case llvm::file_magic::macho_dsym_companion:
    return "macho_dsym_companion";
  case llvm::file_magic::macho_kext_bundle:
    return "macho_kext_bundle";
  case llvm::file_magic::macho_universal_binary:
    return "macho_universal_binary";
  case llvm::file_magic::coff_cl_gl_object:
    return "coff_cl_gl_object";
  case llvm::file_magic::coff_object:
    return "coff_object";
  case llvm::file_magic::coff_import_library:
    return "coff_import_library";
  case llvm::file_magic::pecoff_executable:
    return "pecoff_executable";
  case llvm::file_magic::windows_resource:
    return "windows_resource";
  case llvm::file_magic::wasm_object:
    return "wasm_object";
  case llvm::file_magic::pdb:
    return "pdb";
#if LLVM_VERSION_MAJOR >= 9
  case llvm::file_magic::minidump:
    return "minidump";
  case llvm::file_magic::xcoff_object_32:
    return "xcoff_object_32";
  case llvm::file_magic::xcoff_object_64:
    return "xcoff_object_64";
#endif
  }
}

FileType llvm2cpg::getFileType(llvm2cpg::CPGLogger &logger, const std::string &path) {
  llvm::file_magic magic;

  std::error_code error = llvm::identify_magic(path, magic);
  if (error) {
    logger.uiWarning(std::string("Cannot identify file type: ") + error.message());
    return FileType::Unsupported;
  }

  logger.logInfo(std::string("Detected file type ") + magicName(magic));

  switch (magic) {
  case llvm::file_magic::elf:
  case llvm::file_magic::elf_relocatable:
  case llvm::file_magic::elf_executable:
  case llvm::file_magic::elf_shared_object:
  case llvm::file_magic::elf_core:
    return FileType::Binary;

  case llvm::file_magic::macho_universal_binary:
  case llvm::file_magic::macho_object:
  case llvm::file_magic::macho_executable:
  case llvm::file_magic::macho_fixed_virtual_memory_shared_lib:
  case llvm::file_magic::macho_core:
  case llvm::file_magic::macho_preload_executable:
  case llvm::file_magic::macho_dynamically_linked_shared_lib:
  case llvm::file_magic::macho_dynamic_linker:
  case llvm::file_magic::macho_bundle:
  case llvm::file_magic::macho_dynamically_linked_shared_lib_stub:
  case llvm::file_magic::macho_dsym_companion:
  case llvm::file_magic::macho_kext_bundle:
    return FileType::Binary;

  case llvm::file_magic::archive:
    return FileType::Binary;

  case llvm::file_magic::bitcode:
    return FileType::Bitcode;

  /// Assuming it is a LLVM IR file
  case llvm::file_magic::unknown:
    return FileType::LLVM_IR;

  case llvm::file_magic::coff_cl_gl_object:
  case llvm::file_magic::coff_object:
  case llvm::file_magic::coff_import_library:
  case llvm::file_magic::pecoff_executable:
  case llvm::file_magic::windows_resource:
  case llvm::file_magic::wasm_object:
  case llvm::file_magic::pdb:
#if LLVM_VERSION_MAJOR >= 9
  case llvm::file_magic::minidump:
  case llvm::file_magic::xcoff_object_32:
  case llvm::file_magic::xcoff_object_64:
#endif
    return FileType::Unsupported;
  }
}
