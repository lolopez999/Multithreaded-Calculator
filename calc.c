 /* Project 2 - Multithreaded calculator */
// Name:Andres Lopez


#include "calc.h"

pthread_t adderThread;
pthread_t degrouperThread;
pthread_t multiplierThread;
pthread_t readerThread;
pthread_t sentinelThread;

char buffer[BUF_SIZE];
int num_ops;
int beforelen;
int process = 1;
int addprogress = 1;
int multiprogress = 1;
int degprogress = 1;

static pthread_mutex_t buffer_lock;

char *int2string(int i, char *s)
{
    sprintf(s, "%d", i);
    return s;
}

int string2int(const char *s)
{
    return atoi(s);
}

int isNumeric(char c)
{
    return isdigit(c);
}

void printErrorAndExit(char *msg)
{
    msg = msg ? msg : "An error occured!";
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

int timeToFinish()
{
    return buffer[0] == '.';
}

/* Looks for an addition symbol "+" surrounded by two numbers, e.g. "5+6"
   and, if found, adds the two numbers and replaces the addition subexpression
   with the result ("(5+6)*8" becomes "(11)*8")--remember, you don't have
   to worry about associativity! */
void *adder(void *arg)
{
    int bufferlen;
    int value1, value2;
    int startOffset, remainderOffset;
    int i;
    int result;
    char nString[50];

    while (1) {
      addprogress = 1;
	startOffset = remainderOffset = -1;
	value1 = value2 = -1;

pthread_mutex_lock(&buffer_lock);

	if (timeToFinish())
  {
    pthread_mutex_unlock(&buffer_lock);

	    return NULL;
	}

	bufferlen = (int) strlen(buffer);

	for (i = 0; i < bufferlen; i++) {
beforelen = bufferlen;

  if (buffer[i] == ';') {
    break;
  }
	 if (isdigit(buffer[i])) {

     if(buffer[i] == '+' && buffer[i + 1] == '(') {
      i = i + 2;
     }
     startOffset = i;

     value1 = string2int(buffer + i);

     while (isdigit(buffer[i])) {
       i++;
     }

     if (buffer[i] != 'x' || !isdigit(buffer[i + 1])) {
       continue;
     }
     value2 = string2int(buffer + i + 1);
     result = value1 + value2;
     do {
       i++;

     }
     while (isdigit(buffer[i]));
     remainderOffset = i;
int2string(result, nString);
strcpy(buffer + startOffset,  nString);
strcpy((buffer + startOffset + strlen(nString)), (buffer + remainderOffset));

 bufferlen = strlen(buffer);
 i = startOffset + (strlen(nString)) - 1;
 num_ops++;

   }
	}
if (strlen(nString) == 0 && bufferlen > 0) {
  addprogress = 0;
}

pthread_mutex_unlock(&buffer_lock);
sched_yield();

    }
}

/* Looks for a multiplication symbol "*" surrounded by two numbers, e.g.
   "5*6" and, if found, multiplies the two numbers and replaces the
   mulitplication subexpression with the result ("1+(5*6)+8" becomes
   "1+(30)+8"). */
void *multiplier(void *arg)
{
    int bufferlen;
    int value1, value2;
    int startOffset, remainderOffset;
    int i;
    int result;
    char nString[50];

    while (1) {

  multiprogress = 1;
	startOffset = remainderOffset = -1;
	value1 = value2 = -1;

pthread_mutex_lock(&buffer_lock);

	if (timeToFinish()) {
    pthread_mutex_unlock(&buffer_lock);
	    return NULL;
	}

	bufferlen = strlen(buffer);

	for (i = 0; i < bufferlen; i++) {
	    beforelen = bufferlen;
      if (buffer[i] == ';') {
       break;
     }
      if (isdigit(buffer[i])) {
       if(buffer[i] == '*' && buffer[i + 1] == '(') {
         i = i + 2;
       }
       startOffset = i;
       value1 = atoi(buffer + i);
       while (isdigit(buffer[i])) {
         i++;
       }
       if (buffer[i] != '*' || !isdigit(buffer[i + 1])) {
         continue;
       }
       value2 = atoi(buffer + i + 1);
       result = value1 * value2;
       do {
         i++;
       }
        while (isdigit(buffer[i]));

       remainderOffset = i;
       sprintf(nString, "%d", result);
       strcpy(buffer + startOffset, nString);
       strcpy((buffer + startOffset + strlen(nString)), (buffer + remainderOffset));
       bufferlen = (int) strlen(buffer);
       i = startOffset + ((int) strlen(nString)) - 1;
       num_ops++;
     }
   }
   if(strlen(nString) == 0 && bufferlen > 0) {
     multiprogress = 0;
   }
   pthread_mutex_unlock(&buffer_lock);
   sched_yield();
 }
}
/* Looks for a number immediately surrounded by parentheses [e.g.
   "(56)"] in the buffer and, if found, removes the parentheses leaving
   only the surrounded number. */
void *degrouper(void *arg)
{
    int bufferlen;
    int i;
    int startOffset = 0;

    while (1) {
		    degprogress = 1;
        pthread_mutex_lock(&buffer_lock);

	if (timeToFinish()) {
    pthread_mutex_unlock(&buffer_lock);

	    return NULL;
	}
bufferlen = (int) strlen(buffer);
int naked = 1;
		for (i = 0; i < bufferlen; i++) {

beforelen = bufferlen;
if(naked == 0) {
  break;
}
if (buffer[i] == ';') {
  break;
}
int j = i;
if(bufferlen > 0)
while(j < bufferlen) {
 if(buffer[j] == '(') {
   i = j;
 }
 j ++;
}

if (buffer[i] == '(' && isdigit(buffer[i + 1])) {

 startOffset = i;

 while (buffer[i] != ')') {
   i ++;
   if(buffer[i] == '+' || buffer[i] == '*') {
     naked = 0;
     break;
   }
 }
 if(naked == 0)
   continue;

 strcpy((buffer + i), (buffer + i + 1));
 strcpy((buffer + startOffset), (buffer + startOffset + 1));

 bufferlen -= 2;
 i = startOffset;
 num_ops++;

}
}
if(beforelen == bufferlen && bufferlen > 0) {
degprogress = 0;
}
pthread_mutex_unlock(&buffer_lock);
sched_yield();

}
		}

/* sentinel waits for a number followed by a ; (e.g. "453;") to appear
   at the beginning of the buffer, indicating that the current
   expression has been fully reduced by the other threads and can now be
   output.  It then "dequeues" that expression (and trailing ;) so work can
   proceed on the next (if available). */
void *sentinel(void *arg)
{
    char numberBuffer[20];
    int bufferlen;
    int i;

    while (1) {
pthread_mutex_lock(&buffer_lock);

	if (timeToFinish()) {
    pthread_mutex_unlock(&buffer_lock);
	    return NULL;
	}

  if(addprogress == 0 && multiprogress == 0 && degprogress == 0) {
    printf("No progress can be made\n");
    exit(EXIT_FAILURE);
  }
		bufferlen = strlen(buffer);

	for (i = 0; i < bufferlen; i++) {
	    if (buffer[i] == ';') {
		if (i == 0) {
		    printErrorAndExit("Sentinel found empty expression!");
		} else {
		    numberBuffer[i] = '\0';
		    fprintf(stdout, "%s\n", numberBuffer);
		    strcpy(buffer, & buffer[i + 1]);
		    break;
		}
	    } else if (!isNumeric(buffer[i])) {
		break;
	    } else {
		numberBuffer[i] = buffer[i];
	    }
	}
pthread_mutex_unlock(&buffer_lock);

sched_yield();

    }
}
void *reader(void *arg)
{
    while (1) {
	char tBuffer[100];
	int currentlen;
	int newlen;
	int free;

	fgets(tBuffer, sizeof(tBuffer), stdin);


	newlen = strlen(tBuffer);
	currentlen = strlen(buffer);

	if (tBuffer[newlen - 1] == '\n') {

	    tBuffer[newlen - 1] = tBuffer[newlen];
	    newlen--;
	}
	free = sizeof(buffer) - currentlen - 2;

	while (free < newlen) {

    pthread_mutex_lock(&buffer_lock);

    currentlen = strlen(buffer);
    free = sizeof(buffer) - currentlen - 2;

pthread_mutex_unlock(&buffer_lock);
        sched_yield();
	}
pthread_mutex_lock(&buffer_lock);
	strcat(buffer, tBuffer);
	strcat(buffer, ";");

pthread_mutex_unlock(&buffer_lock);
sched_yield();

	if (tBuffer[0] == '.') {
	    return NULL;
	}
}
}
int smp3_main(int argc, char **argv){
    void *arg = 0;
    if (pthread_mutex_init(&buffer_lock, NULL) != 0){
          printErrorAndExit("Failed");
    }
 if (pthread_create(&degrouperThread, NULL, degrouper, arg)
  || pthread_create(&adderThread, NULL, adder, arg)
  || pthread_create(&multiplierThread, NULL, multiplier, arg)
  || pthread_create(&sentinelThread, NULL, sentinel, arg)
  || pthread_create(&readerThread, NULL, reader, arg)) {
  printErrorAndExit("Failed ");
    }
    pthread_join(sentinelThread, NULL);
    pthread_detach(degrouperThread);
    pthread_detach(multiplierThread);
    pthread_detach(adderThread);
    pthread_detach(sentinelThread);
    pthread_detach(readerThread);
fprintf(stdout, "Performed a total of %d operations\n", num_ops);
pthread_mutex_destroy(&buffer_lock);
    return EXIT_SUCCESS;
}
