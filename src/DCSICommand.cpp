
#include <Queue.h>
#include <DCSIlog.h>
#include <DCSICommand.h>

Queue<char *, MAX_PARAMS + 1> Commands::pq;

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

void Commands::run(const char *cmd)
{

  char buffer[strlen(cmd)];
  strcpy(buffer, cmd);
  removeChars(buffer, (char *)"<>!");

  prepare(buffer);
  const char *c = pq.pop(); // command the rest of the params are still in the queue

  if (CommandMap.containsKey((char *)c))
  {

    Command *cmd = CommandMap.get((char *)c); // get the command from the Map
    cmd->exec(Commands::getCommandParams());  // execute the command found
  }
  else
  {
    ERR(F("Command %s unknown" CR), c);
    pq.clear(); // clear the queue
    return;
  }
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

  TRC(F("LogLevel set from %d to %d" CR), dccLog.getLevel(), ll );
 

  p.clear(); // clear the queue; not necessary if we handle the command properly
  return 1;
}

int handletl(paramType &ptlist, CommandParams &p) 
{
  int ll = atoi(p.pop());
  dccLog.setLevel(ll);

  INFO("INFO message");
  WARN("WARN message");
  FATAL("FATAL message");
  ERR("ERR message");
  TRC("TRC message");

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
static const Command llv((char *)"llv", handleLLV, paramType::NUM_T);

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