#include <stdio.h>
#include <assert.h>
#include "plx9054.h"

main()
{
    plx9054_intcsr intcsr;
    unsigned int val;
    FILE * f = fopen("/proc/plx9054/INTCSR", "r");
    assert(f != NULL);
    fscanf(f, "%08x", &val);
    intcsr.val = val;
    printf("%08x\n", val);

if (intcsr.bit.master_local_err_enable) puts("master_local_err_enable");
if (intcsr.bit.slave_local_err_enable) puts("slave_local_err_enable");
if (intcsr.bit.gen_pci_error) puts("gen_pci_error");
if (intcsr.bit.mbox_enable) puts("mbox_enable");
if (intcsr.bit.pm_int_enable) puts("pm_int_enable");
if (intcsr.bit.pm_int) puts("pm_int");
if (intcsr.bit.pci_error_enable) puts("pci_error_enable");
if (intcsr.bit.pci_error) puts("pci_error");
if (intcsr.bit.pci_int_enable) puts("pci_int_enable");
if (intcsr.bit.pci_doorbell_int_enable) puts("pci_doorbell_int_enable");
if (intcsr.bit.pci_abort_int_enable) puts("pci_abort_int_enable");
if (intcsr.bit.local_int_input_enable) puts("local_int_input_enable");
if (intcsr.bit.retry_abort_enable) puts("retry_abort_enable");
if (intcsr.bit.pci_doorbell) puts("pci_doorbell");
if (intcsr.bit.pci_abort) puts("pci_abort");
if (intcsr.bit.local_int_active) puts("local_int_active");
if (intcsr.bit.local_int_output_enable) puts("local_int_output_enable");
if (intcsr.bit.local_doorbell_int_enable) puts("local_doorbell_int_enable");
if (intcsr.bit.dma0_int_enable) puts("dma0_int_enable");
if (intcsr.bit.dma1_int_enable) puts("dma1_int_enable");
if (intcsr.bit.local_doorbell_int_active) puts("local_doorbell_int_active");
if (intcsr.bit.dma0_int_active) puts("dma0_int_active");
if (intcsr.bit.dma1_int_active) puts("dma1_int_active");
if (intcsr.bit.bist_int_active) puts("bist_int_active");
if (intcsr.bit.master_failed) puts("master_failed");
if (intcsr.bit.dma0_failed) puts("dma0_failed");
if (intcsr.bit.dma1_failed) puts("dma1_failed");
if (intcsr.bit.target_failed) puts("target_failed");
if (intcsr.bit.mbox0) puts("mbox0");
if (intcsr.bit.mbox1) puts("mbox1");
if (intcsr.bit.mbox2) puts("mbox2");
if (intcsr.bit.mbox3) puts("mbox3");

    exit(0);
}
