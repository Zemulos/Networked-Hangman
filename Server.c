 /* Network server for hangman game */
 /* File: hangserver.c */

 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <stdio.h>
 #include <syslog.h>
 #include <signal.h>
 #include <errno.h>
 #include <sys/types.h>
 #include <unistd.h>
 #include <sys/wait.h>>

 extern time_t time ();

 int maxlives = 12;
 char *word [] = {
 # include "words"
 };
 # define NUM_OF_WORDS (sizeof (word) / sizeof (word [0]))
 # define MAXLEN 80 /* Maximum size in the world of Any string */
 # define HANGMAN_TCP_PORT 1066


 play_hangman (int in, int out);

 int main ()
 {
 	int sock, fd;
  socklen_t client_len;
 	struct sockaddr_in server, client;
  int childProcessesActive = 0;   //Keep Track of child processes

 	srand ((int) time ((long *) 0)); /* randomize the seed */

 	sock = socket (AF_INET, SOCK_STREAM, 0);//0 or IPPROTO_TCP
 	if (sock <0) { //This error checking is the code Stevens wraps in his Socket Function etc
 		perror ("creating stream socket");
 		exit (1);
 	}

 	server.sin_family = AF_INET;
 	server.sin_addr.s_addr = htonl(INADDR_ANY);
 	server.sin_port = htons(HANGMAN_TCP_PORT);

 	if (bind(sock, (struct sockaddr *) & server, sizeof(server)) <0) {
 		perror ("binding socket");
	 	exit (2);
 	}

 	listen (sock, 5);

 	while (1) {
 		client_len = sizeof (client);
   
   
 		if ((fd = accept (sock, (struct sockaddr *) &client, &client_len)) <0) {
 			perror ("accepting connection");
 			exit (3);
 		}
   
   pid_t procID = fork(); //Catch return value of fork
   
   if(procID < 0) { //Fork Error
       perror("Error FOrking");
       exit(-1);
   }
   else if(procID == 0) { //Check if child process
    
        close(sock);     //Child does not need parents socket
        play_hangman(fd,fd); //Child server has game logic
        exit(0);             //Close child process
   }
   
   printf("Current Child process: %d\n", procID);
   close(fd);                 //Parent closes child file des
   childProcessesActive++;    //Keep track of number of childs
   
   
   
   //Clean up zombie child processes
   while(childProcessesActive) {
   
    
      printf("Current Child process: %d\n", procID);
      procID = waitpid((pid_t) -1, NULL, WNOHANG); //Non blocking wait
    
      if(procID < 0){
         perror("Error Forking");
         exit(-1);
       }
       else if(procID == 0) {
          break; //Clean up complete
       }
       else{
           childProcessesActive--;   //Child process finished
        
       }
   }
  }
 }

 /* ---------------- Play_hangman () ---------------------*/

 play_hangman (int in, int out)
 {
 	char * whole_word, part_word [MAXLEN],
 	guess[MAXLEN], outbuf [MAXLEN];

 	int lives = maxlives;
 	int game_state = 'I';//I = Incomplete
 	int i, good_guess, word_length;
 	char hostname[MAXLEN];

 	gethostname (hostname, MAXLEN);
 	sprintf(outbuf, "Playing hangman on host% s: \n \n", hostname);
 	write(out, outbuf, strlen (outbuf));

 	/* Pick a word at random from the list */
 	whole_word = word[rand() % NUM_OF_WORDS];
 	word_length = strlen(whole_word);
 	syslog (LOG_USER | LOG_INFO, "server chose hangman word %s", whole_word);

 	/* No letters are guessed Initially */
 	for (i = 0; i <word_length; i++)
 		part_word[i]='-';
 	
	part_word[i] = '\0';

 	sprintf (outbuf, "%s %d \n", part_word, lives);
 	write (out, outbuf, strlen(outbuf));

 	while (game_state == 'I')
 	/* Get a letter from player guess */
 	{
		while (read (in, guess, MAXLEN) <0) {
 			if (errno != EINTR)
 				exit (4);
 			printf ("re-read the startin \n");
 			} /* Re-start read () if interrupted by signal */
 	good_guess = 0;
 	for (i = 0; i <word_length; i++) {
 		if (guess [0] == whole_word [i]) {
 		good_guess = 1;
 		part_word [i] = whole_word [i];
 		}
 	}
 	if (! good_guess) lives--;
 	if (strcmp (whole_word, part_word) == 0)
 		game_state = 'W'; /* W ==> User Won */
 	else if (lives == 0) {
 		game_state = 'L'; /* L ==> User Lost */
 		strcpy (part_word, whole_word); /* User Show the word */
 	}
 	sprintf (outbuf, "%s %d \n", part_word, lives);
 	write (out, outbuf, strlen (outbuf));
 	}
 }