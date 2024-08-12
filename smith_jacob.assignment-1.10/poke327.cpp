#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <windows.h>
#include <cstdlib>

#include "heap.h"
#include "poke327.h"
#include "character.h"
#include "io.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

typedef struct queue_node
{
  int x, y;
  struct queue_node *next;
} queue_node_t;

class dataBase
{
public:
  vector<pokemon> pokemonList;
  vector<moves> movesList;
  vector<pokemon_moves> pmList;
  vector<pokemon_species> psList;
  vector<experience> exList;
  vector<type_names> tnList;
  vector<pokemon_stats> pstList;
  vector<stats> sList;
  vector<pokemon_types> ptList;
};

dataBase d;

world_t world;

pair_t all_dirs[8] = {
    {-1, -1},
    {-1, 0},
    {-1, 1},
    {0, -1},
    {0, 1},
    {1, -1},
    {1, 0},
    {1, 1},
};

static int32_t path_cmp(const void *key, const void *with)
{
  return ((path_t *)key)->cost - ((path_t *)with)->cost;
}

static int32_t edge_penalty(int8_t x, int8_t y)
{
  return (x == 1 || y == 1 || x == MAP_X - 2 || y == MAP_Y - 2) ? 2 : 1;
}

static void dijkstra_path(map_t *m, pair_t from, pair_t to)
{
  static path_t path[MAP_Y][MAP_X], *p;
  static uint32_t initialized = 0;
  heap_t h;
  uint16_t x, y;

  if (!initialized)
  {
    for (y = 0; y < MAP_Y; y++)
    {
      for (x = 0; x < MAP_X; x++)
      {
        path[y][x].pos[dim_y] = y;
        path[y][x].pos[dim_x] = x;
      }
    }
    initialized = 1;
  }

  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      path[y][x].cost = INT_MAX;
    }
  }

  path[from[dim_y]][from[dim_x]].cost = 0;

  heap_init(&h, path_cmp, NULL);

  for (y = 1; y < MAP_Y - 1; y++)
  {
    for (x = 1; x < MAP_X - 1; x++)
    {
      path[y][x].hn = heap_insert(&h, &path[y][x]);
    }
  }

  while ((p = (path_t *)heap_remove_min(&h)))
  {
    p->hn = NULL;

    if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x])
    {
      for (x = to[dim_x], y = to[dim_y];
           (x != from[dim_x]) || (y != from[dim_y]);
           p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y])
      {
        mapxy(x, y) = ter_path;
        heightxy(x, y) = 0;
      }
      heap_delete(&h);
      return;
    }

    if ((path[p->pos[dim_y] - 1][p->pos[dim_x]].hn) &&
        (path[p->pos[dim_y] - 1][p->pos[dim_x]].cost >
         ((p->cost + heightpair(p->pos)) *
          edge_penalty(p->pos[dim_x], p->pos[dim_y] - 1))))
    {
      path[p->pos[dim_y] - 1][p->pos[dim_x]].cost =
          ((p->cost + heightpair(p->pos)) *
           edge_penalty(p->pos[dim_x], p->pos[dim_y] - 1));
      path[p->pos[dim_y] - 1][p->pos[dim_x]].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] - 1][p->pos[dim_x]].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1]
                                           [p->pos[dim_x]]
                                               .hn);
    }
    if ((path[p->pos[dim_y]][p->pos[dim_x] - 1].hn) &&
        (path[p->pos[dim_y]][p->pos[dim_x] - 1].cost >
         ((p->cost + heightpair(p->pos)) *
          edge_penalty(p->pos[dim_x] - 1, p->pos[dim_y]))))
    {
      path[p->pos[dim_y]][p->pos[dim_x] - 1].cost =
          ((p->cost + heightpair(p->pos)) *
           edge_penalty(p->pos[dim_x] - 1, p->pos[dim_y]));
      path[p->pos[dim_y]][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]]
                                           [p->pos[dim_x] - 1]
                                               .hn);
    }
    if ((path[p->pos[dim_y]][p->pos[dim_x] + 1].hn) &&
        (path[p->pos[dim_y]][p->pos[dim_x] + 1].cost >
         ((p->cost + heightpair(p->pos)) *
          edge_penalty(p->pos[dim_x] + 1, p->pos[dim_y]))))
    {
      path[p->pos[dim_y]][p->pos[dim_x] + 1].cost =
          ((p->cost + heightpair(p->pos)) *
           edge_penalty(p->pos[dim_x] + 1, p->pos[dim_y]));
      path[p->pos[dim_y]][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]]
                                           [p->pos[dim_x] + 1]
                                               .hn);
    }
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x]].hn) &&
        (path[p->pos[dim_y] + 1][p->pos[dim_x]].cost >
         ((p->cost + heightpair(p->pos)) *
          edge_penalty(p->pos[dim_x], p->pos[dim_y] + 1))))
    {
      path[p->pos[dim_y] + 1][p->pos[dim_x]].cost =
          ((p->cost + heightpair(p->pos)) *
           edge_penalty(p->pos[dim_x], p->pos[dim_y] + 1));
      path[p->pos[dim_y] + 1][p->pos[dim_x]].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] + 1][p->pos[dim_x]].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1]
                                           [p->pos[dim_x]]
                                               .hn);
    }
  }
}

static int build_paths(map_t *m)
{
  pair_t from, to;

  /*  printf("%d %d %d %d\n", m->n, m->s, m->e, m->w);*/

  if (m->e != -1 && m->w != -1)
  {
    from[dim_x] = 1;
    to[dim_x] = MAP_X - 2;
    from[dim_y] = m->w;
    to[dim_y] = m->e;

    dijkstra_path(m, from, to);
  }

  if (m->n != -1 && m->s != -1)
  {
    from[dim_y] = 1;
    to[dim_y] = MAP_Y - 2;
    from[dim_x] = m->n;
    to[dim_x] = m->s;

    dijkstra_path(m, from, to);
  }

  if (m->e == -1)
  {
    if (m->s == -1)
    {
      from[dim_x] = 1;
      from[dim_y] = m->w;
      to[dim_x] = m->n;
      to[dim_y] = 1;
    }
    else
    {
      from[dim_x] = 1;
      from[dim_y] = m->w;
      to[dim_x] = m->s;
      to[dim_y] = MAP_Y - 2;
    }

    dijkstra_path(m, from, to);
  }

  if (m->w == -1)
  {
    if (m->s == -1)
    {
      from[dim_x] = MAP_X - 2;
      from[dim_y] = m->e;
      to[dim_x] = m->n;
      to[dim_y] = 1;
    }
    else
    {
      from[dim_x] = MAP_X - 2;
      from[dim_y] = m->e;
      to[dim_x] = m->s;
      to[dim_y] = MAP_Y - 2;
    }

    dijkstra_path(m, from, to);
  }

  if (m->n == -1)
  {
    if (m->e == -1)
    {
      from[dim_x] = 1;
      from[dim_y] = m->w;
      to[dim_x] = m->s;
      to[dim_y] = MAP_Y - 2;
    }
    else
    {
      from[dim_x] = MAP_X - 2;
      from[dim_y] = m->e;
      to[dim_x] = m->s;
      to[dim_y] = MAP_Y - 2;
    }

    dijkstra_path(m, from, to);
  }

  if (m->s == -1)
  {
    if (m->e == -1)
    {
      from[dim_x] = 1;
      from[dim_y] = m->w;
      to[dim_x] = m->n;
      to[dim_y] = 1;
    }
    else
    {
      from[dim_x] = MAP_X - 2;
      from[dim_y] = m->e;
      to[dim_x] = m->n;
      to[dim_y] = 1;
    }

    dijkstra_path(m, from, to);
  }

  return 0;
}

static int gaussian[5][5] = {
    {1, 4, 7, 4, 1},
    {4, 16, 26, 16, 4},
    {7, 26, 41, 26, 7},
    {4, 16, 26, 16, 4},
    {1, 4, 7, 4, 1}};

static int smooth_height(map_t *m)
{
  int32_t i, x, y;
  int32_t s, t, p, q;
  queue_node_t *head, *tail, *tmp;
  /*  FILE *out;*/
  uint8_t height[MAP_Y][MAP_X];

  memset(&height, 0, sizeof(height));

  /* Seed with some values */
  for (i = 1; i < 255; i += 20)
  {
    do
    {
      x = rand() % MAP_X;
      y = rand() % MAP_Y;
    } while (height[y][x]);
    height[y][x] = i;
    if (i == 1)
    {
      head = tail = (queue_node_t *)malloc(sizeof(*tail));
    }
    else
    {
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
    }
    tail->next = NULL;
    tail->x = x;
    tail->y = y;
  }

  /*
  out = fopen("seeded.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", MAP_X, MAP_Y);
  fwrite(&height, sizeof (height), 1, out);
  fclose(out);
  */

  /* Diffuse the vaules to fill the space */
  while (head)
  {
    x = head->x;
    y = head->y;
    i = height[y][x];

    if (x - 1 >= 0 && y - 1 >= 0 && !height[y - 1][x - 1])
    {
      height[y - 1][x - 1] = i;
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y - 1;
    }
    if (x - 1 >= 0 && !height[y][x - 1])
    {
      height[y][x - 1] = i;
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y;
    }
    if (x - 1 >= 0 && y + 1 < MAP_Y && !height[y + 1][x - 1])
    {
      height[y + 1][x - 1] = i;
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y + 1;
    }
    if (y - 1 >= 0 && !height[y - 1][x])
    {
      height[y - 1][x] = i;
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x;
      tail->y = y - 1;
    }
    if (y + 1 < MAP_Y && !height[y + 1][x])
    {
      height[y + 1][x] = i;
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x;
      tail->y = y + 1;
    }
    if (x + 1 < MAP_X && y - 1 >= 0 && !height[y - 1][x + 1])
    {
      height[y - 1][x + 1] = i;
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y - 1;
    }
    if (x + 1 < MAP_X && !height[y][x + 1])
    {
      height[y][x + 1] = i;
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y;
    }
    if (x + 1 < MAP_X && y + 1 < MAP_Y && !height[y + 1][x + 1])
    {
      height[y + 1][x + 1] = i;
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y + 1;
    }

    tmp = head;
    head = head->next;
    free(tmp);
  }

  /* And smooth it a bit with a gaussian convolution */
  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      for (s = t = p = 0; p < 5; p++)
      {
        for (q = 0; q < 5; q++)
        {
          if (y + (p - 2) >= 0 && y + (p - 2) < MAP_Y &&
              x + (q - 2) >= 0 && x + (q - 2) < MAP_X)
          {
            s += gaussian[p][q];
            t += height[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
          }
        }
      }
      m->height[y][x] = t / s;
    }
  }
  /* Let's do it again, until it's smooth like Kenny G. */
  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      for (s = t = p = 0; p < 5; p++)
      {
        for (q = 0; q < 5; q++)
        {
          if (y + (p - 2) >= 0 && y + (p - 2) < MAP_Y &&
              x + (q - 2) >= 0 && x + (q - 2) < MAP_X)
          {
            s += gaussian[p][q];
            t += height[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
          }
        }
      }
      m->height[y][x] = t / s;
    }
  }

  /*
  out = fopen("diffused.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", MAP_X, MAP_Y);
  fwrite(&height, sizeof (height), 1, out);
  fclose(out);

  out = fopen("smoothed.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", MAP_X, MAP_Y);
  fwrite(&m->height, sizeof (m->height), 1, out);
  fclose(out);
  */

  return 0;
}

static void find_building_location(map_t *m, pair_t p)
{
  do
  {
    p[dim_x] = rand() % (MAP_X - 3) + 1;
    p[dim_y] = rand() % (MAP_Y - 3) + 1;

    if ((((mapxy(p[dim_x] - 1, p[dim_y]) == ter_path) &&
          (mapxy(p[dim_x] - 1, p[dim_y] + 1) == ter_path)) ||
         ((mapxy(p[dim_x] + 2, p[dim_y]) == ter_path) &&
          (mapxy(p[dim_x] + 2, p[dim_y] + 1) == ter_path)) ||
         ((mapxy(p[dim_x], p[dim_y] - 1) == ter_path) &&
          (mapxy(p[dim_x] + 1, p[dim_y] - 1) == ter_path)) ||
         ((mapxy(p[dim_x], p[dim_y] + 2) == ter_path) &&
          (mapxy(p[dim_x] + 1, p[dim_y] + 2) == ter_path))) &&
        (((mapxy(p[dim_x], p[dim_y]) != ter_mart) &&
          (mapxy(p[dim_x], p[dim_y]) != ter_center) &&
          (mapxy(p[dim_x] + 1, p[dim_y]) != ter_mart) &&
          (mapxy(p[dim_x] + 1, p[dim_y]) != ter_center) &&
          (mapxy(p[dim_x], p[dim_y] + 1) != ter_mart) &&
          (mapxy(p[dim_x], p[dim_y] + 1) != ter_center) &&
          (mapxy(p[dim_x] + 1, p[dim_y] + 1) != ter_mart) &&
          (mapxy(p[dim_x] + 1, p[dim_y] + 1) != ter_center))) &&
        (((mapxy(p[dim_x], p[dim_y]) != ter_path) &&
          (mapxy(p[dim_x] + 1, p[dim_y]) != ter_path) &&
          (mapxy(p[dim_x], p[dim_y] + 1) != ter_path) &&
          (mapxy(p[dim_x] + 1, p[dim_y] + 1) != ter_path))))
    {
      break;
    }
  } while (1);
}

static int place_pokemart(map_t *m)
{
  pair_t p;

  find_building_location(m, p);

  mapxy(p[dim_x], p[dim_y]) = ter_mart;
  mapxy(p[dim_x] + 1, p[dim_y]) = ter_mart;
  mapxy(p[dim_x], p[dim_y] + 1) = ter_mart;
  mapxy(p[dim_x] + 1, p[dim_y] + 1) = ter_mart;

  return 0;
}

static int place_center(map_t *m)
{
  pair_t p;

  find_building_location(m, p);

  mapxy(p[dim_x], p[dim_y]) = ter_center;
  mapxy(p[dim_x] + 1, p[dim_y]) = ter_center;
  mapxy(p[dim_x], p[dim_y] + 1) = ter_center;
  mapxy(p[dim_x] + 1, p[dim_y] + 1) = ter_center;

  return 0;
}

/* Chooses tree or boulder for border cell.  Choice is biased by dominance *
 * of neighboring cells.                                                   */
static terrain_type_t border_type(map_t *m, int32_t x, int32_t y)
{
  int32_t p, q;
  int32_t r, t;
  int32_t miny, minx, maxy, maxx;

  r = t = 0;

  miny = y - 1 >= 0 ? y - 1 : 0;
  maxy = y + 1 <= MAP_Y ? y + 1 : MAP_Y;
  minx = x - 1 >= 0 ? x - 1 : 0;
  maxx = x + 1 <= MAP_X ? x + 1 : MAP_X;

  for (q = miny; q < maxy; q++)
  {
    for (p = minx; p < maxx; p++)
    {
      if (q != y || p != x)
      {
        if (m->map[q][p] == ter_mountain ||
            m->map[q][p] == ter_boulder)
        {
          r++;
        }
        else if (m->map[q][p] == ter_forest ||
                 m->map[q][p] == ter_tree)
        {
          t++;
        }
      }
    }
  }

  if (t == r)
  {
    return rand() & 1 ? ter_boulder : ter_tree;
  }
  else if (t > r)
  {
    if (rand() % 10)
    {
      return ter_tree;
    }
    else
    {
      return ter_boulder;
    }
  }
  else
  {
    if (rand() % 10)
    {
      return ter_boulder;
    }
    else
    {
      return ter_tree;
    }
  }
}

static int map_terrain(map_t *m, int8_t n, int8_t s, int8_t e, int8_t w)
{
  int32_t i, x, y;
  queue_node_t *head, *tail, *tmp;
  //  FILE *out;
  int num_grass, num_clearing, num_mountain, num_forest, num_water, num_total;
  terrain_type_t type;
  int added_current = 0;

  num_grass = rand() % 4 + 2;
  num_clearing = rand() % 4 + 2;
  num_mountain = rand() % 2 + 1;
  num_forest = rand() % 2 + 1;
  num_water = rand() % 2 + 1;
  num_total = num_grass + num_clearing + num_mountain + num_forest + num_water;

  memset(&m->map, 0, sizeof(m->map));

  /* Seed with some values */
  for (i = 0; i < num_total; i++)
  {
    do
    {
      x = rand() % MAP_X;
      y = rand() % MAP_Y;
    } while (m->map[y][x]);
    if (i == 0)
    {
      type = ter_grass;
    }
    else if (i == num_grass)
    {
      type = ter_clearing;
    }
    else if (i == num_grass + num_clearing)
    {
      type = ter_mountain;
    }
    else if (i == num_grass + num_clearing + num_mountain)
    {
      type = ter_forest;
    }
    else if (i == num_grass + num_clearing + num_mountain + num_forest)
    {
      type = ter_water;
    }
    m->map[y][x] = type;
    if (i == 0)
    {
      head = tail = (queue_node_t *)malloc(sizeof(*tail));
    }
    else
    {
      tail->next = (queue_node_t *)malloc(sizeof(*tail));
      tail = tail->next;
    }
    tail->next = NULL;
    tail->x = x;
    tail->y = y;
  }

  /*
  out = fopen("seeded.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", MAP_X, MAP_Y);
  fwrite(&m->map, sizeof (m->map), 1, out);
  fclose(out);
  */

  /* Diffuse the vaules to fill the space */
  while (head)
  {
    x = head->x;
    y = head->y;
    type = m->map[y][x];

    if (x - 1 >= 0 && !m->map[y][x - 1])
    {
      if ((rand() % 100) < 80)
      {
        m->map[y][x - 1] = type;
        tail->next = (queue_node_t *)malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x - 1;
        tail->y = y;
      }
      else if (!added_current)
      {
        added_current = 1;
        m->map[y][x] = type;
        tail->next = (queue_node_t *)malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
      }
    }

    if (y - 1 >= 0 && !m->map[y - 1][x])
    {
      if ((rand() % 100) < 20)
      {
        m->map[y - 1][x] = type;
        tail->next = (queue_node_t *)malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y - 1;
      }
      else if (!added_current)
      {
        added_current = 1;
        m->map[y][x] = type;
        tail->next = (queue_node_t *)malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
      }
    }

    if (y + 1 < MAP_Y && !m->map[y + 1][x])
    {
      if ((rand() % 100) < 20)
      {
        m->map[y + 1][x] = type;
        tail->next = (queue_node_t *)malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y + 1;
      }
      else if (!added_current)
      {
        added_current = 1;
        m->map[y][x] = type;
        tail->next = (queue_node_t *)malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
      }
    }

    if (x + 1 < MAP_X && !m->map[y][x + 1])
    {
      if ((rand() % 100) < 80)
      {
        m->map[y][x + 1] = type;
        tail->next = (queue_node_t *)malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x + 1;
        tail->y = y;
      }
      else if (!added_current)
      {
        added_current = 1;
        m->map[y][x] = type;
        tail->next = (queue_node_t *)malloc(sizeof(*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
      }
    }

    added_current = 0;
    tmp = head;
    head = head->next;
    free(tmp);
  }

  /*
  out = fopen("diffused.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", MAP_X, MAP_Y);
  fwrite(&m->map, sizeof (m->map), 1, out);
  fclose(out);
  */

  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      if (y == 0 || y == MAP_Y - 1 ||
          x == 0 || x == MAP_X - 1)
      {
        mapxy(x, y) = border_type(m, x, y);
      }
    }
  }

  m->n = n;
  m->s = s;
  m->e = e;
  m->w = w;

  if (n != -1)
  {
    mapxy(n, 0) = ter_gate;
    mapxy(n, 1) = ter_path;
  }
  if (s != -1)
  {
    mapxy(s, MAP_Y - 1) = ter_gate;
    mapxy(s, MAP_Y - 2) = ter_path;
  }
  if (w != -1)
  {
    mapxy(0, w) = ter_gate;
    mapxy(1, w) = ter_path;
  }
  if (e != -1)
  {
    mapxy(MAP_X - 1, e) = ter_gate;
    mapxy(MAP_X - 2, e) = ter_path;
  }

  return 0;
}

static int place_boulders(map_t *m)
{
  int i;
  int x, y;

  for (i = 0; i < MIN_BOULDERS || rand() % 100 < BOULDER_PROB; i++)
  {
    y = rand() % (MAP_Y - 2) + 1;
    x = rand() % (MAP_X - 2) + 1;
    if (m->map[y][x] != ter_forest && m->map[y][x] != ter_path)
    {
      m->map[y][x] = ter_boulder;
    }
  }

  return 0;
}

static int place_trees(map_t *m)
{
  int i;
  int x, y;

  for (i = 0; i < MIN_TREES || rand() % 100 < TREE_PROB; i++)
  {
    y = rand() % (MAP_Y - 2) + 1;
    x = rand() % (MAP_X - 2) + 1;
    if (m->map[y][x] != ter_mountain &&
        m->map[y][x] != ter_path &&
        m->map[y][x] != ter_water)
    {
      m->map[y][x] = ter_tree;
    }
  }

  return 0;
}

void rand_pos(pair_t pos)
{
  pos[dim_x] = (rand() % (MAP_X - 2)) + 1;
  pos[dim_y] = (rand() % (MAP_Y - 2)) + 1;
}

void new_hiker()
{
  pair_t pos;
  npc *c;
  int num_poke;

  do
  {
    rand_pos(pos);
  } while (world.hiker_dist[pos[dim_y]][pos[dim_x]] == INT_MAX ||
           world.cur_map->cmap[pos[dim_y]][pos[dim_x]] ||
           pos[dim_x] < 3 || pos[dim_x] > MAP_X - 4 ||
           pos[dim_y] < 3 || pos[dim_y] > MAP_Y - 4);

  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c = new npc;
  c->pos[dim_y] = pos[dim_y];
  c->pos[dim_x] = pos[dim_x];
  c->ctype = char_hiker;
  c->mtype = move_hiker;
  c->dir[dim_x] = 0;
  c->dir[dim_y] = 0;
  c->defeated = 0;
  c->symbol = 'h';
  c->next_turn = 0;
  c->seq_num = world.char_seq_num++;
  heap_insert(&world.cur_map->turn, c);
  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c;

  num_poke = 0;
  for (int i = 0; i < 6; i++)
  {
    int addPokemon = rand() % 100;
    c->pokemonArray[i] = createPokemon();
    num_poke++;
    if (addPokemon > 60)
    {
      break;
      c->num_of_pokemon = num_poke;
    }
  }
  c->num_of_pokemon = num_poke;

  int pots = rand() % 2;
  int rev = rand() % 2;
  c->basePotions = pots;
  c->baseRevives = rev;
  c->numPotions = pots;
  c->numRevives = rev;

  //  printf("Hiker at %d,%d\n", pos[dim_x], pos[dim_y]);
}

void new_rival()
{
  pair_t pos;
  npc *c;
  int num_poke;

  do
  {
    rand_pos(pos);
  } while (world.rival_dist[pos[dim_y]][pos[dim_x]] == INT_MAX ||
           world.rival_dist[pos[dim_y]][pos[dim_x]] < 0 ||
           world.cur_map->cmap[pos[dim_y]][pos[dim_x]] ||
           pos[dim_x] < 3 || pos[dim_x] > MAP_X - 4 ||
           pos[dim_y] < 3 || pos[dim_y] > MAP_Y - 4);

  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c = new npc;
  c->pos[dim_y] = pos[dim_y];
  c->pos[dim_x] = pos[dim_x];
  c->ctype = char_rival;
  c->mtype = move_rival;
  c->dir[dim_x] = 0;
  c->dir[dim_y] = 0;
  c->defeated = 0;
  c->symbol = 'r';
  c->next_turn = 0;
  c->seq_num = world.char_seq_num++;
  heap_insert(&world.cur_map->turn, c);
  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c;

  num_poke = 0;
  for (int i = 0; i < 6; i++)
  {
    int addPokemon = rand() % 100;
    c->pokemonArray[i] = createPokemon();
    num_poke++;
    if (addPokemon > 60)
    {
      break;
      c->num_of_pokemon = num_poke;
    }
  }
  c->num_of_pokemon = num_poke;

  int pots = rand() % 2;
  int rev = rand() % 2;
  c->basePotions = pots;
  c->baseRevives = rev;
  c->numPotions = pots;
  c->numRevives = rev;
}

void new_swimmer()
{
  pair_t pos;
  npc *c;
  int num_poke;

  do
  {
    rand_pos(pos);
  } while (world.cur_map->map[pos[dim_y]][pos[dim_x]] != ter_water ||
           world.cur_map->cmap[pos[dim_y]][pos[dim_x]]);

  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c = new npc;
  c->pos[dim_y] = pos[dim_y];
  c->pos[dim_x] = pos[dim_x];
  c->ctype = char_swimmer;
  c->mtype = move_swim;
  c->dir[dim_x] = 0;
  c->dir[dim_y] = 0;
  c->defeated = 0;
  c->symbol = SWIMMER_SYMBOL;
  c->next_turn = 0;
  c->seq_num = world.char_seq_num++;
  heap_insert(&world.cur_map->turn, c);
  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c;

  num_poke = 0;
  for (int i = 0; i < 6; i++)
  {
    int addPokemon = rand() % 100;
    c->pokemonArray[i] = createPokemon();
    num_poke++;
    if (addPokemon > 60)
    {
      break;
      c->num_of_pokemon = num_poke;
    }
  }
  c->num_of_pokemon = num_poke;

  int pots = rand() % 2;
  int rev = rand() % 2;
  c->basePotions = pots;
  c->baseRevives = rev;
  c->numPotions = pots;
  c->numRevives = rev;
}

void new_char_other()
{
  pair_t pos;
  npc *c;
  int num_poke;

  do
  {
    rand_pos(pos);
  } while (world.rival_dist[pos[dim_y]][pos[dim_x]] == INT_MAX ||
           world.rival_dist[pos[dim_y]][pos[dim_x]] < 0 ||
           world.cur_map->cmap[pos[dim_y]][pos[dim_x]] ||
           pos[dim_x] < 3 || pos[dim_x] > MAP_X - 4 ||
           pos[dim_y] < 3 || pos[dim_y] > MAP_Y - 4);

  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c = new npc;
  c->pos[dim_y] = pos[dim_y];
  c->pos[dim_x] = pos[dim_x];
  c->ctype = char_other;
  switch (rand() % 4)
  {
  case 0:
    c->mtype = move_pace;
    c->symbol = PACER_SYMBOL;
    break;
  case 1:
    c->mtype = move_wander;
    c->symbol = WANDERER_SYMBOL;
    break;
  case 2:
    c->mtype = move_sentry;
    c->symbol = SENTRY_SYMBOL;
    break;
  case 3:
    c->mtype = move_explore;
    c->symbol = EXPLORER_SYMBOL;
    break;
  }
  rand_dir(c->dir);
  c->defeated = 0;
  c->next_turn = 0;
  c->seq_num = world.char_seq_num++;
  heap_insert(&world.cur_map->turn, c);
  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c;

  num_poke = 0;
  for (int i = 0; i < 6; i++)
  {
    int addPokemon = rand() % 100;
    c->pokemonArray[i] = createPokemon();
    num_poke++;
    if (addPokemon > 60)
    {
      break;
      c->num_of_pokemon = num_poke;
    }
  }
  c->num_of_pokemon = num_poke;

  int pots = rand() % 2;
  int rev = rand() % 2;
  c->basePotions = pots;
  c->baseRevives = rev;
  c->numPotions = pots;
  c->numRevives = rev;
}

void place_characters()
{
  world.cur_map->num_trainers = 3;

  // Always place a hiker and a rival, then place a random number of others
  new_hiker();
  new_rival();
  new_swimmer();
  do
  {
    // higher probability of non- hikers and rivals
    switch (rand() % 10)
    {
    case 0:
      new_hiker();
      break;
    case 1:
      new_rival();
      break;
    case 2:
      new_swimmer();
      break;
    default:
      new_char_other();
      break;
    }
    /* Game attempts to continue to place trainers until the probability *
     * roll fails, but if the map is full (or almost full), it's         *
     * impossible (or very difficult) to continue to add, so we abort if *
     * we've tried MAX_TRAINER_TRIES times.                              */
  } while (++world.cur_map->num_trainers < MIN_TRAINERS ||
           ((rand() % 100) < ADD_TRAINER_PROB));
}

void init_pc()
{
  int x, y;

  do
  {
    x = rand() % (MAP_X - 2) + 1;
    y = rand() % (MAP_Y - 2) + 1;
  } while (world.cur_map->map[y][x] != ter_path);

  world.pc.pos[dim_x] = x;
  world.pc.pos[dim_y] = y;
  world.pc.symbol = '@';
  world.pc.isRunning = false;
  world.pc.totalDefeat = false;

  world.cur_map->cmap[y][x] = &world.pc;
  world.pc.next_turn = 0;
  world.pc.num_of_pokemon = 0;

  heap_insert(&world.cur_map->turn, &world.pc);
}

void place_pc()
{
  character *c;

  if (world.pc.pos[dim_x] == 1)
  {
    world.pc.pos[dim_x] = MAP_X - 2;
  }
  else if (world.pc.pos[dim_x] == MAP_X - 2)
  {
    world.pc.pos[dim_x] = 1;
  }
  else if (world.pc.pos[dim_y] == 1)
  {
    world.pc.pos[dim_y] = MAP_Y - 2;
  }
  else if (world.pc.pos[dim_y] == MAP_Y - 2)
  {
    world.pc.pos[dim_y] = 1;
  }

  world.cur_map->cmap[world.pc.pos[dim_y]][world.pc.pos[dim_x]] = &world.pc;

  if ((c = (character *)heap_peek_min(&world.cur_map->turn)))
  {
    world.pc.next_turn = c->next_turn;
  }
  else
  {
    world.pc.next_turn = 0;
  }
}

// New map expects cur_idx to refer to the index to be generated. If that
// map has already been generated then the only thing this does is set
// cur_map.
int new_map(int teleport)
{
    int d, p;
    int e, w, n, s;
    int x, y;

    if (world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x]])
    {
      world.cur_map = world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x]];
      place_pc();

      return 0;
    }

    world.cur_map = world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x]] = (map_t *)malloc(sizeof(*world.cur_map));

    smooth_height(world.cur_map);

    if (!world.cur_idx[dim_y])
    {
      n = -1;
    }
    else if (world.world[world.cur_idx[dim_y] - 1][world.cur_idx[dim_x]])
    {
      n = world.world[world.cur_idx[dim_y] - 1][world.cur_idx[dim_x]]->s;
    }
    else
    {
      n = 3 + rand() % (MAP_X - 6);
    }
    if (world.cur_idx[dim_y] == WORLD_SIZE - 1)
    {
      s = -1;
    }
    else if (world.world[world.cur_idx[dim_y] + 1][world.cur_idx[dim_x]])
    {
      s = world.world[world.cur_idx[dim_y] + 1][world.cur_idx[dim_x]]->n;
    }
    else
    {
      s = 3 + rand() % (MAP_X - 6);
    }
    if (!world.cur_idx[dim_x])
    {
      w = -1;
    }
    else if (world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x] - 1])
    {
      w = world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x] - 1]->e;
    }
    else
    {
      w = 3 + rand() % (MAP_Y - 6);
    }
    if (world.cur_idx[dim_x] == WORLD_SIZE - 1)
    {
      e = -1;
    }
    else if (world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x] + 1])
    {
      e = world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x] + 1]->w;
    }
    else
    {
      e = 3 + rand() % (MAP_Y - 6);
    }

    map_terrain(world.cur_map, n, s, e, w);

    place_boulders(world.cur_map);
    place_trees(world.cur_map);
    build_paths(world.cur_map);
    d = (abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)) +
        abs(world.cur_idx[dim_y] - (WORLD_SIZE / 2)));
    p = d > 200 ? 5 : (50 - ((45 * d) / 200));
    //  printf("d=%d, p=%d\n", d, p);
    if ((rand() % 100) < p || !d)
    {
      place_pokemart(world.cur_map);
    }
    if ((rand() % 100) < p || !d)
    {
      place_center(world.cur_map);
    }

    for (y = 0; y < MAP_Y; y++)
    {
      for (x = 0; x < MAP_X; x++)
      {
        world.cur_map->cmap[y][x] = NULL;
      }
    }

    heap_init(&world.cur_map->turn, cmp_char_turns, delete_character);

    if ((world.cur_idx[dim_x] == WORLD_SIZE / 2) &&
        (world.cur_idx[dim_y] == WORLD_SIZE / 2))
    {
      init_pc();
    }
    else
    {
      place_pc();
    }

    pathfind(world.cur_map);

    if (teleport)
    {
      do
      {
        world.cur_map->cmap[world.pc.pos[dim_y]][world.pc.pos[dim_x]] = NULL;
        world.pc.pos[dim_x] = rand_range(1, MAP_X - 2);
        world.pc.pos[dim_y] = rand_range(1, MAP_Y - 2);
      } while (world.cur_map->cmap[world.pc.pos[dim_y]][world.pc.pos[dim_x]] ||
              (move_cost[char_pc][world.cur_map->map[world.pc.pos[dim_y]]
                                                    [world.pc.pos[dim_x]]] ==
                INT_MAX) ||
              world.rival_dist[world.pc.pos[dim_y]][world.pc.pos[dim_x]] < 0);
      world.cur_map->cmap[world.pc.pos[dim_y]][world.pc.pos[dim_x]] = &world.pc;
      pathfind(world.cur_map);
    }

    place_characters();

    return 0;
}

// The world is global because of its size, so init_world is parameterless
void init_world()
{
  world.quit = 0;
  world.cur_idx[dim_x] = world.cur_idx[dim_y] = WORLD_SIZE / 2;
  world.char_seq_num = 0;
  new_map(0);
}

void delete_world()
{
  int x, y;

  for (y = 0; y < WORLD_SIZE; y++)
  {
    for (x = 0; x < WORLD_SIZE; x++)
    {
      if (world.world[y][x])
      {
        heap_delete(&world.world[y][x]->turn);
        free(world.world[y][x]);
        world.world[y][x] = NULL;
      }
    }
  }
}

void print_hiker_dist()
{
  int x, y;

  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      if (world.hiker_dist[y][x] == INT_MAX)
      {
        printf("   ");
      }
      else
      {
        printf(" %5d", world.hiker_dist[y][x]);
      }
    }
    printf("\n");
  }
}

void print_rival_dist()
{
  int x, y;

  for (y = 0; y < MAP_Y; y++)
  {
    for (x = 0; x < MAP_X; x++)
    {
      if (world.rival_dist[y][x] == INT_MAX || world.rival_dist[y][x] < 0)
      {
        printf("   ");
      }
      else
      {
        printf(" %02d", world.rival_dist[y][x] % 100);
      }
    }
    printf("\n");
  }
}

void leave_map(pair_t d)
{
  if (d[dim_x] == 0)
  {
    world.cur_idx[dim_x]--;
  }
  else if (d[dim_y] == 0)
  {
    world.cur_idx[dim_y]--;
  }
  else if (d[dim_x] == MAP_X - 1)
  {
    world.cur_idx[dim_x]++;
  }
  else
  {
    world.cur_idx[dim_y]++;
  }
  new_map(0);
}

//Refer to title of function, for more info read comments in function.
void game_loop()
{
  character *c;
  npc *n;
  pc *p;
  pair_t d;

  //Essentially, while 'Q' is not pressed, play the game,
  while (!world.quit)
  {
    c = (character *)heap_remove_min(&world.cur_map->turn); // pop from heap to execute turn.
    n = dynamic_cast<npc *>(c);
    p = dynamic_cast<pc *>(c);

    move_func[n ? n->mtype : move_pc](c, d);

    // If player '@' hits the edge of map, transfer to new cell on the world grid.
    world.cur_map->cmap[c->pos[dim_y]][c->pos[dim_x]] = NULL;
    if (p && (d[dim_x] == 0 || d[dim_x] == MAP_X - 1 ||
              d[dim_y] == 0 || d[dim_y] == MAP_Y - 1))
    {
      leave_map(d);
      d[dim_x] = c->pos[dim_x];
      d[dim_y] = c->pos[dim_y];
    }

    //establishes where the player will be on the next map.
    world.cur_map->cmap[d[dim_y]][d[dim_x]] = c;

    
    if (p)
    {
      pathfind(world.cur_map);
    }

    c->next_turn += move_cost[n ? n->ctype : char_pc]
                             [world.cur_map->map[d[dim_y]][d[dim_x]]];

    c->pos[dim_y] = d[dim_y];
    c->pos[dim_x] = d[dim_x];

    heap_insert(&world.cur_map->turn, c);
  }
}

void usage(char *s)
{
  fprintf(stderr, "Usage: %s [-s|--seed <seed>]\n", s);

  exit(1);
}

void openFile(const string &input)
{
  ifstream poke_file;
  // Buffer to store the full path of the executable
  char exePath[MAX_PATH];

  // Get the full path of the executable
  GetModuleFileName(NULL, exePath, MAX_PATH);

  // Convert the C-string to a std::string
  std::string exeDir = exePath;

  // Find the last backslash '\' in the path and erase everything after it
  exeDir = exeDir.substr(0, exeDir.find_last_of("\\/"));

  // Construct the dynamic file path to the Database folder
  std::string filePath = exeDir + "\\pokemonDatabase\\pokedex\\pokedex\\data\\csv\\" + input + ".csv";

  try
  {
    poke_file.open(filePath);
    if (!poke_file.is_open())
    {
      throw runtime_error("File not found or could not be opened.");
    }
  }
  catch (const runtime_error &e)
  {
    cerr << "Error: " << e.what() << endl;
    exit(1);
  }

  vector<vector<string>> content;
  vector<string> row;
  string line;
  string word;

  fstream file(filePath, ios::in);
  if (file.is_open())
  {
    while (getline(file, line))
    {
      row.clear();
      stringstream str(line);

      while (getline(str, word, ','))
      {
        row.push_back(word);
      }
      getline(str, word, ',');
      row.push_back(word);

      content.push_back(row);
    }
  }

  poke_file.close();

  // Use Vector to convert to Class
  if (input == "pokemon")
  {
    pokemon p;
    for (size_t i = 1; i < content.size() - 1; i++)
    {
      p.id = stoi(content[i][0]);
      p.identifier = content[i][1];
      p.species_id = stoi(content[i][2]);
      p.height = stoi(content[i][3]);
      p.weight = stoi(content[i][4]);
      p.base_experience = stoi(content[i][5]);
      p.order = stoi(content[i][6]);
      p.is_default = stoi(content[i][7]);
      d.pokemonList.push_back(p);
    }
    content.clear();
    row.clear();
  }
  else if (input == "moves")
  {
    moves m;

    for (size_t i = 1; i < content.size() - 1; i++)
    {
      m.id = stoi(content[i][0]);
      m.identifier = content[i][1];
      m.generation_id = stoi(content[i][2]);
      m.type_id = stoi(content[i][3]);

      if (content[i][4] == "")
      {
        m.power = INT_MAX;
      }
      else
      {
        m.power = stoi(content[i][4]);
      }

      if (content[i][5] == "")
      {
        m.pp = INT_MAX;
      }
      else
      {
        m.pp = stoi(content[i][5]);
      }

      if (content[i][6] == "")
      {
        m.accuracy = INT_MAX;
      }
      else
      {
        m.accuracy = stoi(content[i][6]);
      }

      m.priority = stoi(content[i][7]);
      m.target_id = stoi(content[i][8]);
      m.damage_class_id = stoi(content[i][9]);
      m.effect_id = stoi(content[i][10]);

      if (content[i][11] == "")
      {
        m.effect_chance = INT_MAX;
      }
      else
      {
        m.effect_chance = stoi(content[i][11]);
      }

      if (content[i][12] == "")
      {
        m.contest_type_id = INT_MAX;
      }
      else
      {
        m.contest_type_id = stoi(content[i][12]);
      }

      if (content[i][13] == "")
      {
        m.contest_effect_id = INT_MAX;
      }
      else
      {
        m.contest_effect_id = stoi(content[i][13]);
      }

      if (content[i][14] == "")
      {
        m.super_contest_effect_id = INT_MAX;
      }
      else
      {
        m.super_contest_effect_id = stoi(content[i][14]);
      }

      d.movesList.push_back(m);
    }
  }

  else if (input == "pokemon_moves")
  {
    pokemon_moves pm;
    size_t i = 1;
    while (i < content.size())
    {
      pm.pokemon_id = stoi(content[i][0]);
      pm.version_group_id = stoi(content[i][1]);
      pm.move_id = stoi(content[i][2]);
      pm.pokemon_move_method_id = stoi(content[i][3]);
      pm.level = stoi(content[i][4]);

      if (content[i][5] == "")
      {
        pm.order = INT_MAX;
      }
      else
      {
        pm.order = stoi(content[i][5]);
      }

      i++;

      d.pmList.push_back(pm);
    }
  }
  else if (input == "pokemon_species")
  {
    pokemon_species ps;
    for (size_t i = 1; i < 10; i++)
    {
      ps.id = stoi(content[i][0]);
      ps.identifier = content[i][1];
      ps.generation_id = stoi(content[i][2]);
      ps.evolves_from_species_id = content[i][3].empty() ? INT_MAX : stoi(content[i][3]);
      ps.evolution_chain_id = stoi(content[i][4]);
      ps.color_id = stoi(content[i][5]);
      ps.shape_id = content[i][6].empty() ? INT_MAX : stoi(content[i][6]);
      ps.habitat_id = content[i][7].empty() ? INT_MAX : stoi(content[i][7]);
      ps.gender_rate = stoi(content[i][8]);
      ps.capture_rate = stoi(content[i][9]);
      ps.base_happiness = stoi(content[i][10]);
      ps.is_baby = stoi(content[i][11]);
      ps.hatch_counter = stoi(content[i][12]);
      ps.has_gender_differences = stoi(content[i][13]);
      ps.growth_rate_id = stoi(content[i][14]);
      ps.forms_switchable = stoi(content[i][15]);
      ps.is_legendary = stoi(content[i][16]);
      ps.is_mythical = stoi(content[i][17]);
      ps.order = stoi(content[i][18]);
      ps.conquest_order = content[i][19].empty() ? INT_MAX : stoi(content[i][19]);
      d.psList.push_back(ps);
    }
  }
  else if (input == "experience")
  {
    experience ex;
    for (size_t i = 1; i < content.size(); i++)
    {
      ex.growth_rate_id = stoi(content[i][0]);
      ex.level = stoi(content[i][1]);
      ex.experience = stoi(content[i][2]);
      d.exList.push_back(ex);
    }
  }
  else if (input == "type_names")
  {
    type_names tn;
    for (size_t i = 1; i < content.size(); i++)
    {
      tn.type_id = stoi(content[i][0]);
      tn.local_language_id = stoi(content[i][1]);
      tn.name = content[i][2];
      d.tnList.push_back(tn);
    }
  }
  else if (input == "pokemon_stats")
  {
    pokemon_stats pst;
    for (size_t i = 1; i < content.size(); i++)
    {
      pst.pokemon_id = stoi(content[i][0]);
      pst.stat_id = stoi(content[i][1]);
      pst.base_stat = stoi(content[i][2]);
      pst.effort = stoi(content[i][3]);
      d.pstList.push_back(pst);
    }
  }
  else if (input == "stats")
  {
    stats s;
    for (size_t i = 1; i < content.size(); i++)
    {
      s.id = stoi(content[i][0]);
      s.damage_class_id = content[i][1].empty() ? INT_MAX : stoi(content[i][1]);
      s.identifier = content[i][2];
      s.is_battle_only = stoi(content[i][3]);
      s.game_index = content[i][4].empty() ? INT_MAX : stoi(content[i][4]);
      d.sList.push_back(s);
    }
  }
  else if (input == "pokemon_types")
  {
    pokemon_types pt;
    for (size_t i = 1; i < content.size(); i++)
    {
      pt.pokemon_id = stoi(content[i][0]);
      pt.type_id = stoi(content[i][1]);
      pt.slot = stoi(content[i][2]);
      d.ptList.push_back(pt);
    }
  }
  else
  {
    cout << "Class Name Error\n";
    exit(1);
  }
}

void levelUpStats(makePokemon p)
{
}

makePokemon *createPokemon()
{
  makePokemon *p = new makePokemon();
  int randPick;
  int randMove1, randMove2;
  int genderPick;
  int IV[6];
  int i, shinyRand;
  int j;
  int manhattan;
  vector<int> moveid;
  vector<moves> moveVec;

  p->kO = false;
  p->isRunning = false;

  randPick = rand() % 1092;
  p->name = d.pokemonList.at(randPick).identifier;

  genderPick = rand() % 2;
  p->gender = genderPick;

  manhattan = abs((world.cur_idx[dim_x]) - ((int)WORLD_SIZE / 2)) + abs((world.cur_idx[dim_y]) - ((int)WORLD_SIZE / 2));

  if (manhattan <= 200)
  {
    p->level = manhattan / 2;
  }
  else
  {
    p->level = (manhattan - 200) / 2;
  }

  if (p->level < 1)
  {
    p->level = 1;
  }
  else if (p->level > 100)
  {
    p->level = 100;
  }

AGAIN:

  for (i = 0; i < 6; i++)
  {
    IV[i] = rand() % 16;
  }

  // STATS
  for (i = 0; i < (int)d.pstList.size(); i++)
  {
    // cout << "if " << d.pstList.at(i).pokemon_id << "== " <<  d.pokemonList.at(randPick - 1).id  << "\n";

    if (d.pstList.at(i).pokemon_id == d.pokemonList.at(randPick).id)
    {

      // hp
      if (d.pstList.at(i).stat_id == 1)
      {
        p->hpBase = d.pstList.at(i).base_stat;
        p->hp = floor(((d.pstList.at(i).base_stat + IV[0]) * 2 * p->level) / 100) + p->level + 10;
        p->hpDam = p->hp;
      }
      // attack
      else if (d.pstList.at(i).stat_id == 2)
      {
        p->attackBase = d.pstList.at(i).base_stat;
        p->attack = floor(((d.pstList.at(i).base_stat + IV[1]) * 2 * p->level) / 100) + 5;
      }
      // defense
      else if (d.pstList.at(i).stat_id == 3)
      {
        p->defenseBase = d.pstList.at(i).base_stat;
        p->defense = floor(((d.pstList.at(i).base_stat + IV[2]) * 2 * p->level) / 100) + 5;
      }
      // special-attack
      else if (d.pstList.at(i).stat_id == 4)
      {
        p->specialAttackBase = d.pstList.at(i).base_stat;
        p->specialAttack = floor(((d.pstList.at(i).base_stat + IV[3]) * 2 * p->level) / 100) + 5;
      }
      // special-defense
      else if (d.pstList.at(i).stat_id == 5)
      {
        p->specialDefenseBase = d.pstList.at(i).base_stat;
        p->specialDefense = floor(((d.pstList.at(i).base_stat + IV[4]) * 2 * p->level) / 100) + 5;
      }
      // speed
      else if (d.pstList.at(i).stat_id == 6)
      {
        p->speedBase = d.pstList.at(i).base_stat;
        p->speed = floor(((d.pstList.at(i).base_stat + IV[5]) * 2 * p->level) / 100) + 5;
      }

      p->accuracy = 100;
      p->evasion = 100;
    }
  }

  // MOVES
  for (i = 0; i < (int)d.pmList.size(); i++)
  {
    if ((d.pmList.at(i).pokemon_id == d.pokemonList.at(randPick).species_id) && (d.pmList.at(i).pokemon_move_method_id == 1) &&
        d.pmList.at(i).level <= p->level)
    {
      moveid.push_back(d.pmList.at(i).move_id);
    }
  }

  if (moveid.empty())
  {
    p->level++;
    goto AGAIN;
  }

  j = 0;
  for (i = 0; i < (int)d.movesList.size(); i++)
  {
    if (d.movesList.at(i).id == moveid.at(j))
    {
      moveVec.push_back(d.movesList.at(i));
      if ((int)moveid.size() - 1 == j)
      {
        break;
      }
      else
      {
        j++;
      }
    }
  }

  if ((int)moveVec.size() == 1)
  {
    p->m[0] = moveVec.at(0);
  }
  else if ((int)moveVec.size() == 2)
  {
    p->m[0] = moveVec.at(0);
    p->m[1] = moveVec.at(1);
  }
  else
  {
    randMove1 = rand() % moveVec.size();
    randMove2 = rand() % moveVec.size();
    while (randMove1 == randMove2)
    {
      randMove2 = rand() % moveVec.size();
    }
    p->m[0] = moveVec.at(randMove1);
    p->m[1] = moveVec.at(randMove2);
  }

  shinyRand = rand() % 8192;
  p->isShiny = (shinyRand < 2000) ? true : false;

  p->experience = 0;
  p->level_up_exp = 100;

  return p;
}

void printPokemon(makePokemon *p)
{
  cout << "Name: " << p->name << "\n";
  cout << "Gender: " << p->gender << "\n";
  cout << "Level: " << p->level << "\n";
  cout << "HP: " << p->hp << "\n";
  cout << p->attack << " ";
  cout << p->defense << " ";
  cout << p->specialAttack << " ";
  cout << p->specialDefense << " ";
  cout << p->speed << " ";
}

int main(int argc, char *argv[])
{
  cout << "Loading Game...." << endl;
  struct timeval tv;
  uint32_t seed;
  int long_arg;
  int do_seed;
  int i;
  string input_pokemon;
  string input_moves;
  string input_pokemon_moves;
  string input_pokemon_species;
  string input_experience;
  string input_type_names;
  string input_pokemon_stats;
  string input_stats;
  string input_pokemon_types;

  do_seed = 1;

  // handling arguments in terminal.
  if (argc > 1)
  {
    for (i = 1, long_arg = 0; i < argc; i++, long_arg = 0)
    {
      if (argv[i][0] == '-')
      { /* All switches start with a dash */
        if (argv[i][1] == '-')
        {
          argv[i]++;    /* Make the argument have a single dash so we can */
          long_arg = 1; /* handle long and short args at the same place.  */
        }
        switch (argv[i][1])
        {
        case 's':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-seed")) ||
              argc < ++i + 1 /* No more arguments */ ||
              !sscanf(argv[i], "%u", &seed) /* Argument is not an integer */)
          {
            usage(argv[0]);
          }
          do_seed = 0;
          break;
        default:
          usage(argv[0]);
        }
      }
      else
      { /* No dash */
        usage(argv[0]);
      }
    }
  }

  if (do_seed)
  {
    /* Allows me to start the game more than once *
     * per second, as opposed to time().          */
    gettimeofday(&tv, NULL);
    seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
  }

  srand(seed);

  // Opening the respective files from the pokemon database.
  input_pokemon = "pokemon";
  openFile(input_pokemon);
  input_moves = "moves";
  openFile(input_moves);
  input_pokemon_moves = "pokemon_moves";
  openFile(input_pokemon_moves);
  input_pokemon_species = "pokemon_species";
  openFile(input_pokemon_species);
  input_experience = "experience";
  openFile(input_experience);
  input_type_names = "type_names";
  openFile(input_type_names);
  input_pokemon_stats = "pokemon_stats";
  openFile(input_pokemon_stats);
  input_stats = "stats";
  openFile(input_stats);
  input_pokemon_types = "pokemon_types";
  openFile(input_pokemon_types);

  // initialize terminal look. For more info, see io.cpp.
  io_init_terminal();

  // initialize world size.
  init_world();

  //
  game_loop();

  delete_world();

  io_reset_terminal();

  return 0;
}
