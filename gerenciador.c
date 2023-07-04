#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define IO 0
#define EXEC 1
#define TAM_MEMORIA 4
#define MAX_PROC 100

int algoritmo = 0;
int memoria[] = {8, 4, 2, 2};       /* Partições da memória */
int MemoriaAux[] = {0, 0, 0, 0};    /* Ocupações da memória */
int qtdProc;            /* Qtd de processos */
int alocados[MAX_PROC];
int prontos[MAX_PROC];
int bloqueados[MAX_PROC];
int finalizados[MAX_PROC];

typedef struct process
{
    int num;      /* Numero do processo*/
    int tam;      /* Tamanho do processo em KB */
    int qtdAcoes; /* Qtd de ações no processo */
    int atual;    /* Ação atual do processo */
    int particao; /* Partição em que o processo está */
    int ex[][2];  /* Vetor guardando ações do processo e o timeslice para execução */
} Process;

int BestFit(Process *p, int index)
{
    int i;
    int melhor = -1, menor = 999999999;

    for (i = 0; i < TAM_MEMORIA; i++)
    {
        if (memoria[i] >= p->tam && MemoriaAux[i] == 0)
        {
            if (memoria[i] - p->tam < menor)
            {
                melhor = i;
                menor = memoria[i] - p->tam;
            }
        }
    }
    if (melhor != -1)
    {
        alocados[index] = 1;
        MemoriaAux[melhor] = index + 1;
        p->particao = melhor;
        return 1;
    }
    else
    {
        prontos[index] = 1;
        p->particao = -1;
        return 0;
    }
}

int FirstFit(Process *p, int index)
{
    int i;

    for (i = 0; i < TAM_MEMORIA; i++)
    {
        if (memoria[i] >= p->tam && MemoriaAux[i] == 0)
        {
            alocados[index] = 1;
            MemoriaAux[i] = index + 1;
            p->particao = i;
            return 1;
        }
    }
    prontos[index] = 1;
    p->particao = -1;
    return 0;
}

int WorstFit(Process *p, int index)
{
    int i;
    int pior = -1;
    int maior_tamanho = 0;

    for (i = 0; i < TAM_MEMORIA; i++)
    {
        if (memoria[i] >= p->tam && MemoriaAux[i] == 0)
        {
            if (memoria[i] > maior_tamanho)
            {
                pior = i;
                maior_tamanho = memoria[i];
            }
        }
    }
    if (pior != -1)
    {
        alocados[index] = 1;
        MemoriaAux[pior] = index + 1;
        p->particao = pior;
        return 1;
    }
    else
    {
        prontos[index] = 1;
        p->particao = -1;
        return 0;
    }
}

void print_processos(Process **p)
{
    int i;
    for (i = 0; i < qtdProc; i++)
    {
        printf("Num: %d\n Tam: %d\n", p[i]->num, p[i]->tam);
    }
}

void print_memoria(int relogio)
{
    int i;
    if (relogio % 4 == 0 || relogio == 0)
    {
        printf("\nTempo: %d u.t. || Memoria: \n", relogio);
        for (i = 0; i < TAM_MEMORIA; i++)
        {
            if (MemoriaAux[i] != 0)
                printf("Particao %d (%d KB): Processo %d\n", i + 1, memoria[i], MemoriaAux[i]);
            else
                printf("Particao %d (%d KB): Vazia\n", i + 1, memoria[i]);
        }

        printf("\n");
    }
}
int swap_proc(Process **p, int i)
{
    int swap = 0;
    int j, temp;

    alocados[i] = 0;
    temp = MemoriaAux[p[i]->particao];
    MemoriaAux[p[i]->particao] = 0;
    for (j = 0; j < qtdProc; j++)
    {
        if (j != i && prontos[j])
        {
            if (algoritmo == 0)
            {
                if (FirstFit(p[j], j)) // roda o first fit para o processo j
                {
                    swap = 1;
                    p[i]->particao = -1;
                    prontos[j] = 0;
                    printf("Processo %d retirado da memoria\n", i + 1);
                    printf("Processo %d inserido na memoria\n", j + 1);
                    return 1;
                }
            }
            else if (algoritmo == 1)
            {
                if (BestFit(p[j], j)) // roda o best fit para o processo j
                {
                    swap = 1;
                    p[i]->particao = -1;
                    prontos[j] = 0;
                    printf("Processo %d retirado da memoria\n", i + 1);
                    printf("Processo %d inserido na memoria\n", j + 1);
                    return 1;
                }
            }
            else if (algoritmo == 2)
            {
                if (WorstFit(p[j], j)) // roda o worst fit para o processo j
                {
                    swap = 1;
                    p[i]->particao = -1;
                    prontos[j] = 0;
                    printf("Processo %d retirado da memoria\n", i + 1);
                    printf("Processo %d inserido na memoria\n", j + 1);
                    return 1;
                }
            }
        }
    }

    if (swap == 0)
    {
        alocados[i] = 1;
        MemoriaAux[p[i]->particao] = temp;
        return 0;
    }

    return 1;
}

void io(Process **p)
{
    int i;
    for (i = 0; i < qtdProc; i++)
    {
        if (bloqueados[i] != 0)
        {
            bloqueados[i]--;
            if (bloqueados[i] == 0)
            {
                p[i]->atual++;
                if (p[i]->atual == p[i]->qtdAcoes)
                {
                    finalizados[i] = 1;
                    prontos[i] = 0;
                    printf("Processo %d finalizado\n", i + 1);
                    swap_proc(p, i);
                }
                else if (!alocados[i])
                {
                    prontos[i] = 1;
                }
            }
        }
    }
}
int terminou()
{
    int i;
    for (i = 0; i < qtdProc; i++)
    {
        if (!finalizados[i])
            return 0;
    }

    return 1;
}
int main(int argc, char **argv)
{
    FILE *arq;
    int i, j;
    int qtdAcoes, tam, num, tipo, todos_io;
    int tempo, max;
    char acao[5];

    if (argc != 2)
    {
        printf("Entrar com arquivo de entrada.\n");
        exit(1);
    }

    arq = fopen(argv[1], "r");
    if (arq == NULL)
    {
        printf("Erro ao abrir o arquivo de entrada\n");
        exit(2);
    }

    printf("***********************\nEscolha o algoritmo de alocacao de memoria: \n0 - First fit\n1 - Best fit\n2 - Worst fit\n***********************\n Escolha uma opção: ");
    scanf("%d", &algoritmo);
    if (algoritmo > 2 || algoritmo < 0)
    {
        printf("Opcao invalida\n");
        exit(3);
    }

    fscanf(arq, "%d", &qtdProc);
    Process *processos[qtdProc];

    for (i = 0; i < qtdProc; i++) // Inicializa os vetores com o tamanho recebido do arquivo
    {
        alocados[i] = 0;
        finalizados[i] = 0;
        prontos[i] = 0;
        bloqueados[i] = 0;
        MemoriaAux[i] = 0;
    }

    printf("PROCESSOS:\n");
    for (i = 0; i < qtdProc; i++)
    {

        fscanf(arq, "%*s #%d - %dKb", &num, &tam); /* lê o numero do processo e seu respectivo tamanho em Kb */
        printf("Processo %d - %dKB\n", num, tam);

        // entra na devida partição
        if (tam < 2)
        {
            tam = 2;
        }
        else if (tam > 2 && tam < 4)
        {
            tam = 4;
        }
        else if (tam > 4 && tam < 8)
        {
            tam = 8;
        }

        fscanf(arq, "%d", &qtdAcoes);

        Process *p = (Process *)malloc(sizeof(*p) + qtdAcoes * sizeof(*p->ex));
        p->num = num;
        p->tam = tam;
        p->qtdAcoes = qtdAcoes;

        for (j = 0; j < qtdAcoes; j++)
        { /* For que lê as ações que devem ser executadas no processo */
            fscanf(arq, "%s %d", acao, &tempo);

            /* Se for IO */
            if (strcmp(acao, "io") == 0)
            {
                printf("IO %d\n", tempo);
                tipo = IO; /* IO é 0 */
            }

            /* Se for EXEC */
            else
            {
                printf("EXEC %d\n", tempo);
                tipo = EXEC; /* EXEC é 1 */
            }

            p->ex[j][0] = tipo;
            p->ex[j][1] = tempo;
        }

        p->atual = 0;
        processos[i] = p;
        printf("\n");
    }

    for (i = 0; i < qtdProc; i++)
    {
        if (algoritmo == 0)
            FirstFit(processos[i], i);
        else if (algoritmo == 1)
            BestFit(processos[i], i);
        else if (algoritmo == 2)
            WorstFit(processos[i], i);
    }

    int relogio = 0;

    while (1)
    {
        if (terminou())
        { // Quebra o while caso todos os processos tenham terminado
            break;
        }

        todos_io = 1;
        for (i = 0; i < qtdProc; i++)
        {

            if (alocados[i] && !bloqueados[i] && !finalizados[i])
            {
                todos_io = 0;
                qtdAcoes = processos[i]->qtdAcoes;

                for (j = processos[i]->atual; j < processos[i]->qtdAcoes; j++)
                {
                    if (processos[i]->ex[processos[i]->atual][0] == EXEC)
                    {
                        printf("No CPU: processo %d em exec faltando %d u.t.\n", i + 1, processos[i]->ex[processos[i]->atual][1]);

                        if (processos[i]->ex[processos[i]->atual][1] > 4)
                            max = 4;
                        else
                            max = processos[i]->ex[processos[i]->atual][1];
                        for (int k = 0; k < max; k++)
                        {
                            processos[i]->ex[processos[i]->atual][1]--;
                            print_memoria(relogio);
                            relogio++;

                            io(processos);
                            if (relogio % 4 == 0)
                            {
                                sleep(1);
                            }
                        }
                        if (processos[i]->ex[processos[i]->atual][1] == 0)
                        {
                            processos[i]->atual++;
                        }
                        else
                        {
                            break;
                        }

                        if (processos[i]->atual >= processos[i]->qtdAcoes)
                        {
                            finalizados[i] = 1;
                            prontos[i] = 0;
                            printf("Processo %d finalizado\n", i + 1);
                            swap_proc(processos, i);
                        }
                    }
                    else
                    {
                        printf("Processo %d em I/O por %d\n", i + 1, processos[i]->ex[processos[i]->atual][1]);

                        bloqueados[i] = processos[i]->ex[j][1];
                        swap_proc(processos, i);

                        break;
                    }
                }
            }
        }
        if (todos_io)
        {
            for (j = 0; j < qtdProc; j++)
            {
                if (alocados[j])
                    swap_proc(processos, j);
            }
            print_memoria(relogio);
            relogio++;
            io(processos);
            if (relogio % 4 == 0)
                sleep(1);
        }
    }
    printf("Tempo total: %d u.t.\n", relogio);
    return 0;
}