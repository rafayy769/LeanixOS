#ifndef TESTMAIN_H
#define TESTMAIN_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

/* represents a single test case */
struct test_case {

	const char *name;
	void (*func) ();	/* note the absence of params, registered functions
							   may or may not take any param */
};

/* Reads a command from the serial port into cmd_buf and returns
   the number of characters read. The command is terminated by a any of these:
   '\0', '\n', '\r'. */

size_t 	read_command ();

/* Send a message string back to the server. A "*" is used to mark the end
	of the message. */

void 	send_msg (const char *msg);

void utoa (unsigned val, char *buf);

/* Useful macros. */

#define PASS() send_msg ("PASSED")
#define FAIL() send_msg ("FAILED")

/* these assertions are helpful, send a failure msg too */

#define ASSERT_TRUE(cond, msg) do { \
    if (!(cond)) { \
        send_msg("FAILED: " msg); \
        return; \
    } \
} while(0)

#define ASSERT_FALSE(cond, msg) ASSERT_TRUE(!(cond), msg)

#define ASSERT_EQ(a, b, msg) do { \
    if ((a) != (b)) { \
        send_msg("FAILED: " msg); \
        return; \
    } \
} while(0)

#define ASSERT_NOT_NULL(ptr, msg) ASSERT_TRUE((ptr) != NULL, msg)
#define ASSERT_NULL(ptr, msg) ASSERT_TRUE((ptr) == NULL, msg)

#endif // TESTMAIN_H
