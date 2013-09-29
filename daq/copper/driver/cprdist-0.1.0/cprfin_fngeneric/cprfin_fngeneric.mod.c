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
	{ 0x3d91fa7b, "struct_module" },
	{ 0x89b301d4, "param_get_int" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0xf75aed76, "register_chrdev" },
	{ 0x9eac042a, "__ioremap" },
	{ 0x852abecf, "__request_region" },
	{ 0x56b9bd32, "pci_bus_read_config_dword" },
	{ 0x5df560b9, "pci_find_device" },
	{ 0xef79ac56, "__release_region" },
	{ 0x1f9cfe83, "iomem_resource" },
	{ 0xedc03953, "iounmap" },
	{ 0xc192d491, "unregister_chrdev" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0x1b7d4074, "printk" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "513892380B9520D989FA8BF");
