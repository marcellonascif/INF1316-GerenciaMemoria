all:
	clear
	gcc -o gerenciador gerenciador.c 
	./gerenciador entrada.txt
	
teste:
	clear
	gcc -o gerenciadorTeste gerenciadorTeste.c 
	./gerenciadorTeste entrada.txt