define float @negate(float %x) {
  %n = fneg float %x
  ret float %n
}