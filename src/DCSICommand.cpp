
#include <Queue.h>
#include <DCSIlog.h>
#include <DCSICommand.h>

Queue<char *, MAX_PARAMS + 1> Commands::pq;
char *Commands::CommandNames[MAX_COMMANDS];
Command *Commands::CommandRef[MAX_COMMANDS];
int Commands::count;

// fwd decl
void removeChars(char *str, char *remove);

void Commands::prepare(char *cmd)
{

  char *token;
  char *rest = cmd;

  while ((token = strtok_r(rest, " ", &rest)))
  {
    TRC(F("Pushing Token: %s" CR), token);
    Cmds.pq.push(token);
  }
}
CommandParams *Commands::getCommandParams() {
  return &pq;
}
void Commands::insert(Command *c)
{
  if (count == MAX_COMMANDS)
    return; // array is full cant insert anything
  Commands::CommandNames[count] = (char *)c->getName();
  Commands::CommandRef[count] = c;
  Commands::count++;
  return;
}
void Commands::run(const char *cmd)
{

  char buffer[strlen(cmd)];
  strcpy(buffer, cmd);
  removeChars(buffer, (char *)"<>!");

  prepare(buffer);
  const char *c = pq.pop(); // command the rest of the params are still in the queue

  // get the command from the Map
  Command *cm = Commands::find((char *)c);

  if (cm != nullptr)
  {
    cm->exec(Commands::getCommandParams()); // execute the command found
  }
  else
  {
    ERR(F("Command %s unknown" CR), c);
    pq.clear(); // clear the queue
    return;
  }
}
Command *Commands::find(char *c)
{
  int i = 0;
  while (strcmp(CommandNames[i], c) != 0)
  {
    i++;
    if (i == MAX_COMMANDS)
      return nullptr;
  }
  return CommandRef[i];
}

/*--------------------------------------------------------*/
// Handler functions for the commands
/*--------------------------------------------------------*/
int handleLLV(paramType &ptlist, CommandParams &p)
{
  INFO("Executing LLV" CR);
  p.clear(); // clear the queue; not necessary if we handle the command properly
  return 1;
}
int handleDiag(paramType &ptlist, CommandParams &p)
{
  INFO("Executing Diag" CR);
  int ll = atoi(p.pop());
  Log.setLevel(ll);

  TRC(F("LogLevel set from %d to %d" CR), dccLog.getLevel(), ll);

  p.clear(); // clear the queue; not necessary if we handle the command properly
  return 1;
}
int handletl(paramType &ptlist, CommandParams &p)
{
  int ll = atoi(p.pop());
  dccLog.setLevel(ll);

  FATAL("1: FATAL message" CR);
  WARN("2: WARN message" CR);
  ERR("3 :ERR message" CR);
  INFO("4: INFO message" CR);
  TRC("5: TRC message" CR);

  p.clear();

  return 1;
}

/*--------------------------------------------------------*/
// Declare Commands we understand
/*--------------------------------------------------------*/
/**
 * @brief send the diagnostcs of the CommandStation to the client provided in the
 * message e;g. <! diag 1>. When the comm part of on the CS sends a diag message the
 * client will be set to the id provided here.
 */
static const Command diag((char *)"dia", handleDiag, paramType::NUM_T);
/**
 * @brief set the loglevel on the CommandStation
 *
 */
static const Command logl((char *)"llv", handleLLV, paramType::NUM_T);

static const Command testlog((char *)"tl", handletl, paramType::NUM_T);

// Helper functions
void removeChars(char *str, char *remove)
{
  int strIndex = 0; // removeIndex;
  for (size_t i = 0; i < strlen(str); i++)
  {
    bool found = false;
    for (size_t j = 0; j < strlen(remove); j++)
      if (str[i] == remove[j])
      {
        found = true;
        break;
      }
    if (!found)
      str[strIndex++] = str[i];
  }
  str[strIndex] = '\0';
}