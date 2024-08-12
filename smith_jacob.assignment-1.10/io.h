#ifndef IO_H
# define IO_H

# include "pair.h"
#include <iostream>

class character;
class makePokemon;

void io_init_terminal(void);
void io_reset_terminal(void);
void addItems(void);
void io_display(void);
void io_handle_input(pair_t dest);
void io_bag(void);
void io_pokeCheck(void);
void io_queue_message(const char *format, ...);
void io_battle(character *aggressor, character *defender);
void selectPokemon(void);

#endif
