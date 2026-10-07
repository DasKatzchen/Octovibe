#ifndef ICOMMUNICATION_H
#define ICOMMUNICATION_H

#include <stddef.h>

//Interface for communication
class ICommunication {

  public:
    virtual bool isOpen() = 0;

    virtual void start() = 0;

    virtual void output(char* data) = 0;

    // Copies the next received message into input (at most size - 1 chars, always
    // null-terminated). Returns false if there is no pending message.
    virtual bool readData(char* input, size_t size) = 0;
};

#endif