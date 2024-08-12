#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <vector>
#include <math.h>

#include "io.h"
#include "character.h"
#include "poke327.h"

using namespace std;

typedef struct io_message {
  /* Will print " --more-- " at end of line when another message follows. *
   * Leave 10 extra spaces for that.                                      */
  char msg[71];
  struct io_message *next;
} io_message_t;

static io_message_t *io_head, *io_tail;

//Intiializing ncurses for background/foreground colors for terminal aesthetics.
void io_init_terminal(void)
{
  initscr();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
  init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
  init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
  init_color(COLOR_GREEN, 161, 349, 173);
  init_color(COLOR_MAGENTA, 345, 349, 357);
  init_color(COLOR_CYAN, 459, 298, 161);
  init_color(COLOR_BLUE, 216, 404, 537);
  init_color(COLOR_YELLOW, 647, 698, 600);
  init_color(COLOR_RED, 624, 224, 275);
}

void io_reset_terminal(void)
{
  endwin();

  while (io_head) {
    io_tail = io_head;
    io_head = io_head->next;
    free(io_tail);
  }
  io_tail = NULL;
}

void io_queue_message(const char *format, ...)
{
  io_message_t *tmp;
  va_list ap;

  if (!(tmp = (io_message_t *) malloc(sizeof (*tmp)))) {
    perror("malloc");
    exit(1);
  }

  tmp->next = NULL;

  va_start(ap, format);

  vsnprintf(tmp->msg, sizeof (tmp->msg), format, ap);

  va_end(ap);

  if (!io_head) {
    io_head = io_tail = tmp;
  } else {
    io_tail->next = tmp;
    io_tail = tmp;
  }
}

static void io_print_message_queue(uint32_t y, uint32_t x)
{
  while (io_head) {
    io_tail = io_head;
    attron(COLOR_PAIR(COLOR_CYAN));
    mvprintw(y, x, "%-80s", io_head->msg);
    attroff(COLOR_PAIR(COLOR_CYAN));
    io_head = io_head->next;
    if (io_head) {
      attron(COLOR_PAIR(COLOR_CYAN));
      mvprintw(y, x + 70, "%10s", " --more-- ");
      attroff(COLOR_PAIR(COLOR_CYAN));
      refresh();
      getch();
    }
    free(io_tail);
  }
  io_tail = NULL;
}

/**************************************************************************
 * Compares trainer distances from the PC according to the rival distance *
 * map.  This gives the approximate distance that the PC must travel to   *
 * get to the trainer (doesn't account for crossing buildings).  This is  *
 * not the distance from the NPC to the PC unless the NPC is a rival.     *
 *                                                                        *
 * Not a bug.                                                             *
 **************************************************************************/
static int compare_trainer_distance(const void *v1, const void *v2)
{
  const character *const *c1 = (const character * const *) v1;
  const character *const *c2 = (const character * const *) v2;

  return (world.rival_dist[(*c1)->pos[dim_y]][(*c1)->pos[dim_x]] -
          world.rival_dist[(*c2)->pos[dim_y]][(*c2)->pos[dim_x]]);
}

static character *io_nearest_visible_trainer()
{
  character **c, *n;
  uint32_t x, y, count;

  c = (character **) malloc(world.cur_map->num_trainers * sizeof (*c));

  /* Get a linear list of trainers */
  for (count = 0, y = 1; y < MAP_Y - 1; y++) {
    for (x = 1; x < MAP_X - 1; x++) {
      if (world.cur_map->cmap[y][x] && world.cur_map->cmap[y][x] !=
          &world.pc) {
        c[count++] = world.cur_map->cmap[y][x];
      }
    }
  }

  /* Sort it by distance from PC */
  qsort(c, count, sizeof (*c), compare_trainer_distance);

  n = c[0];

  free(c);

  return n;
}

void addItems(){
  int r1 = 4 + rand() %11;
  int r2 = 6 + rand() %11;
  int r3 = rand()% 2 + 5;;

  world.pc.numPotions = r1;
  world.pc.numPokeballs = r2;
  world.pc.numRevives = r3;

  world.pc.basePotions = r1;
  world.pc.basePokeballs = r2;
  world.pc.baseRevives = r3;
}

void io_display()
{
  uint32_t y, x;
  character *c;

  clear();
  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      if (world.cur_map->cmap[y][x]) {
        if(world.cur_map->cmap[y][x]->symbol != '@'){
          attron(COLOR_PAIR(COLOR_RED));
          mvaddch(y + 1, x, world.cur_map->cmap[y][x]->symbol);
          attroff(COLOR_PAIR(COLOR_RED));
        }
        else{
           attron(COLOR_PAIR(COLOR_WHITE));
          mvaddch(y + 1, x, world.cur_map->cmap[y][x]->symbol);
          attroff(COLOR_PAIR(COLOR_WHITE));
        }
        

      } else {
        switch (world.cur_map->map[y][x]) {
        case ter_boulder:
          attron(COLOR_PAIR(COLOR_MAGENTA));
          mvaddch(y + 1, x, BOULDER_SYMBOL);
          attroff(COLOR_PAIR(COLOR_MAGENTA));
          break;
        case ter_mountain:
          attron(COLOR_PAIR(COLOR_MAGENTA));
          mvaddch(y + 1, x, MOUNTAIN_SYMBOL);
          attroff(COLOR_PAIR(COLOR_MAGENTA));
          break;
        case ter_tree:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, TREE_SYMBOL);
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        case ter_forest:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, FOREST_SYMBOL);
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        case ter_path:
          attron(COLOR_PAIR(COLOR_YELLOW));
          mvaddch(y + 1, x, PATH_SYMBOL);
          attroff(COLOR_PAIR(COLOR_YELLOW));
          break;
        case ter_gate:
          attron(COLOR_PAIR(COLOR_YELLOW));
          mvaddch(y + 1, x, GATE_SYMBOL);
          attroff(COLOR_PAIR(COLOR_YELLOW));
          break;
        case ter_mart:
          attron(COLOR_PAIR(COLOR_WHITE));
          mvaddch(y + 1, x, POKEMART_SYMBOL);
          attroff(COLOR_PAIR(COLOR_WHITE));
          break;
        case ter_center:
          attron(COLOR_PAIR(COLOR_WHITE));
          mvaddch(y + 1, x, POKEMON_CENTER_SYMBOL);
          attroff(COLOR_PAIR(COLOR_WHITE));
          break;
        case ter_grass:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, TALL_GRASS_SYMBOL);
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        case ter_clearing:
          attron(COLOR_PAIR(COLOR_GREEN));
          mvaddch(y + 1, x, SHORT_GRASS_SYMBOL);
          attroff(COLOR_PAIR(COLOR_GREEN));
          break;
        case ter_water:
          attron(COLOR_PAIR(COLOR_BLUE));
          mvaddch(y + 1, x, WATER_SYMBOL);
          attroff(COLOR_PAIR(COLOR_BLUE));
          break;
        default:
          attron(COLOR_PAIR(COLOR_CYAN));
          mvaddch(y + 1, x, ERROR_SYMBOL);
          attroff(COLOR_PAIR(COLOR_CYAN)); 
       }
      }
    }
  }

  if(world.pc.num_of_pokemon == 0){
    selectPokemon();
    addItems();
  }

  mvprintw(23, 1, "PC position is (%2d,%2d) on map %d%cx%d%c.",
           world.pc.pos[dim_x],
           world.pc.pos[dim_y],
           abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)),
           world.cur_idx[dim_x] - (WORLD_SIZE / 2) >= 0 ? 'E' : 'W',
           abs(world.cur_idx[dim_y] - (WORLD_SIZE / 2)),
           world.cur_idx[dim_y] - (WORLD_SIZE / 2) <= 0 ? 'N' : 'S');
  mvprintw(22, 1, "%d known %s.", world.cur_map->num_trainers,
           world.cur_map->num_trainers > 1 ? "trainers" : "trainer");
  mvprintw(22, 30, "Nearest visible trainer: ");
  if ((c = io_nearest_visible_trainer())) {
    attron(COLOR_PAIR(COLOR_RED));
    mvprintw(22, 55, "%c at vector %d%cx%d%c.",
             c->symbol,
             abs(c->pos[dim_y] - world.pc.pos[dim_y]),
             ((c->pos[dim_y] - world.pc.pos[dim_y]) <= 0 ?
              'N' : 'S'),
             abs(c->pos[dim_x] - world.pc.pos[dim_x]),
             ((c->pos[dim_x] - world.pc.pos[dim_x]) <= 0 ?
              'W' : 'E'));
    attroff(COLOR_PAIR(COLOR_RED));
  } else {
    attron(COLOR_PAIR(COLOR_BLUE));
    mvprintw(22, 55, "NONE.");
    attroff(COLOR_PAIR(COLOR_BLUE));
  }

  io_print_message_queue(0, 0);

  refresh();
}

void selectPokemon(){
  int choosePokemon;
  makePokemon *f;
  makePokemon *s;
  makePokemon *t;
    
  f = createPokemon();
  s = createPokemon();
  t = createPokemon();

  makePokemon *pokeArray[4] = {f, f, s, t};

        WINDOW * win2 = newwin(15,60,4,10);
        refresh();
        box(win2, 0, 0);
        mvwprintw(win2, 3, 20, "Choose Your Pokemon!");
        if(f->isShiny){
            wattron(win2,COLOR_PAIR(COLOR_YELLOW));
            mvwprintw(win2, 5, 22, "1: %s *shiny*", f->name.c_str());
            wattroff(win2,COLOR_PAIR(COLOR_YELLOW));
        }
        else{
            mvwprintw(win2, 5, 22, "1: %s", f->name.c_str());
        }

        if(s->isShiny){
            wattron(win2,COLOR_PAIR(COLOR_YELLOW));
            mvwprintw(win2, 6, 22, "2: %s *shiny*", s->name.c_str());
            wattroff(win2,COLOR_PAIR(COLOR_YELLOW));
        }
        else{
            mvwprintw(win2, 6, 22, "2: %s", s->name.c_str());
        }
        
        if(t->isShiny){
            wattron(win2,COLOR_PAIR(COLOR_YELLOW));
            mvwprintw(win2, 7, 22, "3: %s *shiny*", t->name.c_str());
            wattroff(win2,COLOR_PAIR(COLOR_YELLOW));
        }
        else{
            mvwprintw(win2, 7, 22, "3: %s", t->name.c_str());
        }

        refresh();
        wrefresh(win2);
        

  choosePokemon = 999;
  while (choosePokemon < 0 || choosePokemon > 3){
      mvwprintw(win2, 3, 20, "Choose Your Pokemon!");

      if(f->isShiny){
            wattron(win2,COLOR_PAIR(COLOR_YELLOW));
            mvwprintw(win2, 5, 22, "1: %s *shiny*", f->name.c_str());
            wattroff(win2,COLOR_PAIR(COLOR_YELLOW));
      }
      else{
            mvwprintw(win2, 5, 22, "1: %s", f->name.c_str());
      }

      if(s->isShiny){
            wattron(win2,COLOR_PAIR(COLOR_YELLOW));
            mvwprintw(win2, 6, 22, "2: %s *shiny*", s->name.c_str());
            wattroff(win2,COLOR_PAIR(COLOR_YELLOW));
      }
      else{
            mvwprintw(win2, 6, 22, "2: %s", s->name.c_str());
      }
      
      if(t->isShiny){
            wattron(win2,COLOR_PAIR(COLOR_YELLOW));
            mvwprintw(win2, 7, 22, "3: %s *shiny*", t->name.c_str());
            wattroff(win2,COLOR_PAIR(COLOR_YELLOW));
      }
      else{
            mvwprintw(win2, 7, 22, "3: %s", t->name.c_str());
      }

      refresh();
      wrefresh(win2);
      choosePokemon = getch() - '0'; 
  }



        wclear(win2);
        box(win2, 0, 0);
        wrefresh(win2);
        mvwprintw(win2, 1, 20, "You Have Selected!  ");

        if(pokeArray[choosePokemon]->isShiny){
            wattron(win2,COLOR_PAIR(COLOR_YELLOW));
            mvwprintw(win2, 2, 18, "Name: %s *shiny*", pokeArray[choosePokemon]->name.c_str());
            wattroff(win2,COLOR_PAIR(COLOR_YELLOW));
        }
        else{
            mvwprintw(win2, 2, 22, "Name: %s", pokeArray[choosePokemon]->name.c_str());
        }

        if(pokeArray[choosePokemon]->gender == 0){
            mvwprintw(win2, 3, 22, "Gender: Female");
        }
        else{
            mvwprintw(win2, 3, 22, "Gender: Male");
        }
        
            mvwprintw(win2, 4, 13, "HP: %d Level: %d Attack: %d Defense: %d", 
                                pokeArray[choosePokemon]->hp, pokeArray[choosePokemon]->level, 
                                pokeArray[choosePokemon]->attack, pokeArray[choosePokemon]->defense);
            mvwprintw(win2, 5, 14, "Speed: %d Accuracy: %d Evasion: %d", pokeArray[choosePokemon]->speed, pokeArray[choosePokemon]->accuracy, pokeArray[choosePokemon]->evasion);
             mvwprintw(win2, 6, 14, "Special-Attack:%d Special-Defense:%d", pokeArray[choosePokemon]->specialAttack, pokeArray[choosePokemon]->specialDefense);
            
            if(pokeArray[choosePokemon]->isShiny){
              mvwprintw(win2, 7, 25, "Shiny: Yes");
            }
            else{
              mvwprintw(win2, 7, 25, "Shiny: No");
            }
            
            mvwprintw(win2, 9, 22, "Move 1: %s ", pokeArray[choosePokemon]->m[0].identifier.c_str());
        if(pokeArray[choosePokemon]->m[1].identifier != ""){
            mvwprintw(win2, 10, 22, "Move 2: %s", pokeArray[choosePokemon]->m[1].identifier.c_str());
        }
        else{
            mvwprintw(win2, 10, 22, "Move 2: None");
        }  

        mvwprintw(win2, 12, 16, "Press any key to continue.");
        wrefresh(win2);

  world.pc.num_of_pokemon++;
  world.pc.pokemonArray[0] = pokeArray[choosePokemon];
  getch();
  wclear(win2);

}

uint32_t io_teleport_pc(pair_t dest)
{
  /* Just for fun. And debugging.  Mostly debugging. */

  do {
    dest[dim_x] = rand_range(1, MAP_X - 2);
    dest[dim_y] = rand_range(1, MAP_Y - 2);
  } while (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]                  ||
           move_cost[char_pc][world.cur_map->map[dest[dim_y]]
                                                [dest[dim_x]]] == INT_MAX ||
           world.rival_dist[dest[dim_y]][dest[dim_x]] < 0);

  return 0;
}

static void io_scroll_trainer_list(char (*s)[40], uint32_t count)
{
  uint32_t offset;
  uint32_t i;

  offset = 0;

  while (1) {
    for (i = 0; i < 13; i++) {
      mvprintw(i + 6, 19, " %-40s ", s[i + offset]);
    }
    switch (getch()) {
    case KEY_UP:
      if (offset) {
        offset--;
      }
      break;
    case KEY_DOWN:
      if (offset < (count - 13)) {
        offset++;
      }
      break;
    case 27:
      return;
    }

  }
}

static void io_list_trainers_display(npc **c,
                                     uint32_t count)
{
  uint32_t i;
  char (*s)[40]; /* pointer to array of 40 char */

  s = (char (*)[40]) malloc(count * sizeof (*s));

  mvprintw(3, 19, " %-40s ", "");
  /* Borrow the first element of our array for this string: */
  snprintf(s[0], 40, "You know of %d trainers:", count);
  mvprintw(4, 19, " %-40s ", *s);
  mvprintw(5, 19, " %-40s ", "");

  for (i = 0; i < count; i++) {
    snprintf(s[i], 40, "%16s %c: %2d %s by %2d %s",
             char_type_name[c[i]->ctype],
             c[i]->symbol,
             abs(c[i]->pos[dim_y] - world.pc.pos[dim_y]),
             ((c[i]->pos[dim_y] - world.pc.pos[dim_y]) <= 0 ?
              "North" : "South"),
             abs(c[i]->pos[dim_x] - world.pc.pos[dim_x]),
             ((c[i]->pos[dim_x] - world.pc.pos[dim_x]) <= 0 ?
              "West" : "East"));
    if (count <= 13) {
      /* Handle the non-scrolling case right here. *
       * Scrolling in another function.            */
      mvprintw(i + 6, 19, " %-40s ", s[i]);
    }
  }

  if (count <= 13) {
    mvprintw(count + 6, 19, " %-40s ", "");
    mvprintw(count + 7, 19, " %-40s ", "Hit escape to continue.");
    while (getch() != 27 /* escape */)
      ;
  } else {
    mvprintw(19, 19, " %-40s ", "");
    mvprintw(20, 19, " %-40s ",
             "Arrows to scroll, escape to continue.");
    io_scroll_trainer_list(s, count);
  }

  free(s);
}

static void io_list_trainers()
{
  npc **c;
  uint32_t x, y, count;

  c = (npc **) malloc(world.cur_map->num_trainers * sizeof (*c));

  /* Get a linear list of trainers */
  for (count = 0, y = 1; y < MAP_Y - 1; y++) {
    for (x = 1; x < MAP_X - 1; x++) {
      if (world.cur_map->cmap[y][x] && world.cur_map->cmap[y][x] !=
          &world.pc) {
        c[count++] = dynamic_cast<npc *> (world.cur_map->cmap[y][x]);
      }
    }
  }

  /* Sort it by distance from PC */
  qsort(c, count, sizeof (*c), compare_trainer_distance);

  /* Display it */
  io_list_trainers_display(c, count);
  free(c);

  /* And redraw the map */
  io_display();
}

void io_pokemart()
{
  mvprintw(0, 0, "Welcome to the Pokemart.  Could I interest you in some Pokeballs?");
  refresh();
  world.pc.numPotions = world.pc.basePotions;
  world.pc.numPokeballs = world.pc.basePokeballs;
  world.pc.numRevives = world.pc.baseRevives;
  getch();
  io_display();
  mvprintw(0, 0, "All restocked! Get back out there trainer!");
  refresh();
  getch();
}

void io_pokemon_center()
{
  mvprintw(0, 0, "Welcome to the Pokemon Center.  How can Nurse Joy assist you?");
  refresh();
  getch();
  
  for(int i = 0; i < world.pc.num_of_pokemon; i++){
    world.pc.pokemonArray[i]->hpDam = world.pc.pokemonArray[i]->hp;
    world.pc.pokemonArray[i]->kO = false;
    world.pc.totalDefeat = false;
  }
  io_display();
  mvprintw(0, 0, "All Pokemon Restored! Get back out there trainer!");
  getch();
}

void levelUpStats(makePokemon *p){
  p->hp = floor(((p->hpBase + p->IV[0]) * 2 * p->level)/100) + p->level + 10 ;
  p->hpDam = p->hp;

  p->attack = floor(((p->attackBase + p->IV[1]) * 2 * p->level)/100) + 5;
  p->defense = floor(((p->defenseBase + p->IV[2]) * 2 * p->level)/100) + 5;
  p->specialAttack = floor(((p->specialAttackBase + p->IV[3]) * 2 * p->level)/100) + 5;
  p->specialDefense = floor(((p->specialDefenseBase + p->IV[4]) * 2 * p->level)/100) + 5;
  p->speed = floor(((p->speedBase + p->IV[5]) * 2 * p->level)/100) + 5;
}

bool enemyDefeat(character *n){
  int i = 0;
  int koCount = 0;
  while(i < n->num_of_pokemon){
        if(n->pokemonArray[i]->kO){
          koCount++;
        }
        i++;    
  }
  if(koCount == n->num_of_pokemon){
    return true;
  }
  else{
    return false;
  }
}

bool checkDefeat(){
  int i = 0;
  int koCount = 0;
  while(i < world.pc.num_of_pokemon){
        if(world.pc.pokemonArray[i]->kO){
          koCount++;
        }
        i++;    
  }
  if(koCount == world.pc.num_of_pokemon){
    return true;
  }
  else{
    return false;
  }
}

int damageCalc(int level, int att, moves m, int otherDef, makePokemon *p){
  int random = 85 + rand()% 100;
  int pow;
  int miss = rand()% 100;
  int critProb = rand()% 256;
  int crit;

  if(miss < m.accuracy){
  
    if(m.power == INT_MAX){
      pow = 1;
    }
    else{
      pow = m.power;
    }

    if(critProb < (p->speed/2)){
      crit = 1.5;
    }
    else{
      crit = 1;
    }

    int damage = ((((((2*level)/5)+2) * pow * (att/otherDef))/50)+2) * crit * (random/100) * 1 * 1;
    return damage;
  }
  else{
    return 0;
  }
}

void io_battle(character *aggressor, character *defender)
{
  npc *n = (npc *) ((aggressor == &world.pc) ? defender : aggressor);
  io_display();

        char esp;
        int selectedPokemon;
        int EselectedPokemon;
        int potions;
        int pokeBalls;
        int revives;
        bool action;
        int roundCount;
        int pcHp;
        int enemHp;
        int enemBase;
        int enemPotions;
        int enemRevives;
        int enemPotionsBase;
        int enemRevivesBase;

        esp = 0;
        
        WINDOW * enemWin = newwin(21,28,1,52);
        WINDOW * meWin = newwin(21,28,1,0);
        selectedPokemon = 0;
        EselectedPokemon = 0;
        potions = world.pc.numPotions;
        pokeBalls = world.pc.numPokeballs;
        revives = world.pc.numRevives;
        enemPotionsBase = n->basePotions;
        enemRevivesBase = n->baseRevives;
        enemPotions = n->numPotions;
        enemRevives = n->numRevives;
        pcHp = world.pc.pokemonArray[selectedPokemon]->hpDam;
        enemHp = n->pokemonArray[selectedPokemon]->hpDam;
        enemBase = n->pokemonArray[selectedPokemon]->hp;
        action = false;
        roundCount = 1;

        while(!world.pc.totalDefeat && n->defeated != 1){
          refresh();
          moves prioQue[2];
          wrefresh(meWin);
          wrefresh(enemWin);
          refresh();
          refresh();
          wclear(meWin);
          box(enemWin, 0, 0);
          box(meWin, 0, 0);
          mvwprintw(meWin, 1, 1, "Player:");
          mvwprintw(meWin, 2, 1, "# of Pokemon: %d", world.pc.num_of_pokemon);
           int i = 0;
          // for(i = 0; i < world.pc.num_of_pokemon; i++){
          //   mvwprintw(meWin, i+3, 1, "%d: %s", i+1, world.pc.pokemonArray[i]->name.c_str());
          // }
          mvwprintw(meWin, i+3, 1, "---------------------");
          mvwprintw(meWin, i+4, 1, "Current Pokemon:");
          if(world.pc.pokemonArray[selectedPokemon]->isShiny){
            wattron(meWin,COLOR_PAIR(COLOR_YELLOW));
            mvwprintw(meWin, i+5, 1, "%s *shiny* %s", world.pc.pokemonArray[selectedPokemon]->name.c_str(), (world.pc.pokemonArray[selectedPokemon]->kO) ? "(KO'd)" : "");
            wattroff(meWin,COLOR_PAIR(COLOR_YELLOW));
          }
          else{
            mvwprintw(meWin, i+5, 1, "%s %s", world.pc.pokemonArray[selectedPokemon]->name.c_str(),(world.pc.pokemonArray[selectedPokemon]->kO) ? "(KO'd)" : "" );
          }
          mvwprintw(meWin, i+6, 1, "HP: %d/%d Att:%d Def:%d", pcHp, world.pc.pokemonArray[selectedPokemon]->hp
                                                                    , world.pc.pokemonArray[selectedPokemon]->attack, world.pc.pokemonArray[selectedPokemon]->defense);
          mvwprintw(meWin, i+7, 1, "Level: %d Exp: %d/%d", world.pc.pokemonArray[selectedPokemon]->level,world.pc.pokemonArray[selectedPokemon]->experience, world.pc.pokemonArray[selectedPokemon]->level_up_exp);
          
          mvwprintw(meWin, i+8, 1, "1 - Fight");
          mvwprintw(meWin, i+9, 1, "2 - Bag");
          mvwprintw(meWin, i+10, 1,"3 - Run");
          mvwprintw(meWin, i+11, 1,"4 - Pokemon");

          if(enemPotions > 0 && (n->pokemonArray[EselectedPokemon]->hpDam < ((n->pokemonArray[EselectedPokemon]->hp)/2))){
              char potGet = 0;
              enemPotions = enemPotions - 1;
              while(potGet != 27){
              n->pokemonArray[EselectedPokemon]->hpDam = n->pokemonArray[EselectedPokemon]->hpDam + 20;
              if(n->pokemonArray[EselectedPokemon]->hpDam > n->pokemonArray[EselectedPokemon]->hp){
                n->pokemonArray[EselectedPokemon]->hpDam = n->pokemonArray[EselectedPokemon]->hp;
              }
              enemHp = n->pokemonArray[EselectedPokemon]->hpDam;
              n->numPotions = enemPotions;
           
              WINDOW * enemPotWin = newwin(7,60,10,10);
              refresh();
              box(enemPotWin, 0, 0);
              mvwprintw(enemPotWin, 1, 18, "Trainer:%c used a potion", n->symbol);
              mvwprintw(enemPotWin, 2, 24, "on %s", n->pokemonArray[EselectedPokemon]->name.c_str());
              mvwprintw(enemPotWin, 4, 18, "Press 'escape' to close");
              mvwprintw(enemPotWin, 5, 24, "notification");

              refresh();
              wrefresh(meWin);
              wrefresh(enemWin);
              wrefresh(enemPotWin);
              potGet = getch();
              }
              wrefresh(meWin);
              wrefresh(enemWin);
            }
            if(enemPotions <= 0 && enemRevives > 0 && n->pokemonArray[EselectedPokemon]->kO){
              char revGet = 0;
              enemRevives = enemRevives - 1;
              while(revGet != 27){
              n->pokemonArray[EselectedPokemon]->kO = false;
              n->pokemonArray[EselectedPokemon]->hpDam = enemBase/2;
              enemHp = enemBase/2;
              n->numRevives = enemRevives;
              // wmove(enemWin,5,1);
              //   wclrtoeol(enemWin);
              //   mvwprintw(enemWin, 5, 1, "HP:  %d/%d Att:%d Def:%d", enemHp,enemBase, n->pokemonArray[EselectedPokemon]->attack, n->pokemonArray[EselectedPokemon]->defense);
           
              WINDOW * enemRevWin = newwin(7,60,10,10);
              refresh();
              box(enemRevWin, 0, 0);
              mvwprintw(enemRevWin, 1, 18, "Trainer:%c used a revive", n->symbol);
              mvwprintw(enemRevWin, 2, 24, "on %s", n->pokemonArray[EselectedPokemon]->name.c_str());
              mvwprintw(enemRevWin, 4, 18, "Press 'escape' to close");
              mvwprintw(enemRevWin, 5, 24, "notification");

              refresh();
              wrefresh(meWin);
              wrefresh(enemWin);
              wrefresh(enemRevWin);
              revGet = getch();
              }
            }

            if(n->pokemonArray[EselectedPokemon]->kO && enemRevives <= 0 && !enemyDefeat(n)){
              

              char changeGet = 0;
              WINDOW * enemchangeWin = newwin(7,60,10,10);
              refresh();
              box(enemchangeWin, 0, 0);
              mvwprintw(enemchangeWin, 1, 18, "Enemy Pokemon Fainted.");
              EselectedPokemon++;
              enemHp = n->pokemonArray[EselectedPokemon]->hpDam;
              enemBase = n->pokemonArray[EselectedPokemon]->hpDam;
              while(changeGet != 27){
              mvwprintw(enemchangeWin, 3, 15, "Trainer summons -> %s!", n->pokemonArray[EselectedPokemon]->name.c_str());
              mvwprintw(enemchangeWin, 5, 17, "Press any key to continue.");

              refresh();
              wrefresh(enemchangeWin);
              changeGet = getch();
              break;
              }
              wrefresh(meWin);
              wrefresh(enemWin);
            }

          //Fight
          if(esp == '1'){
              char moveInput = 0;

              while(moveInput != 27){
                WINDOW * moveWin = newwin(8,28,14,0);
                refresh();
                box(moveWin, 0, 0);
                mvwprintw(moveWin, 1, 1, "MoveList!");
                mvwprintw(moveWin, 2, 1, "--------------");
                mvwprintw(moveWin, 3, 1, "1 - %s", world.pc.pokemonArray[selectedPokemon]->m[0].identifier.c_str());
                mvwprintw(moveWin, 4, 1, "2 - %s", world.pc.pokemonArray[selectedPokemon]->m[1].identifier.c_str());
                mvwprintw(moveWin, 6, 1, "Press 'escape' to close");

                refresh();
                wrefresh(moveWin);
                moveInput = getch();

                if(!world.pc.pokemonArray[selectedPokemon]->kO){
                    if(moveInput == '1'){
                      wclear(moveWin);
                      box(moveWin, 0, 0);
                      mvwprintw(moveWin, 2, 1, "%s!",world.pc.pokemonArray[selectedPokemon]->name.c_str());
                      mvwprintw(moveWin, 3, 1, "use %s!", world.pc.pokemonArray[selectedPokemon]->m[0].identifier.c_str());
                      mvwprintw(moveWin, 6, 1, "Press any key to continue.");
                      wrefresh(moveWin);
                      getch();
                      prioQue[0] = world.pc.pokemonArray[selectedPokemon]->m[0];
                      action = true;
                      break;
                    }
                    if(moveInput == '2'){
                      wclear(moveWin);
                      box(moveWin, 0, 0);
                      mvwprintw(moveWin, 2, 1, "%s!",world.pc.pokemonArray[selectedPokemon]->name.c_str());
                      mvwprintw(moveWin, 3, 1, "use %s!", world.pc.pokemonArray[selectedPokemon]->m[1].identifier.c_str());
                      mvwprintw(moveWin, 6, 1, "Press any key to continue.");
                      wrefresh(moveWin);
                      getch();
                      prioQue[0] = (world.pc.pokemonArray[selectedPokemon]->m[1]);
                      action = true;
                      break;
                    }
                }
                else{
                  wclear(moveWin);
                      box(moveWin, 0, 0);
                      mvwprintw(moveWin, 2, 1, "Pokemon Unconscious.");
                      mvwprintw(moveWin, 3, 1, "Select Another One");
                      wrefresh(moveWin);
                      mvwprintw(moveWin, 6, 1, "Press any key to continue");
                      moveInput = getch();
                      break;
                }
              }
          }

          //Bag
          else if(esp == '2'){
            char bagInput = 0;

            while(bagInput != 27){
              WINDOW * bagWin = newwin(8,28,14,0);
              refresh();
              box(bagWin, 0, 0);
              mvwprintw(bagWin, 1, 1, "Bag Inventory!");
              mvwprintw(bagWin, 2, 1, "--------------");
              mvwprintw(bagWin, 3, 1, "1 - Potion:   x%d", potions);
              mvwprintw(bagWin, 4, 1, "2 - Pokeball: x%d", pokeBalls);
              mvwprintw(bagWin, 5, 1, "3 - Revive:   x%d", revives);
              mvwprintw(bagWin, 6, 1, "Press 'escape' to close");
              refresh();
              wrefresh(bagWin);
              bagInput = getch();

              //potion
              if(bagInput == '1' && !world.pc.pokemonArray[selectedPokemon]->kO){
                  int potInput = 0;
                  if(potions > 0){
                      WINDOW * pokeWin = newwin(9,28,13,0);
                      refresh();
                      box(pokeWin, 0, 0);
                      mvwprintw(pokeWin, 1, 1, "Use Potion On Pokemon");
                      int i = 1;
                      mvwprintw(pokeWin, i+1, 1, "%d-%s HP:%d/%d",i, world.pc.pokemonArray[i-1]->name.c_str(), world.pc.pokemonArray[i-1]->hpDam,world.pc.pokemonArray[i-1]->hp);
                      while(i < world.pc.num_of_pokemon){
                        mvwprintw(pokeWin, i + 2, 1, "%d-%s HP:%d/%d",i+1, world.pc.pokemonArray[i]->name.c_str(),world.pc.pokemonArray[i]->hpDam,world.pc.pokemonArray[i]->hp);
                        i++;
                      }
                      refresh();
                      wrefresh(pokeWin);
                      potInput = getch();

                      if(potInput == '1' && (world.pc.pokemonArray[0]->hpDam != world.pc.pokemonArray[0]->hp)){
                        potions--;
                        if(world.pc.pokemonArray[0]->hpDam + 20 > world.pc.pokemonArray[0]->hp){
                          world.pc.pokemonArray[0]->hpDam = world.pc.pokemonArray[0]->hp;
                        }
                        else{
                          world.pc.pokemonArray[0]->hpDam = world.pc.pokemonArray[0]->hpDam + 20;
                        }
                      }
                      if(selectedPokemon == 0){
                        pcHp = pcHp + 20;
                        if(pcHp > world.pc.pokemonArray[0]->hp){
                          pcHp = world.pc.pokemonArray[0]->hp;
                        }
                      }

                      if(potInput == '2' && (world.pc.pokemonArray[1]->hpDam != world.pc.pokemonArray[1]->hp)){
                        potions--;
                        world.pc.pokemonArray[1]->hpDam = world.pc.pokemonArray[1]->hpDam + 20;
                        if(world.pc.pokemonArray[1]->hpDam > world.pc.pokemonArray[1]->hp){
                          world.pc.pokemonArray[1]->hpDam = world.pc.pokemonArray[1]->hp;
                        }
                      }
                       if(selectedPokemon == 1){
                        pcHp = pcHp + 20;
                        if(pcHp > world.pc.pokemonArray[1]->hp){
                          pcHp = world.pc.pokemonArray[1]->hp;
                        }
                      }

                      if(potInput == '3' && (world.pc.pokemonArray[2]->hpDam != world.pc.pokemonArray[2]->hp)){
                        potions--;
                        world.pc.pokemonArray[2]->hpDam = world.pc.pokemonArray[2]->hpDam + 20;
                        if(world.pc.pokemonArray[2]->hpDam > world.pc.pokemonArray[2]->hp){
                          world.pc.pokemonArray[2]->hpDam = world.pc.pokemonArray[2]->hp;
                        }
                      }
                       if(selectedPokemon == 2){
                        pcHp = pcHp + 20;
                        if(pcHp > world.pc.pokemonArray[2]->hp){
                          pcHp = world.pc.pokemonArray[2]->hp;
                        }
                      }

                      if(potInput == '4' && (world.pc.pokemonArray[3]->hpDam != world.pc.pokemonArray[3]->hp)){
                        potions--;
                        world.pc.pokemonArray[3]->hpDam = world.pc.pokemonArray[3]->hpDam + 20;
                        if(world.pc.pokemonArray[3]->hpDam > world.pc.pokemonArray[3]->hp){
                          world.pc.pokemonArray[3]->hpDam = world.pc.pokemonArray[3]->hp;
                        }
                      }
                       if(selectedPokemon == 3){
                        pcHp = pcHp + 20;
                        if(pcHp > world.pc.pokemonArray[3]->hp){
                          pcHp = world.pc.pokemonArray[3]->hp;
                        }
                      }

                      if(potInput == '5' && (world.pc.pokemonArray[4]->hpDam != world.pc.pokemonArray[4]->hp)){
                        potions--;
                        world.pc.pokemonArray[4]->hpDam = world.pc.pokemonArray[4]->hpDam + 20;
                        if(world.pc.pokemonArray[4]->hpDam > world.pc.pokemonArray[4]->hp){
                          world.pc.pokemonArray[4]->hpDam = world.pc.pokemonArray[4]->hp;
                        }
                      }
                       if(selectedPokemon == 4){
                        pcHp = pcHp + 20;
                        if(pcHp > world.pc.pokemonArray[4]->hp){
                          pcHp = world.pc.pokemonArray[4]->hp;
                        }
                      }

                      if(potInput == '6' && (world.pc.pokemonArray[5]->hpDam != world.pc.pokemonArray[5]->hp)){
                        potions--;
                        world.pc.pokemonArray[5]->hpDam = world.pc.pokemonArray[5]->hpDam + 20;
                        if(world.pc.pokemonArray[5]->hpDam > world.pc.pokemonArray[5]->hp){
                          world.pc.pokemonArray[5]->hpDam = world.pc.pokemonArray[5]->hp;
                        }
                        
                      }
                       if(selectedPokemon == 5){
                        potions--;
                        pcHp = pcHp + 20;
                        if(pcHp > world.pc.pokemonArray[5]->hp){
                          pcHp = world.pc.pokemonArray[5]->hp;
                        }
                      }

                      wrefresh(meWin);
                      wmove(meWin,6,1);
                      wclrtoeol(meWin);
                      mvwprintw(meWin, 6, 1, "HP: %d/%d Att:%d Def:%d", world.pc.pokemonArray[selectedPokemon]->hpDam, world.pc.pokemonArray[selectedPokemon]->hp
                                                                    , world.pc.pokemonArray[selectedPokemon]->attack, world.pc.pokemonArray[selectedPokemon]->defense);
                      bagInput = 27;
                    
                  }
                  
              }
              //pokeball
              else if(bagInput == '2'){
                bagInput = 0;
                if(pokeBalls > 0 ){
                      wclear(bagWin);
                      box(bagWin, 0, 0);
                      mvwprintw(bagWin, 2, 1, "Can't Catch Another");
                      mvwprintw(bagWin, 3, 1, "Trainer's Pokemon");
                      mvwprintw(bagWin, 6, 1, "Press any key to continue.");
                      wrefresh(bagWin);
                      getch();
                      bagInput = 27;
                }
                else{
                      wclear(bagWin);
                      box(bagWin, 0, 0);
                      mvwprintw(bagWin, 3, 1, "No Pokeballs in Bag.");
                      wrefresh(bagWin);
                      getch();
                      break;
                }
                
              }
              //revive
              else if(bagInput == '3'){
                  int reviveInput = 0;
                  if(revives > 0){
                      WINDOW * pokeWin = newwin(9,28,13,0);
                      refresh();
                      box(pokeWin, 0, 0);
                      mvwprintw(pokeWin, 1, 1, "Use Revive On Pokemon");
                      int i = 1;
                      mvwprintw(pokeWin, i+1, 1, "%d-%s KO:%s",i, world.pc.pokemonArray[i-1]->name.c_str(), world.pc.pokemonArray[i-1]->kO ? "Yes":"No");
                      while(i < world.pc.num_of_pokemon){
                        mvwprintw(pokeWin, i + 2, 1, "%d-%s KO:%s",i+1, world.pc.pokemonArray[i]->name.c_str(), world.pc.pokemonArray[i]->kO? "Yes":"No");
                        i++;
                      }
                      refresh();
                      wrefresh(pokeWin);
                      reviveInput = getch();

                      if(reviveInput == '1' && world.pc.pokemonArray[0]->kO){
                        revives--;
                        world.pc.pokemonArray[0]->kO = false;
                        world.pc.pokemonArray[0]->hpDam = (world.pc.pokemonArray[0]->hp)/2;
                        if(selectedPokemon == 0){
                          pcHp = (world.pc.pokemonArray[0]->hp)/2;
                        }
                      }
                      

                      if(reviveInput == '2' && world.pc.pokemonArray[1]->kO){
                        revives--;
                        world.pc.pokemonArray[1]->kO = false;
                        world.pc.pokemonArray[1]->hpDam = (world.pc.pokemonArray[1]->hp)/2;

                        if(selectedPokemon == 1){
                          pcHp = (world.pc.pokemonArray[1]->hp)/2;
                        }
                      }
                      

                      if(reviveInput == '3' && world.pc.pokemonArray[2]->kO){
                        revives--;
                        world.pc.pokemonArray[2]->kO = false;
                        world.pc.pokemonArray[2]->hpDam = (world.pc.pokemonArray[2]->hp)/2;
                        if(selectedPokemon == 2){
                          pcHp = (world.pc.pokemonArray[2]->hp)/2;
                        }
                      }
                      

                      if(reviveInput == '4' && world.pc.pokemonArray[3]->kO){
                        revives--;
                        world.pc.pokemonArray[3]->kO = false;
                        world.pc.pokemonArray[3]->hpDam = (world.pc.pokemonArray[3]->hp)/2;
                        if(selectedPokemon == 3){
                          pcHp = (world.pc.pokemonArray[3]->hp)/2;
                        }
                      }
                      

                      if(reviveInput == '5' && world.pc.pokemonArray[4]->kO){
                        revives--;
                        world.pc.pokemonArray[4]->kO = false;
                        world.pc.pokemonArray[4]->hpDam = (world.pc.pokemonArray[4]->hp)/2;
                        if(selectedPokemon == 4){
                          pcHp = (world.pc.pokemonArray[4]->hp)/2;
                        }
                      }
                      

                      if(reviveInput == '6' && world.pc.pokemonArray[5]->kO){
                        revives--;
                        world.pc.pokemonArray[5]->kO = false;
                        world.pc.pokemonArray[5]->hpDam = (world.pc.pokemonArray[5]->hp)/2;
                        if(selectedPokemon == 5){
                        pcHp = (world.pc.pokemonArray[5]->hp)/2;
                        }
                      }
                      
                      wrefresh(meWin);
                      bagInput = 27;
                    
                  }
              }
            }
          }
          else if(esp == '3'){
            char runInput = 0;
            refresh();

            while(runInput != 27){
                WINDOW * bagWin = newwin(8,28,14,0);
                refresh();
                box(bagWin, 0, 0);
                mvwprintw(bagWin, 1, 1, "You Can't Run");
                mvwprintw(bagWin, 2, 1, "From Your Destiny!");
                mvwprintw(bagWin, 3, 1, "--------------");
                mvwprintw(bagWin, 6, 1, "Press 'escape' to close");
                refresh();
                wrefresh(bagWin);
                runInput = getch();
              }
    
          }
          

          //Switch Pokemon
          else if(esp == '4'){
            char pokeInput = 0;
             while(pokeInput != 27){
              WINDOW * pokeWin = newwin(9,28,13,0);
              refresh();
              box(pokeWin, 0, 0);
              mvwprintw(pokeWin, 1, 1, "Select Your Pokemon");
              int i = 1;
              mvwprintw(pokeWin, i+1, 1, "%d-%s HP:%d/%d KO:%s",i, world.pc.pokemonArray[i-1]->name.c_str(), world.pc.pokemonArray[i-1]->hpDam, world.pc.pokemonArray[i-1]->hp, (world.pc.pokemonArray[i-1]->kO) ? "Y": "N");
              while(i < world.pc.num_of_pokemon){
                mvwprintw(pokeWin, i + 2, 1, "%d-%s HP:%d/%d KO:%s",i+1, world.pc.pokemonArray[i]->name.c_str(), world.pc.pokemonArray[i]->hpDam, world.pc.pokemonArray[i]->hp, (world.pc.pokemonArray[i]->kO) ? "Y": "N");
                i++;
              }
              refresh();
              wrefresh(pokeWin);
              pokeInput = getch();

              if(pokeInput == '1' && world.pc.pokemonArray[0] != NULL && !world.pc.pokemonArray[0]->kO){
                selectedPokemon = 0;
                refresh();
                wmove(meWin,5,1);
                wclrtoeol(meWin);
                  if(world.pc.pokemonArray[selectedPokemon]->isShiny){
                    wattron(meWin,COLOR_PAIR(COLOR_YELLOW));
                    mvwprintw(meWin, 5, 1, "%s *shiny*", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                    wattroff(meWin,COLOR_PAIR(COLOR_YELLOW));
                  }
                  else{
                    mvwprintw(meWin, 5, 1, "%s", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                  }
                  mvwprintw(meWin, 6, 1, "HP: %d/%d Att:%d Def:%d", world.pc.pokemonArray[selectedPokemon]->hpDam, world.pc.pokemonArray[selectedPokemon]->hp
                                                                    , world.pc.pokemonArray[selectedPokemon]->attack, world.pc.pokemonArray[selectedPokemon]->defense);
                wrefresh(meWin);
                action = true;
                pcHp = world.pc.pokemonArray[selectedPokemon]->hpDam;
                break;
              }

              else if(pokeInput == '2' && world.pc.pokemonArray[1] != NULL && !world.pc.pokemonArray[1]->kO){
                selectedPokemon = 1;
                refresh();
                wmove(meWin,5,1);
                wclrtoeol(meWin);
                        if(world.pc.pokemonArray[selectedPokemon]->isShiny){
                            wattron(meWin,COLOR_PAIR(COLOR_YELLOW));
                            mvwprintw(meWin, 5, 1, "%s *shiny*", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                            wattroff(meWin,COLOR_PAIR(COLOR_YELLOW));
                          }
                          else{
                            mvwprintw(meWin, 5, 1, "%s", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          }
                          mvwprintw(meWin, 6, 1, "HP: %d/%d Att:%d Def:%d", world.pc.pokemonArray[selectedPokemon]->hpDam, world.pc.pokemonArray[selectedPokemon]->hp
                                                                    , world.pc.pokemonArray[selectedPokemon]->attack, world.pc.pokemonArray[selectedPokemon]->defense);
                wrefresh(meWin);
                pcHp = world.pc.pokemonArray[selectedPokemon]->hpDam;
                action = true;
                break;
              }
              else if(pokeInput == '3' && world.pc.pokemonArray[2] != NULL && !world.pc.pokemonArray[2]->kO){
                selectedPokemon = 2;
                refresh();
                wmove(meWin,5,1);
                wclrtoeol(meWin);
                        if(world.pc.pokemonArray[selectedPokemon]->isShiny){
                            wattron(meWin,COLOR_PAIR(COLOR_YELLOW));
                            mvwprintw(meWin, 5, 1, "%s *shiny*", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                            wattroff(meWin,COLOR_PAIR(COLOR_YELLOW));
                          }
                          else{
                            mvwprintw(meWin, 5, 1, "%s", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          }
                          mvwprintw(meWin, 6, 1, "HP: %d/%d Att:%d Def:%d", world.pc.pokemonArray[selectedPokemon]->hpDam, world.pc.pokemonArray[selectedPokemon]->hp
                                                                    , world.pc.pokemonArray[selectedPokemon]->attack, world.pc.pokemonArray[selectedPokemon]->defense);
                wrefresh(meWin);
                action = true;
                pcHp = world.pc.pokemonArray[selectedPokemon]->hpDam;
                break;
              }
              else if(pokeInput == '4' && world.pc.pokemonArray[3] != NULL && !world.pc.pokemonArray[3]->kO){
                selectedPokemon = 3;
                refresh();
                wmove(meWin,5,1);
                wclrtoeol(meWin);
                        if(world.pc.pokemonArray[selectedPokemon]->isShiny){
                            wattron(meWin,COLOR_PAIR(COLOR_YELLOW));
                            mvwprintw(meWin, 5, 1, "%s *shiny*", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                            wattroff(meWin,COLOR_PAIR(COLOR_YELLOW));
                          }
                          else{
                            mvwprintw(meWin, 5, 1, "%s", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          }
                          mvwprintw(meWin, 6, 1, "HP: %d/%d Att:%d Def:%d", world.pc.pokemonArray[selectedPokemon]->hpDam, world.pc.pokemonArray[selectedPokemon]->hp
                                                                    , world.pc.pokemonArray[selectedPokemon]->attack, world.pc.pokemonArray[selectedPokemon]->defense);
                wrefresh(meWin);
                pcHp = world.pc.pokemonArray[selectedPokemon]->hpDam;
                action = true;
                break;
              }

              else if(pokeInput == '5' && world.pc.pokemonArray[4] != NULL && !world.pc.pokemonArray[4]->kO){
                selectedPokemon = 4;
                refresh();
                wmove(meWin,5,1);
                wclrtoeol(meWin);
                        if(world.pc.pokemonArray[selectedPokemon]->isShiny){
                            wattron(meWin,COLOR_PAIR(COLOR_YELLOW));
                            mvwprintw(meWin, 5, 1, "%s *shiny*", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                            wattroff(meWin,COLOR_PAIR(COLOR_YELLOW));
                          }
                          else{
                            mvwprintw(meWin, 5, 1, "%s", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          }
                          mvwprintw(meWin, 6, 1, "HP: %d/%d Att:%d Def:%d", world.pc.pokemonArray[selectedPokemon]->hpDam, world.pc.pokemonArray[selectedPokemon]->hp
                                                                    , world.pc.pokemonArray[selectedPokemon]->attack, world.pc.pokemonArray[selectedPokemon]->defense);
                wrefresh(meWin);
                pcHp = world.pc.pokemonArray[selectedPokemon]->hpDam;
                action = true;
                break;
              }
              else if(pokeInput == '6' && world.pc.pokemonArray[5] != NULL && !world.pc.pokemonArray[5]->kO){
                selectedPokemon = 5;
                refresh();
                wmove(meWin,5,1);
                wclrtoeol(meWin);
                        if(world.pc.pokemonArray[selectedPokemon]->isShiny){
                            wattron(meWin,COLOR_PAIR(COLOR_YELLOW));
                            mvwprintw(meWin, 5, 1, "%s *shiny*", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                            wattroff(meWin,COLOR_PAIR(COLOR_YELLOW));
                          }
                          else{
                            mvwprintw(meWin, 5, 1, "%s", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          }
                          mvwprintw(meWin, 6, 1, "HP: %d/%d Att:%d Def:%d", world.pc.pokemonArray[selectedPokemon]->hpDam, world.pc.pokemonArray[selectedPokemon]->hp
                                                                    , world.pc.pokemonArray[selectedPokemon]->attack, world.pc.pokemonArray[selectedPokemon]->defense);
                wrefresh(meWin);
                wrefresh(pokeWin);
                pcHp = world.pc.pokemonArray[selectedPokemon]->hpDam;
                action = true;
                break;
              }
             } 
          }

          wclear(enemWin);
          box(enemWin, 0, 0);
          mvwprintw(enemWin, 1, 1, "Trainer: %c", n->symbol);
          mvwprintw(enemWin, 2, 1, "# of Enemy Pokemon: %d", n->num_of_pokemon);
          mvwprintw(enemWin, 3, 1, "---------------------");
          mvwprintw(enemWin, 4, 1, "Current Pokemon:");
          if(n->pokemonArray[EselectedPokemon]->isShiny){
            wattron(enemWin,COLOR_PAIR(COLOR_YELLOW));
            mvwprintw(enemWin, 5, 1, "Name: %s *shiny* lvl:%d", n->pokemonArray[EselectedPokemon]->name.c_str(), n->pokemonArray[EselectedPokemon]->level);
            wattroff(enemWin,COLOR_PAIR(COLOR_YELLOW));
          }
          else{
            mvwprintw(enemWin, 5, 1, "Name: %s lvl:%d", n->pokemonArray[EselectedPokemon]->name.c_str(), n->pokemonArray[EselectedPokemon]->level);
          }
        
          mvwprintw(enemWin, 6, 1, "HP:  %d/%d Att:%d Def:%d", enemHp,enemBase, n->pokemonArray[EselectedPokemon]->attack, n->pokemonArray[EselectedPokemon]->defense);

          mvwprintw(enemWin, 8, 1, "Move 1: %s ", n->pokemonArray[EselectedPokemon]->m[0].identifier.c_str());
          if(n->pokemonArray[EselectedPokemon]->m[1].identifier != ""){
             mvwprintw(enemWin, 9, 1, "Move 2: %s ", n->pokemonArray[EselectedPokemon]->m[1].identifier.c_str());
                int mSel = rand()%2;
                prioQue[1] = ((mSel == 0) ? n->pokemonArray[EselectedPokemon]->m[0]: n->pokemonArray[EselectedPokemon]->m[1]);
           }
          else{
             mvwprintw(enemWin, 9, 1, "Move 2: None");
             prioQue[1] = (n->pokemonArray[EselectedPokemon]->m[0]);
           }

           mvwprintw(enemWin, 11, 1, "Potion:%d/%d Revives:%d/%d", enemPotions, enemPotionsBase, enemRevives, enemRevivesBase);

           mvwprintw(enemWin, 13, 1, "Enemy Pokemon:");
              i = 1;
              mvwprintw(enemWin, i+13, 1, "%s%d-%s HP:%d/%d KO:%s",(EselectedPokemon == i-1)? "*":"", i, n->pokemonArray[i-1]->name.c_str(), n->pokemonArray[i-1]->hpDam, n->pokemonArray[i-1]->hp, (n->pokemonArray[i-1]->kO) ? "Y": "N");
              while(i < n->num_of_pokemon){
                mvwprintw(enemWin, i + 14, 1, "%s%d-%s HP:%d/%d KO:%s",(EselectedPokemon == i)? "*":"",i+1, n->pokemonArray[i]->name.c_str(), n->pokemonArray[i]->hpDam, n->pokemonArray[i]->hp, (n->pokemonArray[i]->kO) ? "Y": "N");
                i++;
              }

          wrefresh(meWin);
          wrefresh(enemWin);


          ///Start of Fight
          vector<int> damageVec;
          int pcDamage;
          
          if(action){
            WINDOW * fightWin = newwin(7,60,10,10);
            refresh();
            wclear(fightWin);
            box(fightWin, 0, 0);
            if(prioQue[0].identifier == ""){
              pcDamage = 0;
            }
            else{
              //int damageCalc(int level, int att, moves m, int otherDef, makePokemon *p){
              pcDamage = damageCalc(world.pc.pokemonArray[selectedPokemon]->level,world.pc.pokemonArray[selectedPokemon]->attack, prioQue[0], n->pokemonArray[EselectedPokemon]->defense, n->pokemonArray[EselectedPokemon]);
            }
            
            int enemDamage = damageCalc(n->pokemonArray[EselectedPokemon]->level, n->pokemonArray[EselectedPokemon]->attack, prioQue[1], world.pc.pokemonArray[selectedPokemon]->defense, world.pc.pokemonArray[selectedPokemon]);

            mvwprintw(fightWin, 1, 1, "Round: %d", roundCount);
            if(prioQue[0].priority > prioQue[1].priority){
              mvwprintw(fightWin, 2, 1, "%s: %d damage to %s", world.pc.pokemonArray[selectedPokemon]->name.c_str(), pcDamage, n->pokemonArray[EselectedPokemon]->name.c_str());
              enemHp = enemHp - pcDamage;
              if(enemHp <= 0){
                enemHp = 0;
                n->pokemonArray[EselectedPokemon]->kO = true;
                mvwprintw(fightWin, 4, 1, "%s is knockedout!", n->pokemonArray[EselectedPokemon]->name.c_str());
                mvwprintw(fightWin, 5, 1, "Enemy Trainer makes a move!");
              }
              else{
                mvwprintw(fightWin, 3, 1, "%s: %d damage to %s", n->pokemonArray[EselectedPokemon]->name.c_str(), enemDamage, world.pc.pokemonArray[selectedPokemon]->name.c_str());
                pcHp = pcHp - enemDamage;
                if(pcHp <= 0){
                  pcHp = 0;
                  world.pc.pokemonArray[selectedPokemon]->kO = true;
                  mvwprintw(fightWin, 4, 1, "%s is knockedout!", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                  mvwprintw(fightWin, 5, 1, "Hurry! Swap Out Pokemon!");
                }
                else{
                  mvwprintw(fightWin, 4, 1, "No Pokemon Were Knocked Out");
                  mvwprintw(fightWin, 5, 1, "Continue the fight!             Press any key to continue.");
                }
              }
            }

            else if(prioQue[0].priority < prioQue[1].priority){
              mvwprintw(fightWin, 2, 1, "%s: %d damage to %s",n->pokemonArray[EselectedPokemon]->name.c_str() , enemDamage, world.pc.pokemonArray[selectedPokemon]->name.c_str());
              pcHp = pcHp - enemDamage;
    
              if(pcHp <= 0){
                  pcHp = 0;
                  world.pc.pokemonArray[selectedPokemon]->kO = true;
                  mvwprintw(fightWin, 4, 1, "%s is knockedout!", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                  mvwprintw(fightWin, 5, 1, "Hurry! Swap Out Pokemon!");
                
              }
              else{
                mvwprintw(fightWin, 3, 1, "%s: %d damage to %s", world.pc.pokemonArray[selectedPokemon]->name.c_str(), pcDamage, n->pokemonArray[EselectedPokemon]->name.c_str());
                enemHp = enemHp - pcDamage;
                if(enemHp <= 0){
                  enemHp = 0;
                  n->pokemonArray[EselectedPokemon]->kO = true;
                  mvwprintw(fightWin, 4, 1, "%s is knockedout!", n->pokemonArray[EselectedPokemon]->name.c_str());
                  mvwprintw(fightWin, 5, 1, "Enemy Trainer makes a move!");
                }
                else{
                  mvwprintw(fightWin, 4, 1, "No Pokemon Were Knocked Out");
                  mvwprintw(fightWin, 5, 1, "Continue the fight!             Press any key to continue.");
                }
              }
            }
              
            else{
                  if(world.pc.pokemonArray[selectedPokemon]->speed > n->pokemonArray[EselectedPokemon]->speed){
                      mvwprintw(fightWin, 2, 1, "%s: %d damage to %s", world.pc.pokemonArray[selectedPokemon]->name.c_str(), pcDamage, n->pokemonArray[EselectedPokemon]->name.c_str());
                      enemHp = enemHp - pcDamage;
                      if(enemHp <= 0){
                          enemHp= 0;
                          n->pokemonArray[EselectedPokemon]->kO = true;
                          mvwprintw(fightWin, 4, 1, "%s is knockedout!", n->pokemonArray[EselectedPokemon]->name.c_str());
                          mvwprintw(fightWin, 5, 1, "Enemy Trainer makes a move!");
                      }
                      else{
                          mvwprintw(fightWin, 3, 1, "%s: %d damage to %s", n->pokemonArray[EselectedPokemon]->name.c_str(), enemDamage, world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          pcHp = pcHp - enemDamage;

                          if(pcHp <= 0){
                            pcHp = 0;
                            world.pc.pokemonArray[selectedPokemon]->kO = true;
                            mvwprintw(fightWin, 4, 1, "%s is knockedout!", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                            mvwprintw(fightWin, 5, 1, "Hurry! Swap Out Pokemon!");
                          }
                          else{
                            mvwprintw(fightWin, 4, 1, "No Pokemon Were Knocked Out");
                            mvwprintw(fightWin, 5, 1, "Continue the fight             Press any key to continue.");
                          }
                      }
                  }

                  else if(world.pc.pokemonArray[selectedPokemon]->speed < n->pokemonArray[EselectedPokemon]->speed){
                      mvwprintw(fightWin, 2, 1, "%s: %d damage to %s",n->pokemonArray[EselectedPokemon]->name.c_str() , enemDamage, world.pc.pokemonArray[selectedPokemon]->name.c_str());
                      pcHp = pcHp - enemDamage;
        
                      if(pcHp <= 0){
                          pcHp = 0;
                          world.pc.pokemonArray[selectedPokemon]->kO = true;
                          mvwprintw(fightWin, 4, 1, "%s is knockedout!", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          mvwprintw(fightWin, 5, 1, "Hurry! Swap Out Pokemon!");
                    
                      }
                      else{
                        mvwprintw(fightWin, 3, 1, "%s: %d damage to %s", world.pc.pokemonArray[selectedPokemon]->name.c_str(), pcDamage, n->pokemonArray[EselectedPokemon]->name.c_str());
                        enemHp = enemHp - pcDamage;
                        if(enemHp <= 0){
                          enemHp = 0;
                          n->pokemonArray[EselectedPokemon]->kO = true;
                          mvwprintw(fightWin, 4, 1, "%s is knockedout!", n->pokemonArray[EselectedPokemon]->name.c_str());
                          mvwprintw(fightWin, 5, 1, "Enemy Trainer makes a move!");
                        }
                        else{
                          mvwprintw(fightWin, 4, 1, "No Pokemon Were Knocked Out");
                          mvwprintw(fightWin, 5, 1, "Continue the fight!             Press any key to continue.");
                        }
                      }
                  }


                  else{
                      int randPick = rand() % 2;

                      if(randPick == 0){
                          mvwprintw(fightWin, 2, 1, "%s: %d damage to %s", world.pc.pokemonArray[selectedPokemon]->name.c_str(), pcDamage, n->pokemonArray[EselectedPokemon]->name.c_str());
                          enemHp = enemHp - pcDamage;
                                if(enemHp <= 0){
                                    enemHp = 0;
                                    n->pokemonArray[EselectedPokemon]->kO = true;
                                    mvwprintw(fightWin, 4, 1, "%s is knockedout!", n->pokemonArray[EselectedPokemon]->name.c_str());
                                    mvwprintw(fightWin, 5, 1, "Enemy Trainer makes a move!");
                                }
            
                                else{
                                    mvwprintw(fightWin, 3, 1, "%s: %d damage to %s", n->pokemonArray[EselectedPokemon]->name.c_str(), enemDamage, world.pc.pokemonArray[selectedPokemon]->name.c_str());
                                    pcHp = pcHp - enemDamage;

                                    if(pcHp <= 0){
                                        pcHp = 0;
                                        world.pc.pokemonArray[selectedPokemon]->kO = true;
                                        mvwprintw(fightWin, 4, 1, "%s is knockedout!", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                                        mvwprintw(fightWin, 5, 1, "Hurry! Swap Out Pokemon!");
                                    }
                                    else{
                                      mvwprintw(fightWin, 4, 1, "No Pokemon Were Knocked Out");
                                      mvwprintw(fightWin, 5, 1, "Continue the fight!             Press any key to continue.");
                                    }
                                }
                      }
                      else{
                          mvwprintw(fightWin, 2, 1, "%s: %d damage to %s",n->pokemonArray[EselectedPokemon]->name.c_str() , enemDamage, world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          pcHp = pcHp - enemDamage;
        
                          if(pcHp <= 0){
                              pcHp = 0;
                              world.pc.pokemonArray[selectedPokemon]->kO = true;
                              mvwprintw(fightWin, 4, 1, "%s is knockedout!", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                              mvwprintw(fightWin, 5, 1, "Hurry! Swap Out Pokemon!");
                          }

                          else{
                            mvwprintw(fightWin, 3, 1, "%s: %d damage to %s", world.pc.pokemonArray[selectedPokemon]->name.c_str(), pcDamage, n->pokemonArray[EselectedPokemon]->name.c_str());
                            enemHp = enemHp - pcDamage;
                            if(enemHp <= 0){
                              enemHp = 0;
                              n->pokemonArray[EselectedPokemon]->kO = true;
                              mvwprintw(fightWin, 4, 1, "%s is knockedout!", n->pokemonArray[EselectedPokemon]->name.c_str());
                              mvwprintw(fightWin, 5, 1, "Enemy Trainer makes a move!");
                            }
                            else{
                              mvwprintw(fightWin, 4, 1, "No Pokemon Were Knocked Out");
                              mvwprintw(fightWin, 5, 1, "Continue the fight!             Press any key to continue.");
                            }
                          }
                      }

                  }
               
            }

            roundCount++;
            world.pc.pokemonArray[selectedPokemon]->hpDam = pcHp;
            n->pokemonArray[EselectedPokemon]->hpDam = enemHp;
            n->pokemonArray[EselectedPokemon]->hp = enemBase;

            wrefresh(fightWin);
            //getch();
            wclear(fightWin);
            wrefresh(meWin);
            wrefresh(enemWin);
            action = false;

            
            

            if(checkDefeat()){
              world.pc.totalDefeat = true;
              char fightGet = 0;
              while(fightGet != 27){
              WINDOW * fightWin = newwin(7,60,10,10);
              refresh();
              wclear(fightWin);
              box(fightWin, 0, 0);
              mvwprintw(fightWin, 1, 16, "All PC's Pokemon Are Unconcoius.");
              mvwprintw(fightWin, 2, 11, "Find the closest Pokemon Center for help.");
              mvwprintw(fightWin, 5, 18, "Press 'escape' to continue.");
              wrefresh(fightWin);
              fightGet = getch();
              }
            }

            if(enemyDefeat(n) && enemRevives <= 0){
              char winGet = 0;
              while(winGet != 27){
              WINDOW * fightWin = newwin(7,60,10,10);
              refresh();
              wclear(fightWin);
              box(fightWin, 0, 0);
              mvwprintw(fightWin, 1, 14, "All Enemy Pokemon Are Unconcious.");
              mvwprintw(fightWin, 2, 26, "You WIN!");
              mvwprintw(fightWin, 5, 16, "Press 'escape' to exit battle.");
              wrefresh(fightWin);
              winGet = getch();
              }

              n->defeated = 1;

              if (n->ctype == char_hiker || n->ctype == char_rival) {
                n->mtype = move_wander;
              }

              WINDOW * fightWin = newwin(7,60,10,10);
              refresh();
              wclear(fightWin);
              box(fightWin, 0, 0);
              mvwprintw(fightWin, 1, 20, "Experience Gained!");
              int fightExp = ((25*n->num_of_pokemon) * (n->pokemonArray[0]->level))*50;
              int secExp = fightExp * .20;
              world.pc.pokemonArray[selectedPokemon]->experience = world.pc.pokemonArray[selectedPokemon]->experience + fightExp;
             
                      i = 0;
                      while(i < world.pc.num_of_pokemon){
                        if(selectedPokemon != i){
                          world.pc.pokemonArray[i]->experience = world.pc.pokemonArray[i]->experience + secExp;
                  
                          mvwprintw(fightWin, i + 3, 1, "%d-%s Exp Gained: +%d %s",i+1, world.pc.pokemonArray[i]->name.c_str(), secExp, (world.pc.pokemonArray[i]->experience > 100) ? "(LEVEL UP!)": "");
                          
                          if(world.pc.pokemonArray[i]->experience >= 100){
                            int levels = world.pc.pokemonArray[i]->experience / 100;
                            int newExp = world.pc.pokemonArray[i]->experience % 100;
                            world.pc.pokemonArray[i]->level = world.pc.pokemonArray[i]->level + levels;
                            world.pc.pokemonArray[i]->experience = newExp;
                            levelUpStats(world.pc.pokemonArray[i]);
                          }
                        }
                        else{
                          mvwprintw(fightWin, i+3, 1, "%d-%s Exp Gained: +%d %s",i+1, world.pc.pokemonArray[selectedPokemon]->name.c_str(), fightExp, (world.pc.pokemonArray[selectedPokemon]->experience > 100) ? "(LEVEL UP!)": "");
                          if(world.pc.pokemonArray[selectedPokemon]->experience >= 100){
                          int levels = world.pc.pokemonArray[selectedPokemon]->experience / 100;
                          int newExp = world.pc.pokemonArray[selectedPokemon]->experience % 100;
                          world.pc.pokemonArray[selectedPokemon]->level = world.pc.pokemonArray[selectedPokemon]->level + levels;
                          world.pc.pokemonArray[selectedPokemon]->experience = newExp;
                          levelUpStats(world.pc.pokemonArray[selectedPokemon]);
                      }
                        }
                        
                        i++;
                      }
              wrefresh(fightWin);
              getch();
            }
          }
        
          wrefresh(meWin);
          wrefresh(enemWin);
          refresh();
          esp = getch();
        }
        
        world.pc.isRunning = false;
        world.pc.numPotions = potions;
        world.pc.numPokeballs = pokeBalls;
        world.pc.numRevives = revives;

    
      
    

}

uint32_t move_pc_dir(uint32_t input, pair_t dest)
{
  dest[dim_y] = world.pc.pos[dim_y];
  dest[dim_x] = world.pc.pos[dim_x];
  int encounter;
  char esp;

  switch (input) {
  case 1:
  case 2:
  case 3:
    dest[dim_y]++;
    break;
  case 4:
  case 5:
  case 6:
    break;
  case 7:
  case 8:
  case 9:
    dest[dim_y]--;
    break;
  }
  switch (input) {
  case 1:
  case 4:
  case 7:
    dest[dim_x]--;
    break;
  case 2:
  case 5:
  case 8:
    break;
  case 3:
  case 6:
  case 9:
    dest[dim_x]++;
    break;
  case '>':
    if (world.cur_map->map[world.pc.pos[dim_y]][world.pc.pos[dim_x]] ==
        ter_mart) {
      io_pokemart();
    }
    if (world.cur_map->map[world.pc.pos[dim_y]][world.pc.pos[dim_x]] ==
        ter_center) {
      io_pokemon_center();
    }
    break;
  }

  if (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]) {
    if (dynamic_cast<npc *> (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]) &&
        ((npc *) world.cur_map->cmap[dest[dim_y]][dest[dim_x]])->defeated) {
      // Some kind of greeting here would be nice
      return 1;
    } else if ((dynamic_cast<npc *>
                (world.cur_map->cmap[dest[dim_y]][dest[dim_x]]))) {
      io_battle(&world.pc, world.cur_map->cmap[dest[dim_y]][dest[dim_x]]);
      // Not actually moving, so set dest back to PC position
      dest[dim_x] = world.pc.pos[dim_x];
      dest[dim_y] = world.pc.pos[dim_y];
    }
  }

  //Wild Pokemon Encounter
  if(move_cost[char_pc][world.cur_map->map[dest[dim_y]][dest[dim_x]]] == 20){
      encounter = rand()% 100;
      int selectedPokemon;
      int potions;
      int pokeBalls;
      int revives;
      bool action;
      int roundCount;
      int pcHp;
      int enemHp;
      int enemBase;

      if(encounter < 15){
        esp = 0;
        
        makePokemon *p = createPokemon();
        WINDOW * win2 = newwin(21,28,1,52);
        WINDOW * meWin = newwin(21,28,1,0);
        selectedPokemon = 0;
        potions = world.pc.numPotions;
        pokeBalls = world.pc.numPokeballs;
        revives = world.pc.numRevives;
        pcHp = world.pc.pokemonArray[selectedPokemon]->hpDam;
        enemHp = p->hpDam;
        enemBase = p->hp;
        action = false;
        roundCount = 1;

        while(!world.pc.isRunning && !p->isRunning && !p->isCaptured && !world.pc.totalDefeat && !p->kO){
          moves prioQue[2];
          wrefresh(meWin);
          wrefresh(win2);
          refresh();
          refresh();
          wclear(meWin);
          box(win2, 0, 0);
          box(meWin, 0, 0);
          mvwprintw(meWin, 1, 1, "Player:");
          mvwprintw(meWin, 2, 1, "# of Pokemon: %d", world.pc.num_of_pokemon);
           int i = 0;
          // for(i = 0; i < world.pc.num_of_pokemon; i++){
          //   mvwprintw(meWin, i+3, 1, "%d: %s", i+1, world.pc.pokemonArray[i]->name.c_str());
          // }
          mvwprintw(meWin, i+3, 1, "---------------------");
          mvwprintw(meWin, i+4, 1, "Current Pokemon:");
          if(world.pc.pokemonArray[selectedPokemon]->isShiny){
            wattron(meWin,COLOR_PAIR(COLOR_YELLOW));
            mvwprintw(meWin, i+5, 1, "%s *shiny* %s", world.pc.pokemonArray[selectedPokemon]->name.c_str(), (world.pc.pokemonArray[selectedPokemon]->kO) ? "(KO'd)" : "");
            wattroff(meWin,COLOR_PAIR(COLOR_YELLOW));
          }
          else{
            mvwprintw(meWin, i+5, 1, "%s %s", world.pc.pokemonArray[selectedPokemon]->name.c_str(),(world.pc.pokemonArray[selectedPokemon]->kO) ? "(KO'd)" : "" );
          }
          mvwprintw(meWin, i+6, 1, "HP: %d/%d Att:%d Def:%d", pcHp, world.pc.pokemonArray[selectedPokemon]->hp
                                                                    , world.pc.pokemonArray[selectedPokemon]->attack, world.pc.pokemonArray[selectedPokemon]->defense);
          mvwprintw(meWin, i+8, 1, "1 - Fight");
          mvwprintw(meWin, i+9, 1, "2 - Bag");
          mvwprintw(meWin, i+10, 1,"3 - Run");
          mvwprintw(meWin, i+11, 1,"4 - Pokemon");

          //Fight
          if(esp == '1'){
              char moveInput = 0;

              while(moveInput != 27){
                WINDOW * moveWin = newwin(8,28,14,0);
                refresh();
                box(moveWin, 0, 0);
                mvwprintw(moveWin, 1, 1, "MoveList!");
                mvwprintw(moveWin, 2, 1, "--------------");
                mvwprintw(moveWin, 3, 1, "1 - %s", world.pc.pokemonArray[selectedPokemon]->m[0].identifier.c_str());
                mvwprintw(moveWin, 4, 1, "2 - %s", world.pc.pokemonArray[selectedPokemon]->m[1].identifier.c_str());
                mvwprintw(moveWin, 6, 1, "Press 'escape' to close");

                refresh();
                wrefresh(moveWin);
                moveInput = getch();

                if(!world.pc.pokemonArray[selectedPokemon]->kO){
                    if(moveInput == '1'){
                      wclear(moveWin);
                      box(moveWin, 0, 0);
                      mvwprintw(moveWin, 2, 1, "%s!",world.pc.pokemonArray[selectedPokemon]->name.c_str());
                      mvwprintw(moveWin, 3, 1, "use %s!", world.pc.pokemonArray[selectedPokemon]->m[0].identifier.c_str());
                      mvwprintw(moveWin, 6, 1, "Press any key to continue.");
                      wrefresh(moveWin);
                      getch();
                      prioQue[0] = world.pc.pokemonArray[selectedPokemon]->m[0];
                      action = true;
                      break;
                    }
                    if(moveInput == '2'){
                      wclear(moveWin);
                      box(moveWin, 0, 0);
                      mvwprintw(moveWin, 2, 1, "%s!",world.pc.pokemonArray[selectedPokemon]->name.c_str());
                      mvwprintw(moveWin, 3, 1, "use %s!", world.pc.pokemonArray[selectedPokemon]->m[1].identifier.c_str());
                      mvwprintw(moveWin, 6, 1, "Press any key to continue.");
                      wrefresh(moveWin);
                      getch();
                      prioQue[0] = (world.pc.pokemonArray[selectedPokemon]->m[1]);
                      action = true;
                      break;
                    }
                }
                else{
                  wclear(moveWin);
                      box(moveWin, 0, 0);
                      mvwprintw(moveWin, 3, 1, "Pokemon Unconscious.");
                      mvwprintw(moveWin, 4, 1, "Select Another One");
                      wrefresh(moveWin);
                      mvwprintw(moveWin, 4, 1, "Press 'escape'.");
                      moveInput = getch();
                      break;
                }
              }
          }

          //Bag
          else if(esp == '2'){
            char bagInput = 0;

            while(bagInput != 27){
              WINDOW * bagWin = newwin(8,28,14,0);
              refresh();
              box(bagWin, 0, 0);
              mvwprintw(bagWin, 1, 1, "Bag Inventory!");
              mvwprintw(bagWin, 2, 1, "--------------");
              mvwprintw(bagWin, 3, 1, "1 - Potion:   x%d", potions);
              mvwprintw(bagWin, 4, 1, "2 - Pokeball: x%d", pokeBalls);
              mvwprintw(bagWin, 5, 1, "3 - Revive:   x%d", revives);
              mvwprintw(bagWin, 6, 1, "Press 'escape' to close");
              refresh();
              wrefresh(bagWin);
              bagInput = getch();

              //potion
              if(bagInput == '1' && !world.pc.pokemonArray[selectedPokemon]->kO){
                int potInput = 0;
                  if(potions > 0){
                      WINDOW * pokeWin = newwin(9,28,13,0);
                      refresh();
                      box(pokeWin, 0, 0);
                      mvwprintw(pokeWin, 1, 1, "Use Potion On Pokemon");
                      int i = 1;
                      mvwprintw(pokeWin, i+1, 1, "%d-%s HP:%d/%d",i, world.pc.pokemonArray[i-1]->name.c_str(), world.pc.pokemonArray[i-1]->hpDam,world.pc.pokemonArray[i-1]->hp);
                      while(i < world.pc.num_of_pokemon){
                        mvwprintw(pokeWin, i + 2, 1, "%d-%s HP:%d/%d",i+1, world.pc.pokemonArray[i]->name.c_str(),world.pc.pokemonArray[i]->hpDam,world.pc.pokemonArray[i]->hp);
                        i++;
                      }
                      refresh();
                      wrefresh(pokeWin);
                      potInput = getch();

                      if(potInput == '1' && (world.pc.pokemonArray[0]->hpDam != world.pc.pokemonArray[0]->hp)){
                        potions--;
                        if(world.pc.pokemonArray[0]->hpDam + 20 > world.pc.pokemonArray[0]->hp){
                          world.pc.pokemonArray[0]->hpDam = world.pc.pokemonArray[0]->hp;
                        }
                        else{
                          world.pc.pokemonArray[0]->hpDam = world.pc.pokemonArray[0]->hpDam + 20;
                        }
                      }
                      if(selectedPokemon == 0){
                        pcHp = pcHp + 20;
                        if(pcHp > world.pc.pokemonArray[0]->hp){
                          pcHp = world.pc.pokemonArray[0]->hp;
                        }
                      }

                      if(potInput == '2' && (world.pc.pokemonArray[1]->hpDam != world.pc.pokemonArray[1]->hp)){
                        if(world.pc.pokemonArray[1]->hpDam + 20 > world.pc.pokemonArray[1]->hp){
                          world.pc.pokemonArray[1]->hpDam = world.pc.pokemonArray[1]->hp;
                        }
                        else{
                          world.pc.pokemonArray[1]->hpDam = world.pc.pokemonArray[1]->hpDam + 20;
                        }
                      }
                       if(selectedPokemon == 1){
                        pcHp = pcHp + 20;
                        if(pcHp > world.pc.pokemonArray[1]->hp){
                          pcHp = world.pc.pokemonArray[1]->hp;
                        }
                      }

                      if(potInput == '3' && (world.pc.pokemonArray[2]->hpDam != world.pc.pokemonArray[2]->hp)){
                        potions--;
                        if(world.pc.pokemonArray[2]->hpDam + 20 > world.pc.pokemonArray[2]->hp){
                          world.pc.pokemonArray[2]->hpDam = world.pc.pokemonArray[2]->hp;
                        }
                        else{
                          world.pc.pokemonArray[2]->hpDam = world.pc.pokemonArray[2]->hpDam + 20;
                        }
                      }
                       if(selectedPokemon == 2){
                        pcHp = pcHp + 20;
                        if(pcHp > world.pc.pokemonArray[2]->hp){
                          pcHp = world.pc.pokemonArray[2]->hp;
                        }
                      }

                      if(potInput == '4' && (world.pc.pokemonArray[3]->hpDam != world.pc.pokemonArray[3]->hp)){
                        potions--;
                        if(world.pc.pokemonArray[3]->hpDam + 20 > world.pc.pokemonArray[3]->hp){
                          world.pc.pokemonArray[3]->hpDam = world.pc.pokemonArray[3]->hp;
                        }
                        else{
                          world.pc.pokemonArray[3]->hpDam = world.pc.pokemonArray[3]->hpDam + 20;
                        }
                      }
                       if(selectedPokemon == 3){
                        pcHp = pcHp + 20;
                        if(pcHp > world.pc.pokemonArray[3]->hp){
                          pcHp = world.pc.pokemonArray[3]->hp;
                        }
                      }

                      if(potInput == '5' && (world.pc.pokemonArray[4]->hpDam != world.pc.pokemonArray[4]->hp)){
                        potions--;
                        if(world.pc.pokemonArray[4]->hpDam + 20 > world.pc.pokemonArray[4]->hp){
                          world.pc.pokemonArray[4]->hpDam = world.pc.pokemonArray[4]->hp;
                        }
                        else{
                          world.pc.pokemonArray[4]->hpDam = world.pc.pokemonArray[4]->hpDam + 20;
                        }
                      }
                       if(selectedPokemon == 4){
                        pcHp = pcHp + 20;
                        if(pcHp > world.pc.pokemonArray[4]->hp){
                          pcHp = world.pc.pokemonArray[4]->hp;
                        }
                      }

                      if(potInput == '6' && (world.pc.pokemonArray[5]->hpDam != world.pc.pokemonArray[5]->hp)){
                        potions--;
                        if(world.pc.pokemonArray[5]->hpDam + 20 > world.pc.pokemonArray[5]->hp){
                          world.pc.pokemonArray[5]->hpDam = world.pc.pokemonArray[5]->hp;
                        }
                        else{
                          world.pc.pokemonArray[5]->hpDam = world.pc.pokemonArray[5]->hpDam + 20;
                        }
                      }
                       if(selectedPokemon == 5){
                        pcHp = pcHp + 20;
                        if(pcHp > world.pc.pokemonArray[5]->hp){
                          pcHp = world.pc.pokemonArray[5]->hp;
                        }
                      }

                      wrefresh(meWin);
                      wmove(meWin,6,1);
                      wclrtoeol(meWin);
                      mvwprintw(meWin, 6, 1, "HP: %d/%d Att:%d Def:%d", world.pc.pokemonArray[selectedPokemon]->hpDam, world.pc.pokemonArray[selectedPokemon]->hp
                                                                    , world.pc.pokemonArray[selectedPokemon]->attack, world.pc.pokemonArray[selectedPokemon]->defense);
                      bagInput = 27;
                    
                  }
                  
              }
              //pokeball
              else if(bagInput == '2'){
                bagInput = 0;
                if(pokeBalls > 0 ){
                    pokeBalls--;
                    if(world.pc.num_of_pokemon <= 5){
                        world.pc.num_of_pokemon++;
                        world.pc.pokemonArray[world.pc.num_of_pokemon-1] = p;
                        p->isCaptured = true;
                        wclear(bagWin);
                        box(bagWin, 0, 0);
                        mvwprintw(bagWin, 3, 1, "%s", p->name.c_str());
                        mvwprintw(bagWin, 4, 1, "has been caught!");
                        wrefresh(bagWin);
                        getch();
                        bagInput = 27;
                    }
                    else{
                      p->isRunning = true;
                      wclear(bagWin);
                      box(bagWin, 0, 0);
                      mvwprintw(bagWin, 3, 1, "%s", p->name.c_str());
                      mvwprintw(bagWin, 4, 1, "has gotten away.");
                      wrefresh(bagWin);
                      getch();
                      break;
                    }
                }
                else{
                      wclear(bagWin);
                      box(bagWin, 0, 0);
                      mvwprintw(bagWin, 3, 1, "No Pokeballs in Bag.");
                      wrefresh(bagWin);
                      getch();
                      break;
                }
                
              }
              //revive
              else if(bagInput == '3'){
                  int reviveInput = 0;
                  if(revives > 0){
                      WINDOW * pokeWin = newwin(9,28,13,0);
                      refresh();
                      box(pokeWin, 0, 0);
                      mvwprintw(pokeWin, 1, 1, "Use Revive On Pokemon");
                      int i = 1;
                      mvwprintw(pokeWin, i+1, 1, "%d-%s KO:%s",i, world.pc.pokemonArray[i-1]->name.c_str(), world.pc.pokemonArray[i-1]->kO ? "Yes":"No");
                      while(i < world.pc.num_of_pokemon){
                        mvwprintw(pokeWin, i + 2, 1, "%d-%s KO:%s",i+1, world.pc.pokemonArray[i]->name.c_str(), world.pc.pokemonArray[i]->kO? "Yes":"No");
                        i++;
                      }
                      refresh();
                      wrefresh(pokeWin);
                      reviveInput = getch();

                      if(reviveInput == '1' && world.pc.pokemonArray[0]->kO){
                        revives--;
                        world.pc.pokemonArray[0]->kO = false;
                        world.pc.pokemonArray[0]->hpDam = (world.pc.pokemonArray[0]->hp)/2;

                        if(selectedPokemon == 0){
                          pcHp = (world.pc.pokemonArray[0]->hp)/2;
                        }
                      }
                      

                      if(reviveInput == '2' && world.pc.pokemonArray[1]->kO){
                        revives--;
                        world.pc.pokemonArray[1]->kO = false;
                        world.pc.pokemonArray[1]->hpDam = (world.pc.pokemonArray[1]->hp)/2;

                        if(selectedPokemon == 1){
                            pcHp = (world.pc.pokemonArray[1]->hp)/2;
                        }
                      }
                      

                      if(reviveInput == '3' && world.pc.pokemonArray[2]->kO){
                        revives--;
                        world.pc.pokemonArray[2]->kO = false;
                        world.pc.pokemonArray[2]->hpDam = (world.pc.pokemonArray[2]->hp)/2;
                          if(selectedPokemon == 2){
                            pcHp = (world.pc.pokemonArray[2]->hp)/2;
                          }
                      }
                      

                      if(reviveInput == '4' && world.pc.pokemonArray[3]->kO){
                        revives--;
                        world.pc.pokemonArray[3]->kO = false;
                        world.pc.pokemonArray[3]->hpDam = (world.pc.pokemonArray[3]->hp)/2;

                         if(selectedPokemon == 3){
                            pcHp = (world.pc.pokemonArray[3]->hp)/2;
                          }
                      }
                     

                      if(reviveInput == '5' && world.pc.pokemonArray[4]->kO){
                        revives--;
                        world.pc.pokemonArray[4]->kO = false;
                        world.pc.pokemonArray[4]->hpDam = (world.pc.pokemonArray[4]->hp)/2;
                        if(selectedPokemon == 4){
                          pcHp = (world.pc.pokemonArray[4]->hp)/2;
                        }
                      }
                      

                      if(reviveInput == '6' && world.pc.pokemonArray[5]->kO){
                        revives--;
                        world.pc.pokemonArray[5]->kO = false;
                        world.pc.pokemonArray[5]->hpDam = (world.pc.pokemonArray[5]->hp)/2;

                        if(selectedPokemon == 5){
                          pcHp = (world.pc.pokemonArray[5]->hp)/2;
                        }
                      }
                      
                      wrefresh(meWin);
                      bagInput = 27;
                    
                  }
              }
            }
          }
          else if(esp == '3'){
            char runInput = 0;
            refresh();

            while(runInput != 27){
              WINDOW * bagWin = newwin(8,28,14,0);
              refresh();
              box(bagWin, 0, 0);
              mvwprintw(bagWin, 1, 1, "Try to flee?");
              mvwprintw(bagWin, 2, 1, "--------------");
              mvwprintw(bagWin, 3, 1, "1 - Escape Battle");
              mvwprintw(bagWin, 6, 1, "Press 'escape' to close");
              refresh();
              wrefresh(bagWin);

              if(runInput == '1'){
                int prob = 50;
                int runProb = rand() %100;
                WINDOW * runWin = newwin(8,28,14,0);
                refresh();
                box(runWin, 0, 0);
                if(prob < runProb){
                    mvwprintw(runWin, 1, 1, "Escape Successful!");
                    world.pc.isRunning = true;
                    refresh();
                    wrefresh(runWin);
                    getch();
                    break;
                }
                else{
                    mvwprintw(runWin, 1, 1, "Escape Failed.");
                    refresh();
                    wrefresh(runWin);
                    runInput = 27;
                    action = true;
                    getch();
                    break;
                }

                refresh();
                wrefresh(runWin);
              }
              refresh();
              runInput = getch();
            }
          }

          //Switch Pokemon
          else if(esp == '4'){
            char pokeInput = 0;
             while(pokeInput != 27){
              WINDOW * pokeWin = newwin(9,28,13,0);
              refresh();
              box(pokeWin, 0, 0);
              mvwprintw(pokeWin, 1, 1, "Select Your Pokemon");
              int i = 1;
              mvwprintw(pokeWin, i+1, 1, "%d-%s HP:%d/%d KO:%s",i, world.pc.pokemonArray[i-1]->name.c_str(), world.pc.pokemonArray[i-1]->hpDam, world.pc.pokemonArray[i-1]->hp, (world.pc.pokemonArray[i-1]->kO) ? "Y": "N");
              while(i < world.pc.num_of_pokemon){
                mvwprintw(pokeWin, i + 2, 1, "%d-%s HP:%d/%d KO:%s",i+1, world.pc.pokemonArray[i]->name.c_str(), world.pc.pokemonArray[i]->hpDam, world.pc.pokemonArray[i]->hp, (world.pc.pokemonArray[i]->kO) ? "Y": "N");
                i++;
              }
              refresh();
              wrefresh(pokeWin);
              pokeInput = getch();

              if(pokeInput == '1' && world.pc.pokemonArray[0] != NULL && !world.pc.pokemonArray[0]->kO){
                selectedPokemon = 0;
                refresh();
                wmove(meWin,5,1);
                wclrtoeol(meWin);
                  if(world.pc.pokemonArray[selectedPokemon]->isShiny){
                    wattron(meWin,COLOR_PAIR(COLOR_YELLOW));
                    mvwprintw(meWin, 5, 1, "%s *shiny*", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                    wattroff(meWin,COLOR_PAIR(COLOR_YELLOW));
                  }
                  else{
                    mvwprintw(meWin, 5, 1, "%s", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                  }
                  mvwprintw(meWin, 6, 1, "HP: %d/%d Att:%d Def:%d", world.pc.pokemonArray[selectedPokemon]->hpDam, world.pc.pokemonArray[selectedPokemon]->hp
                                                                    , world.pc.pokemonArray[selectedPokemon]->attack, world.pc.pokemonArray[selectedPokemon]->defense);
                wrefresh(meWin);
                action = true;
                pcHp = world.pc.pokemonArray[selectedPokemon]->hpDam;
                break;
              }

              else if(pokeInput == '2' && world.pc.pokemonArray[1] != NULL && !world.pc.pokemonArray[1]->kO){
                selectedPokemon = 1;
                refresh();
                wmove(meWin,5,1);
                wclrtoeol(meWin);
                        if(world.pc.pokemonArray[selectedPokemon]->isShiny){
                            wattron(meWin,COLOR_PAIR(COLOR_YELLOW));
                            mvwprintw(meWin, 5, 1, "%s *shiny*", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                            wattroff(meWin,COLOR_PAIR(COLOR_YELLOW));
                          }
                          else{
                            mvwprintw(meWin, 5, 1, "%s", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          }
                          mvwprintw(meWin, 6, 1, "HP: %d/%d Att:%d Def:%d", world.pc.pokemonArray[selectedPokemon]->hpDam, world.pc.pokemonArray[selectedPokemon]->hp
                                                                    , world.pc.pokemonArray[selectedPokemon]->attack, world.pc.pokemonArray[selectedPokemon]->defense);
                wrefresh(meWin);
                pcHp = world.pc.pokemonArray[selectedPokemon]->hpDam;
                action = true;
                break;
              }
              else if(pokeInput == '3' && world.pc.pokemonArray[2] != NULL && !world.pc.pokemonArray[2]->kO){
                selectedPokemon = 2;
                refresh();
                wmove(meWin,5,1);
                wclrtoeol(meWin);
                        if(world.pc.pokemonArray[selectedPokemon]->isShiny){
                            wattron(meWin,COLOR_PAIR(COLOR_YELLOW));
                            mvwprintw(meWin, 5, 1, "%s *shiny*", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                            wattroff(meWin,COLOR_PAIR(COLOR_YELLOW));
                          }
                          else{
                            mvwprintw(meWin, 5, 1, "%s", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          }
                          mvwprintw(meWin, 6, 1, "HP: %d/%d Att:%d Def:%d", world.pc.pokemonArray[selectedPokemon]->hpDam, world.pc.pokemonArray[selectedPokemon]->hp
                                                                    , world.pc.pokemonArray[selectedPokemon]->attack, world.pc.pokemonArray[selectedPokemon]->defense);
                wrefresh(meWin);
                action = true;
                pcHp = world.pc.pokemonArray[selectedPokemon]->hpDam;
                break;
              }
              else if(pokeInput == '4' && world.pc.pokemonArray[3] != NULL && !world.pc.pokemonArray[3]->kO){
                selectedPokemon = 3;
                refresh();
                wmove(meWin,5,1);
                wclrtoeol(meWin);
                        if(world.pc.pokemonArray[selectedPokemon]->isShiny){
                            wattron(meWin,COLOR_PAIR(COLOR_YELLOW));
                            mvwprintw(meWin, 5, 1, "%s *shiny*", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                            wattroff(meWin,COLOR_PAIR(COLOR_YELLOW));
                          }
                          else{
                            mvwprintw(meWin, 5, 1, "%s", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          }
                          mvwprintw(meWin, 6, 1, "HP: %d/%d Att:%d Def:%d", world.pc.pokemonArray[selectedPokemon]->hpDam, world.pc.pokemonArray[selectedPokemon]->hp
                                                                    , world.pc.pokemonArray[selectedPokemon]->attack, world.pc.pokemonArray[selectedPokemon]->defense);
                wrefresh(meWin);
                pcHp = world.pc.pokemonArray[selectedPokemon]->hpDam;
                action = true;
                break;
              }

              else if(pokeInput == '5' && world.pc.pokemonArray[4] != NULL && !world.pc.pokemonArray[4]->kO){
                selectedPokemon = 4;
                refresh();
                wmove(meWin,5,1);
                wclrtoeol(meWin);
                        if(world.pc.pokemonArray[selectedPokemon]->isShiny){
                            wattron(meWin,COLOR_PAIR(COLOR_YELLOW));
                            mvwprintw(meWin, 5, 1, "%s *shiny*", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                            wattroff(meWin,COLOR_PAIR(COLOR_YELLOW));
                          }
                          else{
                            mvwprintw(meWin, 5, 1, "%s", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          }
                          mvwprintw(meWin, 6, 1, "HP: %d/%d Att:%d Def:%d", world.pc.pokemonArray[selectedPokemon]->hpDam, world.pc.pokemonArray[selectedPokemon]->hp
                                                                    , world.pc.pokemonArray[selectedPokemon]->attack, world.pc.pokemonArray[selectedPokemon]->defense);
                wrefresh(meWin);
                pcHp = world.pc.pokemonArray[selectedPokemon]->hpDam;
                action = true;
                break;
              }
              else if(pokeInput == '6' && world.pc.pokemonArray[5] != NULL && !world.pc.pokemonArray[5]->kO){
                selectedPokemon = 5;
                refresh();
                wmove(meWin,5,1);
                wclrtoeol(meWin);
                        if(world.pc.pokemonArray[selectedPokemon]->isShiny){
                            wattron(meWin,COLOR_PAIR(COLOR_YELLOW));
                            mvwprintw(meWin, 5, 1, "%s *shiny*", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                            wattroff(meWin,COLOR_PAIR(COLOR_YELLOW));
                          }
                          else{
                            mvwprintw(meWin, 5, 1, "%s", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          }
                          mvwprintw(meWin, 6, 1, "HP: %d/%d Att:%d Def:%d", world.pc.pokemonArray[selectedPokemon]->hpDam, world.pc.pokemonArray[selectedPokemon]->hp
                                                                    , world.pc.pokemonArray[selectedPokemon]->attack, world.pc.pokemonArray[selectedPokemon]->defense);
                wrefresh(meWin);
                wrefresh(pokeWin);
                pcHp = world.pc.pokemonArray[selectedPokemon]->hpDam;
                action = true;
                break;
              }
             } 
          }

          wclear(win2);
          box(win2, 0, 0);
          mvwprintw(win2, 1, 1, "A Wild Pokemon Appears!");
          if(p->isShiny){
            wattron(win2,COLOR_PAIR(COLOR_YELLOW));
            mvwprintw(win2, 3, 1, "Name: %s *shiny*", p->name.c_str());
            wattroff(win2,COLOR_PAIR(COLOR_YELLOW));
          }
          else{
            mvwprintw(win2, 3, 1, "Name: %s", p->name.c_str());
          }
        
          mvwprintw(win2, 4, 1, "Level: %d", p->level);
          mvwprintw(win2, 5, 1, "Stats: ");
          mvwprintw(win2, 6, 1, "HP:  %d/%d", enemHp,enemBase);
          mvwprintw(win2, 7, 1, "Attack:%d Defense:%d",p->attack, p->defense);
          mvwprintw(win2, 8, 1, "Speed:%d", p->speed);
          mvwprintw(win2, 9, 1, "Accuracy:%d Evasion:%d", p->accuracy, p->evasion);
          if(p->gender == 1){
            mvwprintw(win2, 10, 1, "Gender: %s", "M");
          }
          else if(p->gender == 0){
            mvwprintw(win2, 10, 1, "Gender: %s", "F");
          }

          if(p->isShiny){
            mvwprintw(win2, 11, 1, "Shiny: Yes");
          }
          else{
            mvwprintw(win2, 11, 1, "Shiny: No");
          }

          mvwprintw(win2, 13, 1, "Move 1: %s ", p->m[0].identifier.c_str());
          if(p->m[1].identifier != ""){
            mvwprintw(win2, 14, 1, "Move 2: %s ", p->m[1].identifier.c_str());
            int mSel = rand()%2;
            prioQue[1] = ((mSel == 0) ? p->m[0]: p->m[1]);
          }
          else{
            mvwprintw(win2, 14, 1, "Move 2: None");
            prioQue[1] = (p->m[0]);
          }

          wrefresh(meWin);
          wrefresh(win2);


          ///Start of Fight
          vector<int> damageVec;
          int pcDamage;
          
          if(action){
            WINDOW * fightWin = newwin(7,60,10,10);
            refresh();
            wclear(fightWin);
            box(fightWin, 0, 0);
            if(prioQue[0].identifier == ""){
              pcDamage = 0;
            }
            else{
              pcDamage = damageCalc(world.pc.pokemonArray[selectedPokemon]->level,world.pc.pokemonArray[selectedPokemon]->attack, prioQue[0], p->defense, p);
            }
            
            int enemDamage = damageCalc(p->level, p->attack, prioQue[1], world.pc.pokemonArray[selectedPokemon]->defense, world.pc.pokemonArray[selectedPokemon]);

            mvwprintw(fightWin, 1, 1, "Round: %d", roundCount);
            if(prioQue[0].priority > prioQue[1].priority){
              mvwprintw(fightWin, 2, 1, "%s: %d damage to %s", world.pc.pokemonArray[selectedPokemon]->name.c_str(), pcDamage, p->name.c_str());
              enemHp = enemHp - pcDamage;
              if(enemHp <= 0){
                enemHp = 0;
                p->kO = true;
                mvwprintw(fightWin, 4, 1, "%s is knockedout!", p->name.c_str());
              }
              else{
                mvwprintw(fightWin, 3, 1, "%s: %d damage to %s", p->name.c_str(), enemDamage, world.pc.pokemonArray[selectedPokemon]->name.c_str());
                pcHp = pcHp - enemDamage;
                if(pcHp <= 0){
                  pcHp = 0;
                  world.pc.pokemonArray[selectedPokemon]->kO = true;
                  mvwprintw(fightWin, 4, 1, "%s is knockedout!", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                }
                else{
                  mvwprintw(fightWin, 4, 1, "No Pokemon Were Knocked Out");
                  mvwprintw(fightWin, 5, 1, "Continue the fight!             Press any key to continue.");
                }
              }
            }

            else if(prioQue[0].priority < prioQue[1].priority){
              mvwprintw(fightWin, 2, 1, "%s: %d damage to %s",p->name.c_str() , enemDamage, world.pc.pokemonArray[selectedPokemon]->name.c_str());
              pcHp = pcHp - enemDamage;
    
              if(pcHp <= 0){
                  pcHp = 0;
                  world.pc.pokemonArray[selectedPokemon]->kO = true;
                  mvwprintw(fightWin, 4, 1, "%s is knockedout!", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                
              }
              else{
                mvwprintw(fightWin, 3, 1, "%s: %d damage to %s", world.pc.pokemonArray[selectedPokemon]->name.c_str(), pcDamage, p->name.c_str());
                enemHp = enemHp - pcDamage;
                if(enemHp <= 0){
                  enemHp = 0;
                  p->kO = true;
                  mvwprintw(fightWin, 4, 1, "%s is knockedout!", p->name.c_str());
                }
                else{
                  mvwprintw(fightWin, 4, 1, "No Pokemon Were Knocked Out");
                  mvwprintw(fightWin, 5, 1, "Continue the fight!             Press any key to continue.");
                }
              }
            }
              
            else{
                  if(world.pc.pokemonArray[selectedPokemon]->speed > p->speed){
                      mvwprintw(fightWin, 2, 1, "%s: %d damage to %s", world.pc.pokemonArray[selectedPokemon]->name.c_str(), pcDamage, p->name.c_str());
                      enemHp = enemHp - pcDamage;
                      if(enemHp <= 0){
                          enemHp= 0;
                          p->kO = true;
                          mvwprintw(fightWin, 4, 1, "%s is knockedout!", p->name.c_str());
                      }
                      else{
                          mvwprintw(fightWin, 3, 1, "%s: %d damage to %s", p->name.c_str(), enemDamage, world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          pcHp = pcHp - enemDamage;

                          if(pcHp <= 0){
                            pcHp = 0;
                            world.pc.pokemonArray[selectedPokemon]->kO = true;
                            mvwprintw(fightWin, 4, 1, "%s is knockedout!", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          }
                          else{
                            mvwprintw(fightWin, 4, 1, "No Pokemon Were Knocked Out");
                            mvwprintw(fightWin, 5, 1, "Continue the fight!             Press any key to continue.");
                          }
                      }
                  }

                  else if(world.pc.pokemonArray[selectedPokemon]->speed < p->speed){
                      mvwprintw(fightWin, 2, 1, "%s: %d damage to %s",p->name.c_str() , enemDamage, world.pc.pokemonArray[selectedPokemon]->name.c_str());
                      pcHp = pcHp - enemDamage;
        
                      if(pcHp <= 0){
                          pcHp = 0;
                          world.pc.pokemonArray[selectedPokemon]->kO = true;
                          mvwprintw(fightWin, 4, 1, "%s is knockedout!", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                    
                      }
                      else{
                        mvwprintw(fightWin, 3, 1, "%s: %d damage to %s", world.pc.pokemonArray[selectedPokemon]->name.c_str(), pcDamage, p->name.c_str());
                        enemHp = enemHp - pcDamage;
                        if(enemHp <= 0){
                          enemHp = 0;
                          p->kO = true;
                          mvwprintw(fightWin, 4, 1, "%s is knockedout!", p->name.c_str());
                        }
                        else{
                          mvwprintw(fightWin, 4, 1, "No Pokemon Were Knocked Out");
                          mvwprintw(fightWin, 5, 1, "Continue the fight!             Press any key to continue.");
                        }
                      }
                  }


                  else{
                      int randPick = rand() % 2;

                      if(randPick == 0){
                          mvwprintw(fightWin, 2, 1, "%s: %d damage to %s", world.pc.pokemonArray[selectedPokemon]->name.c_str(), pcDamage, p->name.c_str());
                          enemHp = enemHp - pcDamage;
                                if(enemHp <= 0){
                                    enemHp = 0;
                                    p->kO = true;
                                    mvwprintw(fightWin, 4, 1, "%s is knockedout!", p->name.c_str());
                                }
            
                                else{
                                    mvwprintw(fightWin, 3, 1, "%s: %d damage to %s", p->name.c_str(), enemDamage, world.pc.pokemonArray[selectedPokemon]->name.c_str());
                                    pcHp = pcHp - enemDamage;

                                    if(pcHp <= 0){
                                        pcHp = 0;
                                        world.pc.pokemonArray[selectedPokemon]->kO = true;
                                        mvwprintw(fightWin, 4, 1, "%s is knockedout!", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                                    }
                                    else{
                                      mvwprintw(fightWin, 4, 1, "No Pokemon Were Knocked Out");
                                      mvwprintw(fightWin, 5, 1, "Continue the fight             Press any key to continue.");
                                    }
                                }
                      }
                      else{
                          mvwprintw(fightWin, 2, 1, "%s: %d damage to %s",p->name.c_str() , enemDamage, world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          pcHp = pcHp - enemDamage;
        
                          if(pcHp <= 0){
                              pcHp = 0;
                              world.pc.pokemonArray[selectedPokemon]->kO = true;
                              mvwprintw(fightWin, 4, 1, "%s is knockedout!", world.pc.pokemonArray[selectedPokemon]->name.c_str());
                          }

                          else{
                            mvwprintw(fightWin, 3, 1, "%s: %d damage to %s", world.pc.pokemonArray[selectedPokemon]->name.c_str(), pcDamage, p->name.c_str());
                            enemHp = enemHp - pcDamage;
                            if(enemHp <= 0){
                              enemHp = 0;
                              p->kO = true;
                              mvwprintw(fightWin, 4, 1, "%s is knockedout!", p->name.c_str());
                            }
                            else{
                              mvwprintw(fightWin, 4, 1, "No Pokemon Were Knocked Out");
                              mvwprintw(fightWin, 5, 1, "Continue the fight!             Press any key to continue.");
                            }
                          }
                      }

                  }
            }

            roundCount++;
            world.pc.pokemonArray[selectedPokemon]->hpDam = pcHp;
            p->hpDam = enemHp;
            p->hp = enemBase;

            wrefresh(fightWin);
            //getch();
            wclear(fightWin);
            wrefresh(meWin);
            wrefresh(win2);
            action = false;

            if(checkDefeat()){
              char fightGet = 0;
              while(fightGet != 27){
              world.pc.totalDefeat = true;
              WINDOW * fightWin = newwin(7,60,10,10);
              refresh();
              wclear(fightWin);
              box(fightWin, 0, 0);
              mvwprintw(fightWin, 1, 16, "All PC's Pokemon Are Unconcoius.");
              mvwprintw(fightWin, 2, 11, "Find the closest Pokemon Center for help.");
              mvwprintw(fightWin, 5, 18, "Press 'escape' to continue.");
              wrefresh(fightWin);
              fightGet = getch();
              }
            }
          }
        
          wrefresh(meWin);
          wrefresh(win2);
          refresh();
          esp = getch();
        }
        
        world.pc.isRunning = false;
        world.pc.numPotions = potions;
        world.pc.numPokeballs = pokeBalls;
        world.pc.numRevives = revives;

      }
      
  }
  
  if (move_cost[char_pc][world.cur_map->map[dest[dim_y]][dest[dim_x]]] ==
      INT_MAX) {
    return 1;
  }

  return 0;
}


void io_bag(){
        char bagInput = 0;
        while(bagInput != 27){
          WINDOW * bagWin = newwin(10,20,2,60);
          refresh();
          box(bagWin, 0, 0);
          mvwprintw(bagWin, 1, 1, "Bag Inventory!");
          mvwprintw(bagWin, 2, 1, "--------------");
          mvwprintw(bagWin, 3, 1, "1-Potion:   x%d", world.pc.numPotions);
          mvwprintw(bagWin, 4, 1, "2-Pokeball: x%d", world.pc.numPokeballs);
          mvwprintw(bagWin, 5, 1, "3-Revive:   x%d", world.pc.numRevives);
          mvwprintw(bagWin, 7, 1, "Press 'escape'");
          mvwprintw(bagWin, 8, 1, "to close");
          refresh();
          wrefresh(bagWin);
          bagInput = getch();
          
          if(bagInput == '1'){
              char potionInput = 0;
                  if(world.pc.numPotions > 0){
                      WINDOW * potionWin = newwin(9,28,12,51);
                      refresh();
                      box(potionWin, 0, 0);
                      mvwprintw(potionWin, 1, 1, "Use Potion On Pokemon");
                      int i = 1;
                      mvwprintw(potionWin, i+1, 1, "%d-%s HP:%d/%d",i, world.pc.pokemonArray[i-1]->name.c_str(), world.pc.pokemonArray[i-1]->hpDam, world.pc.pokemonArray[i-1]->hp);
                      while(i < world.pc.num_of_pokemon){
                        mvwprintw(potionWin, i + 2, 1, "%d-%s HP:%d/%d",i+1, world.pc.pokemonArray[i]->name.c_str(), world.pc.pokemonArray[i]->hpDam, world.pc.pokemonArray[i]->hp);
                        i++;
                      }

                      refresh();
                      wrefresh(potionWin);
                      potionInput = getch();

                      if(potionInput == '1' && (world.pc.pokemonArray[0]->hpDam != world.pc.pokemonArray[0]->hp)){
                        world.pc.numPotions--;
                        world.pc.pokemonArray[0]->hpDam = (world.pc.pokemonArray[0]->hpDam + 20 > world.pc.pokemonArray[0]->hp) ? world.pc.pokemonArray[0]->hp: world.pc.pokemonArray[0]->hpDam + 20;
                      }


                      if(potionInput == '2' && (world.pc.pokemonArray[1]->hpDam != world.pc.pokemonArray[1]->hp)){
                        world.pc.numPotions--;
                        world.pc.pokemonArray[1]->hpDam = (world.pc.pokemonArray[1]->hpDam + 20 > world.pc.pokemonArray[1]->hp) ? world.pc.pokemonArray[1]->hp: world.pc.pokemonArray[1]->hpDam + 20;
                      
                      }
                     

                      if(potionInput == '3' && (world.pc.pokemonArray[2]->hpDam != world.pc.pokemonArray[2]->hp)){
                        world.pc.numPotions--;
                        world.pc.pokemonArray[2]->hpDam = (world.pc.pokemonArray[2]->hpDam + 20 > world.pc.pokemonArray[2]->hp) ? world.pc.pokemonArray[2]->hp: world.pc.pokemonArray[2]->hpDam + 20;
                      
                      }
                      

                      if(potionInput == '4' && (world.pc.pokemonArray[3]->hpDam != world.pc.pokemonArray[3]->hp)){
                        world.pc.numPotions--;
                        world.pc.pokemonArray[3]->hpDam = (world.pc.pokemonArray[3]->hpDam + 20 > world.pc.pokemonArray[3]->hp) ? world.pc.pokemonArray[3]->hp: world.pc.pokemonArray[3]->hpDam + 20;
                      
                      }
                      

                      if(potionInput == '5' && (world.pc.pokemonArray[4]->hpDam != world.pc.pokemonArray[4]->hp)){
                        world.pc.numPotions--;
                        world.pc.pokemonArray[4]->hpDam = (world.pc.pokemonArray[4]->hpDam + 20 > world.pc.pokemonArray[4]->hp) ? world.pc.pokemonArray[4]->hp: world.pc.pokemonArray[4]->hpDam + 20;
                      
                      }
                      

                      if(potionInput == '6' && (world.pc.pokemonArray[5]->hpDam != world.pc.pokemonArray[5]->hp)){
                        world.pc.numPotions--;
                        world.pc.pokemonArray[5]->hpDam = (world.pc.pokemonArray[5]->hpDam + 20 > world.pc.pokemonArray[5]->hp) ? world.pc.pokemonArray[5]->hp: world.pc.pokemonArray[5]->hpDam + 20;                    
                      }
                      
                      bagInput = 27;
                    
                  }
          }

          if(bagInput == '2'){
            WINDOW * pokeWin = newwin(9,28,12,51);
            refresh();
            box(pokeWin, 0, 0);
            mvwprintw(pokeWin, 1, 1, "Please don't waste");
            mvwprintw(pokeWin, 2, 1, "Pokeballs.");
            mvwprintw(pokeWin, 3, 1, "They are expensive!");
            refresh();
            wrefresh(pokeWin);
            getch();
            break;
          }


          if(bagInput == '3'){
                  char reviveInput = 0;
                  if(world.pc.numRevives > 0){
                      WINDOW * pokeWin = newwin(9,28,12,51);
                      refresh();
                      box(pokeWin, 0, 0);
                      mvwprintw(pokeWin, 1, 1, "Use Revive On Pokemon");
                      int i = 1;
                      mvwprintw(pokeWin, i+1, 1, "%d-%s KO:%s",i, world.pc.pokemonArray[i-1]->name.c_str(), world.pc.pokemonArray[i-1]->kO ? "Yes":"No");
                      while(i < world.pc.num_of_pokemon){
                        mvwprintw(pokeWin, i + 2, 1, "%d-%s KO:%s",i+1, world.pc.pokemonArray[i]->name.c_str(), world.pc.pokemonArray[i]->kO? "Yes":"No");
                        i++;
                      }

                      refresh();
                      wrefresh(pokeWin);
                      reviveInput = getch();

                      if(reviveInput == '1' && world.pc.pokemonArray[0]->kO){
                        world.pc.numRevives--;
                        world.pc.pokemonArray[0]->kO = false;
                        world.pc.pokemonArray[0]->hpDam = (world.pc.pokemonArray[0]->hp)/2;
                        world.pc.totalDefeat = false;
                      }


                      if(reviveInput == '2' && world.pc.pokemonArray[1]->kO){
                        world.pc.numRevives--;
                        world.pc.pokemonArray[1]->kO = false;
                        world.pc.pokemonArray[1]->hpDam = (world.pc.pokemonArray[1]->hp)/2;
                        world.pc.totalDefeat = false;
                      }
                     

                      if(reviveInput == '3' && world.pc.pokemonArray[2]->kO){
                        world.pc.numRevives--;
                        world.pc.pokemonArray[2]->kO = false;
                        world.pc.pokemonArray[2]->hpDam = (world.pc.pokemonArray[2]->hp)/2;
                        world.pc.totalDefeat = false;
                      }
                      

                      if(reviveInput == '4' && world.pc.pokemonArray[3]->kO){
                        world.pc.numRevives--;
                        world.pc.pokemonArray[3]->kO = false;
                        world.pc.pokemonArray[3]->hpDam = (world.pc.pokemonArray[3]->hp)/2;
                        world.pc.totalDefeat = false;
                      }
                      

                      if(reviveInput == '5' && world.pc.pokemonArray[4]->kO){
                        world.pc.numRevives--;
                        world.pc.pokemonArray[4]->kO = false;
                        world.pc.pokemonArray[4]->hpDam = (world.pc.pokemonArray[4]->hp)/2;
                        world.pc.totalDefeat = false;
                      }
                      

                      if(reviveInput == '6' && world.pc.pokemonArray[5]->kO){
                        world.pc.numRevives--;
                        world.pc.pokemonArray[5]->kO = false;
                        world.pc.pokemonArray[5]->hpDam = (world.pc.pokemonArray[5]->hp)/2;
                        world.pc.totalDefeat = false;
                      }
                      
                      bagInput = 27;
                    
                  }
          }
        }
        io_display();
}
void io_pokeCheck(){
    char pokeCheck = 0;
    while(pokeCheck != 27){
      WINDOW * pokeWin = newwin(10,40,1,40);
      refresh();
      box(pokeWin, 0, 0);
      mvwprintw(pokeWin, 1, 9, "-----Your Pokemon-----");
      int i = 1;
      mvwprintw(pokeWin, i+1, 1, "%d-%s HP: %d/%d KO:%s",i, world.pc.pokemonArray[i-1]->name.c_str(), world.pc.pokemonArray[i-1]->hpDam, world.pc.pokemonArray[i-1]->hp,world.pc.pokemonArray[i-1]->kO ? "Yes":"No");
      while(i < world.pc.num_of_pokemon){
            mvwprintw(pokeWin, i + 2, 1, "%d-%s HP: %d/%d KO:%s",i+1, world.pc.pokemonArray[i]->name.c_str(),world.pc.pokemonArray[i-1]->hpDam, world.pc.pokemonArray[i-1]->hp,world.pc.pokemonArray[i]->kO? "Yes":"No");
            i++;
      }
      mvwprintw(pokeWin, 8, 9, "Press 'escape' to close.");

      refresh();
      wrefresh(pokeWin);
      pokeCheck = getch();
    }
    io_display();
}

void io_teleport_world(pair_t dest)
{
  /* mvscanw documentation is unclear about return values.  I believe *
   * that the return value works the same way as scanf, but instead   *
   * of counting on that, we'll initialize x and y to out of bounds   *
   * values and accept their updates only if in range.                */
  int x = INT_MAX, y = INT_MAX;
  
  world.cur_map->cmap[world.pc.pos[dim_y]][world.pc.pos[dim_x]] = NULL;

  echo();
  curs_set(1);
  do {
    mvprintw(0, 0, "Enter x [-200, 200]:           ");
    refresh();
    mvscanw(0, 21, "%d", &x);
  } while (x < -200 || x > 200);
  do {
    mvprintw(0, 0, "Enter y [-200, 200]:          ");
    refresh();
    mvscanw(0, 21, "%d", &y);
  } while (y < -200 || y > 200);

  refresh();
  noecho();
  curs_set(0);

  x += 200;
  y += 200;

  world.cur_idx[dim_x] = x;
  world.cur_idx[dim_y] = y;

  new_map(1);
  io_teleport_pc(dest);
}

void io_handle_input(pair_t dest)
{
  uint32_t turn_not_consumed;
  int key;

  do {
    switch (key = getch()) {
    case '7':
    case 'y':
    case KEY_HOME:
      turn_not_consumed = move_pc_dir(7, dest);
      break;
    case '8':
    case 'k':
    case KEY_UP:
      turn_not_consumed = move_pc_dir(8, dest);
      break;
    case '9':
    case 'u':
    case KEY_PPAGE:
      turn_not_consumed = move_pc_dir(9, dest);
      break;
    case '6':
    case 'l':
    case KEY_RIGHT:
      turn_not_consumed = move_pc_dir(6, dest);
      break;
    case '3':
    case 'n':
    case KEY_NPAGE:
      turn_not_consumed = move_pc_dir(3, dest);
      break;
    case '2':
    case 'j':
    case KEY_DOWN:
      turn_not_consumed = move_pc_dir(2, dest);
      break;
    case '1':
    case 'b':
    case KEY_END:
      turn_not_consumed = move_pc_dir(1, dest);
      break;
    case '4':
    case 'h':
    case KEY_LEFT:
      turn_not_consumed = move_pc_dir(4, dest);
      break;
    case '5':
    case ' ':
    case '.':
    case KEY_B2:
      dest[dim_y] = world.pc.pos[dim_y];
      dest[dim_x] = world.pc.pos[dim_x];
      turn_not_consumed = 0;
      break;
    case '>':
      turn_not_consumed = move_pc_dir('>', dest);
      break;
    case 'Q':
      dest[dim_y] = world.pc.pos[dim_y];
      dest[dim_x] = world.pc.pos[dim_x];
      world.quit = 1;
      turn_not_consumed = 0;
      break;
      break;
    case 't':
      io_list_trainers();
      turn_not_consumed = 1;
      break;
    case 'p':
      /* Teleport the PC to a random place in the map.              */
      io_teleport_pc(dest);
      turn_not_consumed = 0;
      break;
    case 'f':
      /* Fly to any map in the world.                                */
      io_teleport_world(dest);
      turn_not_consumed = 0;
      break;    
    case 'q':
      /* Demonstrate use of the message queue.  You can use this for *
       * printf()-style debugging (though gdb is probably a better   *
       * option.  Not that it matters, but using this command will   *
       * waste a turn.  Set turn_not_consumed to 1 and you should be *
       * able to figure out why I did it that way.                   */
      io_queue_message("This is the first message.");
      io_queue_message("Since there are multiple messages, "
                       "you will see \"more\" prompts.");
      io_queue_message("You can use any key to advance through messages.");
      io_queue_message("Normal gameplay will not resume until the queue "
                       "is empty.");
      io_queue_message("Long lines will be truncated, not wrapped.");
      io_queue_message("io_queue_message() is variadic and handles "
                       "all printf() conversion specifiers.");
      io_queue_message("Did you see %s?", "what I did there");
      io_queue_message("When the last message is displayed, there will "
                       "be no \"more\" prompt.");
      io_queue_message("Have fun!  And happy printing!");
      io_queue_message("Oh!  And use 'Q' to quit!");

      dest[dim_y] = world.pc.pos[dim_y];
      dest[dim_x] = world.pc.pos[dim_x];
      turn_not_consumed = 0;
      break;
    case 'B':
      //open bag
      io_bag();
      break;
    case 'P':
      io_pokeCheck();
    default:
      /* Also not in the spec.  It's not always easy to figure out what *
       * key code corresponds with a given keystroke.  Print out any    *
       * unhandled key here.  Not only does it give a visual error      *
       * indicator, but it also gives an integer value that can be used *
       * for that key in this (or other) switch statements.  Printed in *
       * octal, with the leading zero, because ncurses.h lists codes in *
       * octal, thus allowing us to do reverse lookups.  If a key has a *
       * name defined in the header, you can use the name here, else    *
       * you can directly use the octal value.                          */
      mvprintw(0, 0, "Unbound key: %#o ", key);
      turn_not_consumed = 1;
    }
    refresh();
  } while (turn_not_consumed);
}
