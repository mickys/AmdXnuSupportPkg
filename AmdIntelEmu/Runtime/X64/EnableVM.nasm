%define AMD_VMCB_EXITCODE_OFFSET  (0x70)
%define AMD_VMCB_RIP_OFFSET       (0x400 + 0x0178)
%define AMD_VMCB_RSP_OFFSET       (0x400 + 0x01D8)

#include "AmdIntelEmu.h"

extern ASM_PFX (AmdIntelEmuInternalInterceptionHandler)

    DEFAULT REL
    SECTION .text

global ASM_PFX (AmdIntelEmuInternalVmrun)
ASM_PFX (AmdIntelEmuInternalVmrun):
  ;
  ; Save the caller stack to return transparently later.
  ;
  pop     rax
  mov     [rcx + AMD_VMCB_RIP_OFFSET], rax
  mov     [rcx + AMD_VMCB_RSP_OFFSET], rsp
  mov     rsp, rdx     ; Set up the new host stack.
  ;
  ; Software must load RAX (EAX in 32-bit mode) with the physical address of
  ; the VMCB, a 4-Kbyte-aligned page that describes a virtual machine to be
  ; executed.
  ;
  mov     rax, rcx

.StartGuest:
  vmrun
  ;
  ; The INIT procedure sets EFER to 0, which means EFER.SVME zero'd.
  ; As this causes undefined behavior during guest runtime, all INITs are
  ; intercepted and taken on host level.  STGI is called to release the
  ; pending INIT.  This sequence is machine code to minimize delays.
  ;
  cmp     qword [rax + AMD_VMCB_EXITCODE_OFFSET], VMEXIT_INIT
  jnz     .InterceptionHandler
  stgi
.HltLoop:
  cli
  hlt
  jmp     .HltLoop
.InterceptionHandler:
  ;
  ; Interception handling code.
  ;
  ; rsp and rax are saved in VMCB.  rbp, rsi, rdi, r12 to r15 and xmm6 to xmm15
  ; are non-volatile (EFIAPI).
  ; C code assumes the "rbx, rcx, rdx" layout.
  ;
  push    r11
  push    r10
  push    r9
  push    r8
  push    rdx
  push    rcx
  push    rbx          ; non-volatile for EFIAPI, but C code may access it.
  mov     rdx, rsp     ; Pass the registers' addresses.
  sub     rsp, 0x88    ; shadow area (4 * 8 bytes) and XMM registers (aligned).
  movaps  [rsp + 0x20], xmm0
  movaps  [rsp + 0x30], xmm1
  movaps  [rsp + 0x40], xmm2
  movaps  [rsp + 0x50], xmm3
  movaps  [rsp + 0x60], xmm4
  movaps  [rsp + 0x70], xmm5

  mov     rcx, rax     ; Pass the VMCB.
  call    ASM_PFX (AmdIntelEmuInternalInterceptionHandler)

  movaps  xmm5, [rsp + 0x70]
  movaps  xmm4, [rsp + 0x60]
  movaps  xmm3, [rsp + 0x50]
  movaps  xmm2, [rsp + 0x40]
  movaps  xmm1, [rsp + 0x30]
  movaps  xmm0, [rsp + 0x20]
  add     rsp, 0x88
  pop     rbx
  pop     rcx
  pop     rdx
  pop     r8
  pop     r9
  pop     r10
  pop     r11

  jmp     .StartGuest  ; Unconditionally resume the guest.
