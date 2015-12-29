/* Definitions for the historical record.
   Copyright (C) 1992-1995, 1998-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file kernel/history.h
 * \brief Definitions for the historical record.
 */

#undef  DEF_HEVT
#define DEF_HEVT(name, CODE, datadescs) CODE,

/*! \brief History Event type. */
typedef enum {

#include "history.def"

    NUMHEVTTYPES
} HistEventType;

/*! \brief Historical Event description. 
 *
 * This is the form of the description of a event type.
 * It contains the name of the event, and the description
 * of the event.
 */
typedef struct a_hevt_defn {
    const char *name;         	/*!< Name of event. */
    const char *datadescs;	/*!< Drescription of event. */
} HevtDefn;

/*! \brief Historical Event. 
 *
 * This ddefines an historical event.
 */
typedef struct a_histevent {
    HistEventType type;         		/*!< type of historical event */
    short startdate;            		/*!< date of event's start */
    short enddate;              		/*!< date of event's end */
    SideMask observers;         	/*!< which sides know about this event */
    struct a_histevent *next;   	/*!< link to next in list */
    struct a_histevent *prev;		/*!< link to previous in list */
    int data[4];                		/*!< data describing event */
} HistEvent;


/*! \brief Unit History.
 *
 * This is a snapshot of key bits of state of a \Unit at a particular
 * moment.
 */
typedef struct a_pastunit {
    short type;                 		/*!< type */
    int id;                     			/*!< truly unique id number */
    const char *name;                 		/*!< the name, if given */
    int number;                 		/*!< semi-unique number */
    short x;                    			/*!< x position of unit in world */
    short y;                    			/*!< y position of unit in world */
    short z;                    			/*!< z position of unit in world */
    struct a_side *side;        		/*!< whose side this unit is on */
    struct a_pastunit *next;    	/*!< pointer to next in list */
} PastUnit;

/*! \brief Reasons for gain. */
enum gain_reasons {
    initial_gain = 0,       	/*!< Initial */
    build_gain = 1,         	/*!< build */
    capture_gain = 2,       	/*1< capture */
    other_gain = 3,         	/*!< other */
    num_gain_reasons = 4    	/*!< number of gain reasons. */
};

/*! \brief Reasons for loss. */
enum loss_reasons {
    combat_loss = 0,        	/*!< Combat */
    capture_loss = 1,       	/*!< Capture */
    starvation_loss = 2,    	/*!< Starvation */
    accident_loss = 3,      	/*!< Accident */
    attrition_loss = 4,		/*!< Attrition */
    disband_loss = 5,       	/*!< Disband */
    other_loss = 6,         	/*!< Other */
    num_loss_reasons = 7    	/*!< number of loss reasons. */
};

/*! \brief Damage resons. */
enum damage_reasons {
    combat_dmg,         	/*!< combat */
    accident_dmg,       	/*!< accident */
    attrition_dmg,       	/*!< attrition */
    starvation_dmg		/*!< starvation */
};

/*! \brief Historical Event Definition array. */
extern HevtDefn hevtdefns[];

/*1 \brief Historical Events list. */
extern HistEvent *history;

/*! \brief Past Unit History list. */
extern PastUnit *past_unit_list;

extern void init_history(void);
extern void start_history(void);
extern HistEvent *create_historical_event(HistEventType type);
extern HistEvent *record_event(HistEventType type, SideMask observers, ...);
extern void record_unit_death(Unit *unit, HistEventType reason);
extern void record_unit_name_change(Unit *unit, char *newname);
extern void record_unit_side_change(Unit *unit, Side *newside,
				    HistEventType reason, Unit *agent);
extern void count_gain(Side *side, int u, enum gain_reasons reason);
extern void count_loss(Side *side, int u, enum loss_reasons reason);
extern void end_history(void);
extern HistEvent *get_nth_history_line(Side *side, int n, HistEvent **nextevt);
extern PastUnit *create_past_unit(int type, int id);
extern PastUnit *find_past_unit(int n);
extern const char *past_unit_desig(PastUnit *pastunit);
extern PastUnit *change_unit_to_past_unit(Unit *unit);
extern void dump_statistics(void);

extern int update_total_hist_lines(Side *side);
extern int build_hist_contents(Side *side, int n, HistEvent **histcontents, int numvis);
extern int total_gain(Side *side, int u);
extern int total_loss(Side *side, int u);
