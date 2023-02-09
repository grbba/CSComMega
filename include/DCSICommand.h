#ifndef DCSICommand_h
#define DCSICommand_h

#include <Arduino.h>
#include <DCSIconfig.h>
#include <DCSIlog.h>
#include <Queue.h>


// to be changed according to the numnr of commands
// defined in the cpp file
#define MAX_COMMANDS 5      // The number of commands the interface supports
#define MAX_PARAMS 5        // max number of parameters for a handler
#define MAX_NAME_LENGTH 4   // max length of a command name

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

class Commands
{
private:
    static char* CommandNames[MAX_COMMANDS];
    static Command* CommandRef[MAX_COMMANDS];
    static int count;
    static CommandParams pq;
    static void prepare(char *cmd); // once a command recieved / prepare for execution i.e.
                                    // fill the queue with the command found and the
                                    // ptr to the parameters represented as strings
public:
    static void run(const char *cmd);
    static CommandParams *getCommandParams();
    static void insert(Command *c);
    static Command *find(char * c);

    Commands() {
        count = 0;
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
        // fill the arrays in the map
        Commands::insert(&self());
    };
};

extern Commands Cmds;
#endif