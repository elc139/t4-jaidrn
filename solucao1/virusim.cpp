//
// Simulaçao de propagaçao de virus.
// Adaptada de um codigo proposto por David Joiner (Kean University).
//
// Uso: OMP_NUM_THREADS= <nro. de threads> virusim <tamanho-da-populacao> <nro. experimentos> <probab. maxima>

#include <cstdio>
#include <vector>
#include <omp.h>
#include <cstdlib>
#include <iostream>
#include "Random.h"
#include "Population.h"

using namespace std;

void
checkCommandLine(int argc, char** argv, int& size, int& trials, int& probs)
{
   if (argc > 1) {
      size = atoi(argv[1]);
   }   
   if (argc > 2) {
      trials = atoi(argv[2]);
   }
   if (argc > 3) {
      probs = atoi(argv[3]);
   }   
}

int
main(int argc, char* argv[])
{

   // parametros dos experimentos
   int population_size = 30;
   int n_trials = 5000;
   int n_probs = 101;

   double* percent_infected; // percentuais de infectados (saida)
   double* prob_spread;      // probabilidades a serem testadas (entrada)
   double prob_min = 0.0;
   double prob_max = 1.0;
   double prob_step;
   int base_seed = 100;

   checkCommandLine(argc, argv, population_size, n_trials, n_probs);

   try {
      Population** pp = (Population**)malloc(omp_get_max_threads()*sizeof(Population*));
	  for(int t = 0; t < omp_get_max_threads(); t++)
	  	pp[t] = new Population(population_size);
      Random rand;

      prob_spread = new double[n_probs];
      percent_infected = new double[n_probs];

      prob_step = (prob_max - prob_min)/(double)(n_probs-1);

      printf("Probabilidade, Percentual Infectado\n");

      //para cada probabilidade, calcula o percentual de pessoas infectadas
      #pragma omp parallel for schedule(runtime) 
      for (int ip = 0; ip < n_probs; ip++) {
         prob_spread[ip] = prob_min + (double) ip * prob_step;
         percent_infected[ip] = 0.0;
         rand.setSeed(base_seed+ip); // nova sequencia de numeros aleatorios

         // executa varios experimentos para esta probabilidade
         for (int it = 0; it < n_trials; it++) {
            // queima floresta até o fogo apagar
            int id = omp_get_thread_num();
            pp[id]->propagateUntilOut(pp[id]->centralPerson(), prob_spread[ip], rand);
            percent_infected[ip] += pp[id]->getPercentInfected();
         }

         // calcula media dos percentuais de arvores queimadas
         percent_infected[ip] /= n_trials;

         // mostra resultado para esta probabilidade
         printf("%lf, %lf\n", prob_spread[ip], percent_infected[ip]);
      }
      for(int i = 0; i <  n_probs; i++)
         printf("%lf, %lf\n", prob_spread[i], percent_infected[i]);

      delete[] prob_spread;
      delete[] percent_infected;
   }
   catch (std::bad_alloc)
   {
      std::cerr << "Erro: alocacao de memoria" << std::endl;
      return 1;
   }

   return 0;
}

