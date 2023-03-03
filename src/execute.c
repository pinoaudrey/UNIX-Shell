/**
 * @file execute.c
 *
 * @brief Implements interface functions between Quash and the environment and
 * functions that interpret an execute commands.
 *
 * @note As you add things to this file you may want to change the method signature
 */

#include "execute.h"

#include <stdio.h>

#include "quash.h"
#include "deque.h"


IMPLEMENT_DEQUE(pidQue, pid_t)
IMPLEMENT_DEQUE(jobQue, QuashJob)

//IDK if we're allowed to include these...
#include <string.h>
#include <sys/wait.h>

bool jobQueInitialized = false;

jobQue myJobQue; //To store jobs in the que

pidQue myPIDs; //To temperarily store pids until moved into a QuashJob struct

int jobCount = 0; //to generate job numbers?

// Remove this and all expansion calls to it
/**
 * @brief Note calls to any function that requires implementation
 */
#define IMPLEMENT_ME()                                                  \
  fprintf(stderr, "IMPLEMENT ME: %s(line %d): %s()\n", __FILE__, __LINE__, __FUNCTION__)

/***************************************************************************
 * Interface Functions
 ***************************************************************************/
#define BSIZE 1024

// Return a string containing the current working directory.
char* get_current_directory(bool* should_free) {
  // TODO: Get the current working directory. This will fix the prompt path.
  // HINT: This should be pretty simple
  // Change this to true if necessary
  *should_free = true;
  return getcwd(NULL, BSIZE);
}

// Returns the value of an environment variable env_var
const char* lookup_env(const char* env_var) {
  // TODO: Lookup environment variables. This is required for parser to be able
  // to interpret variables from the command line and display the prompt
  // correctly
  // HINT: This should be pretty simple
  const char* value = getenv(env_var);
  // TODO: Remove warning silencers
  //(void) env_var; // Silence unused variable warning

  return value;
}

// Check the status of background jobs
void check_jobs_bg_status() {
  // TODO: Check on the statuses of all processes belonging to all background
  // jobs. This function should remove jobs from the jobs queue once all
  // processes belonging to a job have completed.
  IMPLEMENT_ME();
  
  QuashJob currJob; //variable to store the current job in iteration

  if (jobQueInitialized) { //just a failsafe to ensure that the jobQue actually exists

    for (int i = 0; i < length_jobQue(&myJobQue); ++i){ //Iterate through all QuashJobs in myJobQue

      currJob = pop_front_jobQue(&myJobQue); //get the curr QuashJob

      //print_job(currJob.jobID, 1, currJob.cmd); //test print

      //pidQue *pidList = currJob.pids; //Get the pids list belonging to the job
      pid_t currPID; //Set up a variable to store the working current pid

      //currPID = pop_front_pidQue(pidList); //test print
      //print_job(currJob.jobID, peek_front_pidQue(&currJob.pids), currJob.cmd); //test print

      bool jobDone = true; //assume the job is done (will be changed in loop if not)

      for (int i = 0; i < length_pidQue(&currJob.pids); ++i) {
        int status;
        currPID = pop_front_pidQue(&currJob.pids);

        //print_job(currJob.jobID, currPID, currJob.cmd); 

        pid_t check_pid = waitpid(currPID, &status, WNOHANG); //should check if the process is still running
        
        if (check_pid < 0) {
          /*
            Process terminated with an error.
          */
        } else if (check_pid == 0) {
          /*
            The child process is still running.
          */
          jobDone = false;
          push_back_pidQue(&currJob.pids, currPID); //add back to pidQue, since still needed
        } else {
          /*
            The child process is done.
          */
          push_back_pidQue(&currJob.pids, check_pid); //add back to pidQue to track finished processes
          //This might result in an error, as I'm unsure about what happens if you wait on a child
          //process that has already terminated...
        }
      }

      if (jobDone) {
        //currPID = pop_front_pidQue(pidList);
        print_job_bg_complete(currJob.jobID, currPID, currJob.cmd);
      } else {
       push_back_jobQue(&myJobQue, currJob);
      }
    }
  } else {
    printf("Job Que is Empty! Remove this line!");
  }

  // TODO: Once jobs are implemented, uncomment and fill the following line
  // print_job_bg_complete(job_id, pid, cmd);
}

// Prints the job id number, the process id of the first process belonging to
// the Job, and the command string associated with this job
void print_job(int job_id, pid_t pid, const char* cmd) {
  printf("[%d]\t%8d\t%s\n", job_id, pid, cmd);
  fflush(stdout);
}

// Prints a start up message for background processes
void print_job_bg_start(int job_id, pid_t pid, const char* cmd) {
  printf("Background job started: ");
  print_job(job_id, pid, cmd);
}

// Prints a completion message followed by the print job
void print_job_bg_complete(int job_id, pid_t pid, const char* cmd) {
  printf("Completed: \t");
  print_job(job_id, pid, cmd);
}

/***************************************************************************
 * Functions to process commands
 ***************************************************************************/
// Run a program reachable by the path environment variable, relative path, or
// absolute path
void run_generic(GenericCommand cmd) {
  // Execute a program with a list of arguments. The `args` array is a NULL
  // terminated (last string is always NULL) list of strings. The first element
  // in the array is the executable
  char* exec = cmd.args[0];
  char** args = cmd.args;

  // TODO: Remove warning silencers
  //(void) exec; // Silence unused variable warning
  //(void) args; // Silence unused variable warning

  // TODO: Implement run generic
  //IMPLEMENT_ME();
  execvp(exec, args);

  perror("ERROR: Failed to execute program");
}

// Print strings
void run_echo(EchoCommand cmd) {
  // Print an array of strings. The args array is a NULL terminated (last
  // string is always NULL) list of strings.
  char** str = cmd.args;

  // TODO: Remove warning silencers
  //(void) str; // Silence unused variable warning

  // TODO: Implement echo
  //IMPLEMENT_ME();
  for (int i = 0; str[i] != NULL; ++i) {
    printf("%s ", str[i]);
  }
  printf("\n");


  // Flush the buffer before returning
  fflush(stdout);
}

// Sets an environment variable
void run_export(ExportCommand cmd) {
  // Write an environment variable
  const char* env_var = cmd.env_var;
  const char* val = cmd.val;

  // TODO: Remove warning silencers
  //(void) env_var; // Silence unused variable warning
  //(void) val;     // Silence unused variable warning

  // TODO: Implement export.
  // HINT: This should be quite simple.
  //IMPLEMENT_ME();
  setenv(env_var, val, 1);
}

// Changes the current working directory
void run_cd(CDCommand cmd) {
  // Get the directory name
  const char* dir = cmd.dir;

  // Check if the directory is valid
  if (dir == NULL) {
    perror("ERROR: Failed to resolve path");
    return;
  }

  // TODO: Change directory

  // TODO: Update the PWD environment variable to be the new current working
  // directory and optionally update OLD_PWD environment variable to be the old
  // working directory.
  //IMPLEMENT_ME();
  char* newDirectory = getcwd(NULL, BSIZE);
  chdir(dir);
}

// Sends a signal to all processes contained in a job
void run_kill(KillCommand cmd) {
  int signal = cmd.sig;
  int job_id = cmd.job;

  // TODO: Remove warning silencers
  (void) signal; // Silence unused variable warning
  (void) job_id; // Silence unused variable warning

  // TODO: Kill all processes associated with a background job
  IMPLEMENT_ME();
}


// Prints the current working directory to stdout
void run_pwd() {
  // TODO: Print the current working directory
  //IMPLEMENT_ME();
  bool should_free;
  char* currentDirectory = get_current_directory(&should_free);
  printf("%s\n", currentDirectory);
  if(should_free) {
    free(currentDirectory);
  }
  // Flush the buffer before returning
  fflush(stdout);
}

// Prints all background jobs currently in the job list to stdout
void run_jobs() {
  // TODO: Print background jobs
  //IMPLEMENT_ME();

  QuashJob currJob;
  for (int i = 0; i < length_jobQue(&myJobQue); ++i) {
    currJob = pop_front_jobQue(&myJobQue);
    pidQue *pidList = &currJob.pids;
    pid_t firstPID = peek_front_pidQue(pidList);
    print_job(currJob.jobID, firstPID, currJob.cmd);
    push_back_jobQue(&myJobQue, currJob);
  }

  // Flush the buffer before returning
  fflush(stdout);
}

/***************************************************************************
 * Functions for command resolution and process setup
 ***************************************************************************/

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for child processes.
 *
 * This version of the function is tailored to commands that should be run in
 * the child process of a fork.
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void child_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case GENERIC:
    run_generic(cmd.generic);
    break;

  case ECHO:
    run_echo(cmd.echo);
    break;

  case PWD:
    run_pwd();
    break;

  case JOBS:
    run_jobs();
    break;

  case EXPORT:
  case CD:
  case KILL:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for the quash process.
 *
 * This version of the function is tailored to commands that should be run in
 * the parent process (quash).
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void parent_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case EXPORT:
    run_export(cmd.export);
    break;

  case CD:
    run_cd(cmd.cd);
    break;

  case KILL:
    run_kill(cmd.kill);
    break;

  case GENERIC:
  case ECHO:
  case PWD:
  case JOBS:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief Creates one new process centered around the @a Command in the @a
 * CommandHolder setting up redirects and pipes where needed
 *
 * @note Processes are not the same as jobs. A single job can have multiple
 * processes running under it. This function creates a process that is part of a
 * larger job.
 *
 * @note Not all commands should be run in the child process. A few need to
 * change the quash process in some way
 *
 * @param holder The CommandHolder to try to run
 *
 * @sa Command CommandHolder
 */
void create_process(CommandHolder holder, pidQue * parentPidQue) {
  // Read the flags field from the parser
  bool p_in  = holder.flags & PIPE_IN;
  bool p_out = holder.flags & PIPE_OUT;
  bool r_in  = holder.flags & REDIRECT_IN;
  bool r_out = holder.flags & REDIRECT_OUT;
  bool r_app = holder.flags & REDIRECT_APPEND; // This can only be true if r_out
                                               // is true

  // TODO: Remove warning silencers
  (void) p_in;  // Silence unused variable warning
  (void) p_out; // Silence unused variable warning
  (void) r_in;  // Silence unused variable warning
  (void) r_out; // Silence unused variable warning
  (void) r_app; // Silence unused variable warning
  
  int status;
  pid_t pid;
  int pPipe[2], cPipe[2];

  pipe(pPipe);
  pipe(cPipe);

  pid = fork();

  if (pid==0) {
    child_run_command(holder.cmd);
  } else if (pid > 0) {
    push_back_pidQue(parentPidQue, pid);
    parent_run_command(holder.cmd);
  } 

  
  // TODO: Setup pipes, redirects, and new process
  //IMPLEMENT_ME();

  //parent_run_command(holder.cmd); // This should be done in the parent branch of
                                  // a fork
  //child_run_command(holder.cmd); // This should be done in the child branch of a fork
}

// Run a list of commands
void run_script(CommandHolder* holders) {
  
  //Initialize the global myJobQue if uninitialized
  if (!jobQueInitialized) {
    myJobQue = new_jobQue(100);
    jobQueInitialized = true;
  }

  //Test Code
  // pidQue myPidQue = new_pidQue(100);
  // push_back_pidQue(&myPidQue, 1);
  // QuashJob currentJob = {.jobID = 1, .pids = &myPidQue, .cmd = "Hello World!"};
  // push_back_jobQue(&myJobQue, currentJob);
  // QuashJob myJob;
  // myJob = pop_front_jobQue(&myJobQue);
  // print_job(myJob.jobID, 1, myJob.cmd);
  
  if (holders == NULL)
    return;

  check_jobs_bg_status();

  if (get_command_holder_type(holders[0]) == EXIT &&
      get_command_holder_type(holders[1]) == EOC) {
    end_main_loop();
    return;
  }

  //Create an empty pidQue
  pidQue myPidQue;
  myPidQue = new_pidQue(100);

  CommandType type;

  // Run all commands in the `holder` array
  for (int i = 0; (type = get_command_holder_type(holders[i])) != EOC; ++i)
    create_process(holders[i], &myPidQue);

  //Creates a new QuashJob variabe to store the new job
  ++jobCount;
  QuashJob myJob = {.jobID = jobCount, .pids = myPidQue, .cmd = "Test String"}; //This is incorrect, need the string for the cmd

  if (!(holders[0].flags & BACKGROUND)) {
    // Not a background Job
    // TODO: Wait for all processes under the job to complete
    //IMPLEMENT_ME();

    int status;
    for (int i = 0; i < length_pidQue(&myPidQue); ++i) {
      pid_t currPid = pop_front_pidQue(&myPidQue);
      waitpid(currPid, &status, 0);
    }
    destroy_pidQue(&myPidQue);
  }

  else {
    // A background job.
    // TODO: Push the new job to the job queue
    //IMPLEMENT_ME();
    
    push_back_jobQue(&myJobQue, myJob); //Adds current job to the queue

    // TODO: Once jobs are implemented, uncomment and fill the following line
    print_job_bg_start(myJob.jobID, peek_front_pidQue(&myJob.pids), myJob.cmd);
  }
}
