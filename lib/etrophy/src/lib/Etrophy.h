#ifndef __ETROPHY_H__
#define __ETROPHY_H__

/**
 * @mainpage Etrophy Library Documentation
 *
 * @version 0.5.0
 * @date 2012
 *
 * @section intro What is Etrophy ?
 *
 * Etrophy is a library that manages scores, trophies and unlockables. It will
 * store them and provide views to display them. Could be used by games based
 * on EFL.
 *
 * For a better reference, check the following groups:
 * @li @ref Init
 * @li @ref Gamescore
 * @li @ref Score
 * @li @ref Trophy
 * @li @ref Lock
 * @li @ref View
 *
 * Please see the @ref authors page for contact details.
 */

/**
 *
 * @page authors Authors
 *
 * @author Bruno Dilly <bdilly@@profusion.mobi>
 * @author Mike Blumenkrantz <michael.blumenkrantz@@gmail.com>
 *
 * Please contact <enlightenment-devel@lists.sourceforge.net> to get in
 * contact with the developers and maintainers.
 *
 */

#include <Eina.h>
#include <Eet.h>
#include <Evas.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ETROPHY_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ETROPHY_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file
 * @brief These routines are used for Etrophy library interaction.
 */

/**
 * @brief Init / shutdown functions.
 * @defgroup Init  Init / Shutdown
 *
 * @{
 *
 * Functions of obligatory usage, handling proper initialization
 * and shutdown routines.
 *
 * Before the usage of any other function, Etrophy should be properly
 * initialized with @ref etrophy_init() and the last call to Etrophy's
 * functions should be @ref etrophy_shutdown(), so everything will
 * be correctly freed.
 *
 * Etrophy logs everything with Eina Log, using the "etrophy" log domain.
 *
 */

/**
 * Initialize Etrophy.
 *
 * Initializes Etrophy, its dependencies and modules. Should be the first
 * function of Etrophy to be called.
 *
 * @return The init counter value.
 *
 * @see etrophy_shutdown().
 *
 * @ingroup Init
 */
EAPI int etrophy_init(void);

/**
 * Shutdown Etrophy
 *
 * Shutdown Etrophy. If init count reaches 0, all the internal structures will
 * be freed. Any Etrophy library call after this point will leads to an error.
 *
 * @return Etrophy's init counter value.
 *
 * @see etrophy_init().
 *
 * @ingroup Init
 */
EAPI int etrophy_shutdown(void);

/**
 * @}
 */

/**
 * @brief Gamescore API available to add a gamescore for your application.
 * @defgroup Gamescore Gamescore
 *
 * @{
 *
 * Gamescore is the object that will integrate all the components required
 * for the application, like trophies, scores and locks. It's mandatory
 * to have one created gamescore before adding anything else, so generally
 * an application should start by handling it.
 *
 * Example:
 * @code
 * Etrophy_Gamescore *gamescore;
 * gamescore = etrophy_gamescore_load("gamename");
 * if (!gamescore)
 *   {
 *      gamescore = etrophy_gamescore_new("gamename");
 *      // Functions populating the generated gamescore like a bunch of
 *      // @ref etrophy_lock_new(), @ref etrophy_gamescore_lock_add()
 *   }
 * @endcode
 *
 * Gamescore can be created with @ref etrophy_gamescore_new(), or loaded
 * with @ref etrophy_gamescore_load(), @ref etrophy_gamescore_path_load().
 * @ref etrophy_gamescore_edd_get() can be used to get the Eet data descriptor.
 *
 * The gamescore must to be saved with @ref etrophy_gamescore_save() or changes
 * will be lost. It can be cleared with @ref etrophy_gamescore_clear().
 *
 * After usage it should be freed with @ref etrophy_gamescore_free().
 *
 */

/**
 * @typedef Etrophy_Gamescore
 *
 * Gamescore handle, mandatory for Etrophy's usage. Each application
 * need to have one.
 *
 * Created with @ref etrophy_gamescore_new() and deleted with
 * @ref etrophy_gamescore_free().
 *
 * @ingroup Gamescore
 */
typedef struct _Etrophy_Gamescore Etrophy_Gamescore;

/**
 * @brief
 * Create a new gamescore.
 *
 * A gamescore should be used by application. It will be used to manage
 * trophies, locks and players' scores.
 *
 * It must to be saved after edited, using @ref etrophy_gamescore_save().
 *
 * @param gamename Name of the gamescore to be used to load in later
 * uses.
 * @return The gamescore created or @c NULL on error.
 *
 * @see etrophy_gamescore_free()
 * @see etrophy_gamescore_load()
 *
 * @ingroup Gamescore
 */
EAPI Etrophy_Gamescore *etrophy_gamescore_new(const char *gamename);

/**
 * @brief
 * Free a previouly created gamescore.
 *
 * @param gamescore Gamescore handle.
 *
 * @see etrophy_gamescore_new()
 * @see etrophy_gamescore_load()
 *
 * @ingroup Gamescore
 */
EAPI void etrophy_gamescore_free(Etrophy_Gamescore *gamescore);

/**
 * @brief
 * Load a gamescore given a name.
 *
 * An alternative way of loading it is using
 * @ref etrophy_gamescore_path_load(). It will be necessary if the gamescore
 * was previoulsy saved in a non standard Etrophy directory.
 *
 * @param gamename Name used when gamescore was created.
 * @return The gamescore or @c NULL on error.
 *
 * @see etrophy_gamescore_new()
 * @see etrophy_gamescore_save()
 *
 * @ingroup Gamescore
 */
EAPI Etrophy_Gamescore *etrophy_gamescore_load(const char *gamename);

/**
 * @brief
 * Load a gamescore from a give path.
 *
 * An alternative way of loading it is using @ref etrophy_gamescore_load(),
 * and the provided gamescore name will be searched on default Etrophy
 * directories.
 *
 * @param filename Full path to the file.
 * @return The gamescore or @c NULL on error.
 *
 * @ingroup Gamescore
 */
EAPI Etrophy_Gamescore *etrophy_gamescore_path_load(const char *filename);

/**
 * @brief
 * Save a gamescore.
 *
 * There are two ways of loading / saving gamescores. Just using gamescore's
 * name, and the directory will be defined by Etrophy, or passing
 * the full path to the file to be written.
 *
 * The default directory can be set with environment variable @c ETROPHY_PATH
 * or it will try to find user's home. Then the file will be stored inside
 * .etrophy sub directory, with the name of gamescore and extension ".eet".
 *
 * @param filename Full path to the file. If @c NULL, it will be saved on
 * default Etrophy directories.
 * @param gamescore Gamescore handle.
 * @return @c EINA_TRUE if the gamescore was properly saved @c EINA_FALSE
 * on error.
 *
 * @ingroup Gamescore
 */
EAPI Eina_Bool etrophy_gamescore_save(Etrophy_Gamescore *gamescore, const char *filename);

/**
 * @brief
 * Return gamescore Eet data descriptor.
 *
 * Useful when you already have an eet file for your application and wants
 * to merge gamescore on it.
 *
 * @return Eet data descriptor.
 *
 * @ingroup Gamescore
 */
EAPI Eet_Data_Descriptor *etrophy_gamescore_edd_get(void);

/**
 * @brief
 * Clear all the data of the gamescore.
 *
 * It will clear trophies, locks, players' scores.
 * Everything will be deleted.
 *
 * @param gamescore Gamescore handle.
 *
 * @ingroup Gamescore
 */
EAPI void etrophy_gamescore_clear(Etrophy_Gamescore *gamescore);

/**
 * @brief
 * Get the sum of all the trophies points earned by the player.
 *
 * @param gamescore Gamescore handle.
 * @return Sum of achieved trophies points.
 *
 * @see etrophy_gamescore_trophies_total_points_get().
 *
 * @ingroup Gamescore
 */
EAPI unsigned int etrophy_gamescore_trophies_points_get(const Etrophy_Gamescore *gamescore);

/**
 * @brief
 * Get the sum of all the trophies points.
 *
 * It returns a sum of all the points a player could possibly achieves.
 * For the sum of already earned trophies points, use
 * @ref etrophy_gamescore_trophies_points_get().
 *
 * @param gamescore Gamescore handle.
 * @return Sum of all possible trophies points.
 *
 * @ingroup Gamescore
 */
EAPI unsigned int etrophy_gamescore_trophies_total_points_get(const Etrophy_Gamescore *gamescore);

/**
 * @}
 */

/**
 * @brief Functions to manipulate player's scores.
 * @defgroup Score Score
 *
 * @{
 *
 * Scores consists of the player name, amount of points earned, and date
 * and time. It should be added to the specific level. So basically,
 * a gamescore has many levels, and each level many scores saved.
 * So a level just have a name and a list of scores.
 *
 * These two objects are handled by @ref Etrophy_Score and @ref Etrophy_Level.
 *
 * Levels can be created by two ways:
 * @li Creating a level inside a gamescore with
 * @ref etrophy_gamescore_level_add().
 * @li Creating a level with @ref etrophy_level_new() and adding it to
 * gamescore with @ref etrophy_gamescore_level_add();
 *
 * The list of levels in the gamescore can be get with
 * @ref etrophy_gamescore_levels_list_get() and can be cleared with
 * @ref etrophy_gamescore_levels_list_clear(). A specific level can be get
 * with @ref etrophy_gamescore_level_get().
 *
 * Scores can be created using @ref etrophy_score_new() and added to a
 * level with @ref etrophy_gamescore_level_score_add() or directly with
 * @ref etrophy_level_score_add(). A list of scores can be get
 * with @ref etrophy_level_scores_list_get(). But a couple of functions
 * can be very useful:
 * @li @ref etrophy_gamescore_level_hi_score_get() to get the highest score
 * in a specific level and
 * @li @ref etrophy_gamescore_level_hi_score_get() to get the lowest one
 * (useful for games based on time).
 *
 * Score properties can be accessed with:
 * @li @ref etrophy_score_player_name_get()
 * @li @ref etrophy_score_score_get()
 * @li @ref etrophy_score_date_get()
 *
 */

/**
 * @typedef Etrophy_Level
 *
 * Level handle, used to store all the players' scores in the level.
 *
 * Created with @ref etrophy_level_new(), added to gamescore using
 * @ref etrophy_gamescore_level_add() and freed with
 * @ref etrophy_level_free().
 *
 * @ingroup Score
 */
typedef struct _Etrophy_Level Etrophy_Level;

/**
 * @typedef Etrophy_Score
 *
 * Score handle, save the player's scores, name, and date.
 *
 * Created with @ref etrophy_score_new(), added to the level using
 * @ref etrophy_level_score_add() and freed with
 * @ref etrophy_score_free().
 *
 * @ingroup Score
 */
typedef struct _Etrophy_Score Etrophy_Score;

EAPI Etrophy_Level *etrophy_level_new(const char *name);
EAPI void etrophy_level_free(Etrophy_Level *level);
EAPI const char *etrophy_level_name_get(const Etrophy_Level *level);

EAPI const Eina_List *etrophy_level_scores_list_get(const Etrophy_Level *level);
EAPI void etrophy_level_scores_list_clear(Etrophy_Level *level);

EAPI void etrophy_gamescore_level_add(Etrophy_Gamescore *gamescore, Etrophy_Level *level);
EAPI void etrophy_gamescore_level_del(Etrophy_Gamescore *gamescore, Etrophy_Level *level);
EAPI Etrophy_Level *etrophy_gamescore_level_get(Etrophy_Gamescore *gamescore, const char *name);
EAPI const Eina_List *etrophy_gamescore_levels_list_get(const Etrophy_Gamescore *gamescore);
EAPI void etrophy_gamescore_levels_list_clear(Etrophy_Gamescore *gamescore);

EAPI void etrophy_level_score_add(Etrophy_Level *level, Etrophy_Score *escore);
EAPI void etrophy_level_score_del(Etrophy_Level *level, Etrophy_Score *escore);

EAPI Etrophy_Score *etrophy_score_new(const char *player_name, int score);
EAPI void  etrophy_score_free(Etrophy_Score *escore);
EAPI const char *etrophy_score_player_name_get(const Etrophy_Score *escore);
EAPI int etrophy_score_score_get(const Etrophy_Score *escore);
EAPI unsigned int etrophy_score_date_get(const Etrophy_Score *escore);

EAPI Etrophy_Score *etrophy_gamescore_level_score_add(Etrophy_Gamescore *gamescore, const char *level_name, const char *player_name, int score);
EAPI int etrophy_gamescore_level_hi_score_get(const Etrophy_Gamescore *gamescore, const char *level_name);
EAPI int etrophy_gamescore_level_low_score_get(const Etrophy_Gamescore *gamescore, const char *level_name);

/**
 * @}
 */

/**
 * @brief Functions to manipulate player's trophies.
 * @defgroup Trophy Trophy
 *
 * @{
 *
 * A trophy represents a task a user achieved. It can be visible or hidden,
 * and starts locked and only unlocked after the player reaches
 * its goal. For that, a trophy consists of a name, description, visibility
 * the goal, and current points on this task.
 *
 * A new trophy can be created with @ref etrophy_trophy_new() and should
 * be added to the gamescore with @ref etrophy_gamescore_trophy_add().
 * It is handled by @ref Etrophy_Trophy.
 *
 * It's counter can be directly set with @ref etrophy_trophy_counter_set()
 * or incremented using @ref etrophy_trophy_counter_increment(). Current
 * points and goal can be get using @ref etrophy_trophy_goal_get().
 * @ref etrophy_trophy_earned_get() returns if the trophy was achieved
 * or not.
 *
 * Other attributes can be read with:
 * @li @ref etrophy_trophy_name_get()
 * @li @ref etrophy_trophy_description_get()
 * @li @ref etrophy_trophy_visibility_get()
 * @li @ref etrophy_trophy_points_get()
 * @li @ref etrophy_trophy_date_get()
 *
 * The trophies list can be get with
 * @ref etrophy_gamescore_trophies_list_get() and cleared using
 * @ref etrophy_gamescore_trophies_list_clear().
 */

/**
 * @typedef Etrophy_Trophy
 *
 * Trophy handle, keeps score per tasks and goals. It has name, description
 * and visibility.
 *
 * Created with @ref etrophy_trophy_new(), added to gamescore using
 * @ref etrophy_gamescore_trophy_add() and freed with
 * @ref etrophy_level_free().
 *
 * @ingroup Trophy
 */
typedef struct _Etrophy_Trophy Etrophy_Trophy;

/**
 * @enum _Etrophy_Trophy_Visibility
 * @typedef Etrophy_Trophy_Visibility
 *
 * State of a lock: locked or unlocked. Usually locks starts locked and
 * after the player achieves a task it is unlocked.
 *
 * @see etrophy_trophy_new()
 * @see etrophy_trophy_visibility_get()
 *
 * @ingroup Trophy
 */
typedef enum _Etrophy_Trophy_Visibility
{
   ETROPHY_TROPHY_STATE_HIDDEN = 0, /**< Player shouldn't know about its existence */
   ETROPHY_TROPHY_STATE_VISIBLE, /**< Player can know about its existence */
   ETROPHY_TROPHY_STATE_LAST_VALUE /**< kept as sentinel */
} Etrophy_Trophy_Visibility;

/**
 * @enum _Etrophy_Trophy_Points
 * @typedef Etrophy_Trophy_Points
 *
 * Amount of trophies points earned by the player when the trophy is
 * earned, when the counter is equal to the goal.
 *
 * It's suggested to be used as the @c points parameter of
 * @ref etrophy_trophy_new() function, but any non negative integer value can
 * be passed.
 *
 * @see etrophy_trophy_new()
 * @see etrophy_trophy_points_get()
 *
 * @ingroup Trophy
 */
typedef enum _Etrophy_Trophy_Points
{
   ETROPHY_TROPHY_POINTS_VERY_FEW = 10, /**< Used for easy tasks */
   ETROPHY_TROPHY_POINTS_FEW = 25, /**< Used for not so easy tasks */
   ETROPHY_TROPHY_POINTS_MANY = 50, /**< Used for regular tasks */
   ETROPHY_TROPHY_POINTS_LOTS = 100, /**< Used for hard tasks */
   ETROPHY_TROPHY_POINTS_LAST_VALUE /**< kept as sentinel */
} Etrophy_Trophy_Points;

EAPI Etrophy_Trophy *etrophy_trophy_new(const char *name, const char *description, Etrophy_Trophy_Visibility visibility, unsigned int goal, unsigned int points);
EAPI void etrophy_trophy_free(Etrophy_Trophy *trophy);
EAPI const char *etrophy_trophy_name_get(const Etrophy_Trophy *trophy);
EAPI const char *etrophy_trophy_description_get(const Etrophy_Trophy *trophy);
EAPI Etrophy_Trophy_Visibility etrophy_trophy_visibility_get(const Etrophy_Trophy *trophy);
EAPI void etrophy_trophy_goal_get(const Etrophy_Trophy *trophy, unsigned int *goal, unsigned int *counter);
EAPI void etrophy_trophy_counter_set(Etrophy_Trophy *trophy, unsigned int value);
EAPI void etrophy_trophy_counter_increment(Etrophy_Trophy *trophy, unsigned int value);
EAPI Eina_Bool etrophy_trophy_earned_get(const Etrophy_Trophy *trophy);
EAPI unsigned int etrophy_trophy_points_get(const Etrophy_Trophy *trophy);
EAPI unsigned int etrophy_trophy_date_get(const Etrophy_Trophy *trophy);

EAPI void etrophy_gamescore_trophy_add(Etrophy_Gamescore *gamescore, Etrophy_Trophy *trophy);
EAPI void etrophy_gamescore_trophy_del(Etrophy_Gamescore *gamescore, Etrophy_Trophy *trophy);
EAPI Etrophy_Trophy *etrophy_gamescore_trophy_get(Etrophy_Gamescore *gamescore, const char *name);
EAPI const Eina_List *etrophy_gamescore_trophies_list_get(const Etrophy_Gamescore *gamescore);
EAPI void etrophy_gamescore_trophies_list_clear(Etrophy_Gamescore *gamescore);

/**
 * @}
 */

/**
 * @brief Functions to handle locks.
 * @defgroup Lock Lock
 *
 * @{
 *
 * A lock is a very simple and generic concept, generally used for games
 * regarding stuff that should be accesible only after the user does a
 * specific action. For example: a gun is locked until the player kills the
 * first boss; or the "hard" level is unlocked only when the "easy" one is
 * concluded with a high score.
 *
 * It consists of a name, date of update, and its state: locked or unlocked.
 *
 * It is handled by @ref Etrophy_Lock and should be created with
 * @ref etrophy_lock_new() and added to the gamescore using
 * @ref etrophy_gamescore_lock_add().
 *
 * A list of locks can be get with @ref etrophy_gamescore_locks_list_get()
 * and cleared with @ref etrophy_gamescore_locks_list_clear().
 *
 * The state should be changed using @ref etrophy_lock_state_set().
 *
 * Example of usage of locks:
 * @code
 * Etrophy_Gamescore *load_function(Eina_List *locks_names)
 * {
 *    Etrophy_Gamescore *gamescore;
 *    gamescore = etrophy_gamescore_load("gamename");
 *    if (!gamescore)
 *      {
 *         const char *name;
 *         Eina_List *l;
 *         gamescore = etrophy_gamescore_new("gamename");
 *         EINA_LIST_FOREACH(locks_names, l, name)
 *           {
 *              Etrophy_Lock *etrophy_lock;
 *              etrophy_lock = etrophy_lock_new(name,
 *                                              ETROPHY_LOCK_STATE_LOCKED);
 *              etrophy_gamescore_lock_add(gamescore, etrophy_lock);
 *           }
 *      }
 *    return gamescore;
 * }
 * void unlock_function(Etrophy_Gamescore *gamescore, const char *name)
 * {
 *    Etrophy_Lock *lock;
 *    lock = etrophy_gamescore_lock_get(gamescore, name);
 *    etrophy_lock_state_set(lock, ETROPHY_LOCK_STATE_UNLOCKED);
 * }
 * @endcode
 *
 */

/**
 * @typedef Etrophy_Lock
 *
 * Lock handle, has name, lock state and date of update.
 *
 * Created with @ref etrophy_lock_new(), added to gamescore using
 * @ref etrophy_gamescore_lock_add() and freed with
 * @ref etrophy_lock_free().
 *
 * @ingroup Lock
 */
typedef struct _Etrophy_Lock Etrophy_Lock;

/**
 * @enum _Etrophy_Lock_State
 * @typedef Etrophy_Lock_State
 *
 * State of a lock: locked or unlocked. Usually locks starts locked and
 * after the player achieves a task it is unlocked.
 *
 * @see etrophy_lock_new()
 * @see etrophy_lock_state_set()
 * @see etrophy_lock_state_get()
 *
 * @ingroup Lock
 */
typedef enum _Etrophy_Lock_State
{
   ETROPHY_LOCK_STATE_LOCKED = 0, /**< Player doesn't have access to it yet */
   ETROPHY_LOCK_STATE_UNLOCKED, /**< Player already has access to it */
   ETROPHY_LOCK_STATE_LAST_VALUE /**< kept as sentinel, not really a state */
} Etrophy_Lock_State;

EAPI Etrophy_Lock *etrophy_lock_new(const char *name, Etrophy_Lock_State state);
EAPI void etrophy_lock_free(Etrophy_Lock *lock);
EAPI const char *etrophy_lock_name_get(const Etrophy_Lock *lock);
EAPI void etrophy_lock_state_set(Etrophy_Lock *lock, Etrophy_Lock_State state);
EAPI Etrophy_Lock_State etrophy_lock_state_get(const Etrophy_Lock *lock);
EAPI unsigned int etrophy_lock_date_get(const Etrophy_Lock *lock);
EAPI void etrophy_gamescore_lock_add(Etrophy_Gamescore *gamescore, Etrophy_Lock *lock);
EAPI void etrophy_gamescore_lock_del(Etrophy_Gamescore *gamescore, Etrophy_Lock *lock);
EAPI Etrophy_Lock *etrophy_gamescore_lock_get(Etrophy_Gamescore *gamescore, const char *name);
EAPI const Eina_List *etrophy_gamescore_locks_list_get(const Etrophy_Gamescore *gamescore);
EAPI void etrophy_gamescore_locks_list_clear(Etrophy_Gamescore *gamescore);

/**
 * @}
 */

/**
 * @brief Functions that return objects to display gamescore data visually.
 * @defgroup View View
 *
 * @{
 *
 * Beyond data management, Etrophy provides some views, built with Elementary.
 * They consist of Elm Layouts with widgets used to display gamescore data
 * to the application user.
 *
 * It provides views for:
 * @li scores with @ref etrophy_score_layout_add()
 * @li trophies with @ref etrophy_trophies_layout_add()
 * @li locks with @ref etrophy_locks_layout_add()
 *
 */

/**
 * @brief
 * Create a view and populate it with scores data.
 *
 * Create a Elm Layout with a spinner to select the level and a genlist
 * displaying each score. The returned object can be placed anywhere in
 * the application view (layout, boxes, etc).
 *
 * Example:
 * @code
 * static void
 * _scores_show_cb(void *data, Evas_Object *obj __UNUSED__,
 *                 void *event_info __UNUSED__)
 * {
 *     Evas_Object *popup, *bt, *leaderboard;
 *     Game *game = data;
 *
 *     popup = elm_popup_add(game->win);
 *     elm_object_part_text_set(popup, "title,text", "Leaderboard");
 *
 *     bt = elm_button_add(popup);
 *     elm_object_text_set(bt, "OK");
 *     elm_object_part_content_set(popup, "button1", bt);
 *
 *     leaderboard = etrophy_score_layout_add(popup, game->gamescore);
 *     elm_object_content_set(popup, leaderboard);
 *     evas_object_smart_callback_add(bt, "clicked", _scores_hide_cb, popup);
 *
 *     evas_object_show(popup);
 * }
 * @endcode
 *
 * @param parent Evas object to be used as parent.
 * @param gamescore Gamescore handle.
 * @return The view or @c NULL on error.
 *
 * @ingroup View
 */
EAPI Evas_Object *etrophy_score_layout_add(Evas_Object *parent, Etrophy_Gamescore *gamescore);

/**
 * @brief
 * Create a view and populate it with trophies data.
 *
 * NOT IMPLEMENTED.
 *
 * @param parent Evas object to be used as parent.
 * @param gamescore Gamescore handle.
 * @return The view or @c NULL on error.
 *
 * @ingroup View
 */
EAPI Evas_Object *etrophy_trophies_layout_add(Evas_Object *parent, Etrophy_Gamescore *gamescore);

/**
 * @brief
 * Create a view and populate it with locks data.
 *
 * NOT IMPLEMENTED.
 *
 * @param parent Evas object to be used as parent.
 * @param gamescore Gamescore handle.
 * @return The view or @c NULL on error.
 *
 * @ingroup View
 */
EAPI Evas_Object *etrophy_locks_layout_add(Evas_Object *parent, Etrophy_Gamescore *gamescore);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __ETROPHY_H__ */
