int nest_alias(int **x, int **y) {
  int res = x[2][3];
  y[17] = x[1];
  return res; // we can inline either tmp=x[2]; y[17]=x[1]; return tmp[3] or leave the code as is.
              // We cannot inline all of it.
}

int nest_noalias(int **restrict x, int **restrict y) {
  int res = x[2][3];
  y[17] = x[1];
  return res; // this can be fully inlined
}
