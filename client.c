#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>


#define BUFFER_SIZE 1024
#define PORT 7777
#define SIGN 'O'
#define SIGN2 'X'
#define NULL_SPACE ' '
int breakpoint=0;

void display() {
    FILE *fptr=fopen("gameTable2", "r");
    if (fptr==NULL) {
        return;
    }
    char line[10],bigrow[19]; //bigrow is my table
    int coord,i=0;

    memset(bigrow, NULL_SPACE, sizeof(bigrow));

    // DUH I could not store own moves, it would be simpler, but for further security might be right in case
    // 2nd loop var formula [-coord+i]
    while (fgets(line, 10, fptr)) {

        if ( line[0]=='m') {
            coord=line[1]-'0';
            
                bigrow[(coord-1-(coord-1)%3)/3*6+((coord-1)%3)+1]=SIGN; //123 -> 0  456->1  789->2 
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

    FILE *file = fopen("gameTable2", "a");
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


void play(int sockfd, int num) {
if (num > 9) {
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
    printf("INFO: Sending UDP packet\n");

    int sock;
    struct sockaddr_in client_addr;
    char buffer[BUFFER_SIZE];
    char *message = "Hello from UDP client!";

    // Create socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT);
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change this to the server's IP if needed

    // Send message to server
    sendto(sock, message, strlen(message), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
    printf("Message sent: %s\n", message);

    // Receive response from server
    socklen_t addr_len = sizeof(client_addr);
    ssize_t bytes_received = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &addr_len);
    if (bytes_received < 0) {
        perror("Receive failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    buffer[bytes_received] = '\0'; // Null-terminate the received string
    printf("Received from server: %s\n", buffer);
    
    // Close the socket
    close(sock);
}





void tcpGame() {
    printf("INFO: Sending TCP packet\n");

    int sock;
    struct sockaddr_in client;

    char buffer[1024];
    ssize_t bytes_received;

    char *serverip = "127.0.0.1";
    char message[1024];

    

    //create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock<0) {
        perror("Socket create: failed");
        exit(EXIT_FAILURE);
    }

    client.sin_family=AF_INET;
    client.sin_port=htons(PORT);
    if (inet_pton(AF_INET, serverip, &client.sin_addr.s_addr) < 0 ) {
    perror("Invalid address");
    exit(EXIT_FAILURE);
    }

    socklen_t socklen=sizeof(client);
    if (connect(sock, (struct sockaddr*)&client, socklen) < 0 ) {
        perror("conn failed");
        exit(EXIT_FAILURE);
        }
//    strcpy(message, "Hello server !)");
//message="lol";
    //send (sock, message, strlen(message), 0);



    while (1) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received=recv(sock, buffer, sizeof(buffer)-1, 0);
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
            //Uncomm for debugging
            //printf("Received string: %s\n", buffer);
            int num;
            sscanf(buffer, "%d", &num);
            changeGameTables(num, "incoming");
            display();
            int playCoord;
            printf("Fill up coordinate: ");
            scanf("%d", &playCoord);
            changeGameTables(playCoord, "");
            display();
            play(sock, playCoord);
            
            //char * message="PING BACK";
            //int msglen=strlen(message);

        }
    breakpoint++;
    }

    close(sock);


//connect



}



void ask() {
    int choice;
    printf("Choose\n\n1) Start TCP client\n2) Start UDP client\nYour choice: ");
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
    system("rm -f gameTable2"); // Remove gameTable before starting the game,
    // Ask wether start UDP or TCP
    printf("Gameboard goes accordingly\n\n_________\n|1, 2, 3|\n|4, 5, 6|\n|7, 8, 9|\n\n");
    ask();
    return 0;
}
