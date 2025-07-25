#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TAM_HASH 1000
#define SEED1    0x12345678
#define SEED2    0x87654321  

// Struct para armazenar os valores das cidades
typedef struct main {
	int codigo_ibge;
	char nome[100];
	double latitude;
	double longitude;
	int capital;
	int codigo_uf;
	int siafi_id;
	int ddd;
	char fuso_horario[50];
} Cidades;

typedef struct {
    uintptr_t *table;
    int max;
    int size;
    uintptr_t deleted;
    char *(*get_key)(void *);
} thash;

uint32_t hashf(const char* str, uint32_t h){
    /* One-byte-at-a-time Murmur hash 
    Source: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp */
    for (; *str; ++str) {
        h ^= *str;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

//---------------------------
//ARRUMAR ISSO AQUI DEPOIS
//---------------------------
char *get_key(void *reg){
    Cidades *cidade = (Cidades *)reg;
    char *codigo_ibge_str = malloc(sizeof(char) * 20); // Reserva espaço suficiente para armazenar o código IBGE como uma string
    if (codigo_ibge_str == NULL) {
        // Tratamento de erro, se necessário
        return NULL;
    }
    snprintf(codigo_ibge_str, 20, "%d", cidade->codigo_ibge); // Converte o código IBGE para uma string
    return codigo_ibge_str;
}
//---------------------------
//---------------------------
//---------------------------

int hash_constroi(thash * h,int nbuckets, char * (*get_key)(void *) ){
    h->table = calloc(sizeof(void *),nbuckets + 1);
    if (h->table == NULL){
        return EXIT_FAILURE;
    }
    h->max = nbuckets + 1;
    h->size = 0;
    h->deleted = (uintptr_t)&(h->size);
    h->get_key = get_key;
    return EXIT_SUCCESS;
}

int hash_insere(thash * h, void * bucket){
    uint32_t hash1 = hashf(h->get_key(bucket),SEED1);
    uint32_t hash2 = hashf(h->get_key(bucket),SEED2);
    int pos = hash2 % (h->max);
    /*se esta cheio*/
    if (h->max == (h->size + 1)){
        free(bucket);
        return EXIT_FAILURE;
    }else{  /*fazer a insercao*/
        while(h->table[pos] != 0){
            if (h->table[pos] == h->deleted)
                break;
            pos = (pos + 1)%h->max;
        }
        h->table[pos] = (uintptr_t)bucket;
        h->size +=1;
    }
    return EXIT_SUCCESS;
}

void * hash_busca(thash  h, const char * key){
    int pos = hashf(key,SEED1) % (h.max);
    void * ret = NULL;
    while(h.table[pos]!=0 && ret == NULL){
        if (strcmp(h.get_key((void*)h.table[pos]),key) == 0){
            ret = (void *) h.table[pos];
        }else{
            pos = (pos+1) % h.max;
        }
    }
    return ret;
}

void hash_apaga(thash *h){
    int pos;
    for (pos = 0; pos < h->max; pos++){
        if (h->table[pos] != 0){
            if (h->table[pos] != h->deleted){
                free((void*)h->table[pos]);
            }
        }
    }
    free(h->table);
}

int alocar_cidades(thash *tabela){

	FILE *arquivo = fopen("municipios.txt", "r");
	if (arquivo == NULL){
        return EXIT_FAILURE;
    }

    char linha[1024];
    size_t tamanho = 0;


    while(fgets(linha, sizeof(linha), arquivo) != NULL){
    	Cidades *cdd = (Cidades *)malloc(sizeof(Cidades));
    	sscanf(linha, "%*[^:]:%d,%*[^:]:%*[^,],%*[^:]:%f,%*[^:]:%f,%*[^:]:%d,%*[^:]:%d,%*[^:]:%d,%*[^:]:%d,%*[^:]:\"%[^\"]\"", 
            &cdd->codigo_ibge, &cdd->nome,&cdd->latitude, &cdd->longitude, &cdd->capital, &cdd->codigo_uf, &cdd->siafi_id, &cdd->ddd, cdd->fuso_horario);
        hash_insere(tabela, cdd);
    }

    return EXIT_SUCCESS;
}

Cidades consulta_cidade_cod(thash tabela, int codigo){
	char chave[20];
	sprintf(chave, "%d", codigo);
	Cidades *cdd = (Cidades *)hash_busca(tabela, chave);
	if (cdd != NULL) {
        return *cdd; // Retorna a estrutura de Cidades, não o ponteiro
    } else {
        Cidades cidade_vazia = {0}; // Crie uma cidade com todos os valores zerados
        return cidade_vazia; // Retorna uma cidade vazia se não encontrar o código
    }
}


void atividade_1(thash tabela){
    int codigo;
    printf("Informe o codigo da cidade: ");
    scanf("%d", &codigo);
    Cidades cdd = consulta_cidade_cod(tabela, codigo);

    if (cdd.codigo_ibge != 0) {
        printf("Informacoes da cidade '%s'\n", cdd.nome);
        printf("Codigo IBGE: %d\n", cdd.codigo_ibge);
        printf("Latitude: %f\n", cdd.latitude);
        printf("Longitude: %f\n", cdd.longitude);
        printf("Capital: %s\n", cdd.capital ? "Não" : "Sim");
        printf("Codigo UF: %d\n", cdd.codigo_uf);
        printf("SIAFI ID: %d\n", cdd.siafi_id);
        printf("DDD: %d\n", cdd.ddd);
        printf("Fuso Horario: %s\n", cdd.fuso_horario);
    } else {
        printf("Cidade com codigo IBGE %d nao encontrada.\n", codigo);
    }
}


int main() {
	
	// Inicializando a tabela Hash
	thash tabela;
    hash_constroi(&tabela, TAM_HASH, get_key);

	if (alocar_cidades(&tabela) != EXIT_SUCCESS){
        printf("Error\n");
        return EXIT_FAILURE;
    }

    atividade_1(tabela);
    //atividade_2();
    //atividade_3();
	
	hash_apaga(&tabela);

	return 0;
}