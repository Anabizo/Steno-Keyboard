#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x7935867f, "input_event" },
	{ 0x23f25c0a, "__dynamic_pr_debug" },
	{ 0x90a48d82, "__ubsan_handle_out_of_bounds" },
	{ 0xd16a7f0f, "input_allocate_device" },
	{ 0xf58ff02f, "input_register_device" },
	{ 0xfdec3315, "input_free_device" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0x379d1ecb, "input_register_handler" },
	{ 0xfdec3315, "input_unregister_device" },
	{ 0xbd03ed67, "random_kmalloc_seed" },
	{ 0xa62b1cc9, "kmalloc_caches" },
	{ 0xd1f07d8f, "__kmalloc_cache_noprof" },
	{ 0xaffbf64b, "input_register_handle" },
	{ 0xaffbf64b, "input_open_device" },
	{ 0x9479a1e8, "strnlen" },
	{ 0x888b8f57, "strcmp" },
	{ 0xe54e0a6b, "__fortify_panic" },
	{ 0xe4de56b4, "__ubsan_handle_load_invalid_value" },
	{ 0x5531feea, "input_unregister_handler" },
	{ 0xd272d446, "__fentry__" },
	{ 0xe8213e80, "_printk" },
	{ 0x58176f4f, "input_close_device" },
	{ 0x58176f4f, "input_unregister_handle" },
	{ 0xcb8b6ec6, "kfree" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x43a349ca, "strlen" },
	{ 0xab006604, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x7935867f,
	0x23f25c0a,
	0x90a48d82,
	0xd16a7f0f,
	0xf58ff02f,
	0xfdec3315,
	0xd272d446,
	0x379d1ecb,
	0xfdec3315,
	0xbd03ed67,
	0xa62b1cc9,
	0xd1f07d8f,
	0xaffbf64b,
	0xaffbf64b,
	0x9479a1e8,
	0x888b8f57,
	0xe54e0a6b,
	0xe4de56b4,
	0x5531feea,
	0xd272d446,
	0xe8213e80,
	0x58176f4f,
	0x58176f4f,
	0xcb8b6ec6,
	0xd272d446,
	0x43a349ca,
	0xab006604,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"input_event\0"
	"__dynamic_pr_debug\0"
	"__ubsan_handle_out_of_bounds\0"
	"input_allocate_device\0"
	"input_register_device\0"
	"input_free_device\0"
	"__stack_chk_fail\0"
	"input_register_handler\0"
	"input_unregister_device\0"
	"random_kmalloc_seed\0"
	"kmalloc_caches\0"
	"__kmalloc_cache_noprof\0"
	"input_register_handle\0"
	"input_open_device\0"
	"strnlen\0"
	"strcmp\0"
	"__fortify_panic\0"
	"__ubsan_handle_load_invalid_value\0"
	"input_unregister_handler\0"
	"__fentry__\0"
	"_printk\0"
	"input_close_device\0"
	"input_unregister_handle\0"
	"kfree\0"
	"__x86_return_thunk\0"
	"strlen\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");

MODULE_ALIAS("input:b*v*p*e*-e*1,*k*r*a*m*l*s*f*w*");

MODULE_INFO(srcversion, "3AF6939475FD20D7FE7C640");
