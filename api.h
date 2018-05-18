#ifndef API_H
#define API_H


int parse(char *message);
int regUser(const char *username, const char *password);
int authUser(char *username, char *password);
int sendMsg(char* token, char* msg, char* recipient);
int getUpdates(char *username);

#endif /* API_H */
