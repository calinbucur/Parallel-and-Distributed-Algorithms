/*
 * APD - Tema 1
 * Octombrie 2020
 * Bucur Calin-Andrei 332CB
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

char* in_filename_julia;
char* in_filename_mandelbrot;
char* out_filename_julia;
char* out_filename_mandelbrot;
int P; //Numarul de thread-uri pe care voi rula
int** result;
int width, height;

pthread_barrier_t barrier;

// structura pentru un numar complex
typedef struct _complex {
	double a;
	double b;
} complex;

// structura pentru parametrii unei rulari
typedef struct _params {
	int is_julia, iterations;
	double x_min, x_max, y_min, y_max, resolution;
	complex c_julia;
} params;

params par;

// citeste argumentele programului
void get_args(int argc, char** argv)
{
	if (argc < 6) {
		printf("Numar insuficient de parametri:\n\t"
			"./tema1 fisier_intrare_julia fisier_iesire_julia "
			"fisier_intrare_mandelbrot fisier_iesire_mandelbrot Numar_threads\n");
		exit(1);
	}

	in_filename_julia = argv[1];
	out_filename_julia = argv[2];
	in_filename_mandelbrot = argv[3];
	out_filename_mandelbrot = argv[4];
	P = atoi(argv[5]);
}

// citeste fisierul de intrare
void read_input_file(char* in_filename, params* par)
{
	FILE* file = fopen(in_filename, "r");
	if (file == NULL) {
		printf("Eroare la deschiderea fisierului de intrare!\n");
		exit(1);
	}

	fscanf(file, "%d", &par->is_julia);
	fscanf(file, "%lf %lf %lf %lf",
		&par->x_min, &par->x_max, &par->y_min, &par->y_max);
	fscanf(file, "%lf", &par->resolution);
	fscanf(file, "%d", &par->iterations);

	if (par->is_julia) {
		fscanf(file, "%lf %lf", &par->c_julia.a, &par->c_julia.b);
	}

	fclose(file);
}

// scrie rezultatul in fisierul de iesire
void write_output_file(char* out_filename, int** result, int width, int height)
{
	int i, j;

	FILE* file = fopen(out_filename, "w");
	if (file == NULL) {
		printf("Eroare la deschiderea fisierului de iesire!\n");
		return;
	}

	fprintf(file, "P2\n%d %d\n255\n", width, height);
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			fprintf(file, "%d ", result[i][j]);
		}
		fprintf(file, "\n");
	}

	fclose(file);
}

// aloca memorie pentru rezultat
int** allocate_memory(int width, int height)
{
	int** result;
	int i;

	result = malloc(height * sizeof(int*));
	if (result == NULL) {
		printf("Eroare la malloc!\n");
		exit(1);
	}

	for (i = 0; i < height; i++) {
		result[i] = malloc(width * sizeof(int));
		if (result[i] == NULL) {
			printf("Eroare la malloc!\n");
			exit(1);
		}
	}

	return result;
}

// elibereaza memoria alocata
void free_memory(int** result, int height)
{
	int i;

	for (i = 0; i < height; i++) {
		free(result[i]);
	}
	free(result);
}

void* J_and_M(void* arg)
{
	int thread_id = *(int*)arg;

	// Calculez zona din matrice ce va fi acoperita de fiecare thread
	int start = thread_id * (double)width / P;
	int end = (thread_id + 1) * (double)width / P;
	if (width < end) {
		end = width;
	}

	// Ruleaza algoritmul Julia paralelizat
	// Fiecare thread se ocupa de o portiune din latimea matricei
	int w, h, i;
	for (w = start; w < end; w++) {
		for (h = 0; h < height; h++) {
			int step = 0;
			complex z = { .a = w * par.resolution + par.x_min,
							.b = h * par.resolution + par.y_min };

			while (sqrt(pow(z.a, 2.0) + pow(z.b, 2.0)) < 2.0 && step < par.iterations) {
				complex z_aux = { .a = z.a, .b = z.b };

				z.a = pow(z_aux.a, 2) - pow(z_aux.b, 2) + par.c_julia.a;
				z.b = 2 * z_aux.a * z_aux.b + par.c_julia.b;

				step++;
			}

			result[h][w] = step % 256;
		}
	}

	// Astept ca toate thread-urile sa termine de populat matricea
	pthread_barrier_wait(&barrier);

	// Reinitializez start si end
	// Fiecare thread se va ocupa de o portiune din prima jumatate a matricei
	// si de portiunea corespunzatoare din a doua jumatate
	int N = height / 2;
	start = thread_id * (double)N / P;
	end = (thread_id + 1) * (double)N / P;
	if (N < end)
		end = N;

	// transforma rezultatul din coordonate matematice in coordonate ecran
	for (i = start; i < end; i++) {
		int* aux = result[i];
		result[i] = result[height - i - 1];
		result[height - i - 1] = aux;
	}

	// Thread-ul 0 se va ocupa de printare si pregatire pentru Mandelbrot
	// Nu cred ca acestea pot fi paralelizate
	if (thread_id == 0) {
		write_output_file(out_filename_julia, result, width, height);
		free_memory(result, height);

		read_input_file(in_filename_mandelbrot, &par);

		width = (par.x_max - par.x_min) / par.resolution;
		height = (par.y_max - par.y_min) / par.resolution;

		result = allocate_memory(width, height);
	}

	// Restul thread-urilor asteapta sa termine thread-ul 0 de reinitializat matricea
	pthread_barrier_wait(&barrier);

	// Calculez zona din matrice de care se va ocupa fiecare thread
	start = thread_id * (double)width / P;
	end = (thread_id + 1) * (double)width / P;
	if (width < end) {
		end = width;
	}

	// Ruleaza algoritmul Mandelbrot paralelizat
	// Fiecare thread se ocupa de o portiune din latimea matricei
	for (w = start; w < end; w++) {
		for (h = 0; h < height; h++) {
			complex c = { .a = w * par.resolution + par.x_min,
							.b = h * par.resolution + par.y_min };
			complex z = { .a = 0, .b = 0 };
			int step = 0;

			while (sqrt(pow(z.a, 2.0) + pow(z.b, 2.0)) < 2.0 && step < par.iterations) {
				complex z_aux = { .a = z.a, .b = z.b };

				z.a = pow(z_aux.a, 2.0) - pow(z_aux.b, 2.0) + c.a;
				z.b = 2.0 * z_aux.a * z_aux.b + c.b;

				step++;
			}

			result[h][w] = step % 256;
		}
	}

	// Astept ca toate thread-urile sa termine de populat matricea
	pthread_barrier_wait(&barrier);

	// Reinitializez start si end
	// Fiecare thread se va ocupa de o portiune din prima jumatate a matricei
	// si de portiunea corespunzatoare din a doua jumatate
	N = height / 2;
	start = thread_id * (double)N / P;
	end = (thread_id + 1) * (double)N / P;
	if (N < end)
		end = N;

	// Transforma rezultatul din coordonate matematice in coordonate ecran
	for (i = start; i < end; i++) {
		int* aux = result[i];
		result[i] = result[height - i - 1];
		result[height - i - 1] = aux;
	}

	// Thread-ul 0 se ocupa de printare si eliberarea matricei
	if (thread_id == 0) {
		write_output_file(out_filename_mandelbrot, result, width, height);
		free_memory(result, height);
	}

	pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
	// se citesc argumentele programului
	get_args(argc, argv);

	// Am preferat sa citesc inputul pentru Julia in afara thread-urilor
	read_input_file(in_filename_julia, &par);

	width = (par.x_max - par.x_min) / par.resolution;
	height = (par.y_max - par.y_min) / par.resolution;

	result = allocate_memory(width, height);

	int i, r;
	void* status;
	pthread_t threads[P];
	int arguments[P];

	// Initializez bariera
	pthread_barrier_init(&barrier, NULL, P);

	// Creez cele P thread-uri
	for (i = 0; i < P; i++) {
		arguments[i] = i;
		r = pthread_create(&threads[i], NULL, J_and_M, &arguments[i]);

		if (r) {
			printf("Eroare la crearea thread-ului %d\n", i);
			exit(-1);
		}
	}

	// Inchid thread-urile
	for (i = 0; i < P; i++) {
		r = pthread_join(threads[i], &status);
		if (r) {
			printf("Eroare la asteptarea thread-ului %d\n", i);
			exit(-1);
		}
	}

	// Eliberez bariera
	pthread_barrier_destroy(&barrier);

	return 0;
}
