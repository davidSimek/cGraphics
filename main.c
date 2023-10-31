#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>

#define THREAD_COUNT 10 
#define IMAGE_WIDTH 10
#define IMAGE_HEIGHT 10

typedef struct {
    int number;
} ThreadData;

typedef struct {
    char r;
    char g;
    char b;
} Pixel;

void* run(void* arg);
void spawnThreads(ThreadData* data, pthread_t* thread);
int mergeThreads(ThreadData* data, pthread_t* thread);

int digitsCount(int num) {
    int count = 0;

    while (num != 0) {
        num /= 10;
        count++;
    }

    return count;
}

int main(int argc, char *argv[])
{
    int sizeOfHeader =
        3 +                             // P6
        digitsCount(IMAGE_HEIGHT) + 1 + // no \n here! resolution
        digitsCount(IMAGE_WIDTH)  + 1 + 
        4;                              // 255

    int sizeOfBuffer = sizeOfHeader + IMAGE_WIDTH * IMAGE_HEIGHT * 3 + 1; // data

    char* buffer = (char*)malloc(sizeOfBuffer);

    int widhtChars = digitsCount(IMAGE_WIDTH);
    int heightChars = digitsCount(IMAGE_HEIGHT);
    char width[widhtChars];
    char height[heightChars];

    sprintf(width, "%d", IMAGE_WIDTH);
    sprintf(height, "%d", IMAGE_HEIGHT);

    buffer[0] = 'P';
    buffer[1] = '6';
    buffer[2] = '\n';
    for (int i = 0; i < widhtChars; i++) {
        buffer[i + 3] = width[i];
    }
    buffer[4 + widhtChars - 1] = ' ';
    for (int i = 0; i < heightChars; i++) {
        buffer[i + 4 + widhtChars] = height[i];
    }
    buffer[4 + widhtChars + heightChars] = '\n';
    buffer[5 + widhtChars + heightChars] = '2';
    buffer[6 + widhtChars + heightChars] = '5';
    buffer[7 + widhtChars + heightChars] = '5';
    buffer[8 + widhtChars + heightChars] = '\n';
    for (int i = 0; i < IMAGE_HEIGHT; i++) {
        for (int j = 0; j < IMAGE_WIDTH; j++) {
            int index = 3 * (i * IMAGE_HEIGHT + j) + sizeOfHeader;
            buffer[index++] = 255; // R
            buffer[index++] = 128; // G
            buffer[index++] = 64;  // B
        }   
    }



    printf("%s", buffer);
    return 0;
}

// implementations

void* run(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    data->number++;
    return 0;
}

void spawnThreads(ThreadData* data, pthread_t* thread) {
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pthread_create(&thread[i], NULL, run, (void*)data)) {
            fprintf(stderr, "lol, you will not make thread");
        }
    }
}

int mergeThreads(ThreadData* data, pthread_t* thread) {
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pthread_join(thread[i], NULL)) {
            fprintf(stderr, "Error joining thread\n");
            return 1;
        }
    }
    return 0;
}

