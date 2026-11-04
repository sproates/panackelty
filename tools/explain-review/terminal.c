/* POSIX key and resize events for the review build.
 * Sends key/resize events as lines to an unmodified Panack executable. */
#define _DARWIN_C_SOURCE

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
static volatile sig_atomic_t stopping, resized;
static struct termios saved;
static int active;
static void restore(void) {
  if (active) { tcsetattr(STDIN_FILENO, TCSANOW, &saved); active=0;
    fputs("\033[0m\033[?25h\033[?1049l", stdout); fflush(stdout); }
}
static void signal_event(int sig) { if(sig==SIGWINCH) resized=1; else stopping=1; }
static int send_event(int fd,const char *s) {
  size_t n=strlen(s); while(n) { ssize_t k=write(fd,s,n); if(k<0) { if(errno==EINTR)continue; return -1; } s+=k;n-=(size_t)k; } return 0;
}
static int dimensions(int fd) {
  struct winsize size={0}; ioctl(STDOUT_FILENO,TIOCGWINSZ,&size);
  char line[64]; snprintf(line,sizeof(line),"resize\n%u\n%u\n",size.ws_col?size.ws_col:80,size.ws_row?size.ws_row:24);
  return send_event(fd,line);
}
int main(int argc,char **argv) {
  if(argc<2) { fprintf(stderr,"Usage: terminal PROGRAM [ARGUMENTS...]\n");return 2; }
  if(!isatty(0)||!isatty(1)) { fprintf(stderr,"Interactive explain needs a terminal. Omit --interactive for plain output.\n");return 2; }
  if(tcgetattr(0,&saved)<0) {perror("terminal settings");return 1;}
  int channel[2];if(pipe(channel)<0){perror("pipe");return 1;}
  struct sigaction action={0};action.sa_handler=signal_event;sigemptyset(&action.sa_mask);
  sigaction(SIGINT,&action,NULL);sigaction(SIGTERM,&action,NULL);sigaction(SIGHUP,&action,NULL);sigaction(SIGWINCH,&action,NULL);
  signal(SIGPIPE,SIG_IGN);
  /* A failed load may never draw its first frame. Replay errors after the
   * alternate screen closes so the caller can still read them. */
  FILE *errors = tmpfile();
  if (!errors) { perror("explain diagnostics"); close(channel[0]); close(channel[1]); return 1; }
  pid_t child=fork();
  if(child<0){perror("fork");close(channel[0]);close(channel[1]);return 1;}
  if(child==0) {
    close(channel[1]);
    if (dup2(channel[0], STDIN_FILENO) < 0 || dup2(fileno(errors), STDERR_FILENO) < 0) _exit(126);
    close(channel[0]); fclose(errors);
    signal(SIGINT,SIG_DFL);signal(SIGTERM,SIG_DFL);signal(SIGHUP,SIG_DFL);signal(SIGWINCH,SIG_DFL);signal(SIGPIPE,SIG_DFL);
    execvp(argv[1],argv+1);perror("explain launch");_exit(127);
  }
  close(channel[0]);atexit(restore);
  struct termios raw=saved;raw.c_lflag&=(tcflag_t)~(ICANON|ECHO|ISIG);raw.c_iflag&=(tcflag_t)~(IXON|ICRNL);raw.c_cc[VMIN]=1;raw.c_cc[VTIME]=0;
  if(tcsetattr(0,TCSANOW,&raw)<0){perror("raw mode");kill(child,SIGTERM);close(channel[1]);waitpid(child,NULL,0);return 1;}
  active=1;fputs("\033[?1049h\033[?25lChecking project and collecting compiler decisions...",stdout);fflush(stdout);
  int status=0, reaped=0;dimensions(channel[1]);
  while(!stopping) {
    pid_t result=waitpid(child,&status,WNOHANG);if(result==child){reaped=1;break;}
    if(resized){resized=0;if(dimensions(channel[1])<0)break;}
    struct pollfd poller={0,POLLIN,0};int ready=poll(&poller,1,100);
    if(ready<0){if(errno==EINTR)continue;break;} if(!ready)continue;
    if(poller.revents&(POLLHUP|POLLERR|POLLNVAL))break;
    unsigned char key; if(read(0,&key,1)!=1)break;
    const char *event=NULL;
    if(key==27) { /* Read a bounded escape sequence; never block indefinitely. */
      char seq[12]={0};size_t n=0;
      while(n<sizeof(seq)-1 && poll(&poller,1,40)>0) {
        if(read(0,seq+n,1)!=1)break;n++;
        if(n>=2 && seq[n-1]>='A' && seq[n-1]<='Z')break;
        if(n>=2 && seq[n-1]=='~')break;
      }
      if(!strcmp(seq,"[A")||!strcmp(seq,"OA"))event="up\n";
      else if(!strcmp(seq,"[B")||!strcmp(seq,"OB"))event="down\n";
      else if(!strcmp(seq,"[C")||!strcmp(seq,"OC"))event="open\n";
      else if(!strcmp(seq,"[D")||!strcmp(seq,"OD"))event="close\n";
      else if(!strcmp(seq,"[5~"))event="pageup\n";
      else if(!strcmp(seq,"[6~"))event="pagedown\n";
      else if(!n)event="back\n";
    } else {
      switch(key){case 3:case 4:case 'q':event="quit\n";break;case 'k':event="up\n";break;
      case 'j':event="down\n";break;case 'l':case '\r':case '\n':case ' ':event="open\n";break;
      case 'h':event="close\n";break;case 'b':case 127:event="back\n";break;
      case 'o':event="open\n";break;
      case 'd':event="pagedown\n";break;case 'u':event="pageup\n";break;}
    }
    if(event && send_event(channel[1],event)<0)break;
    if(event && !strcmp(event,"quit\n"))break;
  }
  close(channel[1]);
  if(!reaped) { /* EOF/quit normally ends the child; bound shutdown if it fails. */
    for(int i=0;i<20;i++){if(waitpid(child,&status,WNOHANG)==child){reaped=1;break;}poll(NULL,0,25);}
    if(!reaped){kill(child,SIGTERM);waitpid(child,&status,0);}
  }
  restore();
  rewind(errors);
  char message[1024];
  size_t n;
  while ((n = fread(message, 1, sizeof(message), errors)) != 0) fwrite(message, 1, n, stderr);
  fclose(errors);
  return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}
