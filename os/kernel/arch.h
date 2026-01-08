#pragma once

// I'm not doing x86_64 yet, but when I do I'll chuck includes here.
// I just wanted a central place for arch/**/* includes
#if defined(__x86_64__) || defined(X64)

#else
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/io.h>
#include <arch/x86/multiboot.h>
#endif
