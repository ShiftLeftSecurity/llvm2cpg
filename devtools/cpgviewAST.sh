#!/bin/sh
SCRIPT_ABS_PATH=$(readlink -f "$0")
SCRIPT_ABS_DIR=$(dirname $SCRIPT_ABS_PATH)

zcat $@ | $SCRIPT_ABS_DIR/cpgview.py --AST dot -Tpng | feh -
