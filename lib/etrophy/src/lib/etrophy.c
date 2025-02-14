#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <Ecore.h>
#include <Ecore_File.h>
#include <Eet.h>
#include <Eina.h>

#include "Etrophy.h"

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_etrophy_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_etrophy_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_etrophy_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_etrophy_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_etrophy_log_dom, __VA_ARGS__)

#ifdef  __cplusplus
extern "C" {
#endif

int _etrophy_log_dom = -1;
static int _etrophy_init_count = 0;

struct _Etrophy_Trophy
{
   const char               *name;
   const char               *description;
   Etrophy_Trophy_Visibility visibility;
   unsigned int              counter;
   unsigned int              goal;
   unsigned int              points;
   unsigned int              date;
};

struct _Etrophy_Lock
{
   const char        *name;
   Etrophy_Lock_State state;
   unsigned int       date;
};

struct _Etrophy_Score
{
   const char  *player_name;
   int          score;
   unsigned int date;
};

struct _Etrophy_Level
{
   const char *name;
   Eina_List  *scores;
};

struct _Etrophy_Gamescore
{
   Eina_List    *levels;
   Eina_List    *trophies;
   Eina_List    *locks;
   const char   *__eet_filename;
};

static const char GAMESCORE_ENTRY[] = "gamescore";

static Eet_Data_Descriptor *_trophy_descriptor = NULL;
static Eet_Data_Descriptor *_lock_descriptor = NULL;
static Eet_Data_Descriptor *_score_descriptor = NULL;
static Eet_Data_Descriptor *_level_descriptor = NULL;
static Eet_Data_Descriptor *_gamescore_descriptor = NULL;

static inline void
_trophy_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   if (_trophy_descriptor) return;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Etrophy_Trophy);
   _trophy_descriptor = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_trophy_descriptor, Etrophy_Trophy,
                                 "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_trophy_descriptor, Etrophy_Trophy,
                                 "description", description, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_trophy_descriptor, Etrophy_Trophy,
                                 "visibility", visibility, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_trophy_descriptor, Etrophy_Trophy,
                                 "counter", counter, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_trophy_descriptor, Etrophy_Trophy,
                                 "goal", goal, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_trophy_descriptor, Etrophy_Trophy,
                                 "points", points, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_trophy_descriptor, Etrophy_Trophy,
                                 "date", date, EET_T_UINT);
}

static inline void
_trophy_shutdown(void)
{
   if (!_trophy_descriptor) return;
   eet_data_descriptor_free(_trophy_descriptor);
   _trophy_descriptor = NULL;
}

EAPI Etrophy_Trophy *
etrophy_trophy_new(const char *name, const char *description, Etrophy_Trophy_Visibility visibility, unsigned int goal, unsigned int points)
{
   Etrophy_Trophy *trophy;

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   trophy = calloc(1, sizeof(Etrophy_Trophy));
   if (!trophy)
     {
        ERR("Could not calloc Etrophy_Trophy.");
        return NULL;
     }

   trophy->name = eina_stringshare_add(name);
   trophy->description = eina_stringshare_add(description);
   trophy->visibility = visibility;
   trophy->date = (unsigned int)ecore_time_unix_get();
   trophy->goal = goal;
   trophy->points = points;

   return trophy;
}

EAPI void
etrophy_trophy_free(Etrophy_Trophy *trophy)
{
   if (!trophy) return;
   eina_stringshare_del(trophy->name);
   eina_stringshare_del(trophy->description);
   free(trophy);
}

EAPI inline const char *
etrophy_trophy_name_get(const Etrophy_Trophy *trophy)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(trophy, NULL);
   return trophy->name;
}

EAPI inline const char *
etrophy_trophy_description_get(const Etrophy_Trophy *trophy)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(trophy, NULL);
   return trophy->description;
}

EAPI inline Etrophy_Trophy_Visibility
etrophy_trophy_visibility_get(const Etrophy_Trophy *trophy)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(trophy, 0);
   return trophy->visibility;
}

EAPI inline void
etrophy_trophy_goal_get(const Etrophy_Trophy *trophy, unsigned int *goal,
                        unsigned int *counter)
{
   EINA_SAFETY_ON_NULL_RETURN(trophy);
   if (goal) *goal = trophy->goal;
   if (counter) *counter = trophy->counter;
}

EAPI inline unsigned int
etrophy_trophy_points_get(const Etrophy_Trophy *trophy)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(trophy, 0);
   return trophy->points;
}

EAPI inline void
etrophy_trophy_counter_set(Etrophy_Trophy *trophy, unsigned int value)
{
   EINA_SAFETY_ON_NULL_RETURN(trophy);

   if (trophy->counter == trophy->goal) return;
   if (trophy->counter >= value) return;

   trophy->counter = value;
   if (trophy->counter > trophy->goal)
     trophy->counter = trophy->goal;

   trophy->date = (unsigned int)ecore_time_unix_get();
}

EAPI inline void
etrophy_trophy_counter_increment(Etrophy_Trophy *trophy, unsigned int value)
{
   EINA_SAFETY_ON_NULL_RETURN(trophy);

   if (trophy->counter == trophy->goal) return;

   trophy->counter += value;
   if (trophy->counter > trophy->goal)
     trophy->counter = trophy->goal;

   trophy->date = (unsigned int)ecore_time_unix_get();
}

EAPI inline Eina_Bool
etrophy_trophy_earned_get(const Etrophy_Trophy *trophy)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(trophy, EINA_FALSE);
   return trophy->goal == trophy->counter;
}

EAPI inline unsigned int
etrophy_trophy_date_get(const Etrophy_Trophy *trophy)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(trophy, 0);
   return trophy->date;
}

static inline void
_lock_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   if (_lock_descriptor) return;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Etrophy_Lock);
   _lock_descriptor = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_lock_descriptor, Etrophy_Lock, "name",
                                 name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_lock_descriptor, Etrophy_Lock, "state",
                                 state, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_lock_descriptor, Etrophy_Lock, "date",
                                 date, EET_T_UINT);
}

static inline void
_lock_shutdown(void)
{
   if (!_lock_descriptor) return;
   eet_data_descriptor_free(_lock_descriptor);
   _lock_descriptor = NULL;
}

EAPI Etrophy_Lock *
etrophy_lock_new(const char *name, Etrophy_Lock_State state)
{
   Etrophy_Lock *lock;

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   lock = calloc(1, sizeof(Etrophy_Lock));
   if (!lock)
     {
        ERR("Could not calloc Etrophy_Lock.");
        return NULL;
     }

   lock->name = eina_stringshare_add(name);
   lock->state = state;
   lock->date = (unsigned int)ecore_time_unix_get();

   return lock;
}

EAPI void
etrophy_lock_free(Etrophy_Lock *lock)
{
   if (!lock) return;
   eina_stringshare_del(lock->name);
   free(lock);
}

EAPI inline const char *
etrophy_lock_name_get(const Etrophy_Lock *lock)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(lock, NULL);
   return lock->name;
}

EAPI inline Etrophy_Lock_State
etrophy_lock_state_get(const Etrophy_Lock *lock)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(lock, 0);
   return lock->state;
}

EAPI inline void
etrophy_lock_state_set(Etrophy_Lock *lock, Etrophy_Lock_State state)
{
   EINA_SAFETY_ON_NULL_RETURN(lock);
   if (state >= ETROPHY_LOCK_STATE_LAST_VALUE) return;
   lock->state = state;
   lock->date = (unsigned int)ecore_time_unix_get();
}

EAPI inline unsigned int
etrophy_lock_date_get(const Etrophy_Lock *lock)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(lock, 0);
   return lock->date;
}

static inline void
_score_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   if (_score_descriptor) return;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Etrophy_Score);
   _score_descriptor = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_score_descriptor, Etrophy_Score,
                                 "player_name", player_name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_score_descriptor, Etrophy_Score,
                                 "score", score, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_score_descriptor, Etrophy_Score,
                                 "date", date, EET_T_UINT);
}

static inline void
_score_shutdown(void)
{
   if (!_score_descriptor) return;
   eet_data_descriptor_free(_score_descriptor);
   _score_descriptor = NULL;
}

EAPI Etrophy_Score *
etrophy_score_new(const char *player_name, int score)
{
   Etrophy_Score *escore = calloc(1, sizeof(Etrophy_Score));

   if (!escore)
     {
        ERR("Could not calloc Etrophy_Score.");
        return NULL;
     }

   escore->player_name = eina_stringshare_add(player_name);
   escore->score = score;
   escore->date = (unsigned int)ecore_time_unix_get();

   DBG("Score created. Player: %s, score: %i", player_name, score);

   return escore;
}

EAPI void
etrophy_score_free(Etrophy_Score *escore)
{
   if (!escore) return;
   eina_stringshare_del(escore->player_name);
   free(escore);
}

EAPI inline const char *
etrophy_score_player_name_get(const Etrophy_Score *escore)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(escore, NULL);
   return escore->player_name;
}

EAPI inline int
etrophy_score_score_get(const Etrophy_Score *escore)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(escore, 0);
   return escore->score;
}

EAPI inline unsigned int
etrophy_score_date_get(const Etrophy_Score *escore)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(escore, 0);
   return escore->date;
}

static inline void
_level_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   if (_level_descriptor) return;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Etrophy_Level);
   _level_descriptor = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(_level_descriptor, Etrophy_Level,
                                 "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_LIST(_level_descriptor, Etrophy_Level,
                                "scores", scores, _score_descriptor);
}

static inline void
_level_shutdown(void)
{
   if (!_level_descriptor) return;
   eet_data_descriptor_free(_level_descriptor);
   _level_descriptor = NULL;
}

EAPI Etrophy_Level *
etrophy_level_new(const char *name)
{
   Etrophy_Level *level;

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   level = calloc(1, sizeof(Etrophy_Level));
   if (!level)
     {
        ERR("Could not calloc Etrophy_Level.");
        return NULL;
     }

   level->name = eina_stringshare_add(name);
   DBG("Level created: %s", name);

   return level;
}

EAPI void
etrophy_level_free(Etrophy_Level *level)
{
   if (!level) return;
   eina_stringshare_del(level->name);
   if (level->scores)
     {
        Etrophy_Score *scores_elem;
        EINA_LIST_FREE(level->scores, scores_elem)
          etrophy_score_free(scores_elem);
     }
   free(level);
}

EAPI inline const char *
etrophy_level_name_get(const Etrophy_Level *level)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(level, NULL);
   return level->name;
}

static int
_score_cmp(const void *data1, const void *data2)
{
   const Etrophy_Score *escore1 = data1;
   const Etrophy_Score *escore2 = data2;
   return escore2->score - escore1->score;
}

EAPI inline void
etrophy_level_score_add(Etrophy_Level *level, Etrophy_Score *escore)
{
   EINA_SAFETY_ON_NULL_RETURN(level);
   EINA_SAFETY_ON_NULL_RETURN(escore);
   level->scores = eina_list_sorted_insert(level->scores, _score_cmp, escore);
}

EAPI inline void
etrophy_level_score_del(Etrophy_Level *level, Etrophy_Score *escore)
{
   EINA_SAFETY_ON_NULL_RETURN(level);
   EINA_SAFETY_ON_NULL_RETURN(escore);
   level->scores = eina_list_remove(level->scores, escore);
}

EAPI void
etrophy_level_scores_list_clear(Etrophy_Level *level)
{
   EINA_SAFETY_ON_NULL_RETURN(level);
   Etrophy_Score *data;
   EINA_LIST_FREE(level->scores, data)
     etrophy_score_free(data);
}

EAPI inline const Eina_List *
etrophy_level_scores_list_get(const Etrophy_Level *level)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(level, NULL);
   return level->scores;
}

static inline void
_gamescore_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   if (_gamescore_descriptor) return;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Etrophy_Gamescore);
   _gamescore_descriptor = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_LIST(_gamescore_descriptor, Etrophy_Gamescore,
                                "levels", levels, _level_descriptor);
   EET_DATA_DESCRIPTOR_ADD_LIST(_gamescore_descriptor, Etrophy_Gamescore,
                                "trophies", trophies, _trophy_descriptor);
   EET_DATA_DESCRIPTOR_ADD_LIST(_gamescore_descriptor, Etrophy_Gamescore,
                                "locks", locks, _lock_descriptor);
}

static inline void
_gamescore_shutdown(void)
{
   if (!_gamescore_descriptor) return;
   eet_data_descriptor_free(_gamescore_descriptor);
   _gamescore_descriptor = NULL;
}

EAPI Etrophy_Gamescore *
etrophy_gamescore_new(const char *gamename)
{
   Etrophy_Gamescore *gamescore;
   char filename[1024];
   char tmp[5] = "/tmp";
   char *home;

   EINA_SAFETY_ON_NULL_RETURN_VAL(gamename, NULL);
   gamescore = calloc(1, sizeof(Etrophy_Gamescore));
   EINA_SAFETY_ON_NULL_RETURN_VAL(gamescore, NULL);

   home = getenv("ETROPHY_PATH");
   if (!home)
     {
        home = getenv("HOME");
        if (!home)
          home = tmp;
     }

   if (!ecore_file_is_dir(home))
     {
        ERR("Couldn't find a place to store it. Set $ETROPHY_PATH.");
        return NULL;
     }

   snprintf(filename, sizeof(filename), "%s/.etrophy/", home);

   if (!ecore_file_is_dir(filename))
     {
        if (!ecore_file_mkdir(filename))
          {
             ERR("Failed to create %s.", filename);
             return NULL;
          }
        INF("Directory %s created.", filename);
     }
   else
     INF("Base directory: %s.", filename);

   snprintf(filename, sizeof(filename), "%s/.etrophy/%s.eet", home, gamename);
   gamescore->__eet_filename = eina_stringshare_add(filename);

   return gamescore;
}

EAPI void
etrophy_gamescore_free(Etrophy_Gamescore *gamescore)
{
   if (!gamescore) return;
   {
      Etrophy_Level *levels_elem;
      EINA_LIST_FREE(gamescore->levels, levels_elem)
        etrophy_level_free(levels_elem);
   }
   {
      Etrophy_Trophy *trophies_elem;
      EINA_LIST_FREE(gamescore->trophies, trophies_elem)
        etrophy_trophy_free(trophies_elem);
   }
   {
      Etrophy_Lock *locks_elem;
      EINA_LIST_FREE(gamescore->locks, locks_elem)
        etrophy_lock_free(locks_elem);
   }
   free(gamescore);
}

EAPI inline void
etrophy_gamescore_level_add(Etrophy_Gamescore *gamescore, Etrophy_Level *level)
{
   EINA_SAFETY_ON_NULL_RETURN(gamescore);
   EINA_SAFETY_ON_NULL_RETURN(level);
   gamescore->levels = eina_list_append(gamescore->levels, level);
}

EAPI inline void
etrophy_gamescore_level_del(Etrophy_Gamescore *gamescore, Etrophy_Level *level)
{
   EINA_SAFETY_ON_NULL_RETURN(gamescore);
   EINA_SAFETY_ON_NULL_RETURN(level);
   gamescore->levels = eina_list_remove(gamescore->levels, level);
}

EAPI Etrophy_Level *
etrophy_gamescore_level_get(Etrophy_Gamescore *gamescore, const char *name)
{
   Etrophy_Level *level;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(gamescore, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   EINA_LIST_FOREACH(gamescore->levels, l, level)
     if (!strcmp(name, level->name))
       return level;

   return NULL;
}

EAPI void
etrophy_gamescore_levels_list_clear(Etrophy_Gamescore *gamescore)
{
   EINA_SAFETY_ON_NULL_RETURN(gamescore);
   Etrophy_Level *data;
   EINA_LIST_FREE(gamescore->levels, data)
     etrophy_level_free(data);
}

EAPI inline const Eina_List *
etrophy_gamescore_levels_list_get(const Etrophy_Gamescore *gamescore)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(gamescore, NULL);
   return gamescore->levels;
}

EAPI inline void
etrophy_gamescore_trophy_add(Etrophy_Gamescore *gamescore, Etrophy_Trophy *trophy)
{
   EINA_SAFETY_ON_NULL_RETURN(gamescore);
   EINA_SAFETY_ON_NULL_RETURN(trophy);
   gamescore->trophies = eina_list_append(gamescore->trophies, trophy);
}

EAPI inline void
etrophy_gamescore_trophy_del(Etrophy_Gamescore *gamescore, Etrophy_Trophy *trophy)
{
   EINA_SAFETY_ON_NULL_RETURN(gamescore);
   EINA_SAFETY_ON_NULL_RETURN(trophy);
   gamescore->trophies = eina_list_remove(gamescore->trophies, trophy);
}

EAPI Etrophy_Trophy *
etrophy_gamescore_trophy_get(Etrophy_Gamescore *gamescore, const char *name)
{
   Etrophy_Trophy *trophy;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(gamescore, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   EINA_LIST_FOREACH(gamescore->trophies, l, trophy)
     if (!strcmp(name, trophy->name))
       return trophy;

   return NULL;
}

EAPI void
etrophy_gamescore_trophies_list_clear(Etrophy_Gamescore *gamescore)
{
   EINA_SAFETY_ON_NULL_RETURN(gamescore);
   Etrophy_Trophy *data;
   EINA_LIST_FREE(gamescore->trophies, data)
     etrophy_trophy_free(data);
}

EAPI inline const Eina_List *
etrophy_gamescore_trophies_list_get(const Etrophy_Gamescore *gamescore)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(gamescore, NULL);
   return gamescore->trophies;
}

EAPI inline void
etrophy_gamescore_lock_add(Etrophy_Gamescore *gamescore, Etrophy_Lock *lock)
{
   EINA_SAFETY_ON_NULL_RETURN(gamescore);
   EINA_SAFETY_ON_NULL_RETURN(lock);
   gamescore->locks = eina_list_append(gamescore->locks, lock);
}

EAPI inline void
etrophy_gamescore_lock_del(Etrophy_Gamescore *gamescore, Etrophy_Lock *lock)
{
   EINA_SAFETY_ON_NULL_RETURN(gamescore);
   EINA_SAFETY_ON_NULL_RETURN(lock);
   gamescore->locks = eina_list_remove(gamescore->locks, lock);
}

EAPI Etrophy_Lock *
etrophy_gamescore_lock_get(Etrophy_Gamescore *gamescore, const char *name)
{
   Etrophy_Lock *lock;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(gamescore, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   EINA_LIST_FOREACH(gamescore->locks, l, lock)
     if (!strcmp(name, lock->name))
       return lock;

   return NULL;
}

EAPI void
etrophy_gamescore_locks_list_clear(Etrophy_Gamescore *gamescore)
{
   EINA_SAFETY_ON_NULL_RETURN(gamescore);
   Etrophy_Lock *data;
   EINA_LIST_FREE(gamescore->locks, data)
     etrophy_lock_free(data);
}

EAPI void
etrophy_gamescore_clear(Etrophy_Gamescore *gamescore)
{
   EINA_SAFETY_ON_NULL_RETURN(gamescore);
   etrophy_gamescore_levels_list_clear(gamescore);
   etrophy_gamescore_locks_list_clear(gamescore);
   etrophy_gamescore_trophies_list_clear(gamescore);
}

EAPI inline const Eina_List *
etrophy_gamescore_locks_list_get(const Etrophy_Gamescore *gamescore)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(gamescore, NULL);
   return gamescore->locks;
}

EAPI int
etrophy_gamescore_level_hi_score_get(const Etrophy_Gamescore *gamescore, const char *level_name)
{
   Etrophy_Score *escore;
   Etrophy_Level *level;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(gamescore, -1);

   EINA_LIST_FOREACH(gamescore->levels, l, level)
     {
        if (!strcmp(level->name, level_name))
          break;
     }

   if (!level)
     return -1;

   if (!level->scores)
     return -1;

   escore = eina_list_data_get(level->scores);
   return escore->score;
}

EAPI int
etrophy_gamescore_level_low_score_get(const Etrophy_Gamescore *gamescore, const char *level_name)
{
   Etrophy_Score *escore;
   Etrophy_Level *level;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(gamescore, -1);

   EINA_LIST_FOREACH(gamescore->levels, l, level)
     {
        if (!strcmp(level->name, level_name))
          break;
     }

   if (!level)
     return -1;

   if (!level->scores)
     return -1;

   escore = eina_list_data_get(eina_list_last(level->scores));
   return escore->score;
}

EAPI Etrophy_Score *
etrophy_gamescore_level_score_add(Etrophy_Gamescore *gamescore, const char *level_name, const char *player_name, int score)
{
   Etrophy_Score *escore;
   Etrophy_Level *level;
   Eina_List *l;

   escore = etrophy_score_new(player_name, score);
   if (!escore) return NULL;

   EINA_LIST_FOREACH(gamescore->levels, l, level)
     {
        if (!strcmp(level->name, level_name))
          break;
     }

   if (!level)
     {
        level = etrophy_level_new(level_name);
        if (!level)
          {
             etrophy_score_free(escore);
             return NULL;
          }
        gamescore->levels = eina_list_append(gamescore->levels, level);
     }

   level->scores = eina_list_sorted_insert(level->scores, _score_cmp, escore);

   return escore;
}

EAPI inline unsigned int
etrophy_gamescore_trophies_points_get(const Etrophy_Gamescore *gamescore)
{
   unsigned int points = 0;
   Etrophy_Trophy *trophy;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(gamescore, 0);

   EINA_LIST_FOREACH(gamescore->trophies, l, trophy)
     {
        if (trophy->counter == trophy->goal)
          points += trophy->points;
     }

   return points;
}

EAPI inline unsigned int
etrophy_gamescore_trophies_total_points_get(const Etrophy_Gamescore *gamescore)
{
   unsigned int points = 0;
   Etrophy_Trophy *trophy;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(gamescore, 0);

   EINA_LIST_FOREACH(gamescore->trophies, l, trophy)
      points += trophy->points;

   return points;
}

EAPI Eet_Data_Descriptor *
etrophy_gamescore_edd_get(void)
{
   return _gamescore_descriptor;
}

EAPI Etrophy_Gamescore *
etrophy_gamescore_path_load(const char *filename)
{
   Etrophy_Gamescore *gamescore = NULL;
   Eet_File *ef;

   DBG("Trying to load from %s", filename);

   ef = eet_open(filename, EET_FILE_MODE_READ);
   if (!ef)
     {
        WRN("Could not open '%s' for read.", filename);
        return NULL;
     }

   gamescore = eet_data_read(ef, _gamescore_descriptor, GAMESCORE_ENTRY);
   if (!gamescore) goto end;
   gamescore->__eet_filename = eina_stringshare_add(filename);

end:
   eet_close(ef);

   if (gamescore)
     INF("Gamescore loaded from %s", filename);
   else
     ERR("Gamescore failed to load from %s", filename);

   return gamescore;
}

EAPI Eina_Bool
etrophy_gamescore_save(Etrophy_Gamescore *gamescore, const char *filename)
{
   Eet_File *ef;
   Eina_Bool ret;

   DBG("Trying to save %p to %s", gamescore, filename);

   if (filename)
     eina_stringshare_replace(&(gamescore->__eet_filename), filename);
   else if (gamescore->__eet_filename)
     filename = gamescore->__eet_filename;
   else
     return EINA_FALSE;

   ef = eet_open(filename, EET_FILE_MODE_READ_WRITE);
   if (!ef)
     {
        ERR("Could not open '%s' for write.", filename);
        return EINA_FALSE;
     }

   ret = !!eet_data_write(ef, _gamescore_descriptor, GAMESCORE_ENTRY,
                          gamescore, EINA_TRUE);
   eet_close(ef);

   if (ret)
     INF("Gamescore saved to %s", filename);
   else
     ERR("Gamescore failed to save to %s", filename);

   return ret;
}

EAPI Etrophy_Gamescore *
etrophy_gamescore_load(const char *gamename)
{
   char filename[1024];
   char tmp[5] = "/tmp";
   char *home;

   if (!gamename)
     {
        ERR("You need to provide a gamename");
        return NULL;
     }

   home = getenv("ETROPHY_PATH");
   if (!home)
     {
        home = getenv("HOME");
        if (!home)
          home = tmp;
     }

   snprintf(filename, sizeof(filename), "%s/.etrophy/%s.eet", home, gamename);

   return etrophy_gamescore_path_load(filename);
}

EAPI int
etrophy_init(void)
{
   if (++_etrophy_init_count != 1)
     return _etrophy_init_count;

   if (!eina_init())
     {
        EINA_LOG_CRIT("Couldn't init eina.");
        return --_etrophy_init_count;
     }

   _etrophy_log_dom = eina_log_domain_register("etrophy", EINA_COLOR_CYAN);
   if (_etrophy_log_dom < 0)
     {
        EINA_LOG_CRIT("Couldn't create a log domain for etrophy.");
        goto no_log;
     }

   if (!eet_init())
     {
        ERR("Couldn't init eet.");
        goto no_eet;
     }

   if (!ecore_file_init())
     {
        ERR("Couldn't init ecore file.");
        goto no_ecore_file;
     }

   _trophy_init();
   _lock_init();
   _score_init();
   _level_init();
   _gamescore_init();

   INF("Etrophy initialized.");

   return _etrophy_init_count;

no_ecore_file:
   eet_shutdown();
no_eet:
   eina_log_domain_unregister(_etrophy_log_dom);
   _etrophy_log_dom = -1;
no_log:
   eina_shutdown();
   return --_etrophy_init_count;
}

EAPI int
etrophy_shutdown(void)
{
   if (--_etrophy_init_count != 0)
     return _etrophy_init_count;

   _gamescore_shutdown();
   _level_shutdown();
   _score_shutdown();
   _lock_shutdown();
   _trophy_shutdown();

   INF("Etrophy shutdown.");

   eina_log_domain_unregister(_etrophy_log_dom);
   _etrophy_log_dom = -1;

   ecore_file_shutdown();
   eet_shutdown();
   eina_shutdown();

   return _etrophy_init_count;
}

#ifdef  __cplusplus
}
#endif
