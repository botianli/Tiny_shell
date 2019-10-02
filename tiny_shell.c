#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

//ID 260824476
//Name: Li Bo Tian
char *history[100];		//global var to store command
int history_index = 0;
struct rlimit lim;		//for setting limits
void
signHandler (int signal)	//signal handling method for ctr C
{
  printf ("\nwould you like to quit the shell?\n");
  printf
    ("Please type EXIT/exit to confirm, or any other key to continue.\n");
  fflush (stdout);
}

void
ignorHandler (int signal)	// ignore handle for ctr z
{
  char dash[3] = "> ";
  printf ("\nCTRL-Z is ignored,please press enter\n");
  fflush (stdout);

}

void
start ()			//initial buffer to null, greating language;
{
  for (int i = 0; i < 100; i++)
    {
      history[i] = NULL;
    }

}

char *				//get user input, one line at a time
get_a_line ()
{
  //printf("> ");
  fflush (stdout);
  char *buffer = NULL;
  size_t n = 0;
  getline (&buffer, &n, stdin);
  if (strcasecmp (buffer, "EXIT\n") == 0)
    {
      printf ("Bye Bye!\n");
      exit (0);
    }
  return buffer;

}

void				//function to store command
store (char *cmd)
{

  history[history_index] = cmd;
  history_index++;

}

int				//system function call to deal with regular cmd
my_system (char *argv)
{


  store (argv);
  char *token[30];

  int i = 0;

  for (int jj = 0; jj < 30; jj++)
    {
      token[jj] = NULL;
    }
  const char delim[] = " \n\r";	//get argument token;
  token[i] = strtok (argv, delim);

  while (token[i] != NULL)
    {
      i++;
      token[i] = strtok (NULL, delim);
    }
  if (strcmp (token[0], "limit") == 0)
    {				//if command is limit, limit for all child process
      char *empty = NULL;
      if (token[1] == empty)	//if no argument after limit command
	{			//show only current limit;
	  getrlimit (RLIMIT_DATA, &lim);
	  printf ("Data soft limit: %ld\n", lim.rlim_cur);
	  printf ("Data hard limit: %ld\n", lim.rlim_max);
	  fflush (stdout);
	  return 1;
	}
      if (atoi (token[1]) != 0)
	{			//set limit and print result

	  int input = atoi (token[1]);
	  lim.rlim_cur = input;
	  setrlimit (RLIMIT_DATA, &lim);
	  printf ("Data soft limit: %ld\n", lim.rlim_cur);
	  printf ("Data hard limit: %ld\n", lim.rlim_max);
	  fflush (stdout);
	  return 1;
	}
      else
	{
	  printf ("invalid input.\n");
	  return 0;
	}
    }
  int pid;
  pid = fork ();		//create child process

  if (pid < 0)
    {
      fprintf (stderr, "Fork failed.");
      fflush (stdout);
      exit (EXIT_FAILURE);
      return 0;
    }
  else if (pid == 0)
    {				//child process taking care of command


      execvp (token[0], token);
      exit (EXIT_SUCCESS);


    }
  else
    {				//parent process handle internal command history and chdir

      wait (NULL);
      //check for internal command, if command is chdir, excute

      if (strcmp (token[0], "chdir") == 0 || strcmp (token[0], "cd") == 0)
	{
	  char *empty = NULL;
	  char s[100];
	  if (token[1] == empty)
	    {
	      chdir (getenv ("HOME"));
	      getcwd (s, 100);
	      printf ("%s\n", s);
	      fflush (stdout);
	      return 1;
	    }
	  else
	    {
	      chdir (token[1]);
	      getcwd (s, 100);
	      printf ("%s\n", s);

	      return 1;
	    }
	}
      if (strcmp (token[0], "history") == 0)
	{			//if the command is history 
	  int position = 1;
	  for (int i = 0; i < history_index; i++)	//print out history
	    {
	      if (i == 0)
		{
		  printf ("  %d ", position);
		  printf ("%s\n", history[i]);
		  position++;
		}
	      else
		{
		  printf ("  %d ", position);
		  printf ("%s\n", history[i]);
		  position++;
		}
	    }
	  fflush (stdout);
	  return 1;
	}


    }
  return 1;
}


int
pipe_function (char *buffer, char *fifoPath)	//function to deal pip command only
{


  char *token[30];

  int i = 0;

  for (int jj = 0; jj < 30; jj++)
    {
      token[jj] = NULL;
    }
  const char delim[] = " \n\r";
  token[i] = strtok (buffer, delim);

  while (token[i] != NULL)
    {
      i++;
      token[i] = strtok (NULL, delim);
    }
  int size = 0;
  for (int j = 0; j < 30; j++)
    {				//get size
      if (token[j] == NULL)
	{
	  break;
	}
      size++;
    }
  int pivot = 0;
  for (int j = 0; j < size; j++)
    {
      if (strcmp (token[j], "|") == 0)
	{
	  pivot = j;
	  break;
	}
    }
  //get argument before and after  | then put them in to *agrs[]
  int length = size;
  int arg1size = pivot + 1;
  int arg2size = size - pivot;
  // printf("arg1 size :%d\n",arg1size);
  //printf("arg2 size: %d\n",arg2size);
  char *arg1[arg1size];
  char *arg2[arg2size];

  for (int i = 0; i < arg2size; i++)
    {				//all to null iniazly  arg2;
      arg2[i] = NULL;
    }
  for (int i = 0; i < arg1size; i++)
    {				//all to null  initially  arg1;
      arg1[i] = NULL;
    }
  for (int i = 0; i < pivot; i++)
    {				//feed  in argument1
      arg1[i] = token[i];

    }

  int index = pivot + 1;

  for (int i = 0; i < 2; i++)
    {				//feed in argument2
      arg2[i] = token[index];

      index++;
    }
  int fd;
  pid_t p1, p2;
  p1 = fork ();			//fork 2 child
  if (p1 == 0)			//first child excute the first part of command
    {				//by using fifo
      fd = open (fifoPath, O_WRONLY);
      close (STDOUT_FILENO);
      dup (fd);
      execvp (arg1[0], arg1);
      // fflush(stdout);
      close (fd);
    }
  else
    {				//in parent 

      p2 = fork ();

      if (p2 == 0)
	{			//second child execute second part of command
	  fd = open (fifoPath, O_RDONLY);	//using fifo
	  close (STDIN_FILENO);
	  dup (fd);
	  execvp (arg2[0], arg2);
	  close (fd);
	}

      else
	{
	  wait (NULL);

	  return 0;
	}
      return 0;
    }
}

int
main (int argc, char *argv[])
{
  start ();
  printf ("Welcome to my shell, you can start now.\n");
  char *path = argv[1];

  while (1)
    {
      signal (SIGINT, signHandler);	// signal fuction call to monitor ctrc ctr z 
      signal (SIGTSTP, ignorHandler);

      char *line = get_a_line ();

      if (strlen (line) > 1)
	{

	  char pipe = '|';	//check if the command contain pipe argument
	  char chek[40];
	  strcpy (chek, line);
	  char *pt = strchr (chek, pipe);
	  if (pt != NULL)	//if contain pipe argument,  go to pipe function
	    {
	      pipe_function (line, path);
	      store (line);
	    }
	  else			//else go to regualr my_system function
	    {
	      my_system (line);
	    }
	}


    }
  return 0;

}

