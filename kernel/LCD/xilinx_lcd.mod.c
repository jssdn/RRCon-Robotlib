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
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x1c939152, "struct_module" },
	{ 0x26187179, "remove_proc_entry" },
	{ 0x9ef749e2, "unregister_chrdev" },
	{ 0x20030ecd, "ioremap" },
	{ 0xfa358e3c, "create_proc_entry" },
	{ 0x2657c0f6, "register_chrdev" },
	{ 0x20000329, "simple_strtoul" },
	{ 0x43b0c9c3, "preempt_schedule" },
	{ 0xb06bf783, "module_put" },
	{ 0x461ebfa0, "__copy_tofrom_user" },
	{ 0xea147363, "printk" },
	{ 0xa39b4cf2, "udelay" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

