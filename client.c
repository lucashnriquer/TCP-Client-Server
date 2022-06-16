#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

void *ReceberMensagem(void *socket); // recebe a mensagem do servidor

int main(int argc, char *argv[]){
    
	struct sockaddr_in endereco;
	int socket1,tamanho,numeroPorta;
	pthread_t receber;
	char mensagem[1000],serverIp[30],usuario[100],mensagemConcatenada[1200],ip[INET_ADDRSTRLEN];

	strcpy(serverIp,argv[1]);
	numeroPorta = atoi(argv[2]);
	strcpy(usuario,argv[3]);
	socket1 = socket(AF_INET,SOCK_STREAM,0);
	memset(endereco.sin_zero,'\0',sizeof(endereco.sin_zero));
	endereco.sin_family = AF_INET;
	endereco.sin_port = htons(numeroPorta);
	endereco.sin_addr.s_addr = inet_addr(serverIp);

	if(connect(socket1,(struct sockaddr *)&endereco,sizeof(endereco)) < 0){
            
		perror("Conexao nao efetuada");
		exit(1);
	}

	inet_ntop(AF_INET, (struct sockaddr *)&endereco, ip, INET_ADDRSTRLEN);
	printf("Conectado no ip %s, na porta %d\n",serverIp,numeroPorta);
	pthread_create(&receber,NULL,ReceberMensagem,&socket1);// Inicia a thread de receber mensagem
	
	while(fgets(mensagem,1000,stdin) > 0){
		
		if(strcmp(mensagem,"SAIR\n")==0){
                
			close(socket1);
			exit(1);	
		}

		strcpy(mensagemConcatenada,usuario);
		strcat(mensagemConcatenada,":");
		strcat(mensagemConcatenada,mensagem);
		printf("%s",mensagemConcatenada);
		tamanho = write(socket1,mensagemConcatenada,strlen(mensagemConcatenada));//envia a mensagem para o servidor

		if(tamanho < 0){
                
			perror("Mensagem nao enviada");
			exit(1);
		}
		
		memset(mensagem,'\0',sizeof(mensagem));
		memset(mensagemConcatenada,'\0',sizeof(mensagemConcatenada));
	}
	
	pthread_join(receber,NULL);
	close(socket1);
}

void *ReceberMensagem(void *socket){
    
	int socket2 = *((int *)socket);
	char mensagem[1000];
	int tamanho;
	
	while((tamanho = recv(socket2,mensagem,1000,0)) > 0){
            
		mensagem[tamanho] = '\0';
		printf("%s\n",mensagem);
		memset(mensagem,'\0',sizeof(mensagem));
	}
}
