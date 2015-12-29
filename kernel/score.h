/* Definitions relating to scorekeepers in Xconq.
   Copyright (C) 1987-1989, 1991-1997, 1999 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file kernel/score.h
 * \brief Definitions relating to scorekeepers in Xconq.
 */

/*! \brief Scorekeeper Data. */
typedef struct a_scorekeeper {
    short id;                   /*!< unique id number */
    const char *title;         	/*!< title by which this is displayed */
    Obj *when;                  /*!< times at which this will run */
    Obj *who;                   /*!< which sides this applies to */
    SideMask whomask;           /*!< mask of sides this applies to */
    Obj *knownto;               /*!< which sides know about this scorekeeper */
    SideMask knowntomask;	/*! mask of sides this is known to */
    short keepscore;		/*! should we keep score? */
    int initial;                /*!< initial value of a numeric score */
    Obj *trigger;               /*!< test that decides triggering */
    short triggered;            /*!< true when scorekeeper has been triggered */
    Obj *body;                  /*!< the actual effect-causing stuff */
    Obj *record;                /*!< how to record this in scorefile */
    Obj *notes;                 /*!< various notes about the scorekeeper */
    short scorenum;             /*!< index of this scorekeeper's score value */
    struct a_scorekeeper *next; /*!< pointer to the next scorekeeper */
} Scorekeeper;

/*! \brief Iteration over all scorekeepers.
 *
 * walk through list of scorekeepers.
 * \param sk is interation varible of pointer to scorekeeper.
 */
#define for_all_scorekeepers(sk)  \
  for ((sk) = scorekeepers;  (sk) != NULL;  (sk) = (sk)->next)

#ifdef DESIGNERS
#define keeping_score() (numscorekeepers > 0 && numdesigners == 0)
#else
#define keeping_score() (numscorekeepers > 0)
#endif

#define recording_scores() (0)

/*! \brief Scorekeeper list. */
extern Scorekeeper *scorekeepers;

/*! \brief Number of scorekeepers. */
extern int numscorekeepers;

/*! \brief Number of scores. */
extern int numscores;

/*! \brief Any post action scores? ??? */
extern int any_post_action_scores;
/*! \brief Any Post event scores? ??? */
extern int any_post_event_scores;

extern void init_scorekeepers(void);
extern Scorekeeper *create_scorekeeper(void);
extern Scorekeeper *find_scorekeeper(int id);
extern void init_scores(void);
extern void check_pre_turn_scores(void);
extern void check_post_turn_scores(void);
extern void check_post_action_scores(void);
extern void check_post_event_scores(void);
extern void run_scorekeeper(Side *side, Scorekeeper *sk);
extern Obj *eval_sk_form(Side *side, Scorekeeper *sk, Obj *form);
#if (0)
extern int sum_property(Side *side, Obj *form);
#else
extern Obj *sum_uprop(Side *side, Obj *form);
#endif
extern int point_value(Unit *unit);
extern int side_point_value(Side *side);
extern int alliance_point_value(Side *side);
extern int has_allies(Side *side);
extern Obj *eval_sk_test(Side *side, Scorekeeper *sk, Obj *form);
extern void side_wins(Side *side, int why);
extern void side_loses(Side *side, Side *side2, int why);
extern void all_sides_draw(void);
extern void record_into_scorefile(void);
extern const char *get_scores(Side *side);

