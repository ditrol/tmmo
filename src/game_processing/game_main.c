#include "game_main.h"
#include "item.h"
#include <unistd.h>

union_buildings_t barracks = {NULL, 0};
union_buildings_t smithies = {NULL, 0};
union_buildings_t markets = {NULL, 0};
union_buildings_t taverns = {NULL, 0};
union_buildings_t academies = {NULL, 0};

characters_t characters;

map_point_t* _map;
size_t _msize_x;
size_t _msize_y;

static void game_tick();


int game_init(char *map_name)
{
    FILE *fmap;

    size_t msize_x, msize_y;
    size_t buildings_cnt;
    building_t *buildings;

    if(map_name == NULL)
    {
        printf("mapfile should be specified\n");
        return 1;
    }

    fmap = fopen(map_name, "r");
    if (fmap == NULL)
    {
        printf("can't open mapfile: %s\n", map_name);
        return 1;
    }

    fread(&buildings_cnt, sizeof(size_t), 1, fmap);
    if(buildings_cnt != 0)
    {
        buildings = malloc( buildings_cnt * sizeof(building_t) );
        fread(buildings, buildings_cnt * sizeof(building_t), 1, fmap);
    }

    fread(&msize_x, sizeof(size_t), 1, fmap);
    msize_y = msize_x;

    map_point_t *map = malloc( msize_y * msize_x * sizeof(map_point_t) );

    fread(map, msize_x * msize_y * sizeof(map_point_t), 1, fmap);

    /*
     * DEBUG INFO
     */
    printf("mapsize x: %zu, y: %zu\n", msize_x, msize_y);
    for(int i = 0; i < buildings_cnt; i++)
    {
        printf("buildname_id: %i, coord: (%i, %i), size_x: %zu, size_y: %zu\n",
               buildings[i].point.name_id, buildings[i].point.x, buildings[i].point.y, buildings[i].size_x, buildings->size_y);
    }

    game_start(map, msize_x, msize_y, buildings, buildings_cnt);
}


void game_start(map_point_t *map, size_t msize_x, size_t msize_y, building_t *buildings, size_t buildings_cnt)
{
    _map = map;
    _msize_x = msize_x;
    _msize_y = msize_y;

    for(int i = 0; i < buildings_cnt; i++)
    {
        switch(buildings[i].point.name_id / BUILDING_TYPE_OFFSET)
        {
            case 0: barracks.buildings = realloc(barracks.buildings, ++barracks.buildings_cnt * sizeof(building_t));
                *(barracks.buildings + barracks.buildings_cnt - 1) = buildings[i];
                break;

            case 1: smithies.buildings = realloc(smithies.buildings, ++smithies.buildings_cnt * sizeof(building_t));
                *(smithies.buildings + smithies.buildings_cnt - 1) = buildings[i];
                break;

            case 2: markets.buildings = realloc(markets.buildings, ++markets.buildings_cnt * sizeof(building_t));
                *(markets.buildings + markets.buildings_cnt - 1) = buildings[i];
                break;

            case 3: taverns.buildings = realloc(taverns.buildings, ++taverns.buildings_cnt * sizeof(building_t));
                *(taverns.buildings + taverns.buildings_cnt - 1) = buildings[i];
                break;

            case 4: academies.buildings = realloc(academies.buildings, ++academies.buildings_cnt * sizeof(building_t));
                *(academies.buildings + academies.buildings_cnt - 1) = buildings[i];
                break;

        }
    }

    characters.count = 0;

    /*
     * DEBUG INFO
     */
    printf("barracks_cnt: %zu\n", barracks.buildings_cnt);
    printf("smithies_cnt: %zu\n", smithies.buildings_cnt);
    printf("markets: %zu\n", markets.buildings_cnt);
    printf("taverns: %zu\n", taverns.buildings_cnt);
    printf("academies: %zu\n", academies.buildings_cnt);


    while(1)
    {
        game_tick();

        usleep(tick_delay);
    }
}


static void game_tick()
{
    for(int i = 0; i < characters.count; i++)
    {

        if ( (characters.arr[i]->next_step == nowhere && !characters.arr[i]->aggression)
             || ( (clock() - characters.arr[i]->step_start_time) <= characters.arr[i]->step_time) )
            continue;

        (_map + characters.arr[i]->position.y * _msize_x + characters.arr[i]->position.x)->child_object_type = nothing;
        (_map + characters.arr[i]->position.y * _msize_x + characters.arr[i]->position.x)->child_object = NULL;

        character_find_target(characters.arr[i], &characters);
        characters.arr[i]->next_step = character_move_to_target(characters.arr[i], _msize_x, _msize_y, _map);
        characters.arr[i]->step_start_time = clock();

        switch (characters.arr[i]->next_step)
        {
            case west:
                characters.arr[i]->position.x++;
                break;

            case east:
                characters.arr[i]->position.x--;
                break;

            case north:
                characters.arr[i]->position.y++;
                break;

            case south:
                characters.arr[i]->position.y--;
                break;

            case nowhere:
                character_attack(characters.arr[i], &characters, _msize_x, _map, &i);
                break;
        }

        (_map + characters.arr[i]->position.y * _msize_x + characters.arr[i]->position.x)->child_object_type = character;
        (_map + characters.arr[i]->position.y * _msize_x + characters.arr[i]->position.x)->child_object = characters.arr[i];


        /*
         * DEBUG INFO
         *
        printf("after character moved \"%s\" pos: %i, %i, target: %p, child_object: %i, next_step: %i\n", characters.arr[i]->name, characters.arr[i]->position.x, characters.arr[i]->position.y,
               characters.arr[i]->target,
               (_map + characters.arr[i]->position.y * _msize_x + characters.arr[i]->position.x)->child_object_type, characters.arr[i]->next_step );

        for(int j = 0; j < characters.arr[i]->items.count; j++)
        {
            printf("%i:%i) %s (%i)\n", j + 1, characters.arr[i]->items.count, characters.arr[i]->items.arr[j]->name, characters.arr[i]->items.arr[j]->type);
        }
            */

    }
}


