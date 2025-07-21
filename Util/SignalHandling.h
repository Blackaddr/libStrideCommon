
#ifndef UTIL_SIGNALHANDLING_H_
#define UTIL_SIGNALHANDLING_H_

#include <csignal>
#include <csetjmp>

// use this macro in a try block to check if it was reached by a signal or by normal execution
// pass the runtime_error msg as a parameter
#define SIGNAL_ENTRY_CHECK(msg) do { int sig; if (!((sig = setjmp(gBuffer)) == 0)) { throw std::runtime_error(msg); } } while (0)

// macro for restoring default signal handlers
#define RESTORE_DEFAULT_SIGNAL_HANDLERS(x) do { for (int i=0; i < NSIG; i++) { signal(i, gPrevHandler[i]); } } while (0)

// macro for registering custom signal handler
#define REGISTER_CATCH_SIGNAL_HANDLERS(x) do { for (int i=0; i < NSIG; i++) { gPrevHandler[i] = signal(i, catch_signal); } } while (0)

// Place this macro in the global definite area to use the signal handler
#define DEFINE_CUSTOM_SIGNAL_HANDLER \
static jmp_buf gBuffer; \
static void (*gPrevHandler[NSIG])(int);\
static void catch_signal(int signalNumber) {\
    for (int i=0; i < NSIG; i++) { signal(i, catch_signal); }\
    longjmp(gBuffer, signalNumber); }

#endif