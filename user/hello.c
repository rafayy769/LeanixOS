#include <stdio.h>
#include <unistd.h>

int
main ()
{
  
  printf ("Hello, World!\n");
  int pid = fork ();
  if (pid == 0) {
    // Child process
    printf ("Hello from the child process! PID=%d\n", pid);
    // exec ("/fd0/SHALL");   /* exec not impl yet */
    while (1) {}
  } else if (pid > 0) {
    // Parent process
    printf ("Hello from the parent process! PID=%d\n", pid);
    while (1) {}
  } else {
    // Fork failed
    printf ("Fork failed!\n");
  }
  return 0;
}