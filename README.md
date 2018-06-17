# Separador de Tokens
Buscador de tokens de arquivos de texto em C, utilizando árvores de busca binária

Desenvolvido para a disciplina Laboratório de Programação II

Criado por Augusto César Bisognin - Versão 2.1 - Codificação Western Windows 1252

Baseado em [outro aplicativo](https://github.com/augustocb23/buscador) desenvolvido 
  para a disciplina Pesquisa e Ordenação de Dados


O usuário informa o nome de uma pasta - deve ser subpasta de onde está o
executável - e o programa buscará por todos os arquivos de texto nesta pasta.
	Para cada arquivo de texto (.txt) lido, o programa criará uma lista das
palavras encontradas.
	Após todos os arquivos serem lidos, será solicitado os termos da busca.

* O usuário pode:
	1. Informar cada termo da busca, separados por ENTER ou ESPAÇOS
		Quando informar `1`, as palavras serão buscadas e os resultados serão
		exibidos. Se uma palavra aparecer mais de uma vez no arquivo, ela será
		igualmente apresentada como um novo resultado.
		Será informado se um arquivo contiver todos os termos buscados.
	2. Inserir `2` para exibir uma lista de todas as palavras encontradas nos
		arquivos.
	3. Inserir `3` para ver a lista negativa. É possível informar uma lista 
    para ser filtrada pelo indexador através do arquivo listaNegativa.txt. 
    Estas palavras não serão buscadas (mesmo que informadas depois)
	4. Inserir `0` para encerrar o programa.
