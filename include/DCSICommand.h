#ifndef DCSICommand_h
#define DCSICommand_h

#include <Arduino.h>
#include <DCSIconfig.h>
#include <DCSIlog.h>
#include <HashMap.h>
#include <Queue.h>

// The number of commands the interface supports
// to be changed according to the numnr of commands
// defined in the cpp file
#define MAX_COMMANDS 2
// max number of parameters for a handler
#define MAX_PARAMS 5
// max length of a command name
// any length can be fed into the command constructor
// but only up to what has been defined here - 1 will be
// accepted.
#define MAX_NAME_LENGTH 4

// Types of parameters allowed in a command for the CommandStation and/or NetworkStation
enum class paramType
{
    NUM_T, // numbers
    STR_T  // strings
};

using CommandParams = Queue<char *, MAX_PARAMS + 1>;
using CommandHandler = int (*)(paramType &, CommandParams &);


// forward declaration of the Command
class Command;

typedef HashMap<char *, Command *, MAX_COMMANDS> _CommandMap;
static _CommandMap CommandMap;
// map holding all commands. Will be constructed at inital construction of the commands
// as defined in the cpp file

class Commands
{
private:
    static CommandParams pq;
    static void prepare(char *cmd); // once a command recieved / prepare for execution i.e.
                                    // fill the queue with the command found and the
                                    // ptr to the parameters represented as strings
public:
    static void run(const char *cmd);
    static CommandParams *getCommandParams() {
        return &pq;
    }
};

class Command
{
private:
    char name[MAX_NAME_LENGTH]; // has a name max 3 char plus terminator
    paramType pt[MAX_PARAMS];
    CommandHandler f;

    Command &self() { return *this; }

    // recursivley handle the parameter types of the command as provided
    // by the constructor
    int i = 0;                      // i+1 == number of parameters the queue should hold excluding 
                                    // the first element in the queue which will be the command
                                    
    void processTypes(){};
    template <typename T1, typename... T>
    void processTypes(const T1 &t1, const T &...t)
    {
        pt[i++] = t1;
        processTypes(t...);
    }

public:

    auto exec(CommandParams *p) -> void
    {
        // sanity check: Does the parameter type definition hold as many entries as we have in the queue
         if (p->size() == (size_t)(i)) {
            f(*pt,*p);
         } else {
            ERR(F("#%d of parameter types do not correpsond to provided #%d of parameter values" CR), p->size(), i+1);
         }
    }

    char *getName()
    {
        return name;
    }

    template <typename... Args>
    Command(char *n, CommandHandler c, Args... p) /* give it a hashmap to be added to  */
    {
        strncpy(name, n, 3); // set the name
        processTypes(p...);  // initalize the array with the types of the arguments
        f = c;               // set the handler for the command
        CommandMap.put(name, &self()); // add the command to the commands map
    };
};

extern Commands Cmds;
#endif