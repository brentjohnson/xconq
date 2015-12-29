/* Scorekeeper Analysis Functions for AIs
   Copyright (C) 2005 Eric A. McDonald.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*! \file 
    \brief Scorekeeper and Victory Condition Analysis for AIs

    Part of the AI API, Level 1. 
    Provides useful functions that an AI implementation may use to analyze 
    scorekeepers. 
    Nothing in this file should ever be required to implement an AI; 
    everything here is optional.

    \todo Add analysis of scorekeeper trigger.
*/

//! Kinds of AI scorekeeper analysis nodes.

enum AI_SKANKind {
    AI_SKAN_NONE = 0,
    AI_SKAN_UPROPI,
    AI_SKAN_TESTOP
};

//! Storage space for a particular segment of AI analysis of a scorekeeper.
/*!
    Abstract base struct for the various kinds of nodes.
    Maintains a various kinds of entities pertaining to scorekeeper analysis.
*/

struct AI_SKANode {
    //! Identify the kind of node.
    int kind;
    //! Pointer to next node.
    AI_SKANode *next;
    //! Initializer.
    static void init(AI_SKANode *skanode);
    //! Deep copy to a scorekeeper analysis node.
    void copy_to(AI_SKANode *skanode);
};

//! Storage space for uprop and side-utype table.

struct AI_SKANode_UpropInt : public AI_SKANode {
    //! Uprop name.
    const char *upropname;
    //! Table of sides and utypes.
    PackedBoolTable *sides_and_utypes;
    //! Initializer.
    static void init(AI_SKANode_UpropInt *skanode);
    //! "Constructor".
    static AI_SKANode_UpropInt *construct(
	const char *upropname, int *utypeary, SideMask sideset);
    //! Deep copy to a scorekeeper analysis node.
    void copy_to(AI_SKANode_UpropInt *skanode);
    //! Clone a scorekeeper analysis node.
    AI_SKANode_UpropInt *clone();
    //! Clone chain of scorekeeper analysis nodes.
    AI_SKANode_UpropInt *clone_list();
    //! "Destructor".
    static void destroy(AI_SKANode_UpropInt **skanode);
    //! Chain delete of scorekeeper analysis nodes.
    static void destroy_list(AI_SKANode_UpropInt **skanode);
    //! Merge or insert a scorekeeper analysis node.
    static void merge_or_insert_into_list(
	AI_SKANode_UpropInt **skanodep, const char *upropname, int *utypeary,
	SideMask sideset);
};

//! Storage space for test operator.

struct AI_SKANode_TestOp : public AI_SKANode {
    //! Test operator.
    int op;
    //! Initializer.
    static void init(AI_SKANode_TestOp *skanode);
    //! "Constructor".
    static AI_SKANode_TestOp *construct(int testop);
    //! Deep copy to a scorekeeper analysis node.
    void copy_to(AI_SKANode_TestOp *skanode);
    //! Clone a scorekeeper analysis node.
    AI_SKANode_TestOp *clone();
    //! Clone chain of scorekeeper analysis nodes.
    AI_SKANode_TestOp *clone_list();
    //! "Destructor".
    static void destroy(AI_SKANode_TestOp **skanode);
    //! Chain delete of scorekeeper analysis nodes.
    static void destroy_list(AI_SKANode_TestOp **skanode);
};

//! Storage space for AI analysis of a scorekeeper.
/*!
    Various fields pertaining to an AI's understanding of a scorekeeper.
    \todo Utypes that cause draws.
    \todo Add support for new scorekeeper forms.
*/

struct AI_SKAnalysis {
    //! Pointer to next analysis.
    AI_SKAnalysis *next;
    //! Flag bits indicating basic scorekeeper properties.
    /*! Is the scorekeeper understood? Does it cause a draw after some turn?
	Etc...
    */
    int basic_flags;
    //! Flag bits indicating which action/outcome is in effect.
    int action_flags;
    //! Sides which will win on some turn.
    SideMask win_eq_some_turn;
    //! Sides which will lose on some turn.
    SideMask lose_eq_some_turn;
    //! Sides which will win >= some turn.
    SideMask win_ge_some_turn;
    //! Sides which will lose >= some turn.
    SideMask lose_ge_some_turn;
    //! Sides which will win > some turn.
    SideMask win_gt_some_turn;
    //! Sides which will lose > some turn.
    SideMask lose_gt_some_turn;
    //! Sides which will win when their scores equal some number.
    SideMask win_eq_some_score;
    //! Sides which will lose when their scores equal some number.
    SideMask lose_eq_some_score;
    //! Sides which will win when their scores not equal some number.
    SideMask win_ne_some_score;
    //! Sides which will lose when their scores not equal some number.
    SideMask lose_ne_some_score;
    //! Sides which will win when their scores <= some number.
    SideMask win_le_some_score;
    //! Sides which will lose when their scores <= some number.
    SideMask lose_le_some_score;
    //! Sides which will win when their scores < some number.
    SideMask win_lt_some_score;
    //! Sides which will lose when their scores < some number.
    SideMask lose_lt_some_score;
    //! Sides which will win when their scores >= some number.
    SideMask win_ge_some_score;
    //! Sides which will lose when their scores >= some number.
    SideMask lose_ge_some_score;
    //! Sides which will win when their scores > some number.
    SideMask win_gt_some_score;
    //! Sides which will lose when their scores < some number.
    SideMask lose_gt_some_score;
    //! Exact turns that sides will win on.
    int *win_eq_turn;
    //! Exact turns that sides will lose on.
    int *lose_eq_turn;
    //! Exact turn that will end the game in a draw.
    int draw_eq_turn;
    //! Sides win >= turns.
    int *win_ge_turn;
    //! Sides lose >= turns.
    int *lose_ge_turn;
    //! Drawed game >= turn.
    int draw_ge_turn;
    //! Sides win > turns.
    int *win_gt_turn;
    //! Sides lose > turns.
    int *lose_gt_turn;
    //! Drawed game > turn.
    int draw_gt_turn;
    //! Exact scores that sides will win with.
    int *win_eq_score;
    //! Exact scores that sides will lose with.
    int *lose_eq_score;
    //! Exact scores that sides will win without.
    int *win_ne_score;
    //! Exact scores that sides will lose without.
    int *lose_ne_score;
    //! Sides win <= scores.
    int *win_le_score;
    //! Sides lose <= scores.
    int *lose_le_score;
    //! Sides win < scores.
    int *win_lt_score;
    //! Sides lose < scores.
    int *lose_lt_score;
    //! Sides win >= scores.
    int *win_ge_score;
    //! Sides lose >= scores.
    int *lose_ge_score;
    //! Sides win > scores.
    int *win_gt_score;
    //! Sides lose > scores.
    int *lose_gt_score;
    //! Uprops summed equal to some number cause a side to win.
    AI_SKANode_UpropInt *uprops_win_eq_some_sum;
    //! Uprops summed equal some number cause a side to lose.
    AI_SKANode_UpropInt *uprops_lose_eq_some_sum;
    //! Uprops summed not equal some number cause a side to win.
    AI_SKANode_UpropInt *uprops_win_ne_some_sum;
    //! Uprops summed not equal some number cause a side to lose.
    AI_SKANode_UpropInt *uprops_lose_ne_some_sum;
    //! Uprops summed <= some number cause a side to win.
    AI_SKANode_UpropInt *uprops_win_le_some_sum;
    //! Uprops summed <= some number cause a side to lose.
    AI_SKANode_UpropInt *uprops_lose_le_some_sum;
    //! Uprops summed < some number cause a side to win.
    AI_SKANode_UpropInt *uprops_win_lt_some_sum;
    //! Uprops summed < some number cause a side to lose.
    AI_SKANode_UpropInt *uprops_lose_lt_some_sum;
    //! Uprops summed >= some number cause a side to win.
    AI_SKANode_UpropInt *uprops_win_ge_some_sum;
    //! Uprops summed >= some number cause a side to lose.
    AI_SKANode_UpropInt *uprops_lose_ge_some_sum;
    //! Uprops summed > some number cause a side to win.
    AI_SKANode_UpropInt *uprops_win_gt_some_sum;
    //! Uprops summed > some number cause a side to lose.
    AI_SKANode_UpropInt *uprops_lose_gt_some_sum;
    //! Uprops that change a side's score.
    AI_SKANode_UpropInt *uprops_change_score;
    //! Test operators.
    AI_SKANode_TestOp *opstack;
    //! Initializer.
    static void init(AI_SKAnalysis *skanal);
    //! "Constructor".
    static AI_SKAnalysis *construct();
    //! Deep copy to a scorekeeper analysis.
    void copy_to(AI_SKAnalysis *skanal);
    //! Clone a scorekeeper analysis.
    AI_SKAnalysis *clone();
    //! "Destructor".
    static void destroy(AI_SKAnalysis **skanalp);
    //! Push onto operator stack.
    void push_test_op(int testop);
    //! Pop from operator stack.
    AI_SKANode_TestOp *pop_test_op();
    //! Search operator stack/list for a test op.
    int search_test_ops_for(int op);
};

//! Macro for iterating through scorekeeper analyses.

#define for_all_ai_sk_analyses(skanals, skanal) \
    for ((skanal) = (skanals); (skanal); (skanal) = (skanal)->next)

/* Scorekeeper analysis flags. */

#define AI_SK_NONE		0

#define AI_SK_DONT_UNDERSTAND	0x1
#define AI_SK_DONT_TEST		0x2
#define AI_SK_LAST_SIDE		0x10
#define AI_SK_LAST_ALLIANCE	0x20
#define AI_SK_CHANGE_SCORE	0x40

/* Possible scorekeeper actions from AI perspective. */

#define AI_SKA_NONE             0

#define AI_SKA_CHANGE_SCORE     0x1
#define AI_SKA_WIN              0x2
#define AI_SKA_LOSE             0x4
#define AI_SKA_DRAW             0x8
#define AI_SKA_LAST_SIDE        0x10
#define AI_SKA_LAST_ALLIANCE    0x20

/* Possible scorekeeper tests from AI perspective. */

#define AI_SKT_NONE             0

#define AI_SKT_NOT              0x1
#define AI_SKT_AND              0x2
#define AI_SKT_OR               0x4
#define AI_SKT_EQ               0x8
#define AI_SKT_NE               0x10
#define AI_SKT_LE               0x20
#define AI_SKT_LT               0x40
#define AI_SKT_GE               0x80
#define AI_SKT_GT               0x100
#define AI_SKT_PLUS             0x200
#define AI_SKT_MINUS            0x400
#define AI_SKT_MULTIPLY         0x800
#define AI_SKT_DIVIDE           0x1000

//! All possible artithmetic comparisons in a scorekeeper test.

#define AI_SKT_ARITH_COMPARISONS \
    (AI_SKT_EQ | AI_SKT_NE | AI_SKT_LE | AI_SKT_LT | AI_SKT_GE | AI_SKT_GT)

//! All possible artithmetic operations in a scorekeeper test.

#define AI_SKT_ARITH_OPERATIONS \
    (AI_SKT_PLUS | AI_SKT_MINUS | AI_SKT_MULTIPLY | AI_SKT_DIVIDE)

/* Function Declarations */

//! Analyze a scorekeeper 'sum-uprop' form.
extern AI_SKAnalysis *analyze_sk_sum_uprop_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper 'turn' keyword.
extern AI_SKAnalysis *analyze_sk_turn_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper 'score' keyword/form.
extern AI_SKAnalysis *analyze_sk_score_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper arithmetic operation form.
extern AI_SKAnalysis *analyze_sk_arith_op_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper arithmetic '>' form.
extern AI_SKAnalysis *analyze_sk_arith_gt_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper arithmetic '>=' form.
extern AI_SKAnalysis *analyze_sk_arith_ge_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper arithmetic '<' form.
extern AI_SKAnalysis *analyze_sk_arith_lt_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper arithmetic '<=' form.
extern AI_SKAnalysis *analyze_sk_arith_le_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper arithmetic '/=' form.
extern AI_SKAnalysis *analyze_sk_arith_ne_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper arithmetic '=' form.
extern AI_SKAnalysis *analyze_sk_arith_eq_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper arithmetic comparison form.
extern AI_SKAnalysis *analyze_sk_arith_comp_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper boolean 'or' form.
extern AI_SKAnalysis *analyze_sk_bool_or_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper boolean 'and' form.
extern AI_SKAnalysis *analyze_sk_bool_and_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper boolean comparison form.
extern AI_SKAnalysis *analyze_sk_bool_comp_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper 'when' form.
extern AI_SKAnalysis *analyze_sk_when_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper test form that is always true.
extern AI_SKAnalysis * analyze_sk_always_true_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper test form.
extern AI_SKAnalysis *analyze_sk_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent);
//! Analyze a scorekeeper 'if' form.
extern AI_SKAnalysis *analyze_sk_if_action(
    Side *side, Scorekeeper *sk, Obj *skact, AI_SKAnalysis *parent);
//! Analyze a scorekeeper action form.
extern AI_SKAnalysis *analyze_sk_action(
    Side *side, Scorekeeper *sk, Obj *skact, AI_SKAnalysis *parent);
//! Analyze a scorekeeper.
extern AI_SKAnalysis *analyze_scorekeeper(Side *side, Scorekeeper *sk);
//! Analyze all the scorekeepers.
extern AI_SKAnalysis *analyze_scorekeepers(Side *side);
