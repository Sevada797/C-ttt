#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>

#define PORT 7777
#define BUFFER_SIZE 1024
#define SIGN 'X'
#define SIGN2 'O'
#define NULL_SPACE ' '

int breakpoint=0;


void display() {
//
// Later I can cause artificial errors, and use gdb to learn debugging like.. 
// i.e. by removing the (fptr==NULL) check below, 

    FILE *fptr=fopen("gameTable", "r");
    if (fptr==NULL) {
        return;
    }

    char line[10],bigrow[19]; //bigrow is my table
    int coord,i=0;

    memset(bigrow, NULL_SPACE, sizeof(bigrow));

    // DUH I could not store own moves, it would be simpler, but for further security might be right in case
    while (fgets(line, 10, fptr)) {

        if ( line[0]=='m') {
            coord=line[1]-'0';
            bigrow[(coord-1-(coord-1)%3)/3*6+((coord-1)%3)+1]=SIGN; //-1+1=0 so remove -1 123->234 transfer
            
        }
        else {
            coord=line[0]-'0';
            bigrow[(coord-1-(coord-1)%3)/3*6+((coord-1)%3)+1]=SIGN2;
        }
        i++;
        }
    bigrow[0]='|';bigrow[4]='|';bigrow[5]='\n';bigrow[6]='|';bigrow[10]='|';bigrow[11]='\n';bigrow[12]='|';bigrow[16]='|';bigrow[17]='\n';
    bigrow[sizeof(bigrow)-1]='\0';
    printf("%s\n", bigrow);
}

void changeGameTables(int num, char *directions) {
    char buffer2[1];

    FILE *file = fopen("gameTable", "a");
    if (file!=NULL && strcmp(directions, "incoming")==0) {
        fprintf(file, "%d\n", num);
        fclose(file);
    }
    //||
    else if (file!=NULL && strcmp(directions, "incoming")!=0) { 
        fprintf(file, "m%d\n", num);
        fclose(file);
    }
    //||
    else {perror("Error openning file");}


}
// Begining of play() func

void play(int sockfd, int num) { 

    if (num>9) {
        return;
    }
char buffer2[12];

snprintf(buffer2, sizeof(buffer2), "%d", num);
ssize_t result = send(sockfd, &buffer2, strlen(buffer2), 0);
if (result == -1) {
    perror("send failed");
}

}


void udpGame() {

    printf("INFO: Starting UDP socket listener\n");
    
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len;
    ssize_t bytes_received;
    
    sockfd=socket(AF_INET, SOCK_DGRAM, 0);
    
    if (sockfd<0) {
        perror("Couldn't create socket");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);
    
    
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr) ) <0 ) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        addr_len = sizeof(client_addr);
        bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE-1, 0, (struct sockaddr*)&client_addr, &addr_len);
        if (bytes_received <0) {
            perror("Recv failed");
            continue;
        }

        buffer[bytes_received] = '\0';
        printf("Received string: %s\n", buffer);
        const char * response = "Pinging back hehe";
        sendto(sockfd, response, strlen(response), 0, (struct sockaddr*)&client_addr, addr_len);
    
    }
    

}





void tcpGame()
{


printf("INFO: Starting TCP socket listener\n");

    int sockfd, new_sock;
    char buffer[1024]; // Buff to hold actaual data received
    ssize_t bytes_received; // To store num of bytes received

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd<0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // ok we set up server address
    struct sockaddr_in server, client_addr; //server_addr;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT);

    //binding the socket
    if (bind( sockfd, (struct sockaddr *) &server, sizeof(server)  )<0) {
        perror("Can't bind");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 1)<0) {
        perror("Listen failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);


// When used client_addr struct with accept() , the function accept() auto refills these , with the connection maker ip and port details
 
    while (1) {
        socklen_t addr_len = sizeof(client_addr);
        new_sock = accept (sockfd, (struct sockaddr*)&client_addr, &addr_len  );
        if (new_sock<0) {
            printf("Accept failed");
            continue;

        }


        printf("Conn from: %s:%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port) );
        
        int playCoord;
        printf("Fill up coordinate: ");
        scanf("%d", &playCoord);
        changeGameTables(playCoord, "");
        display();
        play(new_sock, playCoord);
        while (1) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received=recv(new_sock, buffer, sizeof(buffer)-1, 0);
        if (bytes_received<0) {
            perror("Receive failed");
            break;
        }
        else if(bytes_received==0) {
            printf("Conn closed");
            break;
        }
        else {
            buffer[bytes_received]='\0';
            //Uncomment for debugging
            //printf("Received string: %s\n", buffer);
            playCoord=atoi(buffer);
            changeGameTables(playCoord, "incoming");
            display();
            printf("Fill up coordinate: ");
            scanf("%d", &playCoord);
            changeGameTables(playCoord, "");
            play(new_sock, playCoord);
            //  int sent=send(sockfd, &new_sock, *message, msglen);

        }
    }
    breakpoint++;
    }


    close(sockfd);
    //listening/accepting connections

 
}




void ask () {
int choice;
printf("Choose conn method\n\n1) Start TCP listener\n2) Start UDP listener\nYour choice: ");
scanf("%d", &choice);

if (choice == 1) {
tcpGame();
}
else if (choice == 2) {
udpGame();
}
else {
    printf ("Invalid choice!!\n\n");
    ask();
}
}


int main() {
    system("rm -f gameTable"); // Remove gameTable before starting the game,
    // Ask wether start UDP or TCP

    printf("Gameboard goes accordingly\n\n_________\n|1, 2, 3|\n|4, 5, 6|\n|7, 8, 9|\n\n");
    ask();
    return 0;

}
