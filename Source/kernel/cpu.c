#include "kernel.h"
#include <headers/x86.h>

#define CPUID_Check(A,B)										\
	if(!((A) & (B))){											\
		kprintf("%4tCPUID: '%s' not supported!%t\n", #B);		\
		++ecCount;												\
	}

uint32 CPU_init(void){
	uint32 eax, edx;
	uint32 ecCount;
	cpuid(CPUID_GETFEATURES, &eax, &edx);

//Check required CPU features
	ecCount = 0;
	CPUID_Check(edx, CPUID_FEAT_EDX_FXSR);
	CPUID_Check(edx, CPUID_FEAT_EDX_FPU);
	CPUID_Check(edx, CPUID_FEAT_EDX_SSE);
	CPUID_Check(edx, CPUID_FEAT_EDX_SEP);

/*	CPUID_Check(edx, CPUID_FEAT_EDX_PGE);
	CPUID_Check(edx, CPUID_FEAT_EDX_VME);
	CPUID_Check(edx, CPUID_FEAT_EDX_DE);
	CPUID_Check(edx, CPUID_FEAT_EDX_PSE);
	CPUID_Check(edx, CPUID_FEAT_EDX_TSC);
	CPUID_Check(edx, CPUID_FEAT_EDX_MSR);
	CPUID_Check(edx, CPUID_FEAT_EDX_PAE);
	CPUID_Check(edx, CPUID_FEAT_EDX_MCE);
	CPUID_Check(edx, CPUID_FEAT_EDX_CX8);
	CPUID_Check(edx, CPUID_FEAT_EDX_APIC);
	CPUID_Check(edx, CPUID_FEAT_EDX_MTRR);
	CPUID_Check(edx, CPUID_FEAT_EDX_MCA);
	CPUID_Check(edx, CPUID_FEAT_EDX_CMOV);
	CPUID_Check(edx, CPUID_FEAT_EDX_PAT);
	CPUID_Check(edx, CPUID_FEAT_EDX_PSE36);
	CPUID_Check(edx, CPUID_FEAT_EDX_PSN);
	CPUID_Check(edx, CPUID_FEAT_EDX_CLF);
	CPUID_Check(edx, CPUID_FEAT_EDX_DTES);
	CPUID_Check(edx, CPUID_FEAT_EDX_ACPI);
	CPUID_Check(edx, CPUID_FEAT_EDX_MMX);
	CPUID_Check(edx, CPUID_FEAT_EDX_SSE2);
	CPUID_Check(edx, CPUID_FEAT_EDX_SS);
	CPUID_Check(edx, CPUID_FEAT_EDX_HTT);
	CPUID_Check(edx, CPUID_FEAT_EDX_TM1);
	CPUID_Check(edx, CPUID_FEAT_EDX_IA64);
	CPUID_Check(edx, CPUID_FEAT_EDX_PBE);*/

	if(ecCount){
		kprintf("%4tCPUID: Unsupported features count: %d%t\n", ecCount);
		return ERRORCODE_FATAL;
	}

//init the FPU
	asm volatile("fninit;");
//init SSE and FXSAVE
	uint32 tmp;
	tmp = get_cr0();
	tmp &= ~CR0_EMULATION;
	tmp |= CR0_MONITOR_COPROCESSOR;
	ldcr0(tmp);
	ldcr4(get_cr4() | CR4_OSFXSR | CR4_OSXMMEXCPT);

	return SUCCES;
}
