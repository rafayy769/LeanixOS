#include <stdio.h>
#include <string.h>
#include <stdbool.h>
bool run_cmd(char* cmd)
{

	// tokenize the command
	char* args[4] = {0};
	int argc = 0;

	char* token = strtok(cmd, " ");
	while (token && argc < 4) {
		args[argc++] = token;
		token = strtok(NULL, " ");
	}

	cmd = args[0]; // the first token is the command

	//! exit command
	if (strcmp (cmd, "exit") == 0) {
		return true;
	}

	//! help
	else if (strcmp (cmd, "help") == 0) {

		printf ("\n");
		printf ("Leenix v0.1 shell\n");
		printf ("with a basic Command Line Interface (CLI)\n\n");
		printf ("Supported commands:\n");
		printf (" - ticks: get total system ticks since init\n");
		printf (" - clear: clears the display\n");
		printf (" - kinfo: print kernel memory info\n");
		printf (" - mdmp <address> <size>: dump memory contents at \
					address for size bytes\n");
		printf (" - sd: dump current stack contents\n");
		printf (" - heap: dump heap info\n");
		printf (" - ptwalk <vstart> <vend>: walk the PTs from vstart to vend\n");
		printf (" - open: open <filename>\n");
		printf (" - elf: elf <filename>\n");
		printf (" - break: trigger int3 breakpoint\n");
		printf (" - help: displays this message\n");
		printf (" - exit: quits and halts the system\n");
	}

	else if (strcmp (cmd, "mdmp") == 0) {
		
		// dump memory contents
		if (argc < 3) {
			printf("Usage: mdmp <size> <address>\n");
			return false;
		}

		printf ("\n");
		uint32_t count = strtol(args[1], NULL, 0); // get the count of bytes to dump
		uint8_t* addr = (uint8_t*) strtol(args[2], NULL, 0); // get the address to dump from

		for (uint32_t i = 0; i < count; i += 16) {
			printf("0x%08x: ", addr + i);
			for (int j = 0; j < 16; j++) {
				if (i + j < count)
					printf("%02x ", addr[i + j]);
				else
					printf("   ");
			}
			printf(" |");
			for (int j = 0; j < 16 && (i + j) < count; j++) {
				char c = addr[i + j];
				printf("%c", (c >= 32 && c <= 126) ? c : '.');
			}
			printf("|\n");
		}

	}

	//! empty command
	else if (strcmp (cmd, "") == 0) {
		// do nothing
	}

	//! trigger a breakpoint
	else if (strcmp (cmd, "break") == 0) {
		asm volatile ("int3;");
	}

	//! print kernel info
	else if (strcmp (cmd, "kinfo") == 0) {
		// extern char kernel_start;
		// extern char kernel_end;
		// printf("Kernel loaded from %p to %p\n", &kernel_start, &kernel_end);
		// printf("Kernel size: %u bytes\n", (uint32_t)(&kernel_end - &kernel_start));
	}

	//! stack dump
	else if (strcmp (cmd, "sd") == 0) {

		uint32_t* esp;
		asm volatile ("mov %%esp, %0" : "=r"(esp));

		printf("\n");
		printf("Stack dump (ESP = 0x%08x):", (uint32_t)esp);
		printf("\n");
		printf("-----------------------------------------\n");
		printf("    Address       Value       \%esp\n");
		printf("-----------------------------------------\n");

		for (int i = -5; i < 5; ++i) {

			uint32_t* addr = esp + i;
			uint32_t val = *addr;

			if (i == 0) {
				printf("-> 0x%08x: 0x%08x   <-- ESP", (uint32_t)addr, val);
				printf("\n");
			} else {
				printf("   0x%08x: 0x%08x\n", (uint32_t)addr, val);
			}

		}
		printf("-----------------------------------------\n\n");

	}

	else if (strcmp (cmd, "echo") == 0) {
		char str[1024] = "";  // make sure it's large enough

		for (int i = 1; i< argc; i++) {
			strcat(str, args[i]);   // append argument
			strcat(str, " ");       // append a space
		}
		str[strlen(str) - 1] = '\0';
		printf ("%s\n", str);
	}

	else if (strcmp (cmd,"repeat") == 0){
		if (argc > 1){

			long n = strtol(args[1],NULL,10);
			char str[1024] = "";  // make sure it's large enough

			for (int i = 2; i< argc; i++) {
				strcat(str, args[i]);   // append argument
				strcat(str, " ");       // append a space
			}
			str[strlen(str) - 1] = '\0';
			for (long i = 0; i < n;i++){
				printf ("%s ", str);
			}
			printf ("%s\n","");
		}

	}

	else if (strcmp (cmd, "color") == 0) {
		// terminal_settext_color(10);
	}

	else if (strcmp (cmd, "bgcolor") == 0) {
		// terminal_setbg_color(8);
	}

	//! invalid command
	else {
		printf ("sh: error: Unkown command\n");
	}

	return false;

}

int main()
{
	printf ("\n");
	// enable_interrupts();
	char cmd_buf[128]; // allocate memory for command buffer
	while (1) {
		printf (" $ ");
		getline (cmd_buf, 128); // read command from terminal
		if (run_cmd(cmd_buf)) {
			printf ("exiting shell...\n");
			return 0; // exit the shell if the command is "exit"
		}
	}
}