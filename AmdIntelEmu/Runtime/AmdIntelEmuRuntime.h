#ifndef AMD_INTEL_EMU_RUNTIME_H_
#define AMD_INTEL_EMU_RUNTIME_H_

#include "../AmdIntelEmu.h"
#include "hde/hde.h"

VOID
AmdIntelEmuInternalSingleStepRip (
  IN OUT AMD_INTEL_EMU_THREAD_CONTEXT      *ThreadContext,
  IN     AMD_INTEL_EMU_SINGLE_STEP_RESUME  ResumeHandler,
  IN     UINTN                             ResumeContext
  );

UINT64
AmdIntelEmuInternalReadMsrValue64 (
  IN CONST UINT64                   *Rax,
  IN CONST AMD_INTEL_EMU_REGISTERS  *Registers
  );

VOID
AmdIntelEmuInternalWriteMsrValue64 (
  IN OUT UINT64                   *Rax,
  IN OUT AMD_INTEL_EMU_REGISTERS  *Registers,
  IN     UINT64                   Value
  );

VOID
AmdIntelEmuInternalGetRipInstruction (
  IN  CONST AMD_VMCB_SAVE_STATE_AREA_NON_ES  *SaveState,
  OUT hdes                                   *Instruction
  );

VOID
AmdIntelEmuInternalInjectUd (
  IN OUT AMD_VMCB_CONTROL_AREA  *Vmcb
  );

VOID
AmdIntelEmuInternalInjectDf (
  IN OUT AMD_VMCB_CONTROL_AREA  *Vmcb
  );

VOID
AmdIntelEmuInternalInjectGp (
  IN OUT AMD_VMCB_CONTROL_AREA  *Vmcb,
  IN     UINT8                  ErrorCode
  );

AMD_INTEL_EMU_THREAD_CONTEXT *
AmdIntelEmuInternalGetThreadContext (
  IN CONST AMD_VMCB_CONTROL_AREA  *Vmcb
  );

VOID
AmdIntelEmuInternalQueueEvent (
  IN OUT AMD_VMCB_CONTROL_AREA  *Vmcb,
  IN     CONST AMD_VMCB_EVENT   *Event
  );

VOID
AmdIntelEmuInternalMmioLapicSetPage (
  IN VOID  *Page
  );

VOID
AmdIntelEmuInternalInitializeMmioInfo (
  IN OUT AMD_INTEL_EMU_MMIO_INFO  *MmioInfo
  );

extern BOOLEAN mAmdIntelEmuInternalNrip;
extern BOOLEAN mAmdIntelEmuInternalNp;

extern CONST AMD_INTEL_EMU_MSR_INTERCEPT_INFO mAmdIntelEmuInternalMsrIntercepts[];
extern CONST UINTN                            mAmdIntelEmuInternalNumMsrIntercepts;

extern UINT64 *mAmdIntelEmuInternalLapicAddress;

#endif // AMD_INTEL_EMU_RUNTIME_H_
