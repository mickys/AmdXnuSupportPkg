#include <Base.h>

#include <Register/ArchitecturalMsr.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

#include "../../../AmdIntelEmuRuntime.h"

VOID
AmdIntelEmuInternalUdSysexit (
  IN OUT AMD_VMCB_CONTROL_AREA    *Vmcb,
  IN OUT AMD_INTEL_EMU_REGISTERS  *Registers,
  IN     CONST hdes               *Instruction
  )
{
  AMD_VMCB_SAVE_STATE_AREA_NON_ES *SaveState;
  MSR_IA32_SYSENTER_CS_REGISTER   SysenterCs;
  BOOLEAN                         Operand64;
  IA32_SEGMENT_ATTRIBUTES         SegAttrib;

  ASSERT (Vmcb != NULL);
  ASSERT (Registers != NULL);
  ASSERT (Instruction != NULL);

  if (Instruction->p_lock != 0) {
    AmdIntelEmuInternalInjectUd (Vmcb);
    return;
  }

  SaveState = (AMD_VMCB_SAVE_STATE_AREA_NON_ES *)(UINTN)Vmcb->VmcbSaveState;
  ASSERT (SaveState != NULL);

  if (SaveState->CPL != 0) {
    AmdIntelEmuInternalInjectGp (Vmcb, 0);
    return;
  }

  SysenterCs.Uint64 = AsmReadMsr64 (MSR_IA32_SYSENTER_CS);
  if ((SysenterCs.Bits.CS & 0xFFFCU) == 0) {
    AmdIntelEmuInternalInjectGp (Vmcb, 0);
    return;
  }

  Operand64 = (Instruction->rex_w != 0);
  //
  // Operating system provides CS; RPL forced to 3
  //
  if (Operand64) {
    SaveState->RSP         = Registers->Rcx;
    SaveState->RIP         = Registers->Rdx;
    SaveState->CS.Selector = (UINT16)((SysenterCs.Bits.CS + 32) | 3U);
  } else {
    SaveState->RSP         = BitFieldRead64 (Registers->Rcx, 0, 31);
    SaveState->RIP         = BitFieldRead64 (Registers->Rdx, 0, 31);
    SaveState->CS.Selector = (UINT16)((SysenterCs.Bits.CS + 16) | 3U);
  }

  SaveState->CS.Base = 0;
  //
  // With 4-KByte granularity, implies a 4-GByte limit
  //
  SaveState->CS.Limit = 0x0FFFFF;
  SegAttrib.Uint16    = SaveState->CS.Attributes;
  SegAttrib.Bits.Type = 11;
  SegAttrib.Bits.S    = 1;
  SegAttrib.Bits.DPL  = 3;
  SegAttrib.Bits.P    = 1;
  if (Operand64) {
    //
    // 64-bit code segment
    //
    SegAttrib.Bits.L = 1;
    //
    // Required if CS.L = 1
    //
    SegAttrib.Bits.DB = 0;
  } else {
    //
    // return to protected mode or compatibility mode
    //
    SegAttrib.Bits.L = 0;
    //
    // 32-bit code segment
    //
    SegAttrib.Bits.DB = 1;
  }
  //
  // 4-KByte granularity
  //
  SegAttrib.Bits.G         = 1;
  SaveState->CS.Attributes = SegAttrib.Uint16;

  SaveState->CPL = 3;
  //
  // SS just above CS
  //
  SaveState->SS.Selector = (SaveState->CS.Selector + 8);
  //
  // Flat segment
  //
  SaveState->SS.Base = 0;
  //
  // With 4-KByte granularity, implies a 4-GByte limit
  //
  SaveState->SS.Limit = 0x0FFFFF;
  SegAttrib.Uint16    = SaveState->SS.Attributes;
  //
  // Read/write data, accessed
  //
  SegAttrib.Bits.Type = 3;
  SegAttrib.Bits.S    = 1;
  SegAttrib.Bits.DPL  = 3;
  SegAttrib.Bits.P    = 1;
  //
  // 32-bit stack segment
  //
  SegAttrib.Bits.DB = 1;
  //
  // 4-KByte granularity
  //
  SegAttrib.Bits.G         = 1;
  SaveState->SS.Attributes = SegAttrib.Uint16;

  Vmcb->VmcbCleanBits.Bits.SEG = 0;
}
