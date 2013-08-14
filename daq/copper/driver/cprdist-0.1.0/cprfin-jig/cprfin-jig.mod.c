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
	{ 0x4086729e, "register_chrdev" },
	{ 0x7561ed, "pci_bus_read_config_dword" },
	{ 0x9eac042a, "__ioremap" },
	{ 0x4c3af445, "__request_region" },
	{ 0x82ede2af, "pci_find_slot" },
	{ 0x8bb33e7d, "__release_region" },
	{ 0xdc3eaf70, "iomem_resource" },
	{ 0xedc03953, "iounmap" },
	{ 0xc192d491, "unregister_chrdev" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0x1b7d4074, "printk" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "E6BAE433961789D216D261F");
