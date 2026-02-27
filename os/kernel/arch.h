#pragma once

/* #if defined(__x86_64__) || defined(X64)
#include <arch/x86_64/gdt.h>
#include <arch/x86_64/idt.h>
#include <arch/x86/io.h>
#else */

#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/io.h>
#include <arch/x86/paging.h>
#include <arch/x86/pit.h>
#include <arch/x86/syscall.h>
#include <arch/x86/task.h>
#include <arch/x86/tss.h>
#include <arch/x86/user.h>

// #endif
