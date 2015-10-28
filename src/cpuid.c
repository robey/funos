#include <stdbool.h>
#include "buffer.h"
#include "cpuid.h"

static inline void cpuid(int code, uint32_t regs[4]) {
  asm volatile("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3]) : "a" (code));
}

#define _copy(n, x) do { \
  info->id[n] = regs[x] & 0xff; \
  info->id[n + 1] = regs[x] >> 8; \
  info->id[n + 2] = regs[x] >> 16; \
  info->id[n + 3] = regs[x] >> 24; \
} while (0)

void cpuid_get(cpu_info *info) {
  uint32_t regs[4];

  cpuid(0, regs);
  _copy(0, 1);
  _copy(4, 3);
  _copy(8, 2);
  info->highest_feature = regs[0];

  cpuid(1, regs);
  info->feature_edx = regs[3];
  info->feature_ecx = regs[2];

  cpuid(0x80000000, regs);
  info->highest_extended_feature = regs[0];
  if (info->highest_extended_feature >= 1) {
    cpuid(0x80000001, regs);
    info->extended_feature_edx = regs[3];
    info->extended_feature_ecx = regs[2];
  } else {
    info->extended_feature_edx = 0;
    info->extended_feature_ecx = 0;
  }
}

#define add3(s) do { if (!buffer_append(buffer, s, 3)) return false; } while (0)
#define add4(s) do { if (!buffer_append(buffer, s, 4)) return false; } while (0)
#define add5(s) do { if (!buffer_append(buffer, s, 5)) return false; } while (0)
bool cpuid_explain(cpu_info *info, buffer_t *buffer) {
  if (!buffer_append(buffer, info->id, 12)) return false;

  // add "interesting" flags, as defined by me.
  if (info->feature_edx & CPUID_FEAT_EDX_FPU) add4(" fpu");
  if (info->feature_edx & CPUID_FEAT_EDX_PSE) add4(" pse");
  if (info->feature_edx & CPUID_FEAT_EDX_TSC) add4(" tsc");
  if (info->feature_edx & CPUID_FEAT_EDX_MSR) add4(" msr");
  if (info->feature_edx & CPUID_FEAT_EDX_PAE) add4(" pae");
  if (info->feature_edx & CPUID_FEAT_EDX_CX8) add4(" cx8");
  if (info->feature_edx & CPUID_FEAT_EDX_APIC) add5(" apic");
  if (info->feature_edx & CPUID_FEAT_EDX_CMOV) add5(" cmov");
  if (info->feature_edx & CPUID_FEAT_ECX_CX16) add5(" cx16");
  if (info->feature_edx & CPUID_FEAT_ECX_AES) add4(" aes");
  if (info->feature_edx & CPUID_FEAT_ECX_HYPERVISOR) add5(" FAKE");
  if (info->extended_feature_edx & CPUID_EXT_FEAT_EDX_NX) add3(" nx");
  if (info->extended_feature_edx & CPUID_EXT_FEAT_EDX_LM) add5(" long");

  return true;
}
