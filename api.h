#ifndef API_H
#define API_H


char* parse(char *message);
int regUser(const char *username, const char *password);
char* authUser(const char *username, const char *password);
char* sendMsg(const char *username, const char *token, const char *msg, const char *recipient);
char* getUpdates(char *username, char *token);
char* formatError();
char* formatResponse(char* status, json_t *params);

#endif /* API_H */
