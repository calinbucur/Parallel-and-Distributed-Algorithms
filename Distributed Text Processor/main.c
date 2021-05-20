// Bucur Calin-Andrei
// 332CB
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "mpi.h"

// Strings used for checking if a character is a consonant
char* cons = "qwrtypsdfghjklzxcvbnm";
char* big_cons = "QWRTYPSDFGHJKLZXCVBNM";

char* in_file; // Input file
long P; // Max number of threads a worker can start
int lines; // Number of lines in the paragraph
char ** par; // The paragraph
int thread_num; // Number of threads necessary to process the paragraph
int parags; // Number of paragraphs

// Thread function
// Reads the file and sends each pargraph to the corresponding worker
void* sender(void* arg) {
	// Id indicates the genre the thread is responsible for
	int id = *(int*)arg;
	int nr = 0; // Counts the number of pragraphs in the file
	// Get the genre name
	char* genre;
	if(id == 0) {
		genre = "horror";
	}
	else if(id == 1) {
		genre = "comedy";
	}
	else if(id == 2) {
		genre = "fantasy";
	}
	else {
		genre = "science-fiction";
	}
	FILE *in = fopen(in_file, "r"); // Open the input file
	char* gen = malloc(20 * sizeof(char));
	// Read the whole file
	while(fscanf(in, "%s", gen) != EOF) {
		// If I find a pargraph of the right genre
		if(strcmp(gen, genre) == 0) {
			nr++; // Increment the pragraph number
			char** par = malloc(2500 * sizeof(char*));
			for(int i = 0; i < 2500; i++) {
				par[i] = calloc(1800, sizeof(char));
			}
			char* line  = calloc(1800, sizeof(char));
			size_t line_size  = 1800;
			int crt = 0;
			getline(&line, &line_size, in);
			// Read line by line
			// Until I reach an empty line (end of paragraph)
			while ((getline(&line, &line_size, in) >= 0) && (strcmp(line, "\n") != 0)){        		
        		// Add it to the paragraph array
        		strcpy(par[crt], line);
        		crt++;
    		}
    		// Send the number of lines to the corresponding worker
    		// The tag indicates the paragraphs order in the file
    		MPI_Send(&crt, 1, MPI_INT, id + 1, nr, MPI_COMM_WORLD);
    		// Send the paragraph line by line to the worker
    		for(int i = 0; i < crt; i++) {
    			int len = strlen(par[i]);
    			MPI_Send(&len, 1, MPI_INT, id + 1, 0, MPI_COMM_WORLD);
    			MPI_Send(par[i], len + 1, MPI_CHAR, id + 1, 0, MPI_COMM_WORLD);
    		}
		}
		// Else just increment the pragraph number
		else if(strcmp(gen, "horror") == 0 || strcmp(gen, "comedy") == 0
			|| strcmp(gen, "fantasy") == 0 || strcmp(gen, "science-fiction") == 0) {
			nr++;
		}
	}
	// Set the total number of paragraphs
	parags = nr;
	// This signals the worker to stop waiting for paragraphs
	int kill = -1;
	MPI_Send(&kill, 1, MPI_INT, id + 1, 0, MPI_COMM_WORLD);
	free(gen);
	fclose(in);
}

// Thread function for processing horror paragraphs
void* horror(void* arg) {
	int id = *(int*)arg;
	// Get the lines the thread is responsible for
	int start = 20 * id;
	int end = 20 * (id + 1);
	if(lines < end) {
		end = lines;
	}
	// When it's done, if the paragraph was too long to be covered by the threads
	// reassigns a new zone
	do {
		char *crt = calloc(1800, sizeof(char));
		for(int i = start; i < end; i++) {
			strcpy(crt, par[i]); // Auxiliary
			int it = 0;
			// Go through the line
			for(int j = 0; j < strlen(crt); j++) {
				// If it's a lowercase consonant add it twice
				if(strchr(cons, crt[j])) {
					par[i][it] = crt[j];
					par[i][it + 1] = crt[j];
					it += 2;
				}
				// If it's a upperrcase consonant add it uppercase then lowercase
				else if(strchr(big_cons, crt[j])) {
					par[i][it] = crt[j];
					par[i][it + 1] = tolower(crt[j]);
					it += 2;
				}
				// Else just add it
				else {
					par[i][it] = crt[j];
					it++;
				}
				par[i][it] = 0;
			}
		}
		free(crt);
		// Calculate the new zone
		start += 20 * thread_num;
		end = start + 20;
		if(lines < end) {
			end = lines;
		}
	} while(start < lines);
}

// Thread function for processing comedy paragraphs
void* comedy(void* arg) {
	int id = *(int*)arg;
	int start = 20 * id;
	int end = 20 * (id + 1);
	if(lines < end) {
		end = lines;
	}
	do {
		char *crt = calloc(1800, sizeof(char));
		for(int i = start; i < end; i++) {
			strcpy(crt, par[i]);
			int it = 0;
			int even = 0; // Indicates if the position in the word is even
			for(int j = 0; j < strlen(crt); j++) {
				// If it's a lowercase letter in a even position
				// Make it uppercase and change the even flag 
				if(crt[j] >= 97 && crt[j] <= 122 && even) {
					par[i][it] = toupper(crt[j]);
					it++;
					even = !even;
				}
				// If it's a space reset the even flag
				else if(crt[j] == ' ') {
					par[i][it] = crt[j];
					it++;
					even = 0;
				}
				// Else just leave it be and change the even flag
				else {
					par[i][it] = crt[j];
					it++;
					even = !even;
				}
				par[i][it] = 0;
			}
		}
		free(crt);
		start += 20 * thread_num;
		end = start + 20;
		if(lines < end) {
			end = lines;
		}
	} while(start < lines);
}

// Thread function for processing fantasy paragraphs
void* fantasy(void* arg) {
	int id = *(int*)arg;
	int start = 20 * id;
	int end = 20 * (id + 1);
	if(lines < end) {
		end = lines;
	}
	do {
		char *crt = calloc(1800, sizeof(char));
		for(int i = start; i < end; i++) {
			strcpy(crt, par[i]);
			int it = 0;
			int word = 1; // Indicates if we are at the start of a word
			for(int j = 0; j < strlen(crt); j++) {
				// If we are at the begining of a word
				// Make the character uppercase
				// And make the word flag false
				if(word) {
					par[i][it] = toupper(crt[j]);
					it++;
					word = 0;
				}
				// If we are at a space
				// That means a new word may follow
				// So make the word flag true
				else if(crt[j] == ' ') {
					par[i][it] = crt[j];
					it++;
					word = 1;
				}
				// Else just leave it be
				else {
					par[i][it] = crt[j];
					it++;
				}
				par[i][it] = 0;
			}
		}
		free(crt);
		start += 20 * thread_num;
		end = start + 20;
		if(lines < end) {
			end = lines;
		}
	} while(start < lines);
}

// Auxiliary function for reversing
char* aux_sci_fi(char* crt) {
	char* rev = calloc(1800, sizeof(char));
	int it = 0;
	// Place the pointer at the end of the word
	for(int i = 0; i < strlen(crt); i++) {
		if(crt[i] == ' ' || crt[i] == '\n') {
			break;
		}
		else {
			it++;
		}
	}
	// Go backwards to get the reverse word
	for(int i = it - 1; i >= 0; i--) {
		rev[strlen(rev)] = crt[i];
	}
	return rev;
}

// Thread function for processing science-fiction paragraphs
void* sci_fi(void* arg) {
	int id = *(int*)arg;
	int start = 20 * id;
	int end = 20 * (id + 1);
	if(lines < end) {
		end = lines;
	}
	do {
		char *crt = calloc(1800, sizeof(char));
		for(int i = start; i < end; i++) {
			strcpy(crt, par[i]);
			//printf("%s\n", crt);
			int it = 0;
			int word = 1; // Word counter
			for(int j = 0; j < strlen(crt); j++) {
				// If we are at the 7th word
				if(word == 7) {
					// Reset the counter
					word = 0;
					// Reverse it
					char* rev = aux_sci_fi(crt + j);
					// And add it to the line
					strncat(par[i], rev, strlen(rev));
					it += strlen(rev);
					j += strlen(rev) - 1;
					free(rev);
				} else {
					// If we meet a space increment the counter 
					if(crt[j] == ' ') {
						par[i][it] = crt[j];
						it++;
						word++;
					}
					else {
						par[i][it] = crt[j];
						it++;
					}
					par[i][it] = 0;
				}
			}
		}
		free(crt);
		start += 20 * thread_num;
		end = start + 20;
		if(lines < end) {
			end = lines;
		}
	} while(start < lines);
}

// Thread function that receives paragraphs from the master thread
// And sends them to the processing threads
void* reader(void* arg) {
	int id = *(int*)arg;
	int nr;
	void* proc;
	// Get the corresponding function
	if(id == 1) {
		proc = horror;
	}
	else if(id == 2) {
		proc = comedy;
	}
	else if(id == 3) {
		proc = fantasy;
	}
	else {
		proc = sci_fi;
	}
	while(1) {
   		MPI_Status status;
   		// Receive the number of lines of the paragraph
   		MPI_Recv(&lines, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
   		nr = status.MPI_TAG; // This is the order of the pargraph in the file
   		// If the number of lines is negative 
   		// This is the signal to stop
   		if(lines < 0) {
   			break;
   		}
   		par = malloc(lines * sizeof(char*));
   		for(int i = 0; i < lines; i++) {
			par[i] = calloc(1800, sizeof(char));
		}
		// Receive the paragraph line by line
		for(int i = 0; i < lines; i++) {
			int len;
			MPI_Recv(&len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
			MPI_Recv(par[i], len + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
		}
		// Determine the number of threads we are going to use
		thread_num = (lines - 1) / 20 + 1;
		if(thread_num > P - 1) {
			thread_num = P - 1;
		}
		int i, r;
		void* stat;
		pthread_t threads[thread_num];
		int arguments[thread_num];

		// Start the processing threads
		for (i = 0; i < thread_num; i++) {
			arguments[i] = i;
			r = pthread_create(&threads[i], NULL, proc, &arguments[i]);

			if (r) {
				printf("Eroare la crearea thread-ului %d\n", i);
				exit(-1);
			}
		}

		// Wait for the processing threads
		for (i = 0; i < thread_num; i++) {
			r = pthread_join(threads[i], &stat);
			if (r) {
				printf("Eroare la asteptarea thread-ului %d\n", i);
				exit(-1);
			}
		}
		// Send the number of lines to the master node
		// Send it with a tag indicating it's order
		MPI_Send(&lines, 1, MPI_INT, 0, nr, MPI_COMM_WORLD);
		// Send the paragraph to the master node line by line
		for(int i = 0; i < lines; i++) {
			int len = strlen(par[i]);
    		MPI_Send(&len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			MPI_Send(par[i], len + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
		}
   	}
}

int main(int argc, char* argv[]) {

	// Get the maximum number of threads
	P = sysconf(_SC_NPROCESSORS_CONF);

	if(argc != 2) {
		return 1;
	}
	else {
		// Get the input file
		in_file = argv[1];
	}


	// Initialize stuff
	int provided;
	int  numtasks, rank, len;
    char hostname[MPI_MAX_PROCESSOR_NAME];
	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Get_processor_name(hostname, &len);

    // The main node
    if(rank == 0) {
    	int i, r;
		void* status;
		pthread_t threads[4];
		int arguments[4];

		// Start the 4 threads for reading the file
		for (i = 0; i < 4; i++) {
			arguments[i] = i;
			r = pthread_create(&threads[i], NULL, sender, &arguments[i]);

			if (r) {
				printf("Eroare la crearea thread-ului %d\n", i);
				exit(-1);
			}
		}

		// Wait for them
		for (i = 0; i < 4; i++) {
			r = pthread_join(threads[i], &status);
			if (r) {
				printf("Eroare la asteptarea thread-ului %d\n", i);
				exit(-1);
			}
		}

		// Determine the output file
		char* point = strstr(in_file, ".txt");
		char* out_file = calloc(20, sizeof(char));
		strncpy(out_file, in_file, point - in_file + 1);
		strcat(out_file, "out");

		FILE* out = fopen(out_file, "w");
		// Receive the processed in the order they were in the input file
		// Order is determined by the message tag
		for(int j = 1; j <= parags; j++) {
			MPI_Status stat;
			// Receive the number of lines of the paragraph
			MPI_Recv(&lines, 1, MPI_INT, MPI_ANY_SOURCE, j, MPI_COMM_WORLD, &stat);
			int src = stat.MPI_SOURCE;
			char* x = calloc(1800, sizeof(char));
			// Depending on the source print the pragraph name
			if(src == 1) {
				fprintf(out, "horror\n");
			}
			else if(src == 2) {
				fprintf(out, "comedy\n");
			}
			else if(src == 3) {
				fprintf(out, "fantasy\n");
			}
			else {
				fprintf(out, "science-fiction\n");
			}
			// receive and print the paragraph line by line
			for(int k = 0; k < lines; k++) {
				int leng;
				MPI_Recv(&leng, 1, MPI_INT, src, 0, MPI_COMM_WORLD, &stat);
				MPI_Recv(x, leng + 1, MPI_CHAR, src, 0, MPI_COMM_WORLD, &stat);
				fprintf(out, "%s", x);
			}
			fprintf(out, "\n");
		}
		fprintf(out, "\n");
    }
    // Worker processes just start the reader thread
    else {
		int i, r;
		void* status;
		pthread_t threads[1];
		int arguments[1];

		arguments[0] = rank;
		r = pthread_create(&threads[0], NULL, reader, &arguments[0]);
		if (r) {
			printf("Eroare la crearea thread-ului %d\n", 0);
			exit(-1);
		}
		r = pthread_join(threads[0], &status);
		if (r) {
			printf("Eroare la asteptarea thread-ului %d\n", 0);
			exit(-1);
		}
    }

    MPI_Finalize();
    return 0;
}