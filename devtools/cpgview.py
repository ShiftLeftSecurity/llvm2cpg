#!/bin/python3
import cpg_pb2 as cpg
import sys
import html
import argparse

#regenerate `cpg_pb2` by `protoc -I .. --python_out=. ../cpg.proto`
#TODO: Add this step to the build process. Afterwards, remove cpg_pb2 from the repo

valmap = {'string_value': 'i8*',
            'bool_value' : 'i1',
            'int_value' : 'i32',
            'long_value' : 'i64',
            'float_value' : 'f32',
            'double_value' : 'f64',
            'string_list' : 'i8**',
            'bool_list' : 'i1*',
            'int_list' : 'i32*',
            'long_list' : 'i64*',
            'float_list' : 'f32*',
            'double_list' : 'f64*'
            }


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--CFG', dest='CFG', action='store_true')
    parser.add_argument('--AST', dest='AST', action='store_true')

    args = parser.parse_args()

    msg = cpg.CpgOverlay()
    msg.ParseFromString(sys.stdin.buffer.read())

    cfg_preds = {}
    cfg_succs = {}
    for e in msg.edge:
        tn = cpg.CpgStruct.Edge.EdgeType.Name(e.type)
        if tn == 'CFG':
            if e.src in cfg_succs.keys():
                cfg_succs[e.src].append(e.dst)
            else:
                cfg_succs[e.src] = [e.dst]
            if e.dst in cfg_preds.keys():
                cfg_preds[e.dst].append(e.src)
            else:
                cfg_preds[e.dst] = [e.src]

    res = ['''digraph g {
graph [
rankdir = "LR"
];
node [
fontsize = "16"
shape = "plain"
];\n''']
    print("#CFG: ", args.CFG)
    for n in msg.node:
        typ = cpg.CpgStruct.Node.NodeType.Name(n.type)
        properties = []
        for p in n.property:
            pn = cpg.NodePropertyName.Name(p.name)
            vt = p.value.WhichOneof('value') # WTFLOL google API design? 
            val = getattr(p.value, vt)  
            properties.append("<TR><TD>%s %s</TD><TD>%s</TD></TR>" %(pn, valmap[vt], html.escape("%s" % (val,))))
        label = """<<TABLE><TR><TD colspan="2">%s %s</TD></TR><TR><TD colspan="2">preds: %s, succs: %s</TD></TR>%s</TABLE>>""" \
            % (typ, n.key, cfg_preds.get(n.key, []), cfg_succs.get(n.key, []), "\n".join(properties))
        res.append(''' "node%s" [shape="plain" label=%s]\n''' % (n.key, label))

    res.append('\n# AST edges \nedge [penwidth = 3, constraint = %s];' %('false' if args.AST else 'true'))
    for e in msg.edge:
        tn = cpg.CpgStruct.Edge.EdgeType.Name(e.type)
        if tn == 'AST':
            res.append("node%s -> node%s;" %(e.src, e.dst))

    res.append('\n# CFG edges \nedge [constraint = %s, penwidth = 3, color="red"];' %('false' if not args.CFG else 'true'))
    for e in msg.edge:
        tn = cpg.CpgStruct.Edge.EdgeType.Name(e.type)

        if tn == 'CFG':
            res.append("node%s -> node%s;" %(e.src, e.dst))

    res.append('\n# other edges \n edge [constraint = false, penwidth = 3, color="green"];')
    for e in msg.edge:
        tn = cpg.CpgStruct.Edge.EdgeType.Name(e.type)
        if tn != 'CFG' and tn != 'AST':
            res.append("node%s -> node%s [label=%s];" %(e.src, e.dst, tn))    

    res.append('}')

    print('\n'.join(res))
        
