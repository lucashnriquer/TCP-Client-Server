#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

int clientes[40];
int quantidadeClientes = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char buffer[200];
pthread_t monitor;

struct informacoes{
    
	int numeroSocket;
	char ip[INET_ADDRSTRLEN];
};

void *Monitor(void* dado);// Escreve os eventos do servidor em um arquivo de texto

void Enviar(char *mensagem,int aux);// Envia a mensagem para todos os clientes

void *ReceberMensagem(void *socket);// Função que realiza a troca de informações entre  servidor e cliente

int main(int argc,char *argv[]){
    
	struct sockaddr_in endereco1,endereco2;
	int socket1,socket2,numeroPorta;
	socklen_t tamanhoEndereco;
	pthread_t receber;
	struct informacoes cliente;
	char ip[INET_ADDRSTRLEN];
	FILE* archive;
	
	archive = fopen("servidor.txt","w");// Limpa o arquivo na inicialização do servidor
	fclose(archive);
	numeroPorta = atoi(argv[1]);
	socket1 = socket(AF_INET,SOCK_STREAM,0);
	memset(endereco1.sin_zero,'\0',sizeof(endereco1.sin_zero));
	endereco1.sin_family = AF_INET;
	endereco1.sin_port = htons(numeroPorta);
	endereco1.sin_addr.s_addr = inet_addr("127.0.0.1");
	tamanhoEndereco = sizeof(endereco2);

	if(bind(socket1,(struct sockaddr *)&endereco1,sizeof(endereco1)) != 0){
            
		perror("O servidor nao conseguiu se conectar a porta.");
		exit(1);
	}

	if(listen(socket1,5) != 0){
            
		perror("O servidor nao conseguiu escutar a porta.");
		exit(1);
	}
	
    	strcpy(buffer,"Servidor inicializado na porta ");
    	strcat(buffer,argv[1]);
    	strcat(buffer,", aguardando conexao com os clientes ... \n");
    	printf("%s",buffer);
    	pthread_create(&monitor,NULL,Monitor,&buffer);
    
	while(1){
            
		if((socket2 = accept(socket1,(struct sockaddr *)&endereco2,&tamanhoEndereco)) < 0){
		    
			perror("O servidor nao conseguiu aceitar o cliente,");
			exit(1);
		}
    
		pthread_mutex_lock(&mutex);
		inet_ntop(AF_INET, (struct sockaddr *)&endereco2, ip, INET_ADDRSTRLEN);
		
		
		strcpy(buffer,ip);
		strcat(buffer," Conectado\n");
		printf("%s",buffer);
		pthread_create(&monitor,NULL,Monitor,&buffer);
		cliente.numeroSocket = socket2;
		strcpy(cliente.ip,ip);
		clientes[quantidadeClientes] = socket2;
		quantidadeClientes++;
		pthread_create(&receber,NULL,ReceberMensagem,&cliente);
		pthread_mutex_unlock(&mutex);
	}
	
	return 0;
}

void *Monitor(void* dado){

	char aux[1000];

	strcpy(aux, (char*)dado);
	pthread_mutex_lock(&mutex);
	FILE* servidor;
	servidor = fopen("servidor.txt","a");
	fprintf(servidor,"%s",aux);
	fclose(servidor);
	pthread_mutex_unlock(&mutex);
}

void Enviar(char *mensagem,int aux){
    
	int i;
	
	pthread_mutex_lock(&mutex);
	
	for(i = 0; i < quantidadeClientes; i++){
            
		if(clientes[i] != aux){
		    
			if(send(clientes[i],mensagem,strlen(mensagem),0) < 0){
			    
				perror("Falha de envio");
				continue;
			}
		}
	}
	
	pthread_mutex_unlock(&mutex);
}

void *ReceberMensagem(void *socket){
    
	struct informacoes cliente = *((struct informacoes *)socket);
	char mensagem[1000];
	int tamanho,i,j;
	
	while((tamanho = recv(cliente.numeroSocket,mensagem,1000,0)) > 0){
            
		mensagem[tamanho] = '\0';
		strcpy(buffer,cliente.ip);
        	strcat(buffer," ");
        	strcat(buffer,mensagem);
		printf("%s",buffer);
		pthread_create(&monitor,NULL,Monitor,&buffer);
		Enviar(mensagem,cliente.numeroSocket);
		memset(mensagem,'\0',sizeof(mensagem));
	}
	
	pthread_mutex_lock(&mutex);
	strcpy(buffer,cliente.ip);
	strcat(buffer," Desconectado\n");
	printf("%s",buffer);
	pthread_create(&monitor,NULL,Monitor,&buffer);
	
	for(i = 0; i < quantidadeClientes; i++){
            
		if(clientes[i] == cliente.numeroSocket){
		    
			j = i;
    
			while(j < quantidadeClientes-1){
                    
				clientes[j] = clientes[j+1];
				j++;
			}
		}
	}
	
	quantidadeClientes--;
	pthread_mutex_unlock(&mutex);
}
