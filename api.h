#ifndef API_H
#define API_H


int initApi(Usertable*);
char* parse(char*);
int regUser(const char*, const char*);
char* authUser(const char*, const char*);
char* sendMsg(const char*, const char*, const char*, const char*);
char* getUpdates(const char*, const char*);
char* formatError();
char* formatResponse(char*, json_t*);

#endif /* API_H */
