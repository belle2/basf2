#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x89e24b9c, "struct_module" },
	{ 0xd5028665, "create_proc_entry" },
	{ 0x9eac042a, "__ioremap" },
	{ 0x7561ed, "pci_bus_read_config_dword" },
	{ 0x78021fa2, "pci_bus_read_config_word" },
	{ 0xb7046d9a, "pci_find_device" },
	{ 0x4086729e, "register_chrdev" },
	{ 0xb2fd5ceb, "__put_user_4" },
	{ 0xd986dad1, "kernel_fpu_begin" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x5d57df57, "remove_proc_entry" },
	{ 0xc192d491, "unregister_chrdev" },
	{ 0xa03d6a57, "__get_user_4" },
	{ 0x1b7d4074, "printk" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "D5EF059A397276ADB272D7E");
