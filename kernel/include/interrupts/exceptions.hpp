#pragma once

struct interrupt_frame;
__attribute__((interrupt)) void exceptionZeroDivide(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionDebug(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionNonMaskableInterrupt(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionBreakpoint(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionOverflow(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionBoundRangeExceeded(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionInvalidOpcode(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionDeviceUnavl(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionDoubleFault(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionInvalidTTS(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionSegmentNotPresent(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionSSFault(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionGPFault(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionPageFault(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionx87FloatingPoint(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionAlignmentCheck(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionMachineCheck(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionSIMDFloatingPoint(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionVirtuallization(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionControlProtection(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionHypervisorInjection(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionVMMCom(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionSecurity(interrupt_frame *frame);
__attribute__((interrupt)) void exceptionTripleFault(interrupt_frame *frame);