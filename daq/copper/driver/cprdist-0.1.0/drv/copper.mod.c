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
	{ 0x8e4729ed, "struct_module" },
	{ 0x4c3af445, "__request_region" },
	{ 0x3e402ba8, "pci_bus_read_config_byte" },
	{ 0xdc3eaf70, "iomem_resource" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0xab978df6, "malloc_sizes" },
	{ 0xa7c35c6b, "getnstimeofday" },
	{ 0xa22a96d3, "remove_proc_entry" },
	{ 0x2e3e969a, "queue_work" },
	{ 0x4fe5ea1e, "pci_find_slot" },
	{ 0xda4008e6, "cond_resched" },
	{ 0x4654fd57, "__create_workqueue" },
	{ 0x1b7d4074, "printk" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0x75e8f29, "pci_bus_write_config_dword" },
	{ 0x72998f8c, "destroy_workqueue" },
	{ 0x9eac042a, "__ioremap" },
	{ 0x1d676acf, "boot_tvec_bases" },
	{ 0x19070091, "kmem_cache_alloc" },
	{ 0x423cae99, "pci_bus_read_config_dword" },
	{ 0x107d6ba3, "__get_free_pages" },
	{ 0x26e96637, "request_irq" },
	{ 0x4292364c, "schedule" },
	{ 0x1acd2470, "register_chrdev" },
	{ 0xf17212d3, "create_proc_entry" },
	{ 0x8bb33e7d, "__release_region" },
	{ 0x9941ccb8, "free_pages" },
	{ 0x59968f3c, "__wake_up" },
	{ 0x37a0cba, "kfree" },
	{ 0xc8f02aeb, "prepare_to_wait" },
	{ 0xedc03953, "iounmap" },
	{ 0xc192d491, "unregister_chrdev" },
	{ 0x865edc9b, "finish_wait" },
	{ 0x60a4461c, "__up_wakeup" },
	{ 0xb742fd7, "simple_strtol" },
	{ 0x25da070, "snprintf" },
	{ 0x96b27088, "__down_failed" },
	{ 0xf2a644fb, "copy_from_user" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "C36E01ACCEEBABF4B71221C");
