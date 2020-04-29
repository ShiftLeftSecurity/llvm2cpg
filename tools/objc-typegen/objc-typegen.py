import os
import sys
import re
import clang.cindex
from typing import Dict, List, Set


def get_framework_name(location: clang.cindex.SourceLocation, default: str) -> str:
    if default == "NSObject":
        return "Foundation"
    r = re.compile("/([A-Za-z]+).framework/")
    res = r.search(location.file.name)
    if res is not None:
        return res.group(1)
    return default


def cleanup_type(type_name: str) -> str:
    res = type_name.replace(" _Nullable", "")
    res = res.replace(" _Nonnull", "")
    res = re.compile("<.*>").sub("", res)
    return res


def get_type_name(typ: clang.cindex.Type) -> str:
    kind = Traversals.type_kind(typ)
    if kind == clang.cindex.TypeKind.POINTER or kind == clang.cindex.TypeKind.OBJCOBJECTPOINTER:
        return get_type_name(typ.get_pointee()) + "*"
    return typ.spelling


class ObjCParam:
    def __init__(self, type_name: str, name: str):
        self.type_name = cleanup_type(type_name)
        self.name = name

    @staticmethod
    def create(cursor: clang.cindex.Cursor) -> List['ObjCParam']:
        t = Traversals(cursor)
        return t.objc_method_params


class ObjCProperty:
    def __init__(self, type_name: str, name: str):
        self.type_name = cleanup_type(type_name)
        self.name = name

    @staticmethod
    def create(cursor: clang.cindex.Cursor) -> List['ObjCProperty']:
        t = Traversals(cursor)
        return t.objc_properties


class ObjCMethod:
    counter: int = 0

    def __init__(self, is_class: bool, name: str, ret_type: str, params: List[ObjCParam]):
        ObjCMethod.counter += 1
        self.is_class = is_class
        self.name = name
        self.ret_type = cleanup_type(ret_type)
        self.params = [ObjCParam("id", "self"), ObjCParam("SEL", "_cmd")] + params

    @staticmethod
    def create(cursor: clang.cindex.Cursor) -> 'ObjCMethod':
        t = Traversals(cursor)
        return ObjCMethod(t.is_class_method, t.method_name, t.return_type, t.objc_method_params)

    @property
    def name_with_parameters(self) -> str:
        if len(self.params) == 0:
            return self.name

        chunks = list(filter(lambda x: len(x) != 0, self.name.split(":")))
        params = self.params
        if len(params) != len(chunks):
            print(
                "Parameters mismatch: {} <> {}".format(chunks, list(map(lambda x: x.type_name + " " + x.name, params))),
                file=sys.stderr)
            params = params[-len(chunks):]
            print(
                "Truncated actual parameters to {}".format(list(map(lambda x: x.type_name + " " + x.name, params))),
                file=sys.stderr)

        name_chunks = []
        for tup in zip(chunks, params):
            name_chunks.append("{}:({}){}".format(tup[0], tup[1].type_name, tup[1].name))
        return " ".join(name_chunks)

    def dump_objc(self) -> None:
        prefix = "+" if self.is_class else "-"
        output = "{} ({}){};".format(prefix, self.ret_type, self.name_with_parameters)
        print(output)


class ObjCBackingStorage:
    def __init__(self, protocols: List[str], class_methods: List[ObjCMethod],
                 instance_methods: List[ObjCMethod], properties: List[ObjCProperty]):
        ObjCClass.counter += 1
        self.protocols = protocols
        self.class_methods: List[ObjCMethod] = list()
        self.instance_methods: List[ObjCMethod] = list()
        self.properties = properties
        self.instance_methods_set: Set[str] = set()
        self.class_methods_set: Set[str] = set()
        for m in (instance_methods + class_methods):
            self.add_method(m)
        self.__patch_property_getters()

    def __patch_property_getters(self) -> None:
        if len(self.properties) == 0:
            return
        props: Dict[str, str] = dict()
        for prop in self.properties:
            props[prop.name] = prop.type_name
        for method in self.instance_methods:
            if props.get(method.name) is not None:
                method.ret_type = props[method.name]

    @property
    def sorted_instance_methods(self) -> List[ObjCMethod]:
        return sorted(self.instance_methods, key=lambda m: m.name)

    @property
    def sorted_class_methods(self) -> List[ObjCMethod]:
        return sorted(self.class_methods, key=lambda m: m.name)

    @property
    def instance_type_methods(self) -> List[ObjCMethod]:
        def f(m: ObjCMethod): return m.ret_type == "instancetype"

        return list(filter(f, self.class_methods)) + list(filter(f, self.instance_methods))

    def __add_method(self, m: ObjCMethod, names: Set[str], methods: List[ObjCMethod]) -> None:
        if m.name not in names:
            names.add(m.name)
            methods.append(m)

    def add_method(self, m: ObjCMethod) -> None:
        if m.is_class:
            self.__add_method(m, self.class_methods_set, self.class_methods)
        else:
            self.__add_method(m, self.instance_methods_set, self.instance_methods)


class ObjCClass:
    counter: int = 0

    def __init__(self, framework: str, name: str, super_class_name: str, protocols: List[str],
                 class_methods: List[ObjCMethod],
                 instance_methods: List[ObjCMethod], properties: List[ObjCProperty]):
        ObjCClass.counter += 1
        self.framework = framework
        self.name = name
        self.super_class_name = super_class_name
        self.backing = ObjCBackingStorage(protocols, class_methods, instance_methods, properties)

    @staticmethod
    def create(cursor: clang.cindex.Cursor) -> 'ObjCClass':
        t = Traversals(cursor)
        return ObjCClass(get_framework_name(cursor.location, t.class_name), t.class_name, t.super_class_name,
                         t.protocols, t.class_methods, t.instance_methods, t.objc_properties)

    @property
    def is_root_class(self) -> bool:
        return len(self.super_class_name) == 0

    @property
    def has_protocols(self) -> bool:
        return len(self.backing.protocols) != 0

    def add_method(self, m: ObjCMethod) -> None:
        self.backing.add_method(m)

    def dump_objc(self) -> None:
        definition = "@interface " + self.name
        if not self.is_root_class:
            definition += " : " + self.super_class_name
        if self.has_protocols:
            definition += " <" + ", ".join(self.backing.protocols) + ">"
        print(definition)
        for method in self.backing.class_methods + self.backing.instance_methods:
            method.dump_objc()
        print("@end")


class ObjCCategory:
    counter: int = 0

    def __init__(self, framework: str, name: str, class_name: str, protocols: List[str],
                 class_methods: List[ObjCMethod],
                 instance_methods: List[ObjCMethod], properties: List[ObjCProperty]):
        ObjCCategory.counter += 1
        self.framework = framework
        self.name = name
        self.class_name = class_name
        self.backing = ObjCBackingStorage(protocols, class_methods, instance_methods, properties)

    @staticmethod
    def create(cursor: clang.cindex.Cursor) -> 'ObjCCategory':
        t = Traversals(cursor)
        default_name = '{}+{}'.format(t.category_class_name, t.category_name)
        return ObjCCategory(get_framework_name(cursor.location, default_name), t.category_name, t.category_class_name,
                            t.protocols, t.class_methods, t.instance_methods, t.objc_properties)

    @property
    def full_name(self) -> str:
        return "{}({})".format(self.class_name, self.name)

    def dump_objc(self) -> None:
        definition = "@interface {} ({})".format(self.class_name, self.name)
        print(definition)
        for method in self.backing.class_methods + self.backing.instance_methods:
            method.dump_objc()
        print("@end")


class Traversals:
    def __init__(self, cursor: clang.cindex.Cursor):
        self.cursor = cursor

    @staticmethod
    def kind(cursor: clang.cindex.Cursor):
        try:
            return cursor.kind
        except ValueError:
            return clang.cindex.CursorKind.UNEXPOSED_DECL

    @staticmethod
    def type_kind(cursor: clang.cindex.Type):
        try:
            return cursor.kind
        except ValueError:
            return clang.cindex.TypeKind.UNEXPOSED

    @property
    def children(self) -> List[clang.cindex.Cursor]:
        return self.cursor.get_children()

    @property
    def is_objc_method_decl(self) -> bool:
        return (self.kind(self.cursor) == clang.cindex.CursorKind.OBJC_INSTANCE_METHOD_DECL) or \
               (self.kind(self.cursor) == clang.cindex.CursorKind.OBJC_CLASS_METHOD_DECL)

    @property
    def is_objc_interface_decl(self) -> bool:
        return self.kind(self.cursor) == clang.cindex.CursorKind.OBJC_INTERFACE_DECL

    @property
    def is_objc_category_decl(self) -> bool:
        return self.kind(self.cursor) == clang.cindex.CursorKind.OBJC_CATEGORY_DECL

    @property
    def is_class_method(self) -> bool:
        assert self.is_objc_method_decl
        return self.kind(self.cursor) == clang.cindex.CursorKind.OBJC_CLASS_METHOD_DECL

    @property
    def objc_method_params(self) -> List[ObjCParam]:
        assert self.is_objc_method_decl
        params: List[ObjCParam] = list()
        for child in self.children:
            if self.kind(child) == clang.cindex.CursorKind.PARM_DECL:
                params.append(ObjCParam(get_type_name(child.type), child.spelling))
        return params

    @property
    def objc_properties(self) -> List[ObjCProperty]:
        assert self.is_objc_interface_decl or self.is_objc_category_decl
        properties: List[ObjCProperty] = list()
        for child in self.children:
            if self.kind(child) == clang.cindex.CursorKind.OBJC_PROPERTY_DECL:
                properties.append(ObjCProperty(get_type_name(child.type), child.spelling))
        return properties

    @property
    def return_type(self) -> str:
        assert self.is_objc_method_decl
        for child in self.children:
            if self.kind(child) == clang.cindex.CursorKind.TYPE_REF:
                return get_type_name(child.type)
        return "void"

    @property
    def class_name(self) -> str:
        assert self.is_objc_interface_decl
        return self.cursor.spelling

    @property
    def super_class_name(self) -> str:
        assert self.is_objc_interface_decl
        for child in self.children:
            if self.kind(child) == clang.cindex.CursorKind.OBJC_SUPER_CLASS_REF:
                return child.spelling
        return ""

    @property
    def category_name(self) -> str:
        assert self.is_objc_category_decl
        return self.cursor.spelling

    @property
    def category_class_name(self) -> str:
        assert self.is_objc_category_decl
        for child in self.children:
            if self.kind(child) == clang.cindex.CursorKind.OBJC_CLASS_REF:
                return child.spelling
        return ""

    @property
    def method_name(self) -> str:
        assert self.is_objc_method_decl
        return self.cursor.spelling

    @property
    def protocols(self) -> List[str]:
        assert self.is_objc_interface_decl or self.is_objc_category_decl
        protocols: List[str] = list()
        for child in self.children:
            if self.kind(child) == clang.cindex.CursorKind.OBJC_PROTOCOL_REF:
                protocols.append(child.spelling)
        return protocols

    def __objc_methods__(self, cursor_kind: clang.cindex.CursorKind) -> List[ObjCMethod]:
        assert self.is_objc_interface_decl or self.is_objc_category_decl
        methods: List[ObjCMethod] = list()
        for child in self.children:
            if self.kind(child) == cursor_kind:
                methods.append(ObjCMethod.create(child))
        return methods

    @property
    def class_methods(self) -> List[ObjCMethod]:
        assert self.is_objc_interface_decl or self.is_objc_category_decl
        return self.__objc_methods__(clang.cindex.CursorKind.OBJC_CLASS_METHOD_DECL)

    @property
    def instance_methods(self) -> List[ObjCMethod]:
        assert self.is_objc_interface_decl or self.is_objc_category_decl
        return self.__objc_methods__(clang.cindex.CursorKind.OBJC_INSTANCE_METHOD_DECL)


class PolicyFormatter:
    @staticmethod
    def __comment(contents: str) -> str:
        return '# {}\n'.format(contents)

    @staticmethod
    def __inherit(class_name: str, super_class_name: str) -> str:
        return 'INHERIT TYPEDECL -f "{}" TYPE -f "{}"\n'.format(class_name, super_class_name)

    @staticmethod
    def __realize(type_name: str) -> str:
        return 'REALIZE TYPEDECL -f "{}" TYPE -f "{}"\n'.format(type_name, type_name)

    @staticmethod
    def __bind(name: str, class_name: str, full_name: str) -> str:
        return 'BIND -n "{}" -s "" METHOD -f "{}" TYPEDECL -f "{}"\n'.format(name, full_name, class_name)

    @staticmethod
    def __typeassert_ret(full_name: str, return_type: str, location: str) -> str:
        ret_type = location if return_type == "instancetype" else return_type
        return 'TYPEASSERT METHOD -f "{}" RET SUBTYPE TYPE -f "{}"\n'.format(full_name, ret_type)

    @staticmethod
    def __typeassert_par(full_name: str, index: int, type_name: str) -> str:
        return 'TYPEASSERT METHOD -f "{}" PAR -i {} SUBTYPE TYPE -f "{}"\n'.format(full_name, index, type_name)

    @staticmethod
    def __method_ast_edge(name: str, location: str) -> str:
        return '#EDGE BOTH AST -s METHOD -f "{}" -d TYPEDECL -f "{}"\n'.format(name, location)

    def format_class(self, cls: ObjCClass) -> str:
        comment = self.__comment(cls.name)
        inherit = self.__comment("Root Class")
        if not cls.is_root_class:
            inherit = self.__inherit(cls.name, cls.super_class_name)
            inherit += self.__inherit(cls.name + "*", cls.super_class_name + "*")
            inherit += self.__inherit(cls.name + '$', cls.super_class_name + '$')
            inherit += self.__inherit(cls.name + '$*', cls.super_class_name + '$*')
        return comment + inherit

    def format_category(self, cat: ObjCCategory) -> str:
        comment = self.__comment(cat.full_name)
        return comment

    def format_method(self, method: ObjCMethod, class_name: str, location: str) -> str:
        prefix = "+" if method.is_class else "-"
        full_name = '{}[{} {}]'.format(prefix, location, method.name)
        comment = self.__comment(full_name)
        binding = self.__bind(method.name, class_name, full_name)
        binding += self.__bind(method.name, class_name + "*", full_name)
        ret_type_class = class_name[0:-1] if class_name.endswith("$") else class_name
        ret_type = self.__typeassert_ret(full_name, method.ret_type, ret_type_class)
        method_ast = self.__method_ast_edge(full_name, location)
        params = []
        for index, param in enumerate(method.params, start=1):
            par = self.__typeassert_par(full_name, index, param.type_name)
            params.append(par)
        return comment + binding + method_ast + ret_type + ''.join(params)


class TypeHierarchy:
    def __init__(self, sdk_path: str, toolchain_path: str):
        self.sdk_path: str = sdk_path
        self.toolchain_path: str = toolchain_path
        self.class_storage: Dict[str, ObjCClass] = dict()
        self.category_storage: Dict[str, ObjCCategory] = dict()
        self.index = clang.cindex.Index.create()
        self.class_framework_storage: Dict[str, List[ObjCClass]] = dict()
        self.category_framework_storage: Dict[str, List[ObjCCategory]] = dict()
        self.subclasses_mapping: Dict[str, List[str]] = dict()

    def classes(self, framework) -> List[ObjCClass]:
        return sorted(self.class_framework_storage.get(framework, list()), key=lambda c: c.name)

    def categories(self, framework) -> List[ObjCCategory]:
        return sorted(self.category_framework_storage.get(framework, list()), key=lambda c: c.full_name)

    def get_headers(self, framework: str):
        headers_path = self.sdk_path + "/System/Library/Frameworks/" + framework + ".framework/Headers"
        if not os.path.exists(headers_path):
            print("Skipping {}\n{} does not exist".format(framework, headers_path))
            return list()
        return map(lambda h: headers_path + "/" + h, filter(lambda f: f.endswith(".h"), os.listdir(headers_path)))

    def process_framework(self, framework_name: str) -> None:
        print("Processing {}.framework".format(framework_name))
        args = ["-x", "objective-c-header", "--sysroot=" + self.sdk_path, "-I/usr/local/include",
                "-I" + self.toolchain_path + "/usr/lib/clang/11.0.0/include",
                "-I" + self.toolchain_path + "/usr/include",
                "-I" + self.sdk_path + "/usr/include"]
        for header in self.get_headers(framework_name):
            tu = self.index.parse(header, args)
            self.add_translation_unit(tu)

    def add_translation_unit(self, unit: clang.cindex.TranslationUnit) -> None:
        for child in unit.cursor.get_children():
            cursor_kind = Traversals.kind(child)
            if cursor_kind == clang.cindex.CursorKind.OBJC_INTERFACE_DECL:
                self.add_class(child)
            elif cursor_kind == clang.cindex.CursorKind.OBJC_CATEGORY_DECL:
                self.add_category(child)

    def add_class(self, cursor: clang.cindex.Cursor) -> None:
        name = cursor.spelling
        if self.class_storage.get(name) is None:
            cls = ObjCClass.create(cursor)
            self.class_storage[name] = cls
            classes = self.class_framework_storage.get(cls.framework, list())
            classes.append(cls)
            self.class_framework_storage[cls.framework] = classes
            if len(cls.super_class_name) != 0:
                subclasses = self.subclasses_mapping.get(cls.super_class_name, list())
                subclasses.append(cls.name)
                self.subclasses_mapping[cls.super_class_name] = subclasses

    def add_category(self, cursor: clang.cindex.Cursor) -> None:
        t = Traversals(cursor)
        category_name = t.category_name
        class_name = t.category_class_name
        full_name = "{}+{}".format(class_name, category_name)
        if self.category_storage.get(full_name) is None:
            cat = ObjCCategory.create(cursor)
            self.category_storage[full_name] = cat
            categories = self.category_framework_storage.get(cat.framework, list())
            categories.append(cat)
            self.category_framework_storage[cat.framework] = categories

    def __propagate_instancetype_methods(self, name: str, subclass_name: str):
        superclass = self.class_storage[name]
        subclass = self.class_storage[subclass_name]
        for method in superclass.backing.instance_type_methods:
            subclass.add_method(method)

    def propagate_instancetype_methods(self):
        worklist: List[str] = list()
        for super_class_name in self.subclasses_mapping.keys():
            if len(self.class_storage[super_class_name].super_class_name) == 0:
                worklist.append(super_class_name)
        while len(worklist) != 0:
            name = worklist.pop(0)
            for subclass in self.subclasses_mapping.get(name, list()):
                self.__propagate_instancetype_methods(name, subclass)
                worklist.append(subclass)

    def dump_policy(self, framework: str, sdk: str = "./") -> None:
        print("Writing {}.policy".format(framework))
        path = sdk + "/" + framework
        if not os.path.exists(path):
            os.makedirs(path)
        policy: PolicyFormatter = PolicyFormatter()
        for cls in self.classes(framework):
            with open(path + "/" + cls.name + ".policy", 'w') as f:
                print(policy.format_class(cls), file=f)

                for m in cls.backing.sorted_class_methods:
                    print(policy.format_method(m, cls.name + '$', cls.name), file=f)
                for m in cls.backing.sorted_instance_methods:
                    print(policy.format_method(m, cls.name, cls.name), file=f)

        for cat in self.categories(framework):
            with open(path + "/" + cat.class_name + ".policy", 'a') as f:
                print(policy.format_category(cat), file=f)
                for m in cat.backing.sorted_class_methods:
                    print(policy.format_method(m, cat.class_name + '$', cat.full_name), file=f)
                for m in cat.backing.sorted_instance_methods:
                    print(policy.format_method(m, cat.class_name, cat.full_name), file=f)

    def dump_policies(self, sdk: str = "./") -> None:
        self.propagate_instancetype_methods()
        if not os.path.exists(sdk):
            os.makedirs(sdk)
        for framework in self.class_framework_storage.keys():
            self.dump_policy(framework, sdk)

    def dump_objc(self):
        for cls in self.class_storage.values():
            cls.dump_objc()
        for cat in self.category_storage.values():
            cat.dump_objc()


def print_recursively(cursor: clang.cindex.Cursor, level: int = 0):
    padding = " " * (level * 2)
    print("%s%s (%s) -> %s" % (padding, Traversals.kind(cursor), cursor.type.spelling, cursor.spelling))
    for child in cursor.get_children():
        print_recursively(child, level + 1)


clang.cindex.Config.set_library_path(os.environ.get("LIBCLANG_PATH"))


def process(sdk_name: str, sdk_path: str, toolchain: str, frameworks: List[str]) -> None:
    hierarchy = TypeHierarchy(sdk_path, toolchain)
    for framework in frameworks:
        hierarchy.process_framework(framework)
    hierarchy.dump_policies(sdk_name)

    print("\n\nStats:\nclasses: {}\ncategories: {}\nmethods: {}".format(ObjCClass.counter, ObjCCategory.counter,
                                                                        ObjCMethod.counter))


def get_frameworks(sdk: str) -> List[str]:
    frameworks_path = sdk + "/System/Library/Frameworks"
    return list(map(lambda f: f.replace(".framework", ""),
                    filter(lambda f: f.endswith(".framework"), os.listdir(frameworks_path))))


def get_sdk_path(sdk: str) -> str:
    return "/Applications/Xcode.app/Contents/Developer/Platforms/{}.platform/Developer/SDKs/{}.sdk".format(sdk, sdk)


defaultToolchain = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain"
SDKs: List[str] = ["MacOSX", "iPhoneOS", "iPhoneSimulator", "WatchOS", "WatchSimulator", "AppleTVOS",
                   "AppleTVSimulator"]


def run_all():
    for sdk in SDKs:
        sdk_path = get_sdk_path(sdk)
        if not os.path.exists(sdk_path):
            print("SDK path doesn't exist: {}".format(sdk_path))
            exit(42)

    for sdk in SDKs:
        print("Processing {}.sdk".format(sdk))
        sdk_path = get_sdk_path(sdk)
        process(sdk, sdk_path, defaultToolchain, get_frameworks(sdk_path))


current = "MacOSX"
sdk_path = get_sdk_path(current)
frameworks = ["CoreData"]
# frameworks = get_frameworks(sdk_path)
process(current, sdk_path, defaultToolchain, frameworks)
# run_all()
