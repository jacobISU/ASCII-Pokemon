#ifndef POKE327_H
# define POKE327_H

# include <stdlib.h>
# include <assert.h>
# include <string.h>
# include <string>

# include "heap.h"
# include "pair.h"
# include "character.h"

using namespace std;

/* Returns true if random float in [0,1] is less than *
 * numerator/denominator.  Uses only integer math.    */
# define rand_under(numerator, denominator) \
  (rand() < ((RAND_MAX / denominator) * numerator))

/* Returns random integer in [min, max]. */
# define rand_range(min, max) ((rand() % (((max) + 1) - (min))) + (min))

# define UNUSED(f) ((void) f)

#define MAP_X              80
#define MAP_Y              21
#define MIN_TREES          10
#define MIN_BOULDERS       10
#define TREE_PROB          95
#define BOULDER_PROB       95
#define WORLD_SIZE         401

#define MIN_TRAINERS     7
#define ADD_TRAINER_PROB 60

#define MOUNTAIN_SYMBOL       '%'
#define BOULDER_SYMBOL        '0'
#define TREE_SYMBOL           '4'
#define FOREST_SYMBOL         '^'
#define GATE_SYMBOL           '#'
#define PATH_SYMBOL           '#'
#define POKEMART_SYMBOL       'M'
#define POKEMON_CENTER_SYMBOL 'C'
#define TALL_GRASS_SYMBOL     ':'
#define SHORT_GRASS_SYMBOL    '.'
#define WATER_SYMBOL          '~'
#define ERROR_SYMBOL          '&'

#define PC_SYMBOL       '@'
#define HIKER_SYMBOL    'h'
#define RIVAL_SYMBOL    'r'
#define EXPLORER_SYMBOL 'e'
#define SENTRY_SYMBOL   's'
#define PACER_SYMBOL    'p'
#define SWIMMER_SYMBOL  'm'
#define WANDERER_SYMBOL 'w'

#define mappair(pair) (m->map[pair[dim_y]][pair[dim_x]])
#define mapxy(x, y) (m->map[y][x])
#define heightpair(pair) (m->height[pair[dim_y]][pair[dim_x]])
#define heightxy(x, y) (m->height[y][x])

typedef enum __attribute__ ((__packed__)) terrain_type {
  ter_boulder,
  ter_tree,
  ter_path,
  ter_mart,
  ter_center,
  ter_grass,
  ter_clearing,
  ter_mountain,
  ter_forest,
  ter_water,
  ter_gate,
  num_terrain_types,
  ter_debug
} terrain_type_t;

extern int32_t move_cost[num_character_types][num_terrain_types];

extern void (*move_func[num_movement_types])(character *, pair_t);

typedef struct map {
  terrain_type_t map[MAP_Y][MAP_X];
  uint8_t height[MAP_Y][MAP_X];
  character *cmap[MAP_Y][MAP_X];
  heap_t turn;
  int32_t num_trainers;
  int8_t n, s, e, w;
} map_t;

typedef struct world {
  map_t *world[WORLD_SIZE][WORLD_SIZE];
  pair_t cur_idx;
  map_t *cur_map;
  /* Please distance maps in world, not map, since *
   * we only need one pair at any given time.      */
  int hiker_dist[MAP_Y][MAP_X];
  int rival_dist[MAP_Y][MAP_X];
  class pc pc;
  int quit;
  int add_trainer_prob;
  int char_seq_num;
} world_t;

/* Even unallocated, a WORLD_SIZE x WORLD_SIZE array of pointers is a very *
 * large thing to put on the stack.  To avoid that, world is a global.     */
extern world_t world;

extern pair_t all_dirs[8];

#define rand_dir(dir) {     \
  int _i = rand() & 0x7;    \
  dir[0] = all_dirs[_i][0]; \
  dir[1] = all_dirs[_i][1]; \
}

typedef struct path {
  heap_node_t *hn;
  uint8_t pos[2];
  uint8_t from[2];
  int32_t cost;
} path_t;

//Pokemon Class
// id,identifier,species_id,height,weight,base_experience,order,is_default
// 1,bulbasaur,1,7,69,64,1,1
class pokemon
{
  public:

  int id;
  string identifier;
  int species_id;
  int height;
  int weight;
  int base_experience;
  int order;
  int is_default;
};

//Moves Class
//id,identifier,generation_id,type_id,power,pp,accuracy,priority,target_id,
//damage_class_id,effect_id,effect_chance,contest_type_id,contest_effect_id,super_contest_effect_id
//1,pound,1,1,40,35,100,0,10,2,1,,5,1,5
class moves
{
  public:

  int id;
  string identifier;
  int generation_id;
  int type_id;
  int power;
  int pp;
  int accuracy;
  int priority;
  int target_id;
  int damage_class_id;
  int effect_id;
  int effect_chance;
  int contest_type_id;
  int contest_effect_id;
  int super_contest_effect_id;
};

//Pokemon Moves
//pokemon_id,version_group_id,move_id,pokemon_move_method_id,level,order
//1,1,14,4,0,
class pokemon_moves
{
  public:

  int pokemon_id;
  int version_group_id;
  int move_id;
  int pokemon_move_method_id;
  int level;
  int order;
};

//Pokemon species
//id,identifier,generation_id,evolves_from_species_id,evolution_chain_id,color_id,shape_id,habitat_id,gender_rate,
//capture_rate,base_happiness,is_baby,hatch_counter,has_gender_differences,growth_rate_id,forms_switchable,
//is_legendary,is_mythical,order,conquest_order
//1,bulbasaur,1,,1,5,8,3,1,45,50,0,20,0,4,0,0,0,1,
class pokemon_species
{
  public:

  int id;
  string identifier;
  int generation_id;
  int evolves_from_species_id;
  int evolution_chain_id,color_id;
  int shape_id;
  int habitat_id;
  int gender_rate;
  int capture_rate;
  int base_happiness;
  int is_baby;
  int hatch_counter;
  int has_gender_differences;
  int growth_rate_id;
  int forms_switchable;
  int is_legendary;
  int is_mythical;
  int order;
  int conquest_order;
};

//Experience
//growth_rate_id,level,experience
//1,1,0
class experience
{
  public:

  int growth_rate_id;
  int level;
  int experience;
};

//Type Names
//type_id,local_language_id,name
//1,1,ノーマル
class type_names
{
  public:

  int type_id;
  int local_language_id;
  string name;
};

//pokemon_stats
//pokemon_id,stat_id,base_stat,effort
//1,1,45,0
class pokemon_stats
{
  public:

  int pokemon_id;
  int stat_id;
  int base_stat;
  int effort;
};

//Stats
//id,damage_class_id,identifier,is_battle_only,game_index
//1,,hp,0,1
class stats
{
  public :

  int id;
  int damage_class_id;
  string identifier;
  int is_battle_only;
  int game_index;
};


//Pokemon Types
//pokemon_id,type_id,slot
//1,12,1
class pokemon_types
{
  public:

  int pokemon_id;
  int type_id;
  int slot;
};


class makePokemon{

  public:

  std::string name;
  int gender;
  int level;
  int hp;
  int hpBase;
  int hpDam;
  int attack;
  int attackBase;
  int defense;
  int defenseBase;
  int speed;
  int speedBase;
  int specialDefense;
  int specialDefenseBase;
  int specialAttack;
  int specialAttackBase;
  int accuracy;
  int evasion;
  int experience;
  int level_up_exp;
  int IV[6];
  moves m[2];
  bool isShiny;
  bool kO;
  bool isRunning;
  bool isCaptured;
  int type_id;
};

makePokemon *createPokemon();
int new_map(int teleport);

#endif
