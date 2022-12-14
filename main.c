#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <pthread.h>
#include <semaphore.h>

#define ALTURA 50
#define LARGURA 150

int TAMANHO_INICIAL_1 = 20;
int TAMANHO_INICIAL_2 = 20;

int firstRun = 1;

int bufferApple[10][2];
int in = 0;
int out = 0;
int apple = 3;

int M[ALTURA][LARGURA] = {{0}};
sem_t movement;
sem_t palco;
sem_t vagos;
sem_t preenchidos;
sem_t tela;

int findInBuffer(int x, int y) {
    int i;
    for(i = 0; i < sizeof(bufferApple); i++) {
        if(bufferApple[i][x] && bufferApple[i][y]) {
            return i;
        }
    }
    return -1;
}

void drawApple(int x, int y) {
    COORD coord;
    HANDLE hOut;

    coord.X = x;
    coord.Y = y;
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    sem_wait(&tela);
    SetConsoleTextAttribute(hOut,10);
    SetConsoleCursorPosition(hOut,coord);
    printf("%c", 254);
    sem_post(&tela);
}

void* createApple() {
    srand(time(NULL));

    if(firstRun == 1) {
        int i;
        for(i = 0; i < 10; i++) {
            int randX = (int)(LARGURA-3)*(rand()%101/100.0)+1;
            int randY = (int)(ALTURA-3)*(rand()%101/100.0)+1;

            //if(M[randY][randX] != 0) {
            //    createApple();
            //}

            sem_wait(&vagos);
            sem_wait(&palco);
            M[randY][randX] = 3;
            bufferApple[in][0] = randX;
            bufferApple[in][1] = randY;
            in = (in + 1) % 10;
            drawApple(randX, randY);
            sem_post(&palco);
            sem_post(&preenchidos);
        }
        firstRun = 0;
    } else {
        int randX = (int)(LARGURA-3)*(rand()%101/100.0)+1;
        int randY = (int)(ALTURA-3)*(rand()%101/100.0)+1;

        while(M[randY][randX] != 0) {
            randX = (int)(LARGURA-3)*(rand()%101/100.0)+1;
            randY = (int)(ALTURA-3)*(rand()%101/100.0)+1;
        }

        sem_wait(&vagos);
        sem_wait(&palco);
        M[randY][randX] = 3;
        bufferApple[in][0] = randX;
        bufferApple[in][1] = randY;
        in = (in + 1) % 10;
        drawApple(randX, randY);
        sem_post(&palco);
        sem_post(&preenchidos);
    }


    return NULL;
}

void eatApple(int snake, int x, int y) {
    sem_wait(&preenchidos);
    sem_wait(&palco);
    if(snake == 1) {
        TAMANHO_INICIAL_1 += 5;
        int index = findInBuffer(x, y);
        bufferApple[index][x] = 0;
        bufferApple[index][y] = 0;
    }
    if(snake == 2) {
        TAMANHO_INICIAL_2 += 5;
        int index = findInBuffer(x, y);
        bufferApple[index][x] = 0;
        bufferApple[index][y] = 0;
    }
    sem_post(&palco);
    sem_post(&vagos);
    mostrarPontuacao();
    createApple();
}



void imprimirCoordenada(int x, int y, int cor)
{
    COORD coord;
    HANDLE hOut;

    coord.X = x;
    coord.Y = y;
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    sem_wait(&tela);
    SetConsoleTextAttribute(hOut,cor);
    SetConsoleCursorPosition(hOut,coord);
    printf("%c",219);

    SetConsoleTextAttribute(hOut,7);
    sem_post(&tela);
}

void imprimirQuadro()
{
    int i;
    for(i=0; i<LARGURA; i++)
    {
        imprimirCoordenada(i,0,7);
        imprimirCoordenada(i,ALTURA-1,7);
    }
    for(i=0; i<ALTURA; i++)
    {
        imprimirCoordenada(0,i,7);
        imprimirCoordenada(LARGURA-1,i,7);
    }
}

void imprimirMatriz()
{
    int i, j;
    for(i=0; i<ALTURA; i++)
    {
        for(j=0; j<LARGURA; j++)
        {
            printf("%d\t",M[i][j]);
            if(j == LARGURA)
            {
                printf("\n");
            }
        }
    }
}

void* snake1()
{

    int buffer[ALTURA*LARGURA][2] = {{0}};
    int cima = 0, baixo = 0, esquerda = 0, direita = 0;
    int x0 = (int)(0.25*LARGURA);
    int y0 = (int)(0.75*ALTURA);
    int i = y0;
    int j = x0;
    int iMovimento = 0;
    int iApagar, jApagar;

    while(i<ALTURA-1 && i>=1 && j<LARGURA-1 && j>1 && M[i][j] != 1 && M[i][j] != 1 && M[i][j] != 2)
    {

        cima = GetAsyncKeyState('W');
        baixo = GetAsyncKeyState(0x53);
        esquerda = GetAsyncKeyState(0x41);
        direita = GetAsyncKeyState(0x44);

        if(cima)
        {
            esquerda = 0;
            direita = 0;
            while(!esquerda && !direita && i<ALTURA-1 && i>=1 && j<LARGURA-1 && j>1 && M[i][j] != 1 && M[i][j] != 2)
            {

                sem_wait(&movement);

                if(M[i][j] == 3) {
                    eatApple(1, j, i);
                }

                buffer[iMovimento][1] = i;
                buffer[iMovimento][2] = j;
                if(iMovimento >= TAMANHO_INICIAL_1)
                {
                    iApagar = buffer[iMovimento-TAMANHO_INICIAL_1][1];
                    jApagar = buffer[iMovimento-TAMANHO_INICIAL_1][2];
                    M[iApagar][jApagar] = 0;
                    imprimirCoordenada(jApagar,iApagar,0);
                    buffer[iMovimento-TAMANHO_INICIAL_1][1] = 0;
                    buffer[iMovimento-TAMANHO_INICIAL_1][2] = 0;
                }

                M[i][j] = 1;
                imprimirCoordenada(j,i,11);

                Sleep(15);
                i--;
                iMovimento++;

                sem_post(&movement);
                esquerda = GetAsyncKeyState(0x41);
                direita = GetAsyncKeyState(0x44);
            }
        }

        if(baixo)
        {
            esquerda = 0;
            direita = 0;
            while(!esquerda && !direita && i<ALTURA-1 && i>=1 && j<LARGURA-1 && j>1 && M[i][j] != 1 && M[i][j] != 2)
            {

                sem_wait(&movement);

                if(M[i][j] == 3) {
                    eatApple(1, j, i);
                }

                buffer[iMovimento][1] = i;
                buffer[iMovimento][2] = j;
                if(iMovimento >= TAMANHO_INICIAL_1)
                {
                    iApagar = buffer[iMovimento-TAMANHO_INICIAL_1][1];
                    jApagar = buffer[iMovimento-TAMANHO_INICIAL_1][2];
                    M[iApagar][jApagar] = 0;
                    imprimirCoordenada(jApagar,iApagar,0);
                    buffer[iMovimento-TAMANHO_INICIAL_1][1] = 0;
                    buffer[iMovimento-TAMANHO_INICIAL_1][2] = 0;
                }

                M[i][j] = 1;
                imprimirCoordenada(j,i,11);
                Sleep(15);
                i++;
                iMovimento++;

                sem_post(&movement);
                esquerda = GetAsyncKeyState(0x41);
                direita = GetAsyncKeyState(0x44);
            }
        }

        if(direita)
        {
            baixo = 0;
            cima = 0;
            while(!cima && !baixo && i<ALTURA-1 && i>=1 && j<LARGURA-1 && j>1 && M[i][j] != 1 && M[i][j] != 2)
            {

                sem_wait(&movement);

                if(M[i][j] == 3) {
                    eatApple(1, j, i);
                }

                buffer[iMovimento][1] = i;
                buffer[iMovimento][2] = j;
                if(iMovimento >= TAMANHO_INICIAL_1)
                {
                    iApagar = buffer[iMovimento-TAMANHO_INICIAL_1][1];
                    jApagar = buffer[iMovimento-TAMANHO_INICIAL_1][2];
                    M[iApagar][jApagar] = 0;
                    imprimirCoordenada(jApagar,iApagar,0);
                    buffer[iMovimento-TAMANHO_INICIAL_1][1] = 0;
                    buffer[iMovimento-TAMANHO_INICIAL_1][2] = 0;
                }

                M[i][j] = 1;
                imprimirCoordenada(j,i,11);
                Sleep(15);
                j++;
                iMovimento++;

                sem_post(&movement);
                cima = GetAsyncKeyState(0x57);
                baixo = GetAsyncKeyState(0x53);
            }
        }

        if(esquerda)
        {
            baixo = 0;
            cima = 0;
            while(!cima && !baixo && i<ALTURA-1 && i>=1 && j<LARGURA-1 && j>1 && M[i][j] != 1 && M[i][j] != 2)
            {

                sem_wait(&movement);

                if(M[i][j] == 3) {
                    eatApple(1, j, i);
                }

                buffer[iMovimento][1] = i;
                buffer[iMovimento][2] = j;
                if(iMovimento >= TAMANHO_INICIAL_1)
                {
                    iApagar = buffer[iMovimento-TAMANHO_INICIAL_1][1];
                    jApagar = buffer[iMovimento-TAMANHO_INICIAL_1][2];
                    M[iApagar][jApagar] = 0;
                    imprimirCoordenada(jApagar,iApagar,0);
                    buffer[iMovimento-TAMANHO_INICIAL_1][1] = 0;
                    buffer[iMovimento-TAMANHO_INICIAL_1][2] = 0;
                }

                M[i][j] = 1;

                imprimirCoordenada(j,i,11);

                Sleep(15);
                j--;
                iMovimento++;

                sem_post(&movement);
                cima = GetAsyncKeyState(0x57);
                baixo = GetAsyncKeyState(0x53);
            }
        }
    }
    printf("Azul Morreu!");
    Sleep(1000);
    exit(0);
    return NULL;
}


void* snake2()
{

    int buffer[ALTURA*LARGURA][2] = {{0}};
    int cima = 0, baixo = 0, esquerda = 0, direita = 0;
    int x0 = (int)(0.25*LARGURA);
    int y0 = (int)(0.50*ALTURA);
    int i = y0;
    int j = x0;
    int iMovimento = 0;
    int iApagar, jApagar;

    while(i<ALTURA-1 && i>=1 && j<LARGURA-1 && j>1 && M[i][j] != 1 && M[i][j] != 2)
    {

        cima = GetAsyncKeyState(VK_UP);
        baixo = GetAsyncKeyState(VK_DOWN);
        esquerda = GetAsyncKeyState(VK_LEFT);
        direita = GetAsyncKeyState(VK_RIGHT);

        if(cima)
        {
            esquerda = 0;
            direita = 0;
            while(!esquerda && !direita && i<ALTURA-1 && i>=1 && j<LARGURA-1 && j>1 && M[i][j] != 1 && M[i][j] != 2)
            {


                sem_wait(&movement);
                if(M[i][j] == 3) {
                    eatApple(2, j, i);
                }
                buffer[iMovimento][1] = i;
                buffer[iMovimento][2] = j;
                if(iMovimento >= TAMANHO_INICIAL_2)
                {
                    iApagar = buffer[iMovimento-TAMANHO_INICIAL_2][1];
                    jApagar = buffer[iMovimento-TAMANHO_INICIAL_2][2];
                    M[iApagar][jApagar] = 0;
                    imprimirCoordenada(jApagar,iApagar,0);
                    buffer[iMovimento-TAMANHO_INICIAL_2][1] = 0;
                    buffer[iMovimento-TAMANHO_INICIAL_2][2] = 0;
                }

                M[i][j] = 1;
                imprimirCoordenada(j,i,12);
                Sleep(15);
                i--;
                iMovimento++;

                sem_post(&movement);
                esquerda = GetAsyncKeyState(VK_LEFT);
                direita = GetAsyncKeyState(VK_RIGHT);
            }
        }

        if(baixo)
        {
            esquerda = 0;
            direita = 0;
            while(!esquerda && !direita && i<ALTURA-1 && i>=1 && j<LARGURA-1 && j>1 && M[i][j] != 1 && M[i][j] != 2)
            {


                sem_wait(&movement);
                if(M[i][j] == 3) {
                    eatApple(2, j, i);
                }
                buffer[iMovimento][1] = i;
                buffer[iMovimento][2] = j;
                if(iMovimento >= TAMANHO_INICIAL_2)
                {
                    iApagar = buffer[iMovimento-TAMANHO_INICIAL_2][1];
                    jApagar = buffer[iMovimento-TAMANHO_INICIAL_2][2];
                    M[iApagar][jApagar] = 0;
                    imprimirCoordenada(jApagar,iApagar,0);
                    buffer[iMovimento-TAMANHO_INICIAL_2][1] = 0;
                    buffer[iMovimento-TAMANHO_INICIAL_2][2] = 0;
                }

                M[i][j] = 1;

                imprimirCoordenada(j,i,12);

                Sleep(15);
                i++;
                iMovimento++;

                sem_post(&movement);
                esquerda = GetAsyncKeyState(VK_LEFT);
                direita = GetAsyncKeyState(VK_RIGHT);
            }
        }

        if(direita)
        {
            baixo = 0;
            cima = 0;
            while(!cima && !baixo && i<ALTURA-1 && i>=1 && j<LARGURA-1 && j>1 && M[i][j] != 1 && M[i][j] != 2)
            {

                sem_wait(&movement);
                if(M[i][j] == 3) {
                    eatApple(2, j, i);
                }
                buffer[iMovimento][1] = i;
                buffer[iMovimento][2] = j;
                if(iMovimento >= TAMANHO_INICIAL_2)
                {
                    iApagar = buffer[iMovimento-TAMANHO_INICIAL_2][1];
                    jApagar = buffer[iMovimento-TAMANHO_INICIAL_2][2];
                    M[iApagar][jApagar] = 0;
                    imprimirCoordenada(jApagar,iApagar,0);
                    buffer[iMovimento-TAMANHO_INICIAL_2][1] = 0;
                    buffer[iMovimento-TAMANHO_INICIAL_2][2] = 0;
                }

                M[i][j] = 1;

                imprimirCoordenada(j,i,12);

                Sleep(15);
                j++;
                iMovimento++;

                sem_post(&movement);
                cima = GetAsyncKeyState(VK_UP);
                baixo = GetAsyncKeyState(VK_DOWN);
            }
        }

        if(esquerda)
        {
            baixo = 0;
            cima = 0;
            while(!cima && !baixo && i<ALTURA-1 && i>=1 && j<LARGURA-1 && j>1 && M[i][j] != 1 && M[i][j] != 2)
            {

                sem_wait(&movement);
                if(M[i][j] == 3) {
                    eatApple(2, j, i);
                }
                buffer[iMovimento][1] = i;
                buffer[iMovimento][2] = j;
                if(iMovimento >= TAMANHO_INICIAL_2)
                {
                    iApagar = buffer[iMovimento-TAMANHO_INICIAL_2][1];
                    jApagar = buffer[iMovimento-TAMANHO_INICIAL_2][2];
                    M[iApagar][jApagar] = 0;
                    imprimirCoordenada(jApagar,iApagar,0);
                    buffer[iMovimento-TAMANHO_INICIAL_2][1] = 0;
                    buffer[iMovimento-TAMANHO_INICIAL_2][2] = 0;
                }

                M[i][j] = 1;

                imprimirCoordenada(j,i,12);

                Sleep(15);
                j--;
                iMovimento++;


                sem_post(&movement);
                cima = GetAsyncKeyState(VK_UP);
                baixo = GetAsyncKeyState(VK_DOWN);
            }
        }
    }

    printf("Vermelho Morreu!");
    Sleep(1000);
    exit(0);

    int o;
    for(o = 0; o < sizeof(buffer)/sizeof(int); o++)
    {
        buffer[i][0] = 0;
        buffer[i][1] = 0;
    }

    return NULL;
}

void mostrarPontuacao()
{
    COORD coord;
    HANDLE hOut;

    coord.X = 5;
    coord.Y = 0;
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);


    //SetConsoleTextAttribute(hOut,cor);
    SetConsoleCursorPosition(hOut,coord);
    printf("Snake 1: %d ", TAMANHO_INICIAL_1);

    coord.X = 7;
    printf("Snake 2: %d", TAMANHO_INICIAL_2);

    SetConsoleTextAttribute(hOut,7);

}

int main()
{
    pthread_t th1;
    pthread_t th2;

    pthread_t tApple;

    sem_init(&movement,0,1);
    sem_init(&palco,0,1);
    sem_init(&vagos,0,10);
    sem_init(&preenchidos,0,0);
    sem_init(&tela,0,1);

    system("cls");
    imprimirQuadro();
    mostrarPontuacao();

    pthread_create(&tApple, NULL, createApple, NULL);
    pthread_create(&th1, NULL, snake1, NULL);
    pthread_create(&th2, NULL, snake2, NULL);

    pthread_join(tApple, NULL);
    pthread_join(th1, NULL);
    pthread_join(th2, NULL);


    return 0;
}
