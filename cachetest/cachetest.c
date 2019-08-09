/* CacheTest.c - module for dynamic loading */

#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/err.h>
#include <grub/dl.h>
#include <grub/extcmd.h>
#include <grub/i18n.h>
#include <grub/term.h>
#include "cachetest.h"

GRUB_MOD_LICENSE ("GPLv3+");

static grub_err_t
grub_cmd_cachetest (grub_extcmd_context_t ctxt __attribute__ ((unused)),
		int argc __attribute__ ((unused)),
		char **args __attribute__ ((unused)))
{

	int ikey = 0;
	grub_printf ("%s\n", _("Alexander Comm Test: version 0.7"));
	grub_printf("\n");
	CacheTest_Hello();
	grub_printf("\n");

	while (ikey  != 27)
	{
		grub_printf("\n");
		grub_printf("################################################\n");
		grub_printf("Please, enter command:\n");

		// Generic commands
		grub_printf("1. Display processor identification info\n");
		grub_printf("2. Display Control Registers\n");
		grub_printf("3. Display info about available perf monitoring facilities\n");
		grub_printf("4. Invalidate cache\n");
		if (Cache_Flag == 1) grub_printf("5. Disable cache\n");
		else grub_printf("5. Enable cache\n");
		grub_printf("6. Reset cache\n");

		// Experimental commands	
		grub_printf("a. Print address of array\n");
		grub_printf("b. L1 cache bandwidth test (1x read)\n");
		grub_printf("c. L1 cache bandwidth test (10x read)\n");
		grub_printf("d. L1 cache bandwidth test (100x read)\n");

		// L1
		grub_printf("e. L1 cache bandwidth test (1000x read)\n");
		grub_printf("f. L1 cache bandwidth test (1000x read, MMX)\n");
		grub_printf("g. L1 cache bandwidth test (1000x read, XMM)\n");
		grub_printf("h. L1 cache latency test   (1000x read)\n");

		// L2
		grub_printf("i. L2 cache bandwidth test (чтение 1000)\n");
		grub_printf("j. L2 cache bandwidth test (1000x read, MMX)\n");
		grub_printf("k. L2 cache bandwidth test (1000x read, XMM)\n");
		grub_printf("l. L2 cache latency test   (10x read)\n");
		grub_printf("m. L2 cache latency test   (20x read)\n");
		grub_printf("n. L2 cache latency test   (30x read)\n");

		// L3
		grub_printf("o. L3 cache bandwidth test (1000x read)\n");
		grub_printf("p. L3 cache bandwidth test (1000x read, MMX)\n");
		grub_printf("q. L3 cache bandwidth test (1000x read, XMM)\n");
		grub_printf("r. L3 cache latency test   (1000x read)\n");

		// RAM
		grub_printf("s. RAM cache bandwidth test (1000x read)\n");
		grub_printf("t. RAM cache bandwidth test (1000x read, MMX)\n");
		grub_printf("u. RAM cache bandwidth test (1000x read, XMM)\n");
		grub_printf("v. RAM cache latency test   (1000x read)\n");

		grub_printf("ESC. Exit\n");
		grub_printf("################################################\n");
		grub_printf("\n");
		ikey = grub_getkey();

		switch (ikey)
		{
			// Generic commands
			case '1': CacheTest_Proc_ReadCPUID0(); break;
			case '2': CacheTest_Proc_ReadCRx(); break;
			case '3': CacheTest_Proc_ReadCPUID_Perf(); break;
			case '4': CacheTest_Proc_WBINVD(); break;
			case '5': CacheTest_Proc_InvertCache(); break;
			case '6': CacheTest_Proc_ResetCache(); break;
			case '9': CacheTest_Proc_Test_L1_rd_asm_AVX(1000);  break;

			// Experimental commands
			case 'a': CacheTest_Proc_RdAddr();  break;
			case 'b': CacheTest_Proc_Test_L1_rd_asm(1);  break;
			case 'c': CacheTest_Proc_Test_L1_rd_asm(10);  break;
			case 'd': CacheTest_Proc_Test_L1_rd_asm(100);  break;

			// L1
			case 'e': CacheTest_Proc_Test_L1_rd_asm(1000);  break;
			case 'f': CacheTest_Proc_Test_L1_rd_asm_MMX(1000);  break;
			case 'g': CacheTest_Proc_Test_L1_rd_asm_XMM(1000);  break;
			case 'h': CacheTest_Proc_Test_L1_rd_asm_lat(1000);  break;

			// L2
			case 'i': CacheTest_Proc_Test_L2_rd_asm(1000);  break;
			case 'j': CacheTest_Proc_Test_L2_rd_asm_MMX(1000);  break;
			case 'k': CacheTest_Proc_Test_L2_rd_asm_XMM(1000);  break;
			case 'l': CacheTest_Proc_Test_L2_rd_asm_lat(10);  break;
			case 'm': CacheTest_Proc_Test_L2_rd_asm_lat(20);  break;
			case 'n': CacheTest_Proc_Test_L2_rd_asm_lat(30);  break;

			// L3
			case 'o': CacheTest_Proc_Test_L3_rd_asm(1000);  break;
			case 'p': CacheTest_Proc_Test_L3_rd_asm_MMX(1000);  break;
			case 'q': CacheTest_Proc_Test_L3_rd_asm_XMM(1000);  break;
			case 'r': CacheTest_Proc_Test_L3_rd_asm_lat(1000);  break;

			// RAM
			case 's': CacheTest_Proc_Test_RAM_rd_asm(1000);  break; 
			case 't': CacheTest_Proc_Test_RAM_rd_asm_MMX(1000);  break;
			case 'u': CacheTest_Proc_Test_RAM_rd_asm_XMM(1000);  break;
			case 'v': CacheTest_Proc_Test_RAM_rd_asm_lat(1000);  break;

			case 27: grub_printf("Exiting. Goodbye.\n"); break;
			default: grub_printf("Command (%x) incorrect, please repeat!\n", ikey);
		}
	}
	
	return 0;
}

static grub_extcmd_t cmd;

GRUB_MOD_INIT(cachetest)
{
  cmd = grub_register_extcmd ("cachetest", grub_cmd_cachetest, 0, 0,
			      N_("Selective cache bandwidth/latency tests"), 0);
}

GRUB_MOD_FINI(cachetest)
{
  grub_unregister_extcmd (cmd);
}
