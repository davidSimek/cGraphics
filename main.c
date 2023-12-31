#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define THREAD_COUNT 10 

// this is extremely general structure
// you simply just pass it arguments in struct which you have to cast to void
// then you pass function that follows void (*execute)(void*) signature and it will pass it that args
typedef struct {
    void* arg;
    void (*execute)(void*);
} ThreadData;

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Pixel;

typedef struct {
    const int width;
    const int height;
    int headerSize;
    size_t size;
} ImgBufferConfig;

void* run(void* arg);
void spawnThreads(const ThreadData* data, pthread_t* thread, const int threadCount);
int mergeThreads(ThreadData* data, pthread_t* thread);
unsigned char* createBuffer(ImgBufferConfig* bufferConfig);
int digitsCount(int num);

// not part of library, user defined:
typedef struct {
    int line;
} PerLineData;
void perLine(void* s) {
    PerLineData* data = (PerLineData*)s;
    printf("currently on line %d\n", data->line);
}


int main(int argc, char *argv[])
{
    ImgBufferConfig bufferConfig = {100, 100, 0, 0}; 
    unsigned char* buffer = createBuffer(&bufferConfig);

    // make array of thread data for each line
    ThreadData* data = (ThreadData*)malloc(sizeof(pthread_t) * bufferConfig.height);
    PerLineData* perLineData = (PerLineData*)malloc(sizeof(PerLineData) * bufferConfig.height);
    for (int i = 0; i < bufferConfig.height; i++) {
        perLineData[i].line = i;
        data[i].arg = &perLineData[i];
        data[i].execute = perLine;
    }

    // make array of threads for each line
    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * bufferConfig.height);


    spawnThreads(data, threads, bufferConfig.height);
    mergeThreads(data, threads);

    printf("%s", buffer);
    free(buffer);
    free(data);
    free(threads);
    free(perLineData);
    return 0;
}

// implementations

int digitsCount(int num) {
    int count = 0;

    while (num != 0) {
        num /= 10;
        count++;
    }

    return count;
}

unsigned char* createBuffer(ImgBufferConfig* bufferConfig) {
    bufferConfig->headerSize = 
        3 +                             // P6
        digitsCount(bufferConfig->height) + 1 + // no \n here! resolution
        digitsCount(bufferConfig->width)  + 1 + 
        4;                              // 255

    bufferConfig->size = bufferConfig->headerSize + bufferConfig->width * bufferConfig->height * 3 + 1;

    unsigned char* buffer = (unsigned char*)malloc(bufferConfig->size);

    int widhtChars = digitsCount(bufferConfig->width);
    int heightChars = digitsCount(bufferConfig->height);
    char width[widhtChars];
    char height[heightChars];

    sprintf(width, "%d", bufferConfig->width);
    sprintf(height, "%d", bufferConfig->height);

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
    
    // fill it with white initialy | should be handled differently in future
    for (int i = 0; i < bufferConfig->height; i++) {
        for (int j = 0; j < bufferConfig->width; j++) {
            int index = 3 * (i * bufferConfig->height + j) + bufferConfig->headerSize;
            buffer[index++] = 255; // R
            buffer[index++] = 255; // G
            buffer[index++] = 255;  // B
        }   
    }
    
    return buffer;
}

void* run(void* arg) {
     ThreadData* data = (ThreadData*)arg;
     data->execute(data->arg);
     return 0;
}

void spawnThreads(const ThreadData* data, pthread_t* thread, const int threadCount) {
    for (int i = 0; i < threadCount; i++) {
        if (pthread_create(&thread[i], NULL, run, (void*)&data[i])) {
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
