/* Programa de ejemplo de semaphore. */
/* para compilar usar: gcc -o tarea3_ejemplo tarea3_ejemplo.c -lpthread */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


typedef struct {     //estructura creada para los datos de caja 1 y caja 2
  int capacidad_caja;
  float* almacen;
  int cantidad_clientes;
  int demora_min_cliente;
  int demora_max_cliente;
  int demora_min_caja;
  int demora_max_caja;
  sem_t puede_producir;
  sem_t puede_consumir;
} datos_compartidos_t;


void* produce(void* data);
void* consume(void* data);
void* produce1(void* data);
void* consume1(void* data);
int random_entre(int min, int max);

int main(int argc, char* argv[]) {
  int error;
  struct timespec tiempo_ini;
  struct timespec tiempo_fin;
  pthread_t cliente1,cliente2,caja1,caja2;    //se crean las variables para la caja 2, que sería cliente2 y caja2
  datos_compartidos_t datos_compartidos;
  datos_compartidos_t datos_compartidos1;     //se crea una variable con la estructura datos compartidos para la caja 2

  srandom(time(NULL));

  if (argc == 5) {
    datos_compartidos.capacidad_caja=random_entre(5,15);        //Se le asigna un numero random para la capacidad de caja que va entre 5 y 15
    datos_compartidos.cantidad_clientes= random_entre(1,10);    //Se le asigna un numero random para la cant de clientes que va entre 1 y 10
    datos_compartidos.demora_min_cliente=atoi(argv[1]);
    datos_compartidos.demora_max_cliente=atoi(argv[2]);
    datos_compartidos.demora_min_caja=atoi(argv[3]);
    datos_compartidos.demora_max_caja=atoi(argv[4]);

    datos_compartidos1.capacidad_caja=random_entre(5,15);       //Se le asigna un numero random para la capacidad de caja que va entre 5 y 15
    datos_compartidos1.cantidad_clientes= random_entre(1,10);   //Se le asigna un numero random para la cant de clientes que va entre 1 y 10
    datos_compartidos1.demora_min_cliente= datos_compartidos.demora_min_cliente;
    datos_compartidos1.demora_max_cliente=datos_compartidos.demora_max_cliente;
    datos_compartidos1.demora_min_caja=datos_compartidos.demora_max_caja;
    datos_compartidos1.demora_max_caja=datos_compartidos.demora_max_caja;
  } else {
    printf("Usar: %s demora_min_cliente demora_max_cliente demora_min_caja demora_max_caja\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  datos_compartidos.almacen = (float*) calloc(datos_compartidos.capacidad_caja, sizeof(float));
  datos_compartidos1.almacen = (float*) calloc(datos_compartidos1.capacidad_caja, sizeof(float));
  sem_init(&datos_compartidos.puede_producir, 0, datos_compartidos.capacidad_caja);
  sem_init(&datos_compartidos.puede_consumir, 0, 1);
    sem_init(&datos_compartidos1.puede_producir, 0, datos_compartidos.capacidad_caja);
    sem_init(&datos_compartidos1.puede_consumir, 0, 1);
  clock_gettime(CLOCK_MONOTONIC, &tiempo_ini);



//caja numero 1
  error = pthread_create(&cliente1, NULL, produce, &datos_compartidos);
  if (error == 0) {
    error = pthread_create(&caja1, NULL, consume, &datos_compartidos);
    if (error != 0) {
      fprintf(stderr, "error: no puede crear consumidor\n");
      error = 1;
    }
  } else {
    fprintf(stderr, "error: no puede crear productor\n");
    error = 1;
  }
  if (error == 0) {
    pthread_join(cliente1, NULL);
    pthread_join(caja1, NULL);
  }

//se crea un hilo independiente para la caja numero 2
 error = pthread_create(&cliente2, NULL, produce1, &datos_compartidos1);
  if (error == 0) {
    error = pthread_create(&caja2, NULL, consume1, &datos_compartidos1);
    
    if (error != 0) {
      fprintf(stderr, "error: no puede crear consumidor\n");
      error = 1;
    }
  } else {
    fprintf(stderr, "error: no puede crear productor\n");
    error = 1;
  }
  if (error == 0) {
    pthread_join(cliente2, NULL);
    pthread_join(caja2, NULL);
  }

  clock_gettime(CLOCK_MONOTONIC, &tiempo_fin);



  float periodo = (tiempo_fin.tv_sec - tiempo_ini.tv_sec) + 
          (tiempo_fin.tv_nsec - tiempo_ini.tv_nsec) * 1e-9;
  printf("Tiempo de ejecución: %.9lfs\n", periodo);
  sem_destroy(&datos_compartidos.puede_consumir);
  sem_destroy(&datos_compartidos.puede_producir);
  sem_destroy(&datos_compartidos1.puede_consumir);
  sem_destroy(&datos_compartidos1.puede_producir);
  free(datos_compartidos.almacen);

  return EXIT_SUCCESS;
}

void* produce(void* data) {
  datos_compartidos_t* datos_compartidos = (datos_compartidos_t*)data;
  int contador = 0;
  for (int ronda = 0; ronda < datos_compartidos->cantidad_clientes; ++ronda) {
    printf("INICIO RONDA P: %i\n", ronda);
    for (int indice = 0; indice < datos_compartidos->capacidad_caja; ++indice) {
      sem_wait(&datos_compartidos->puede_producir);
      usleep(1000 * random_entre(datos_compartidos->demora_min_cliente, datos_compartidos->demora_max_cliente));
      datos_compartidos->almacen[indice] = ++contador;
      printf("Indice almacen %i se produce %lg\n", indice, datos_compartidos->almacen[indice]);
      sem_post(&datos_compartidos->puede_consumir);
    }
  }
  return NULL;
}
void* produce1(void* data) {
  datos_compartidos_t* datos_compartidos1 = (datos_compartidos_t*)data;
  int contador = 0;
  for (int ronda = 0; ronda < datos_compartidos1->cantidad_clientes; ++ronda) {
    printf("INICIO RONDA P: %i\n", ronda);
    for (int indice = 0; indice < datos_compartidos1->capacidad_caja; ++indice) {
      sem_wait(&datos_compartidos1->puede_producir);
      usleep(1000 * random_entre(datos_compartidos1->demora_min_cliente, datos_compartidos1->demora_max_cliente));
      datos_compartidos1->almacen[indice] = ++contador;
      printf("Indice almacen %i se produce %lg\n", indice, datos_compartidos1->almacen[indice]);
      sem_post(&datos_compartidos1->puede_consumir);
    }
  }
  return NULL;
}

void* consume(void* data) {
  datos_compartidos_t* datos_compartidos = (datos_compartidos_t*)data;
  for (int ronda = 0; ronda < datos_compartidos->cantidad_clientes; ++ronda) {
    printf("\t\tINICIO RONDA C: %i\n", ronda);
    for (int indice = 0; indice < datos_compartidos->capacidad_caja; ++indice) {
      sem_wait(&datos_compartidos->puede_consumir);
      float value = datos_compartidos->almacen[indice];
      usleep(1000 * random_entre(datos_compartidos->demora_min_caja
        , datos_compartidos->demora_max_caja));
      printf("\t\tIndice almacen %i se consume %lg\n", indice, value);
      sem_post(&datos_compartidos->puede_producir);
    }
  }
  return NULL;
}
void* consume1(void* data) {
  datos_compartidos_t* datos_compartidos1 = (datos_compartidos_t*)data;
  for (int ronda = 0; ronda < datos_compartidos1->cantidad_clientes; ++ronda) {
    printf("\t\tINICIO RONDA C: %i\n", ronda);
    for (int indice = 0; indice < datos_compartidos1->capacidad_caja; ++indice) {
      sem_wait(&datos_compartidos1->puede_consumir);
      float value = datos_compartidos1->almacen[indice];
      usleep(1000 * random_entre(datos_compartidos1->demora_min_caja
        , datos_compartidos1->demora_max_caja));
      printf("\t\tIndice almacen %i se consume %lg\n", indice, value);
      sem_post(&datos_compartidos1->puede_producir);
    }
  }
  return NULL;
}
 //funcion random
int random_entre(int min, int max) {
  int aux=0;
  if (max > min)
    aux=random() % (max - min);
  return min + aux;
}