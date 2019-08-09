#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"

#define ARR0_SIZE 4000
#define ARR1_SIZE 2000000
#define ARR2_SIZE 32000
#define ARR3_SIZE 400000000

///////////////////////
//// CPUID leaves ////
const int ID_Leaf = 0x00;
const int Perf_Leaf = 0x0A;

///////////////////////////////
//// Register Addresses ////
const int IA32_PMCx = 0x00C1;
const int IA32_PERFEVTSELx = 0x0186;

//const int IA32_FIXED_CTR_CTRL = 0x0309;
//const int IA32_FIXED_CTR_CTRL = 0x038D;

//const int IA32_PERF_GLOBAL_STATUS= 0x038E;
const int IA32_PERF_GLOBAL_CTRL = 0x038F;
//const int IA32_PERF_GLOBAL_OVF_CTRL = 0x0390;

//////////////////////
//// Event codes ////
// Instructions_Retired
const int ES_Instructions_Retired = 0xC0;
const int UM_Instructions_Retired = 0x00;
// Unhalted Core Cycles
const int ES_Unhalted_Core_Cycles = 0x3C;
const int UM_Unhalted_Core_Cycles = 0x00;
// MEM_LOAD_UOPS_RETIRED_L1_HIT
const int ES_MEM_LOAD_UOPS_RETIRED_L1_HIT = 0xD1;
const int UM_MEM_LOAD_UOPS_RETIRED_L1_HIT = 0x01;
// MEM_LOAD_UOPS_RETIRED_L2_HIT
const int ES_MEM_LOAD_UOPS_RETIRED_L2_HIT = 0xD1;
const int UM_MEM_LOAD_UOPS_RETIRED_L2_HIT = 0x02;
// MEM_LOAD_UOPS_RETIRED_L3_HIT
const int ES_MEM_LOAD_UOPS_RETIRED_L3_HIT = 0xD1;
const int UM_MEM_LOAD_UOPS_RETIRED_L3_HIT = 0x04;
// MEM_LOAD_UOPS_RETIRED_L3_MISS
const int ES_MEM_LOAD_UOPS_RETIRED_L3_MISS = 0xD1;
const int UM_MEM_LOAD_UOPS_RETIRED_L3_MISS = 0x20;
// 

static int Cache_Flag = 1;

static int CacheTest_Hello(void)
{
	grub_printf("Hello from CacheTest!\n");
	return 0;
}

struct CacheTest_GPRs
{
	int eax;
	int ebx;
	int ecx;
	int edx;
};

/////////////// Application workloads /////////////////
static int array0[ARR0_SIZE] __attribute__((aligned(0x0100)));  	//16 KB array
static int array1[ARR3_SIZE] __attribute__((aligned(0x0100)));  	//4 MB array

static void InitArray0(void)
{
	int i = 0;

	for (i = 0; i < ARR0_SIZE; i++)
	{
		array0[i] = i;
	}
}

static void InitArray01(void)
{
	int i = 0;

	for (i = 0; i < ARR0_SIZE; i++)
	{
		array0[i] = (int)array0 + (i + 1)*4;
	}
}

static void InitArray1(void)
{
	int i = 0;

	for (i = 0; i < ARR1_SIZE; i++)
	{
		array1[i] = (int)array1 + (i + 1)*4;
	}
}

static void InitArray2(void)
{
	int i = 0;

	for (i = 0; i < ARR2_SIZE; i++)
	{
		array1[i] = (int)array1 + (i + 1)*4;
	}
}

static void InitArray3(void)
{
	int i = 0;

	for (i = 0; i < ARR3_SIZE; i++)
	{
		array1[i] = (int)array1 + (i + 1)*4;
	}
}

static inline void WorkLoad_getarray(int wamount)
{
	int i;
	int acc = 0;
	for (i=0; i<wamount; i++)
	{
		acc = acc + array0[i];
	}
}

//////////// Manipulating CRx registers definitions //////////////
static int ReadCR0(void)
{
	int cr0;

	asm volatile ( "movl %%cr0, %%eax"
			: "=a" (cr0)
		);
	
	return cr0;
}

static void WriteCR0(int cr0)
{
	asm volatile ( "movl %%eax, %%cr0"
			:
			: "a" (cr0)
		);
}

static int ReadCR2(void)
{
	int cr2;

	asm volatile ( "movl %%cr2, %%eax"
			: "=a" (cr2)
		);
	
	return cr2;
}

static int ReadCR3(void)
{
	int cr3;

	asm volatile ( "movl %%cr3, %%eax"
			: "=a" (cr3)
		);
	
	return cr3;
}

static int ReadCR4(void)
{
	int cr4;

	asm volatile ( "movl %%cr4, %%eax"
			: "=a" (cr4)
		);
	
	return cr4;
}

//////////// Reading CPUID leaf wrapper ////////////////
static struct CacheTest_GPRs CacheTest_ReadCPUID(int leaf)
{
	struct CacheTest_GPRs GPRs;

	asm volatile ( "cpuid"
			: "=a" (GPRs.eax), "=b" (GPRs.ebx), "=c" (GPRs.ecx), "=d" (GPRs.edx)
			: "a" (leaf)
		);

	return GPRs;
}

///////////// Special Instructions wrapper ///////////////////
static inline void CacheTest_WBINVD(void)
{
	asm volatile ( "wbinvd");
}

///////////// Reading and writing MSR register wrapper //////////////////
static inline struct CacheTest_GPRs CacheTest_RDMSR(int addr)
{
	struct CacheTest_GPRs GPRs;

	asm volatile ( "rdmsr"
			: "=a" (GPRs.eax), "=b" (GPRs.ebx), "=c" (GPRs.ecx), "=d" (GPRs.edx)
			: "c" (addr)
		);

	return GPRs;
}


static inline void CacheTest_WRMSR(int addr, int edx, int eax)
{
	asm volatile ( "wrmsr"
			: 
			: "d" (edx), "a"(eax), "c"(addr)
		);
}

static void CacheTest_TurnOffCache(void)
{
	// Setting CR0
	int CR0 = ReadCR0();
	//grub_printf("Old CR0 value: %x\n", CR0);
	CR0 = CR0 | 0x40000000;		// Setting CD = 1
	CR0 = CR0 & 0xDFFFFFFF;	// Setting NW = 0
	WriteCR0(CR0);
	//CR0 = ReadCR0();
	//grub_printf("New CR0 value: %x\n", CR0);

	// Invalidating cache
	CacheTest_WBINVD();

	// Setting IA32_MTRR_DEF_TYPE MSR
	//struct CacheTest_GPRs GPRs;
	//GPRs = CacheTest_RDMSR(0x000002FF);
	//grub_printf("Old IA32_MTRR_DEF_TYPE value: %x\n", GPRs.eax);
	int MTRR = 0x00000000;
	CacheTest_WRMSR(0x02FF, 0, MTRR);
	//GPRs = CacheTest_RDMSR(0x02FF);
	//grub_printf("New IA32_MTRR_DEF_TYPE value: %x\n", GPRs.eax);
}

static void CacheTest_TurnOnCache(void)
{
	// Setting CR0
	int CR0 = ReadCR0();
	//grub_printf("Old CR0 value: %x\n", CR0);
	CR0 = CR0 & 0xBFFFFFFF;		// Setting CD = 0
	CR0 = CR0 & 0xDFFFFFFF;	// Setting NW = 0
	WriteCR0(CR0);
	//CR0 = ReadCR0();
	//grub_printf("New CR0 value: %x\n", CR0);

	// Setting IA32_MTRR_DEF_TYPE MSR
	//struct CacheTest_GPRs GPRs;
	//GPRs = CacheTest_RDMSR(0x000002FF);
	//grub_printf("Old IA32_MTRR_DEF_TYPE value: %x\n", GPRs.eax);
	int MTRR = 0x00000C00;
	CacheTest_WRMSR(0x02FF, 0, MTRR);
	//GPRs = CacheTest_RDMSR(0x02FF);
	//grub_printf("New IA32_MTRR_DEF_TYPE value: %x\n", GPRs.eax);
}

////////////// Displaying PMC state ///////////////////
/*
static void Display_PMC_global(void)
{
	struct CacheTest_GPRs GPRs;
	grub_printf("#### Reading global settings ####\n");

	// Trying to read registers
	GPRs = CacheTest_RDMSR(IA32_PERF_GLOBAL_CTRL);
	grub_printf("IA32_PERF_GLOBAL_CTRL:\n");
	grub_printf("eax (hex): %x\n", GPRs.eax);
	grub_printf("edx (hex): %x\n", GPRs.edx);
	grub_printf("\n");
}


static CacheTest_GPRs Display_PMC_all(int number)
{
	struct CacheTest_GPRs GPRs;
	grub_printf("#### Reading PMC %d ####\n", number);

	GPRs = CacheTest_RDMSR((IA32_PERFEVTSELx + number));
	grub_printf("IA32_PERFEVTSEL:\n");
	grub_printf("eax (hex): %x\n", GPRs.eax);
	grub_printf("edx (hex): %x\n", GPRs.edx);
	grub_printf("\n");

	GPRs = CacheTest_RDMSR((IA32_PMCx + number));
	grub_printf("IA32_PMC:\n");
	grub_printf("eax (dec): %d\n", GPRs.eax);
	grub_printf("edx (dec): %d\n", GPRs.edx);
	grub_printf("\n");
	return GPRs;
}
*/

static void Display_PMC(int number)
{
	struct CacheTest_GPRs GPRs;
	GPRs = CacheTest_RDMSR((IA32_PMCx + number));
	grub_printf("#### Reading PMC %d ####\n", number);
	grub_printf("eax (dec): %d\n", GPRs.eax);
	grub_printf("edx (dec): %d\n", GPRs.edx);
	grub_printf("\n");
}

static void Display_PMCx(void)
{
	Display_PMC(0);
	Display_PMC(1);
	Display_PMC(2);
	Display_PMC(3);
}

/////////////// Clearing counetrs ///////////////////
static void CacheTest_ClearCounters(void)
{
	CacheTest_WRMSR(IA32_PMCx, 0, 0);
	CacheTest_WRMSR((IA32_PMCx + 1), 0, 0);
	CacheTest_WRMSR((IA32_PMCx + 2), 0, 0);
	CacheTest_WRMSR((IA32_PMCx + 3), 0, 0);
}

/////////////// Configuring counter registers /////////////////
static inline void Configure_IA32_PERF_GLOBAL_CTRL(int PMC0_EN, int PMC1_EN)
{
	int cfg_word = (PMC0_EN & 0x00000001) | ((PMC1_EN & 0x00000001) << 1);
	//grub_printf("Writing configuration word to IA32_PERF_GLOBAL_CTRL: %x\n", cfg_word);
	CacheTest_WRMSR(IA32_PERF_GLOBAL_CTRL, 0, cfg_word);
}

static inline void Configure_IA32_PERFEVTSEL(int number, int EventSelect, int UMASK, int USR, int OS, int E, int PC, int INT, int EN, int INV, int CMASK)
{
	int cfg_word = (EventSelect & 0x000000FF) | ((UMASK & 0x000000FF) << 8) | ((USR & 0x00000001) << 16) | ((OS & 0x00000001) << 17) | ((E & 0x00000001) << 18) | ((PC & 0x00000001) << 19) | ((INT & 0x00000001) << 20) | ((EN & 0x00000001) << 22) | ((INV & 0x00000001) << 23) | ((CMASK & 0x000000FF) << 24);
	grub_printf("Writing configuration word to IA32_PERFEVTSEL0: %x\n", cfg_word);
	CacheTest_WRMSR((IA32_PERFEVTSELx + number), 0, cfg_word);
}

static inline void SetCounters_CacheHit(void)
{
	grub_printf("Configuring PMC0 for cycle count\n");
	Configure_IA32_PERFEVTSEL(0, ES_Unhalted_Core_Cycles, UM_Unhalted_Core_Cycles, 1, 1, 0, 0, 0, 1, 0, 0);

	grub_printf("Configuring PMC1 for uops retired to L1 hit\n");
	Configure_IA32_PERFEVTSEL(1, ES_MEM_LOAD_UOPS_RETIRED_L1_HIT, UM_MEM_LOAD_UOPS_RETIRED_L1_HIT, 1, 1, 0, 0, 0, 1, 0, 0);

	grub_printf("Configuring PMC2 for uops retired to L2 hit\n");
	Configure_IA32_PERFEVTSEL(2, ES_MEM_LOAD_UOPS_RETIRED_L2_HIT, UM_MEM_LOAD_UOPS_RETIRED_L2_HIT, 1, 1, 0, 0, 0, 1, 0, 0);

	grub_printf("Configuring PMC3 for uops retired to L3 hit\n");
	Configure_IA32_PERFEVTSEL(3, ES_MEM_LOAD_UOPS_RETIRED_L3_HIT, UM_MEM_LOAD_UOPS_RETIRED_L3_HIT, 1, 1, 0, 0, 0, 1, 0, 0);
}

static inline void SetCounters_CacheHit_L3Miss(void)
{
	grub_printf("Configuring PMC0 for uops retired to L3 miss\n");
	Configure_IA32_PERFEVTSEL(0, ES_MEM_LOAD_UOPS_RETIRED_L3_MISS, UM_MEM_LOAD_UOPS_RETIRED_L3_MISS, 1, 1, 0, 0, 0, 1, 0, 0);

	grub_printf("Configuring PMC1 for uops retired to L1 hit\n");
	Configure_IA32_PERFEVTSEL(1, ES_MEM_LOAD_UOPS_RETIRED_L1_HIT, UM_MEM_LOAD_UOPS_RETIRED_L1_HIT, 1, 1, 0, 0, 0, 1, 0, 0);

	grub_printf("Configuring PMC2 for uops retired to L2 hit\n");
	Configure_IA32_PERFEVTSEL(2, ES_MEM_LOAD_UOPS_RETIRED_L2_HIT, UM_MEM_LOAD_UOPS_RETIRED_L2_HIT, 1, 1, 0, 0, 0, 1, 0, 0);

	grub_printf("Configuring PMC3 for uops retired to L3 hit\n");
	Configure_IA32_PERFEVTSEL(3, ES_MEM_LOAD_UOPS_RETIRED_L3_HIT, UM_MEM_LOAD_UOPS_RETIRED_L3_HIT, 1, 1, 0, 0, 0, 1, 0, 0);
}

//////////////// Application procedures ////////////////////
static void CacheTest_Proc_ReadCPUID0(void)
{
	struct CacheTest_GPRs GPRs;

	GPRs = CacheTest_ReadCPUID(ID_Leaf);
	grub_printf("CPUID 0x00h:\n");
	grub_printf("eax: %x\n", GPRs.eax);
	grub_printf("ebx: %x\n", GPRs.ebx);
	grub_printf("ecx: %x\n", GPRs.ecx);
	grub_printf("edx: %x\n", GPRs.edx);
	grub_printf("\n");
}

static void CacheTest_Proc_ReadCRx(void)
{
	int CR0 = ReadCR0();
	grub_printf("CR0: %x\n", CR0);
	if ((CR0 & 0x00000001) == 0x00000001) grub_printf("Protection : enabled\n");
	else grub_printf("Protection : disabled\n");
	if ((CR0 & 0x80000000) == 0x80000000) grub_printf("Paging : enabled\n");
	else grub_printf("Paging : disabled\n");
	
	grub_printf("CR2: %x\n", ReadCR2());
	grub_printf("CR3: %x\n", ReadCR3());
	grub_printf("CR4: %x\n", ReadCR4());

	grub_printf("\n");
}

static void CacheTest_Proc_ReadCPUID_Perf(void)
{
	struct CacheTest_GPRs GPRs;
	
	GPRs = CacheTest_ReadCPUID(Perf_Leaf);
	grub_printf("CPUID 0x0Ah:\n");
	grub_printf("eax: %x\n", GPRs.eax);
	grub_printf("ebx: %x\n", GPRs.ebx);
	grub_printf("ecx: %x\n", GPRs.ecx);
	grub_printf("edx: %x\n", GPRs.edx);
	grub_printf("\n");

	int version = GPRs.eax & 0x000000FF;
	int msr_num = (GPRs.eax & 0x0000FF00) >> 8;
	int PMC_bitwidth = (GPRs.eax & 0x00FF0000) >> 16;
	int FIXED_num = GPRs.edx & 0x0000001F;
	int FIXED_bitwidth = (GPRs.edx & 0x00001FE0) >> 5;

	grub_printf("monitoring version: %d\n", version);
	grub_printf("number of MSRs: %d\n", msr_num);
	grub_printf("PMC bit width: %d\n", PMC_bitwidth);
	grub_printf("number of FIXED MSRs: %d\n", FIXED_num);
	grub_printf("FIXED bit width: %d\n", FIXED_bitwidth);
	grub_printf("\n");
}

static void CacheTest_Proc_WBINVD(void)
{
	CacheTest_WBINVD();
	grub_printf("Cache invalidated\n");
}


static void CacheTest_Proc_InvertCache(void)
{
	if (Cache_Flag == 1)
	{
		Cache_Flag = 0;
		grub_printf("Cache will be disabled for experiments\n");
	}
	else
	{
		Cache_Flag = 1;
		grub_printf("Cache will be enabled for experiments\n");
	}
}

static void CacheTest_Proc_ResetCache(void)
{
	CacheTest_TurnOffCache();
	CacheTest_TurnOnCache();
	grub_printf("Cache reset\n");
}

static void CacheTest_Proc_RdAddr(void)
{
	int address = (int)array0;
	grub_printf("Address of array: %x\n", address);
}

/*
static void CacheTest_Proc_Test_L1(int wamount)
{
	CacheTest_ClearCounters();

	grub_printf("Configuring PMC0 for cycle count\n");
	Configure_IA32_PERFEVTSEL(0, ES_Unhalted_Core_Cycles, UM_Unhalted_Core_Cycles, 1, 1, 0, 0, 0, 1, 0, 0);

	grub_printf("Configuring PMC1 for uops retired to L1 hit\n");
	Configure_IA32_PERFEVTSEL(1, ES_MEM_LOAD_UOPS_RETIRED_L1_HIT, UM_MEM_LOAD_UOPS_RETIRED_L1_HIT, 1, 1, 0, 0, 0, 1, 0, 0);

	InitArray0();

	Configure_IA32_PERF_GLOBAL_CTRL(1, 1);
	WorkLoad_getarray(wamount);
	CacheTest_WRMSR(IA32_PERF_GLOBAL_CTRL, 0, 0x00000000);
	
	Display_PMC_global();
	Display_PMC(0);
	Display_PMC(1);
}
*/

static void CacheTest_Proc_Test_L1_rd_asm(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array0;

	InitArray0();

	
	asm volatile ( 	"mov $0x00000003, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_rdasm: ;\n"
				      	"mov (%%ebx), %%edi ;\n"
					"add $16, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_rdasm ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);


	Display_PMCx();
}

static void CacheTest_Proc_Test_L1_rd_asm_MMX(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array0;

	InitArray0();

	
	asm volatile (  "mov $0x00000003, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_MMX: ;\n"
					"movq (%%ebx), %%mm0 ;\n"
					"add $16, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_MMX ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);


	Display_PMCx();
}


static void CacheTest_Proc_Test_L1_rd_asm_XMM(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array0;

	InitArray0();

	
	asm volatile (  "mov $0x00000003, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_XMM: ;\n"
					"movaps (%%ebx), %%xmm0 ;\n"
					"add $16, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_XMM ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);

	Display_PMCx();
}

static void CacheTest_Proc_Test_L1_rd_asm_AVX(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array0;

	InitArray0();

	
	asm volatile (  "mov $0x00000003, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_AVX: ;\n"
					//"movaps (%%ebx), %%xmm0 ;\n"
					//"vmovapd %%ymm0, %%ymm1 ;\n"
					"vaddpd %%ymm2, %%ymm1, %%ymm1 ;\n"
					"add $16, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_AVX ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);

	Display_PMCx();
}

static void CacheTest_Proc_Test_L1_rd_asm_lat(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array0;

	InitArray01();

	
	asm volatile (  "mov $0x0000000F, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_lat: ;\n"
					"mov (%%ebx), %%edi ;\n"
					"mov %%edi, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_lat ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);


	Display_PMCx();
}

static void CacheTest_Proc_Test_L2_rd_asm(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array1;

	CacheTest_TurnOffCache();
	CacheTest_TurnOnCache();	

	InitArray2();

	asm volatile (  "mov $0x0000000F, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_L2_BW: ;\n"
					"mov (%%ebx), %%edi ;\n"
					"add $128, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_L2_BW ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);


	Display_PMCx();
}

static void CacheTest_Proc_Test_L2_rd_asm_MMX(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array1;

	CacheTest_TurnOffCache();
	CacheTest_TurnOnCache();	

	InitArray2();

	asm volatile (  "mov $0x0000000F, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_L2_BW_MMX: ;\n"
					"movq (%%ebx), %%mm0 ;\n"
					"add $128, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_L2_BW_MMX ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);


	Display_PMCx();
}

static void CacheTest_Proc_Test_L2_rd_asm_XMM(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array1;

	CacheTest_TurnOffCache();
	CacheTest_TurnOnCache();	

	InitArray2();

	asm volatile (  "mov $0x0000000F, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_L2_BW_XMM: ;\n"
					"movaps (%%ebx), %%xmm0 ;\n"
					"add $128, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_L2_BW_XMM ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);

	Display_PMCx();
}

static void CacheTest_Proc_Test_L2_rd_asm_lat(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array1;

	CacheTest_TurnOffCache();
	CacheTest_TurnOnCache();

	InitArray2();

	asm volatile (  "mov $0x0000000F, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_L2_lat: ;\n"
					"mov (%%ebx), %%edi ;\n"
					"add $2000, %%edi ;\n"
					"mov %%edi, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_L2_lat ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);


	Display_PMCx();
}

static void CacheTest_Proc_Test_L3_rd_asm(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array1;

	CacheTest_TurnOffCache();
	CacheTest_TurnOnCache();	

	InitArray1();

	asm volatile (  "mov $0x0000000F, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_L3_BW: ;\n"
					"mov (%%ebx), %%edi ;\n"
					"add $1000, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_L3_BW ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);


	Display_PMCx();
}

static void CacheTest_Proc_Test_L3_rd_asm_MMX(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array1;

	CacheTest_TurnOffCache();
	CacheTest_TurnOnCache();	

	InitArray1();

	asm volatile (  "mov $0x0000000F, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_L3_BW_MMX: ;\n"
					"movq (%%ebx), %%mm0 ;\n"
					"add $1000, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_L3_BW_MMX ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);


	Display_PMCx();
}

static void CacheTest_Proc_Test_L3_rd_asm_XMM(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array1;

	CacheTest_TurnOffCache();
	CacheTest_TurnOnCache();	

	InitArray1();

	asm volatile (  "mov $0x0000000F, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_L3_BW_XMM: ;\n"
					"movaps (%%ebx), %%xmm0 ;\n"
					"add $992, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_L3_BW_XMM ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);

	Display_PMCx();
}

static void CacheTest_Proc_Test_L3_rd_asm_lat(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array1;

	CacheTest_TurnOffCache();
	CacheTest_TurnOnCache();

	InitArray1();

	asm volatile (  "mov $0x0000000F, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_L3_lat: ;\n"
					"mov (%%ebx), %%edi ;\n"
					"add $2000, %%edi ;\n"
					"mov %%edi, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_L3_lat ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);


	Display_PMCx();
}

static void CacheTest_Proc_Test_RAM_rd_asm(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array1;

	CacheTest_TurnOffCache();
	CacheTest_TurnOnCache();	

	InitArray3();

	asm volatile (  "mov $0x0000000F, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_RAM_BW: ;\n"
					"mov (%%ebx), %%edi ;\n"
					"add $12000, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_RAM_BW ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);


	Display_PMCx();
}

static void CacheTest_Proc_Test_RAM_rd_asm_MMX(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array1;

	CacheTest_TurnOffCache();
	CacheTest_TurnOnCache();	

	InitArray3();

	asm volatile (  "mov $0x0000000F, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_RAM_BW_MMX: ;\n"
					"movq (%%ebx), %%mm0 ;\n"
					"add $12000, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_RAM_BW_MMX ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);


	Display_PMCx();
}

static void CacheTest_Proc_Test_RAM_rd_asm_XMM(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array1;

	CacheTest_TurnOffCache();
	CacheTest_TurnOnCache();	

	InitArray3();

	asm volatile (  "mov $0x0000000F, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_RAM_BW_XMM: ;\n"
					"movaps (%%ebx), %%xmm0 ;\n"
					"add $11904, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_RAM_BW_XMM ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);

	Display_PMCx();
}

static void CacheTest_Proc_Test_RAM_rd_asm_lat(int cycles)
{
	CacheTest_ClearCounters();

	SetCounters_CacheHit();

	int arr_address = (int)array1;

	CacheTest_TurnOffCache();
	CacheTest_TurnOnCache();

	InitArray3();

	asm volatile (  "mov $0x0000000F, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
					"mov %%esi, %%eax ;\n"
					"work_cycle_RAM_lat: ;\n"
					"mov (%%ebx), %%edi ;\n"
					"add $12000, %%edi ;\n"
					"mov %%edi, %%ebx ;\n"
					"dec %%eax ;\n"
					"jnz work_cycle_RAM_lat ;\n"
					"mov $0x00000000, %%eax ;\n"
					"mov $0x00000000, %%edx ;\n"
					"mov $0x0000038F, %%ecx ;\n"
					"wrmsr ;\n"
			: 
			: "b"(arr_address), "S"(cycles)
		);


	Display_PMCx();
}
#pragma GCC diagnostic pop
