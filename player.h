#ifndef PLAYER_H
#define PLAYER_H
void play_module(struct Player *p, const struct Module *m, 
                  unsigned long count);
void init_player(struct Player *p, struct Module *m);
#endif
