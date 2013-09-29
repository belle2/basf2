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
	{ 0x852abecf, "__request_region" },
	{ 0x70169a39, "pci_bus_read_config_byte" },
	{ 0x1f9cfe83, "iomem_resource" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0xab978df6, "malloc_sizes" },
	{ 0xa7c35c6b, "getnstimeofday" },
	{ 0xdff44f21, "remove_proc_entry" },
	{ 0x8c0be9b4, "queue_work" },
	{ 0xa315c08b, "pci_find_slot" },
	{ 0xda4008e6, "cond_resched" },
	{ 0xf2766228, "__create_workqueue" },
	{ 0x1b7d4074, "printk" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0x8b571460, "pci_bus_write_config_dword" },
	{ 0x31153eb8, "destroy_workqueue" },
	{ 0x9eac042a, "__ioremap" },
	{ 0x1d676acf, "boot_tvec_bases" },
	{ 0x19070091, "kmem_cache_alloc" },
	{ 0x56b9bd32, "pci_bus_read_config_dword" },
	{ 0x107d6ba3, "__get_free_pages" },
	{ 0x26e96637, "request_irq" },
	{ 0x4292364c, "schedule" },
	{ 0xf75aed76, "register_chrdev" },
	{ 0x84a35e1b, "create_proc_entry" },
	{ 0xef79ac56, "__release_region" },
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
