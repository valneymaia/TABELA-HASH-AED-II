#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX_NAVES 100
#define MAX_RECURSOS 100

typedef struct passageiro {
    char nome[50];
    int idade;
    char planeta_origem[50];
    int id;
} Passageiro;

typedef struct recurso {
    char nome[50];
    int quantidade;
} Recurso;

typedef struct espaco_nave {
    int prioridade;
    Passageiro *passageiros;
    int num_passageiros;
    Recurso *recursos;
    int num_recursos;
} EspacoNave;

typedef struct fila_prioridade {
    EspacoNave *naves[MAX_NAVES];
    int tamanho;
} FilaPrioridade;

typedef struct hash_entry {
    char recursos[3][50];
    int ocorrencias;
} HashEntry;

HashEntry *tabelaHash[MAX_RECURSOS];

FilaPrioridade *criar_fila() {
    FilaPrioridade *fila = malloc(sizeof(FilaPrioridade));
    fila->tamanho = 0;
    return fila;
}

void trocar(EspacoNave **a, EspacoNave **b) {
    EspacoNave *temp = *a;
    *a = *b;
    *b = temp;
}

void subir_na_fila(FilaPrioridade *fila, int i) {
    while (i > 0 && fila->naves[i]->prioridade > fila->naves[(i - 1) / 2]->prioridade) {
        trocar(&fila->naves[i], &fila->naves[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

void descer_na_fila(FilaPrioridade *fila, int i) {
    int maior = i;
    int esquerda = 2 * i + 1;
    int direita = 2 * i + 2;

    if (esquerda < fila->tamanho && fila->naves[esquerda]->prioridade > fila->naves[maior]->prioridade) {
        maior = esquerda;
    }

    if (direita < fila->tamanho && fila->naves[direita]->prioridade > fila->naves[maior]->prioridade) {
        maior = direita;
    }

    if (maior != i) {
        trocar(&fila->naves[i], &fila->naves[maior]);
        descer_na_fila(fila, maior);
    }
}

// Função de hash para três strings
int hash(char *str1, char *str2, char *str3) {
    // Ordena os recursos alfabeticamente
    char recursos_ordenados[3][50];
    strcpy(recursos_ordenados[0], str1);
    strcpy(recursos_ordenados[1], str2);
    strcpy(recursos_ordenados[2], str3);
    qsort(recursos_ordenados, 3, 50, (int (*)(const void *, const void *))strcmp);

    unsigned long hash = 5381;
    int c;

    // Calcula o hash dos recursos ordenados
    for (int i = 0; i < 3; i++) {
        char *str = recursos_ordenados[i];
        while ((c = *str++)) {
            hash = ((hash << 5) + hash) + c;
        }
    }

    return (int)(hash % MAX_RECURSOS);
}


void inserir_na_fila(FilaPrioridade *fila, EspacoNave *nave) {
    if (fila->tamanho == MAX_NAVES) {
        printf("Erro: a fila esta cheia.\n");
        return;
    }

    // Inicialize um array de recursos em comum com 0
    int recursos_comuns[MAX_RECURSOS] = {0};

    // Atualize a tabelaHash com as combinações de recursos da nave
    for (int i = 0; i < nave->num_recursos; i++) {
        for (int j = 0; j < i; j++) {
            for (int k = 0; k < j; k++) {
                char *recurso1 = nave->recursos[i].nome;
                char *recurso2 = nave->recursos[j].nome;
                char *recurso3 = nave->recursos[k].nome;
                int h = hash(recurso1, recurso2, recurso3);

                // Verifique se essa combinação já ocorreu
                if (tabelaHash[h] != NULL) {
                    recursos_comuns[h]++;
                    if (recursos_comuns[h] >= 3) {
                        printf("A expansao da abertura ocorreu.\n");
                        return;
                    }
                } else {
                    tabelaHash[h] = malloc(sizeof(HashEntry));
                    strcpy(tabelaHash[h]->recursos[0], recurso1);
                    strcpy(tabelaHash[h]->recursos[1], recurso2);
                    strcpy(tabelaHash[h]->recursos[2], recurso3);
                }
            }
        }
    }

    // A nave contribui para a expansão, então a inserimos na fila
    fila->naves[fila->tamanho] = nave;
    subir_na_fila(fila, fila->tamanho);
    fila->tamanho++;
}

EspacoNave *remover_da_fila(FilaPrioridade *fila) {
    if (fila->tamanho == 0) {
        return NULL;
    }

    EspacoNave *nave = fila->naves[0];
    fila->tamanho--;
    fila->naves[0] = fila->naves[fila->tamanho];
    descer_na_fila(fila, 0);
    return nave;
}

void imprimir_fila(FilaPrioridade *fila) {
    for (int i = 0; i < fila->tamanho; i++) {
        printf("Prioridade: %d\n", fila->naves[i]->prioridade);
        printf("Passageiros:\n");
        for (int j = 0; j < fila->naves[i]->num_passageiros; j++) {
            printf("  Nome: %s, Idade: %d, Planeta de origem: %s, ID: %d\n", 
                fila->naves[i]->passageiros[j].nome, 
                fila->naves[i]->passageiros[j].idade, 
                fila->naves[i]->passageiros[j].planeta_origem, 
                fila->naves[i]->passageiros[j].id
            );
        }
        printf("Recursos:\n");
        for (int j = 0; j < fila->naves[i]->num_recursos; j++) {
            printf("  Nome: %s, Quantidade: %d\n", 
                fila->naves[i]->recursos[j].nome, 
                fila->naves[i]->recursos[j].quantidade
            );
        }
        printf("\n");
    }
}

void atribuir_nova_prioridade(EspacoNave *nave) {
    if ((double)rand() / RAND_MAX < 0.1) {
        nave->prioridade = rand() % 100; // Novo valor de prioridade aleatório entre 0 e 99
    }
}

int ocorreu_expansao(FilaPrioridade *fila) {
    for (int i = 0; i < fila->tamanho; i++) {
        for (int j = 0; j < i; j++) {
            // Verificar se a combinação de recursos ocorreu anteriormente
            EspacoNave *nave1 = fila->naves[i];
            EspacoNave *nave2 = fila->naves[j];
            int recursos_iguais = 0;

            for (int k = 0; k < nave1->num_recursos; k++) {
                for (int l = 0; l < nave2->num_recursos; l++) {
                    if (strcmp(nave1->recursos[k].nome, nave2->recursos[l].nome) == 0) {
                        recursos_iguais++;
                    }
                }
            }

            if (recursos_iguais >= 3) {
                // Chame a função hash e verifique se a combinação de recursos já ocorreu
                int h = hash(nave1->recursos[0].nome, nave1->recursos[1].nome, nave1->recursos[2].nome);
                printf("Combinacao de recursos: %s, %s, %s\n", nave1->recursos[0].nome, nave1->recursos[1].nome, nave1->recursos[2].nome);

                if (tabelaHash[h] != NULL) {
                    printf("Combinacao ja ocorreu na tabela hash.\n");
                    return 1; // Ocorrência de expansão
                }
                printf("Combinacao ainda nao ocorreu na tabela hash.\n");
            }
        }
    }
    return 0; // Nenhuma expansão ocorreu
}

int main() {
    srand(time(NULL));

    FilaPrioridade *fila = criar_fila();
    for (int i = 0; i < MAX_RECURSOS; i++) {
        tabelaHash[i] = NULL;
    }

    int opcao;
    do {
        printf(" .  *  . . .  .");
        printf("Menu: * . .  * . *  . * . .  *\n");
        printf("1. Inserir espaconave na fila\n");
        printf("2. Remover e imprimir espaconave com maior prioridade\n");
        printf("3. Imprimir todas as espaçonaves na fila\n");
        printf("4. Verificar expansão da abertura\n"); // Opção para verificar a expansão
        printf("5. Sair\n");
        printf("Escolha uma opcao: ");
        printf("\n* .  *  . . .    .");
        printf(" * . .  * .  *  . * . .  *\n");
        scanf("%d", &opcao);

        if (opcao == 1) {
            // (Código para inserir uma espaçonave na fila)
            EspacoNave *nave = malloc(sizeof(EspacoNave));

            printf("Digite a prioridade da espaconave: ");
            scanf("%d", &nave->prioridade);

            printf("Digite o numero de passageiros: ");
            scanf("%d", &nave->num_passageiros);
            nave->passageiros = malloc(nave->num_passageiros * sizeof(Passageiro));
            for (int i = 0; i < nave->num_passageiros; i++) {
                printf("Digite o nome do passageiro %d: ", i + 1);
                scanf("%s", nave->passageiros[i].nome);
                printf("Digite a idade do passageiro %d: ", i + 1);
                scanf("%d", &nave->passageiros[i].idade);
                printf("Digite o planeta de origem do passageiro %d: ", i + 1);
                scanf("%s", nave->passageiros[i].planeta_origem);
                printf("Digite o ID do passageiro %d: ", i + 1);
                scanf("%d", &nave->passageiros[i].id);
            }

            printf("Digite o numero de recursos transportados: ");
            scanf("%d", &nave->num_recursos);
            nave->recursos = malloc(nave->num_recursos * sizeof(Recurso));
            for (int i = 0; i < nave->num_recursos; i++) {
                printf("Digite o nome do recurso %d: ", i + 1);
                scanf("%s", nave->recursos[i].nome);
                printf("Digite a quantidade do recurso %d: ", i + 1);
                scanf("%d", &nave->recursos[i].quantidade);
            }
            atribuir_nova_prioridade(nave);
            inserir_na_fila(fila, nave);
        } else if (opcao == 2) {
            // (Código para remover e imprimir espaçonave com maior prioridade)
              EspacoNave *nave = remover_da_fila(fila);
            if (nave == NULL) {
                printf("A fila esta vazia.\n");
            } else {
                printf("Espaçonave removida:\n");
                printf("Prioridade: %d\n", nave->prioridade);
                printf("Passageiros:\n");
                for (int i = 0; i < nave->num_passageiros; i++) {
                    printf("  Nome: %s, Idade: %d, Planeta de origem: %s, ID: %d\n", 
                        nave->passageiros[i].nome, 
                        nave->passageiros[i].idade, 
                        nave->passageiros[i].planeta_origem, 
                        nave->passageiros[i].id
                    );
                }
                printf("Recursos:\n");
                for (int i = 0; i < nave->num_recursos; i++) {
                    printf("  Nome: %s, Quantidade: %d\n", 
                        nave->recursos[i].nome, 
                        nave->recursos[i].quantidade
                    );
                }
            }
        } else if (opcao == 3) {
            // (Código para imprimir todas as espaçonaves na fila)
            imprimir_fila(fila);
        } else if (opcao == 4) {
            int expansao = ocorreu_expansao(fila);
            if (expansao) {
                printf("A expansao da abertura ocorreu.\n");
            } else {
                printf("A expansao da abertura ainda nao ocorreu.\n");
            }
        }
        

    } while (opcao != 5);

    // Liberar memória alocada para as naves
    for (int i = 0; i < fila->tamanho; i++) {
        free(fila->naves[i]->passageiros);
        free(fila->naves[i]->recursos);
        free(fila->naves[i]);
    }

    free(fila);

    return 0;
}















