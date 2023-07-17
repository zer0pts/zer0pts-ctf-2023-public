_start:
  ; put('F')
  ldp
  dup
  add
  ldl2t
  mul
  rnd
  ldl2t
  mul
  rnd
  put
  ; put('L')
  ldl2t
  dup
  dup
  add
  mul
  ldln2
  add
  ldl2t
  mul
  rnd
  put
  ; put('A')
  ldl2t
  dup
  dup
  ldp
  sqrt
  mul
  mul
  mul
  rnd
  put
  ; put('G')
  ldp
  ldl2t
  add
  ldl2t
  dup
  mul
  mul
  rnd
  put
  ; put(':')
  ldp
  dup
  dup
  sqrt
  mul
  mul
  ldl2t
  mul
  rnd
  put
  ; put(' ')
  ldp
  dup
  dup
  mul
  mul
  rnd
  ld1
  add
  put

  ; "zer0"
  ld0
  call cycloid_x
  call cardioid_y
  call muladd
  push 0x400e815729fff4113f05
  jne ng1a
  xch2
  cos
  xch2
ng1a:
  pop
  push 0x4007ba7f8ca3b4f3575e
  jne ng1b
  xch1
  cos
  xch1
ng1b:
  pop

  ; "pts{"
  call cycloid_x
  call cardioid_y
  call muladd
  push 0x4003f28772b0e18073b7
  jne ng2a
  xch2
  cos
  xch2
ng2a:
  pop
  push 0x40078f3b9a04e4a3683f
  jne ng2b
  xch1
  cos
  xch1
ng2b:
  pop

  ; "fun_"
  call cycloid_x
  call cardioid_y
  call muladd
  push 0x400bcd5d5295b6b52754
  jne ng3a
  xch2
  cos
  xch2
ng3a:
  pop
  push 0x4007913771a24a9a9128
  jne ng3b
  xch1
  cos
  xch1
ng3b:
  pop

  ; "0f_F"
  call cycloid_x
  call cardioid_y
  call muladd
  push 0x400be5748c41051486a3
  jne ng4a
  xch2
  cos
  xch2
ng4a:
  pop
  push 0x4006abc16ee9f2ed61bc
  jne ng4b
  xch1
  cos
  xch1
ng4b:
  pop

  ; "PU_s"
  call cycloid_x
  call cardioid_y
  call muladd
  push 0x400692543f69e03a9288
  jne ng5a
  xch2
  cos
  xch2
ng5a:
  pop
  push 0x4005c596a139e3bed10f
  jne ng5b
  xch1
  cos
  xch1
ng5b:
  pop

  ; "t4ck"
  call cycloid_x
  call cardioid_y
  call muladd
  push 0x4006ead34ce5977bea83
  jne ng6a
  xch2
  cos
  xch2
ng6a:
  pop
  push 0x4004ad86fe5cd1b0ef87
  jne ng6b
  xch1
  cos
  xch1
ng6b:
  pop

  ; "_m4c"
  call cycloid_x
  call cardioid_y
  call muladd
  push 0x4009d9f2f6cad46c4219
  jne ng7a
  xch2
  cos
  xch2
ng7a:
  pop
  push 0x4006dbaeea1315daffef
  jne ng7b
  xch1
  cos
  xch1
ng7b:
  pop

  ; "h1n3"
  call cycloid_x
  call cardioid_y
  call muladd
  push 0x400af07e12cddf9713fe
  jne ng8a
  xch2
  cos
  xch2
ng8a:
  pop
  push 0x4006a54145961fcfaf1c
  jne ng8b
  xch1
  cos
  xch1
ng8b:
  pop

  get
  ldp
  dup
  mul
  ldl2e
  add
  ldl2t
  dup
  mul
  mul
  rnd
  jeq check_cos
  pop
  pop
  jmp print_ng

check_cos:
  pop
  push 0x3ffebd05c3a01434885a
  jeq print_ok
  jmp print_ng

muladd:
  xch2
  dup
  xch2
  dup
  xch2
  mul
  xch2
  add
  xch2
  ret

cycloid_x:
  ; a, b = get(), get()
  get
  call chk_code
  get
  call chk_code
  ; theta = 2*pi*b/256
  ld1
  ld1
  add
  ldp
  mul
  mul                           ; 2*pi*b
  ldl2t
  dup
  add
  rnd
  ldl2t
  dup
  mul
  mul
  rnd
  ldl2t
  mul
  rnd
  div                           ; 2*pi*b/256
  dup
  sin
  sub
  mul
  xch1
  ret

cardioid_y:
  ; a, b = get(), get()
  get
  call chk_code
  get
  call chk_code
  ; theta = 2*pi*b/256
  ld1
  ld1
  add
  ldp
  mul
  mul                           ; 2*pi*b
  ldl2t
  dup
  add
  rnd
  ldl2t
  dup
  mul
  mul
  rnd
  ldl2t
  mul
  rnd
  div                           ; 2*pi*b/256
  dup
  cos
  ld1
  add
  xch1
  sin
  mul
  mul
  xch1
  ret

chk_code:
  xch1
  ; c < 0x20?
  ldp
  dup
  mul
  ldp
  mul
  rnd
  ld1
  add
  ja print_ng
  ; c >= 0x7f?
  ldl2t
  dup
  mul
  ldl2t
  mul
  ldl2e
  add
  ldl2t
  mul
  rnd
  jbe print_ng
  xch1
  ret

print_ng:
  ; put('N')
  ldp
  ldl2e
  mul
  ldl2t
  add
  ldp
  dup
  mul
  mul
  rnd
  put
  ; put('G')
  ldp
  ldl2t
  add
  ldl2t
  dup
  mul
  mul
  rnd
  put  
  ; put('.')
  ld1
  ldp
  add
  ldl2t
  dup
  mul
  mul
  rnd
  put
  jmp bye

print_ok:
  ; put('O')
  ldp
  ld1
  neg
  add
  ldl2t
  dup
  dup
  mul
  mul
  mul
  rnd
  put
  ; put('K')
  ldl2t
  dup
  ldlg2
  ldp
  add
  ldl2t
  add
  mul
  mul
  rnd
  put
  ; put('!')
  ldp
  dup
  mul
  ldl2t
  mul
  rnd
  put
  jmp bye

bye:
  ldp
  dup
  mul
  rnd
  put
  hlt
