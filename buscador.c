#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>
#include <ctype.h>

/*tamanho máximo de cada palavra*/
#define PALAVRA_TAM 64
/*tamanho do buffer lido (linha)*/
#define BUFFER 1024
/*lista de delimitadores dos tokens*/
#define DELIMITADORES "\n':/|\\!?. ()[]{},;-\""
#define DELIMITADORES_MAX 32

/*listas encadeadas*/
typedef struct Lista {
	void *ponteiro;
	struct Lista *prox;
} lista;
lista *lista_cria();
lista *lista_adiciona(lista *anterior, void *item);
void lista_apaga(lista *item);
void lista_imprime(lista *pasta);
void *lista_troca(lista *arquivo1, lista *arquivo2);
/*árvores de busca*/
typedef struct Arvore {
	char *string;
	int quant;
	struct Arvore *direita;
	struct Arvore *esquerda;
} arvore;
arvore *arvore_cria(char *string);
void arvore_imprime(const arvore *arv, bool quant);
arvore *arvore_adiciona(arvore *arv, char *palavra);
bool arvore_busca(const arvore *arv, const char *string);
/*strings dinamicamente alocadas*/
char *palavra_cria(char *palavra);
char *palavra_realoca(char *palavra, int tamanho);
void palavra_copia(char *ponteiro, const char *palavra);
/*arquivos*/
typedef struct Arquivo {
	char *nome;
	arvore *palavras;
	int acertos;
	arvore *encontradas;
	FILE *desc;
} arquivo;
arquivo *arquivo_cria(char *nome);
arvore *arquivo_carrega(arquivo *arq);
lista *arquivo_maior(lista *arquivos);
lista *le_pasta(char *pasta);
void busca_palavras(lista *pasta, lista *termos);
arquivo *negativa;

int main() {
	/*Define a codificação*/
	setlocale(LC_ALL, "portuguese");
	char palavra[PALAVRA_TAM];

	/*lê o nome da pasta*/
	setbuf(stdout, 0);
	printf("Digite o nome da pasta: ");
	scanf("%s", palavra);
	fopen(palavra, "r");

	/*carrega a lista de palavras*/
	printf("\n");
	lista *pasta = le_pasta(palavra);

	/*le os termos da busca e salva em uma lista*/
	while (true) {
		bool retorna = false;
		printf("\n");
		printf("Digite os termos a serem buscados:\n"
			   "\tNão diferencia maiúsculas de minúsculas\n");
		printf("Insira 1 para continuar, 2 para listar todas as palavras,\n"
			   "\t3 para visualizar a lista negativa ou 0 para sair.\n");
		/*cria uma lista para salvar os termos*/
		lista *termos;
		scanf("%s", palavra);
		/*se digitar '1', busca os termos*/
		if (strcmp(palavra, "1") == 0) {
			printf("\tNenhum termo inserido!\n");
			continue;
		}
		/*se digitar '2', exibe uma lista completa dos tokens encontrados*/
		if (strcmp(palavra, "2") == 0) {
			printf("\n");
			printf("Palavras encontradas:\n");
			lista_imprime(pasta);
			continue;
		}
		/*se digitar 3, exibe a lista negativa*/
		if (strcmp(palavra, "3") == 0) {
			printf("\n");
			if (negativa->palavras != NULL) {
				printf("Lista negativa:\n"
					   "\tPalavras a serem ignoradas\n");
				arvore_imprime(negativa->palavras, false);
			} else {
				printf("Lista negativa não definida ou em branco\n"
					   "\tInforme a lista para ignorar em listaNegativa.txt\n");
			}
			continue;
		}
		/*se digitar '0', volta ao menu anterior*/
		if (strcmp(palavra, "0") == 0)
			return 0;
		/*copia o termo lido para uma string dinamicamente alocada*/
		lista *termo = termos = lista_cria();
		termo->ponteiro = palavra_cria(palavra);
		printf("%s: adicionada à busca\n"
			   "\tDigite 1 para confirmar, 0 para sair\n", palavra);
		/*continua lendo e adicionando a lista até ser digitado '0' ou '1'*/
		while (true) {
			scanf("%s", palavra);
			if (strcmp(palavra, "1") == 0)
				break;
			/*se digitar '0', volta ao menu anterior*/
			if (strcmp(palavra, "0") == 0) {
				retorna = true;
				break;
			}
			termo = lista_adiciona(termo, palavra_cria(palavra));
			printf("%s: adicionada à busca\n"
				   "\tDigite 1 para confirmar, 0 para cancelar\n", palavra);
		}
		printf("\n");
		if (!retorna) {
			/*busca a lista de palavras lidas*/
			printf("Resultados encontrados:\n");
			busca_palavras(pasta, termos);
		}
		/*apaga a lista de palavras para permitir uma nova busca*/
		lista_apaga(termos);
	}
}

/*lê todos os arquivos de uma pasta e salva em uma lista*/
lista *le_pasta(char *pasta) {
	/*declara a pasta e arquivo*/
	DIR *dir;
	struct dirent *pasta_atual;
	arquivo *arquivo_aberto;
	pasta = palavra_realoca(pasta, 1);
	strcat(pasta, "/");
	/*abre a pasta e declara as strings*/
	dir = opendir(pasta);
	if (dir == NULL) {
		printf("Falha ao abrir a pasta!\n");
		printf("\tVerifique se o nome foi digitado corretamente e se voce possui\n"
			   "\tos privilégios necessários para acessar a pasta.\n");
		exit(6);
	}
	/*verifica se existe o arquivo [pasta]/listaNegativa.txt*/
	negativa = arquivo_cria(pasta);
	negativa->nome = palavra_realoca(negativa->nome, strlen("listaNegativa.txt"));
	strcat(negativa->nome, "listanegativa.txt");
	negativa->desc = fopen(negativa->nome, "r");
	if (negativa->desc != NULL) {
		printf("Lista negativa encontrada. Buscando exceções...\n");
		negativa->palavras = arquivo_carrega(negativa);
		if (negativa->palavras == NULL)
			printf("\tNenhum termo encontrado\n");
		else
			printf("\t%d termos encontrados\n", negativa->acertos);
		fclose(negativa->desc);
	} else
		printf("Lista negativa não encontrada ou corrompida\n");
	printf("\n");
	/*cria a lista de arquivos*/
	lista *arquivos = lista_cria();
	lista *primeiro_arquivo = arquivos;
	bool primeiro = true;
	/*busca cada arquivo na pasta e salva na lista*/
	printf("Buscando todas as palavras em %s...\n", pasta);
	while ((pasta_atual = readdir(dir)) != NULL) {
		/*verifica se não são as pastas padrão (/. e /..)*/
		if (strcmp(".", pasta_atual->d_name) == 0 || strcmp("..", pasta_atual->d_name) == 0)
			continue;
		/*verifica se é um arquivo de texto*/
		char *ext = strrchr(pasta_atual->d_name, '.');
		if (strcmp(ext, ".txt") != 0) {
			printf("\t%s\tIgnorado\n", pasta_atual->d_name);
			continue;
		}
		/*carrega o nome completo do arquivo*/
		char *nome = palavra_realoca(pasta, strlen(pasta_atual->d_name) + 1);
		printf("\t%s", pasta_atual->d_name);
		strcat(nome, pasta_atual->d_name);
		/*verifica se não é a lista negativa*/
		palavra_copia(nome, nome); /*passa para minúsculas*/
		if (strcmp(nome, negativa->nome) == 0) {
			printf("\tIgnorado\n");
			continue;
		}
		/*abre o arquivo e testa*/
		arquivo_aberto = arquivo_cria(nome);
		arquivo_aberto->desc = fopen(arquivo_aberto->nome, "r");
		if (arquivo_aberto->desc == NULL) {
			printf("\tFalha ao abrir arquivo\n");
			free(nome);
			continue;
		}
		/*verifica se é o primeiro arquivo da lista*/
		if (primeiro) {
			arquivos->ponteiro = arquivo_aberto;
			primeiro = false;
		} else
			/*adiciona o arquivo à lista*/
			arquivos = lista_adiciona(arquivos, arquivo_aberto);
		/*lê o arquivo e separa os tokens, adicionando-os à lista*/
		printf("\tlendo...\n");
		arquivo_aberto->palavras = arquivo_carrega(arquivo_aberto);
		/*fecha o arquivo*/
		if (arquivo_aberto->palavras == NULL)
			printf("\t\tNenhuma palavra encontrada\n");
		else
			printf("\t\t%d palavra(s) encontrada(s)\n", arquivo_aberto->acertos);
		fclose(arquivo_aberto->desc);
	}
	/*fecha a pasta*/
	closedir(dir);
	printf("Leitura de arquivos concluída!\n");
	/*retorna a lista de arquivos lidos*/
	return primeiro_arquivo;
}

/*recebe uma lista de arquivos e busca por uma árvore de palavras*/
void busca_palavras(lista *pasta, lista *termos) {
	/*declaração das listas*/
	lista *arquivos = pasta;
	lista *palavra;
	arquivo *arquivo_atual;
	printf("Buscando o(s) termo(s) da lista...\n");
	/*para cada arquivo informado*/
	while (arquivos != NULL) {
		bool primeiro = true;
		/*carrega o arquivo*/
		arquivo_atual = arquivos->ponteiro;
		/*cria uma lista para salvar os resultados*/
		arquivo_atual->encontradas = NULL;
		arquivo_atual->acertos = 0;
		/*para cada termos, compara com as palavra do arquivo*/
		palavra = termos;
		while (palavra != NULL) {
			/*se encontrar, incrementa o contador e salva na lista*/
			if (arvore_busca(arquivo_atual->palavras, palavra->ponteiro)) {
				if (primeiro) {
					arquivo_atual->encontradas = arvore_cria(palavra->ponteiro);
					primeiro = false;
				}
				arvore_adiciona(arquivo_atual->encontradas, palavra->ponteiro);
				arquivo_atual->acertos++;
			}
			palavra = palavra->prox;
		}
		arquivos = arquivos->prox;
	}
	/*ordena a lista de arquivos por acertos*/
	arquivos = pasta;
	while (arquivos != NULL) {
		lista *maior = arquivo_maior(arquivos);
		if (arquivos->ponteiro != maior->ponteiro)
			lista_troca(arquivos, maior);
		arquivos = arquivos->prox;
	}
	/*imprime os resultados*/
	arquivos = pasta;
	while (arquivos != NULL) {
		/*carrega o arquivo*/
		arquivo_atual = arquivos->ponteiro;
		/*exibe os resultados*/
		printf("\t%s\n", arquivo_atual->nome);
		if (arquivo_atual->acertos) {
			printf("\t\t%d resultado(s) encontrado(s):\n", arquivo_atual->acertos);
			arvore_imprime(arquivo_atual->encontradas, false);
		} else
			printf("\t\tNenhum resultado encontrado\n");
		arquivos = arquivos->prox;
	}
	printf("Busca concluída.\n");
}

/*retorna o nó da lista com maior número de acertos*/
lista *arquivo_maior(lista *arquivos) {
	lista *arquivo_maior = arquivos->ponteiro;
	int maior = -1;
	while (arquivos != NULL) {
		arquivo *arquivo_atual = arquivos->ponteiro;
		if (arquivo_atual->acertos > maior) {
			maior = arquivo_atual->acertos;
			arquivo_maior = arquivos;
		}
		arquivos = arquivos->prox;
	}
	return arquivo_maior;
}

/*recebe dois itens da lista de arquivos e troca de lugar*/
void *lista_troca(lista *arquivo1, lista *arquivo2) {
	arquivo *aux = arquivo1->ponteiro;
	arquivo1->ponteiro = arquivo2->ponteiro;
	arquivo2->ponteiro = aux;
}

/*imprime todas as palavras de cada arquivo da lista*/
void lista_imprime(lista *pasta) {
	while (pasta != NULL) {
		/*declara as listas*/
		arquivo *arquivo_atual = pasta->ponteiro;
		printf("\t%s\n", arquivo_atual->nome);
		/*verifica se há palavras no arquivo*/
		if (arquivo_atual->palavras == NULL)
			printf("\t\tNenhuma palavra encontrada\n");
		else
			/*imprime cada string encontrada*/
			arvore_imprime(arquivo_atual->palavras, true);
		pasta = pasta->prox;
	}
	printf("Fim da lista\n");
}

/*FUNÇÕES SECUNDÁRIAS*/
/*aloca um arquivo*/
arquivo *arquivo_cria(char *nome) {
	arquivo *ponteiro = malloc(sizeof(arquivo));
	if (ponteiro == NULL) {
		printf("Memória insuficiente!\n\tFalha ao alocar memória para a lista de arquivos.\n");
		exit(10);
	}
	ponteiro->nome = nome;
	ponteiro->palavras = NULL;
	ponteiro->acertos = 0;
	return ponteiro;
}

/*busca os tokens no arquivo e adiciona à arvore*/
arvore *arquivo_carrega(arquivo *arq) {
	char *palavra;
	char delimitadores[DELIMITADORES_MAX] = DELIMITADORES;
	while (!feof(arq->desc)) { /*NOLINT*/
		/*lê uma linha do arquivo e copia para o buffer*/
		char buffer[BUFFER];
		if (fgets(buffer, BUFFER, arq->desc) == NULL)
			continue;
		/*busca o primeiro token e testa se não era uma linha em branco*/
		palavra = strtok(buffer, delimitadores);
		if (palavra == NULL)
			continue;
		/*copia para a lista*/
		char *string = palavra_cria(palavra);
		/*verifica se não está na lista negativa*/
		if (!arvore_busca(negativa->palavras, string)) {
			arq->palavras = arvore_adiciona(arq->palavras, string);
			arq->acertos++;
		}
		free(string);
		/*continua separando cada palavra e adicionando na lista*/
		while (true) {
			/*copia em minúsculo*/
			string = palavra_cria(strtok(NULL, delimitadores));
			if (string != NULL) {
				/*verifica se não está na lista negativa*/
				if (!arvore_busca(negativa->palavras, string)) {
					arq->palavras = arvore_adiciona(arq->palavras, string);
					arq->acertos++;
				}
				free(string);
			} else
				break;
		}
	}
	return arq->palavras;
}

/*implementação de listas encadeadas de ponteiros*/
/*cria uma nova lista*/
lista *lista_cria() {
	lista *ponteiro = malloc(sizeof(lista));
	if (ponteiro == NULL) {
		printf("Memória insuficiente!\n\tEncerre alguns aplicativos e tente novamente.\n");
		exit(11);
	}
	ponteiro->prox = NULL;
	ponteiro->ponteiro = NULL;
	return ponteiro;
}

/*adiciona um novo item à lista e avança para o próximo*/
lista *lista_adiciona(lista *anterior, void *item) {
	lista *novo = malloc(sizeof(lista));
	if (novo == NULL) {
		printf("Memória insuficiente!\n\tEncerre alguns aplicativos e tente novamente.\n");
		exit(12);
	}
	novo->ponteiro = item;
	anterior->prox = novo;
	novo->prox = NULL;
	return novo;
}

/*apaga todos os itens de uma lista*/
void lista_apaga(lista *item) {
	while (item != NULL) {
		lista *prox = item->prox;
		free(item->ponteiro);
		free(item);
		item = prox;
	}
}

/*cria uma árvore de busca*/
arvore *arvore_cria(char *string) {
	arvore *ponteiro = malloc(sizeof(arvore));
	if (ponteiro == NULL) {
		printf("Memória insuficiente!\n\tEncerre alguns aplicativos e tente novamente.\n");
		exit(13);
	}
	ponteiro->quant = 0;
	ponteiro->string = palavra_cria(string);
	ponteiro->direita = NULL;
	ponteiro->esquerda = NULL;
	return ponteiro;
}

/*recebe uma arvore e uma palavra, insere na arvore de forma recursiva*/
arvore *arvore_adiciona(arvore *arv, char *palavra) {
	/*se não existir o nó*/
	if (arv == NULL) {
		arv = arvore_cria(palavra);
		return arv;
	}
	/*verifica se é a mesma palavra já presente na folha*/
	if (strcmp(palavra, arv->string) == 0) {
		arv->quant++;
		return arv;
	}
	/*adiciona à arvore*/
	if (strcmp(palavra, arv->string) < 0)
		arv->esquerda = arvore_adiciona(arv->esquerda, palavra);
	else
		arv->direita = arvore_adiciona(arv->direita, palavra);
	/*retorna a árvore*/
	return arv;
}

/*imprime todas as palavras de uma árvore, da esquerda para a direita*/
void arvore_imprime(const arvore *arv, bool quant) {
	if (arv->string != NULL) {
		if (arv->esquerda != NULL)
			arvore_imprime(arv->esquerda, quant);
		printf("\t\t%s", arv->string);
		if (quant)
			printf(" - %dx\n", arv->quant + 1);
		else
			printf("\n");
		if (arv->direita != NULL)
			arvore_imprime(arv->direita, quant);
	}
}

/*busca uma string em uma arvore*/
bool arvore_busca(const arvore *arv, const char *string) {
	/*se não existir o nó*/
	if (arv == NULL)
		return false;
	/*verifica se é a mesma palavra*/
	if (strcmp(string, arv->string) == 0) {
		return true;
	}
	/*desce um nível na arvore*/
	if (strcmp(string, arv->string) < 0)
		return arvore_busca(arv->esquerda, string);
	else
		return arvore_busca(arv->direita, string);
}

/*copia uma string para um novo endereço de memória dinamicamente alocado*/
char *palavra_cria(char *palavra) {
	if (palavra == NULL)
		return NULL;

	char *string = malloc(sizeof(char) * strlen(palavra) + 1);
	if (string == NULL) {
		printf("Memória insuficiente!\n\tLibere mais memoria e tente novamente.\n");
		exit(14);
	}
	palavra_copia(string, palavra);
	return string;
}

/*recebe uma string e um tamanho para expandi-la*/
char *palavra_realoca(char *palavra, int tamanho) {
	char *string = malloc(sizeof(char) * (strlen(palavra) + tamanho));
	if (string == NULL) {
		printf("Memória insuficiente!\n\tLibere mais memoria e tente novamente.\n");
		exit(15);
	}
	strcpy(string, palavra);
	return string;
}

/*copia uma string para outro ponteiro, alterando para minúsculas*/
void palavra_copia(char *ponteiro, const char *palavra) {
	int i, letra;
	for (i = 0;; i++) {
		ponteiro[i] = (char) tolower((int) palavra[i]);
		letra = palavra[i + 1];
		if (letra == '\0') {
			ponteiro[i + 1] = '\0';
			break;
		}
	}
}
