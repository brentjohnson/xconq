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

    \note Nothing in this file should ever be required to implement an AI; 
	    everything here is optional.

    The way scorekeepers are analyzed by the code in this file is not too 
    tricky. We simply assume the consequent (action/outcome), and then 
    work with the antecedents (tests) to find out what conditions would 
    cause the consequent to be true (i.e., the action to be performed).
    The simplest antecedent is the value of truth, and this is the default 
    value unless an antecedent (test) is specified to be in correspondance 
    with the assumed consequent (action/outcome). For the purposes of 
    our analysis, 'when' and 'trigger' conditions may be regarded as 
    antecedents that are universal to the entire scorekeeper body, and 
    therefore are in correspondance with every consequent in the body.
*/

#include "conq.h"
#include "kernel.h"
#include "aiscore.h"

//! Initializer for AI_SKANode.

void
AI_SKANode::
init(AI_SKANode *skanode)
{
    assert_error(skanode, 
		 "Attempted to manipulate a NULL AI scorekeeper analysis node");
    skanode->kind = AI_SKAN_NONE;
    skanode->next = NULL;
}

//! Deep copy to a scorekeeper analysis node.

void
AI_SKANode::
copy_to(AI_SKANode *skanode)
{
    assert_error(skanode, 
		 "Attempted to copy to a NULL scorekeeper analysis node");
    skanode->kind = kind;
    skanode->next = next;
}

//! Initializer for AI_SKANode_UpropInt.

void
AI_SKANode_UpropInt::
init(AI_SKANode_UpropInt *skanode)
{
    assert_error(skanode, 
		 "Attempted to manipulate a NULL AI scorekeeper analysis node");
    AI_SKANode::init((AI_SKANode *)skanode);
    skanode->upropname = NULL;
    skanode->sides_and_utypes = create_packed_bool_table(numsides, numutypes);
    init_packed_bool_table(skanode->sides_and_utypes);
}

//! "Constructor" for AI_SKANode_UpropInt.

AI_SKANode_UpropInt *
AI_SKANode_UpropInt::
construct(const char *upropname, int *utypeary, SideMask sideset)
{
    AI_SKANode_UpropInt *skanode = NULL;
    Side *side = NULL;
    int u = NONUTYPE;

    assert_error(utypeary, "Attempted to access a NULL array of utypes");
    assert_error(upropname, "Attempted to use a NULL uprop name");
    skanode = (AI_SKANode_UpropInt *)xmalloc(sizeof(AI_SKANode_UpropInt));
    AI_SKANode_UpropInt::init(skanode);
    skanode->upropname = upropname;
    for_all_sides(side) {
	if (!side_in_set(side, sideset))
	  continue;
	for_all_unit_types(u) {
	    set_packed_bool(skanode->sides_and_utypes, side->id, u, 
			    utypeary[u]);
	}
    }
    return skanode;
}

//! Deep copy a scorekeeper analysis node.

void
AI_SKANode_UpropInt::
copy_to(AI_SKANode_UpropInt *skanode)
{
    Side *side = NULL;
    int u = NONUTYPE;
    
    assert_error(skanode, 
		 "Attempted to copy to a NULL scorekeeper analysis node");
    AI_SKANode::copy_to((AI_SKANode *)skanode);
    skanode->upropname = upropname;
    for_all_sides(side) {
	for_all_unit_types(u) {
	    set_packed_bool(skanode->sides_and_utypes, side->id, u,
			    get_packed_bool(sides_and_utypes, side->id, u));
	}
    }
}

//! Clone a scorekeeper analysis node.

AI_SKANode_UpropInt *
AI_SKANode_UpropInt::
clone()
{
    AI_SKANode_UpropInt *skanode = NULL;

    skanode = (AI_SKANode_UpropInt *)xmalloc(sizeof(AI_SKANode_UpropInt));
    AI_SKANode_UpropInt::init(skanode);
    copy_to(skanode);
    return skanode;
}

//! Clone chain of scorekeeper analysis nodes.

AI_SKANode_UpropInt *
AI_SKANode_UpropInt::
clone_list()
{
    AI_SKANode_UpropInt *skanode = NULL;

    skanode = clone();
    if (next)
      skanode->next = ((AI_SKANode_UpropInt *)next)->clone_list();
    return skanode;
}

//! "Destructor".

void
AI_SKANode_UpropInt::
destroy(AI_SKANode_UpropInt **skanodep)
{
    AI_SKANode_UpropInt *skanode = NULL;

    assert_error(skanodep && *skanodep, 
		 "Attempted to destroy a NULL scorekeeper analysis node");
    skanode = *skanodep;
    if (skanode->sides_and_utypes)
      destroy_packed_bool_table(skanode->sides_and_utypes);
    free(skanode);
    *skanodep = NULL;
}

//! Chain delete of scorekeeper nodes.

void
AI_SKANode_UpropInt::
destroy_list(AI_SKANode_UpropInt **skanodep)
{
    assert_error(skanodep && *skanodep, 
		 "Attempted to destroy a NULL scorekeeper analysis node");
    if ((*skanodep)->next)
      AI_SKANode_UpropInt::destroy_list(
	(AI_SKANode_UpropInt **)&((*skanodep)->next));
    AI_SKANode_UpropInt::destroy(skanodep);
}

//! Merge or insert a scorekeeper analysis node.

void 
AI_SKANode_UpropInt::
merge_or_insert_into_list(
        AI_SKANode_UpropInt **skanodep, const char *upropname, int *utypeary, 
	SideMask sideset)
{
    AI_SKANode_UpropInt *skanode = NULL;
    int u = NONUTYPE;
    Side *side = NULL;
    int matched = FALSE;

    assert_error(skanodep,
"Attempted to merge into an invalid scorekeeper analysis node list");
    assert_error(utypeary, "Attempted to access a NULL array of utypes");
    assert_error(upropname, "Attempted to use a NULL uprop name");
    if (*skanodep) {
	for (skanode = *skanodep; skanode; 
	     skanode = (AI_SKANode_UpropInt *)skanode->next) {
	    if (!strcmp(upropname, skanode->upropname)) {
		matched = TRUE;
		for_all_sides(side) {
		    if (!side_in_set(side, sideset))
		      continue;
		    for_all_unit_types(u) {
			set_packed_bool(skanode->sides_and_utypes, side->id, u,
					get_packed_bool(
					    skanode->sides_and_utypes, 
					    side->id, u) |
					utypeary[u]);
		    }
		}
		break;
	    }
	}
	if (!matched) {
	    for (skanode = *skanodep; skanode->next; 
		 skanode = (AI_SKANode_UpropInt *)skanode->next); 
	    skanode->next = 
		AI_SKANode_UpropInt::construct(upropname, utypeary, sideset);
	}
    }
    else {
      *skanodep = AI_SKANode_UpropInt::construct(upropname, utypeary, sideset);
    }
}

//! Initialize.

void
AI_SKANode_TestOp::
init(AI_SKANode_TestOp *skanode)
{
    assert_error(skanode, 
		 "Attempted to manipulate a NULL AI scorekeeper analysis node");
    AI_SKANode::init(skanode);
    skanode->op = AI_SKT_NONE;
}

//! "Constructor".

AI_SKANode_TestOp *
AI_SKANode_TestOp::
construct(int testop)
{
    AI_SKANode_TestOp *skanode = NULL;

    skanode = (AI_SKANode_TestOp *)xmalloc(sizeof(AI_SKANode_TestOp));
    AI_SKANode_TestOp::init(skanode);
    skanode->op = testop;
    return skanode;
}

//! Deep copy to a scorekeeper analysis node.

void
AI_SKANode_TestOp::
copy_to(AI_SKANode_TestOp *skanode)
{
    assert_error(skanode, 
		 "Attempted to copy to a NULL scorekeeper analysis node");
    AI_SKANode::copy_to((AI_SKANode *)skanode);
    skanode->op = op;
}

//! Clone a scorekeeper analysis node.

AI_SKANode_TestOp *
AI_SKANode_TestOp::
clone()
{
    AI_SKANode_TestOp *skanode = NULL;

    skanode = (AI_SKANode_TestOp *)xmalloc(sizeof(AI_SKANode_TestOp));
    AI_SKANode_TestOp::init(skanode);
    copy_to(skanode);
    return skanode;
}

//! Clone chain of scorekeeper analysis nodes.

AI_SKANode_TestOp *
AI_SKANode_TestOp::
clone_list()
{
    AI_SKANode_TestOp *skanode = NULL;

    skanode = clone();
    if (next)
      skanode->next = ((AI_SKANode_TestOp *)next)->clone_list();
    return skanode;
}

//! "Destructor".

void
AI_SKANode_TestOp::
destroy(AI_SKANode_TestOp **skanodep)
{
    AI_SKANode_TestOp *skanode = NULL;

    assert_error(skanodep && *skanodep, 
		 "Attempted to destroy a NULL scorekeeper analysis node");
    skanode = *skanodep;
    free(skanode);
    *skanodep = NULL;
}

//! Chain delete of scorekeeper nodes.

void
AI_SKANode_TestOp::
destroy_list(AI_SKANode_TestOp **skanodep)
{
    assert_error(skanodep && *skanodep, 
		 "Attempted to destroy a NULL scorekeeper analysis node");
    if ((*skanodep)->next)
      AI_SKANode_TestOp::destroy_list(
	(AI_SKANode_TestOp **)&((*skanodep)->next));
    AI_SKANode_TestOp::destroy(skanodep);
}

//! Initializer for AI_SKAnalysis.

void
AI_SKAnalysis::
init(AI_SKAnalysis *skanal)
{
    assert_error(skanal, "Attempted to manipulate a NULL scorekeeper analysis");
    skanal->win_eq_turn = (int *)xmalloc(numsides * sizeof(int));
    skanal->lose_eq_turn = (int *)xmalloc(numsides * sizeof(int));
    skanal->win_ge_turn = (int *)xmalloc(numsides * sizeof(int));
    skanal->lose_ge_turn = (int *)xmalloc(numsides * sizeof(int));
    skanal->win_gt_turn = (int *)xmalloc(numsides * sizeof(int));
    skanal->lose_gt_turn = (int *)xmalloc(numsides * sizeof(int));
    skanal->win_eq_score = (int *)xmalloc(numsides * sizeof(int));
    skanal->lose_eq_score = (int *)xmalloc(numsides * sizeof(int));
    skanal->win_ne_score = (int *)xmalloc(numsides * sizeof(int));
    skanal->lose_ne_score = (int *)xmalloc(numsides * sizeof(int));
    skanal->win_le_score = (int *)xmalloc(numsides * sizeof(int));
    skanal->lose_le_score = (int *)xmalloc(numsides * sizeof(int));
    skanal->win_lt_score = (int *)xmalloc(numsides * sizeof(int));
    skanal->lose_lt_score = (int *)xmalloc(numsides * sizeof(int));
    skanal->win_ge_score = (int *)xmalloc(numsides * sizeof(int));
    skanal->lose_ge_score = (int *)xmalloc(numsides * sizeof(int));
    skanal->win_gt_score = (int *)xmalloc(numsides * sizeof(int));
    skanal->lose_gt_score = (int *)xmalloc(numsides * sizeof(int));
}

//! "Constructor" for AI_SKAnalysis.

AI_SKAnalysis *
AI_SKAnalysis::
construct()
{
    AI_SKAnalysis *skanal = NULL;

    skanal = (AI_SKAnalysis *)xmalloc(sizeof(AI_SKAnalysis));
    AI_SKAnalysis::init(skanal);
    return skanal;
}

//! Deep copy to a scorekeeper analysis.

void
AI_SKAnalysis::
copy_to(AI_SKAnalysis *skanal)
{
    assert_error(skanal, "Attempted to copy to a NULL scorekeeper analysis");
    skanal->next = next;
    skanal->basic_flags = basic_flags;
    skanal->action_flags = action_flags;
    skanal->win_eq_some_turn = win_eq_some_turn;
    skanal->lose_eq_some_turn = lose_eq_some_turn;
    skanal->win_ge_some_turn = win_ge_some_turn;
    skanal->lose_ge_some_turn = lose_ge_some_turn;
    skanal->win_gt_some_turn = win_gt_some_turn;
    skanal->lose_gt_some_turn = lose_gt_some_turn;
    skanal->win_eq_some_score = win_eq_some_score;
    skanal->lose_eq_some_score = lose_eq_some_score;
    skanal->win_ne_some_score = win_ne_some_score;
    skanal->lose_ne_some_score = lose_ne_some_score;
    skanal->win_le_some_score = win_le_some_score;
    skanal->lose_le_some_score = lose_le_some_score;
    skanal->win_lt_some_score = win_lt_some_score;
    skanal->lose_lt_some_score = lose_lt_some_score;
    skanal->win_ge_some_score = win_ge_some_score;
    skanal->lose_ge_some_score = lose_ge_some_score;
    skanal->win_gt_some_score = win_gt_some_score;
    skanal->lose_gt_some_score = lose_gt_some_score;
    memcpy(skanal->win_eq_turn, win_eq_turn, numsides * sizeof(int));
    memcpy(skanal->lose_eq_turn, lose_eq_turn, numsides * sizeof(int));
    skanal->draw_eq_turn = draw_eq_turn;
    memcpy(skanal->win_ge_turn, win_ge_turn, numsides * sizeof(int));
    memcpy(skanal->lose_ge_turn, lose_ge_turn, numsides * sizeof(int));
    skanal->draw_ge_turn = draw_ge_turn;
    memcpy(skanal->win_gt_turn, win_gt_turn, numsides * sizeof(int));
    memcpy(skanal->lose_gt_turn, lose_gt_turn, numsides * sizeof(int));
    skanal->draw_gt_turn = draw_gt_turn;
    memcpy(skanal->win_eq_score, win_eq_score, numsides * sizeof(int));
    memcpy(skanal->lose_eq_score, lose_eq_score, numsides * sizeof(int));
    memcpy(skanal->win_ne_score, win_ne_score, numsides * sizeof(int));
    memcpy(skanal->lose_ne_score, lose_ne_score, numsides * sizeof(int));
    memcpy(skanal->win_le_score, win_le_score, numsides * sizeof(int));
    memcpy(skanal->lose_le_score, lose_le_score, numsides * sizeof(int));
    memcpy(skanal->win_lt_score, win_lt_score, numsides * sizeof(int));
    memcpy(skanal->lose_lt_score, lose_lt_score, numsides * sizeof(int));
    memcpy(skanal->win_ge_score, win_ge_score, numsides * sizeof(int));
    memcpy(skanal->lose_ge_score, lose_ge_score, numsides * sizeof(int));
    memcpy(skanal->win_gt_score, win_gt_score, numsides * sizeof(int));
    memcpy(skanal->lose_gt_score, lose_gt_score, numsides * sizeof(int));
    if (uprops_win_eq_some_sum)
      skanal->uprops_win_eq_some_sum = uprops_win_eq_some_sum->clone_list();
    if (uprops_lose_eq_some_sum)
      skanal->uprops_lose_eq_some_sum = uprops_lose_eq_some_sum->clone_list();
    if (uprops_win_ne_some_sum)
      skanal->uprops_win_ne_some_sum = uprops_win_ne_some_sum->clone_list();
    if (uprops_lose_ne_some_sum)
      skanal->uprops_lose_ne_some_sum = uprops_lose_ne_some_sum->clone_list();
    if (uprops_win_le_some_sum)
      skanal->uprops_win_le_some_sum = uprops_win_le_some_sum->clone_list();
    if (uprops_lose_le_some_sum)
      skanal->uprops_lose_le_some_sum = uprops_lose_le_some_sum->clone_list();
    if (uprops_win_lt_some_sum)
      skanal->uprops_win_lt_some_sum = uprops_win_lt_some_sum->clone_list();
    if (uprops_lose_lt_some_sum)
      skanal->uprops_lose_lt_some_sum = uprops_lose_lt_some_sum->clone_list();
    if (uprops_win_ge_some_sum)
      skanal->uprops_win_ge_some_sum = uprops_win_ge_some_sum->clone_list();
    if (uprops_lose_ge_some_sum)
      skanal->uprops_lose_ge_some_sum = uprops_lose_ge_some_sum->clone_list();
    if (uprops_win_gt_some_sum)
      skanal->uprops_win_gt_some_sum = uprops_win_gt_some_sum->clone_list();
    if (uprops_lose_gt_some_sum)
      skanal->uprops_lose_gt_some_sum = uprops_lose_gt_some_sum->clone_list();
    if (uprops_change_score)
      skanal->uprops_change_score = uprops_change_score->clone_list();
    if (opstack)
      skanal->opstack = opstack->clone_list();
}

//! Clone a scorekeeper analysis.

AI_SKAnalysis *
AI_SKAnalysis::
clone()
{
    AI_SKAnalysis *skanal = NULL;

    skanal = AI_SKAnalysis::construct();
    copy_to(skanal);
    return skanal;
}

//! "Destructor".

void
AI_SKAnalysis::
destroy(AI_SKAnalysis **skanalp)
{
    AI_SKAnalysis *skanal = NULL;

    assert_error(skanalp && *skanalp,
		 "Attempted to destroy a NULL scorekeeper analysis");
    skanal = *skanalp;
    if (skanal->win_eq_turn)
      free(skanal->win_eq_turn);
    if (skanal->lose_eq_turn)
      free(skanal->lose_eq_turn);
    if (skanal->win_ge_turn)
      free(skanal->win_ge_turn);
    if (skanal->lose_ge_turn)
      free(skanal->lose_ge_turn);
    if (skanal->win_gt_turn)
      free(skanal->win_gt_turn);
    if (skanal->lose_gt_turn)
      free(skanal->lose_gt_turn);
    if (skanal->win_eq_score)
      free(skanal->win_eq_score);
    if (skanal->lose_eq_score)
      free(skanal->lose_eq_score);
    if (skanal->win_ne_score)
      free(skanal->win_ne_score);
    if (skanal->lose_ne_score)
      free(skanal->lose_ne_score);
    if (skanal->win_le_score)
      free(skanal->win_le_score);
    if (skanal->lose_le_score)
      free(skanal->lose_le_score);
    if (skanal->win_lt_score)
      free(skanal->win_lt_score);
    if (skanal->lose_lt_score)
      free(skanal->lose_lt_score);
    if (skanal->win_ge_score)
      free(skanal->win_ge_score);
    if (skanal->lose_ge_score)
      free(skanal->lose_ge_score);
    if (skanal->win_gt_score)
      free(skanal->win_gt_score);
    if (skanal->lose_gt_score)
      free(skanal->lose_gt_score);
    if (skanal->uprops_win_eq_some_sum)
      AI_SKANode_UpropInt::destroy_list(&(skanal->uprops_win_eq_some_sum));
    if (skanal->uprops_lose_eq_some_sum)
      AI_SKANode_UpropInt::destroy_list(&(skanal->uprops_lose_eq_some_sum));
    if (skanal->uprops_win_ne_some_sum)
      AI_SKANode_UpropInt::destroy_list(&(skanal->uprops_win_ne_some_sum));
    if (skanal->uprops_lose_ne_some_sum)
      AI_SKANode_UpropInt::destroy_list(&(skanal->uprops_lose_ne_some_sum));
    if (skanal->uprops_win_le_some_sum)
      AI_SKANode_UpropInt::destroy_list(&(skanal->uprops_win_le_some_sum));
    if (skanal->uprops_lose_le_some_sum)
      AI_SKANode_UpropInt::destroy_list(&(skanal->uprops_lose_le_some_sum));
    if (skanal->uprops_win_lt_some_sum)
      AI_SKANode_UpropInt::destroy_list(&(skanal->uprops_win_lt_some_sum));
    if (skanal->uprops_lose_lt_some_sum)
      AI_SKANode_UpropInt::destroy_list(&(skanal->uprops_lose_lt_some_sum));
    if (skanal->uprops_win_ge_some_sum)
      AI_SKANode_UpropInt::destroy_list(&(skanal->uprops_win_ge_some_sum));
    if (skanal->uprops_lose_ge_some_sum)
      AI_SKANode_UpropInt::destroy_list(&(skanal->uprops_lose_ge_some_sum));
    if (skanal->uprops_win_gt_some_sum)
      AI_SKANode_UpropInt::destroy_list(&(skanal->uprops_win_gt_some_sum));
    if (skanal->uprops_lose_gt_some_sum)
      AI_SKANode_UpropInt::destroy_list(&(skanal->uprops_lose_gt_some_sum));
    if (skanal->uprops_change_score)
      AI_SKANode_UpropInt::destroy_list(&(skanal->uprops_change_score));
    if (skanal->opstack)
      AI_SKANode_TestOp::destroy_list(&(skanal->opstack));
    free(skanal);
    *skanalp = NULL;
}

//! Push onto operator stack.

void
AI_SKAnalysis::
push_test_op(int testop)
{
    AI_SKANode_TestOp *skanode = NULL;

    skanode = AI_SKANode_TestOp::construct(testop);
    skanode->next = opstack;
    opstack = skanode;
}

//! Pop from operator stack.

AI_SKANode_TestOp *
AI_SKAnalysis::
pop_test_op()
{
    AI_SKANode_TestOp *skanode = NULL;

    skanode = opstack;
    if (opstack)
      opstack = (AI_SKANode_TestOp *)opstack->next;
    return skanode;
}

//! Search for test operators, and return first one encountered.

int 
AI_SKAnalysis::
search_test_ops_for(int op)
{
    AI_SKANode_TestOp *curop = NULL;

    for (curop = opstack; curop; curop = (AI_SKANode_TestOp *)curop->next) {
	if (curop->op & op)
	  return curop->op;
    }
    return 0;
}

//! Analyze a scorekeeper 'sum-uprop' form.
/*! \todo Handle GDL memory leak involving uprop name. */

AI_SKAnalysis *
analyze_sk_sum_uprop_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL;
    const char *upropname = NULL;
    int u = NONUTYPE;
    Obj *uprop = lispnil;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(sktest, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    assert_error(parent->action_flags, 
		 "No action set for scorekeeper analysis");
    /* Handle other analyses on our chain, if any. */
    if (parent->next)
      analyze_sk_sum_uprop_test(side, sk, sktest, parent->next);
    /* Assign to parent scorekeeper analysis. */
    skanal = parent;
    /* Prep data structures. */
    for_all_unit_types(u)
      tmp_u_array[u] = FALSE;
    /* Parse utype(s). */
    fill_utype_array_from_lisp(tmp_u_array, car(sktest));
    /* Parse uprop. */
    uprop = cadr(sktest);
    while (symbolp(uprop) && boundp(uprop))
      uprop = eval_symbol(uprop);
    if (symbolp(uprop))
      uprop = new_string(c_string(uprop));
    if (stringp(uprop)) 
      upropname = c_string(uprop);
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tExpected a uprop string but found something else.\n");
	return skanal;
    }
    /* Analyze the context. */
    /* Check if 'sum-uprop' is in context of arithmetic comparisons. */
    if (skanal->search_test_ops_for(AI_SKT_ARITH_COMPARISONS)) {
	switch (skanal->action_flags) {
	  case AI_SKA_WIN:
	    switch(skanal->search_test_ops_for(AI_SKT_ARITH_COMPARISONS)) {
	      case AI_SKT_EQ:
		AI_SKANode_UpropInt::merge_or_insert_into_list(
		    &(skanal->uprops_win_eq_some_sum), 
		    upropname, tmp_u_array, sk->whomask);
		break;
	      case AI_SKT_NE:
		AI_SKANode_UpropInt::merge_or_insert_into_list(
		    &(skanal->uprops_win_ne_some_sum), 
		    upropname, tmp_u_array, sk->whomask);
		break;
	      case AI_SKT_LE:
		AI_SKANode_UpropInt::merge_or_insert_into_list(
		    &(skanal->uprops_win_le_some_sum), 
		    upropname, tmp_u_array, sk->whomask);
		break;
	      case AI_SKT_LT:
		AI_SKANode_UpropInt::merge_or_insert_into_list(
		    &(skanal->uprops_win_lt_some_sum), 
		    upropname, tmp_u_array, sk->whomask);
		break;
	      case AI_SKT_GE:
		AI_SKANode_UpropInt::merge_or_insert_into_list(
		    &(skanal->uprops_win_ge_some_sum), 
		    upropname, tmp_u_array, sk->whomask);
		break;
	      case AI_SKT_GT:
		AI_SKANode_UpropInt::merge_or_insert_into_list(
		    &(skanal->uprops_win_gt_some_sum), 
		    upropname, tmp_u_array, sk->whomask);
		break;
	      default:
		skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		DMprintf(
"\t'sum-uprop' is in a context that cannot be understood.\n");
		break;
	    }
	    break;
	  case AI_SKA_LOSE:
	    switch(skanal->search_test_ops_for(AI_SKT_ARITH_COMPARISONS)) {
	      case AI_SKT_EQ:
		AI_SKANode_UpropInt::merge_or_insert_into_list(
		    &(skanal->uprops_lose_eq_some_sum), 
		    upropname, tmp_u_array, sk->whomask);
		break;
	      case AI_SKT_NE:
		AI_SKANode_UpropInt::merge_or_insert_into_list(
		    &(skanal->uprops_lose_ne_some_sum), 
		    upropname, tmp_u_array, sk->whomask);
		break;
	      case AI_SKT_LE:
		AI_SKANode_UpropInt::merge_or_insert_into_list(
		    &(skanal->uprops_lose_le_some_sum), 
		    upropname, tmp_u_array, sk->whomask);
		break;
	      case AI_SKT_LT:
		AI_SKANode_UpropInt::merge_or_insert_into_list(
		    &(skanal->uprops_lose_lt_some_sum), 
		    upropname, tmp_u_array, sk->whomask);
		break;
	      case AI_SKT_GE:
		AI_SKANode_UpropInt::merge_or_insert_into_list(
		    &(skanal->uprops_lose_ge_some_sum), 
		    upropname, tmp_u_array, sk->whomask);
		break;
	      case AI_SKT_GT:
		AI_SKANode_UpropInt::merge_or_insert_into_list(
		    &(skanal->uprops_lose_gt_some_sum), 
		    upropname, tmp_u_array, sk->whomask);
		break;
	      default:
		skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		DMprintf(
"\t'sum-uprop' is in a context that cannot be understood.\n");
		break;
	    }
	    break;
	  default:
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    DMprintf(
"\t'sum-uprop' is in a context that cannot be understood.\n");
	    break;
	}
    }
    /* Else if 'sum-uprop' is probably changing a score. */
    else if (AI_SKA_CHANGE_SCORE & skanal->action_flags) {
	AI_SKANode_UpropInt::merge_or_insert_into_list(
	    &(skanal->uprops_change_score), upropname, tmp_u_array, 
	    sk->whomask);
    }
    /* Else, 'sum-uprop' is in context that we don't understand. */
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\t'sum-uprop' is in a context that cannot be understood.\n");
    }
    return skanal;
}

//! Analyze a scorekeeper 'turn' keyword.

AI_SKAnalysis *
analyze_sk_turn_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL;
    enum keywords keycode;
    Side *side2 = NULL;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(sktest, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    assert_error(parent->action_flags, 
		 "No action set for scorekeeper analysis");
    /* Handle other analyses on our chain, if any. */
    if (parent->next)
      analyze_sk_turn_test(side, sk, sktest, parent->next);
    /* Assign to parent scorekeeper analysis. */
    skanal = parent;
    /* Make sure that we are dealing with a keyword. */
    if (symbolp(sktest) && !boundp(sktest)) {
	keycode = (enum keywords)keyword_code(c_string(sktest));
	/* Make sure that it is the 'turn' keyword. */
	if (keycode != K_TURN) {
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    DMprintf("\tExpected 'turn' keyword but found '%s'.\n",
		     c_string(sktest));
	    return skanal;
	}
    }
    /* Else we are not dealing with a keyword. */
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tExpected 'turn' keyword but found something else.\n");
	return skanal;
    }
    /* Try to figure out what to note based on context. */
    if (skanal->search_test_ops_for(AI_SKT_ARITH_COMPARISONS)) {
	switch (skanal->action_flags) {
	  case AI_SKA_WIN:
	    if (skanal->search_test_ops_for(AI_SKT_EQ)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->win_eq_some_turn = 
			add_side_to_set(side2, skanal->win_eq_some_turn);
		}
	    }
	    else if (skanal->search_test_ops_for(AI_SKT_GE)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->win_ge_some_turn = 
			add_side_to_set(side2, skanal->win_ge_some_turn);
		}
	    }
	    else if (skanal->search_test_ops_for(AI_SKT_GT)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->win_gt_some_turn = 
			add_side_to_set(side2, skanal->win_gt_some_turn);
		}
	    }
	    else {
		skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		DMprintf(
"\tCannot understand 'turn' keyword in given context.\n");
	    }
	    break;
	  case AI_SKA_LOSE:
	    if (skanal->search_test_ops_for(AI_SKT_EQ)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->lose_eq_some_turn = 
			add_side_to_set(side2, skanal->lose_eq_some_turn);
		}
	    }
	    else if (skanal->search_test_ops_for(AI_SKT_GE)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->lose_ge_some_turn = 
			add_side_to_set(side2, skanal->lose_ge_some_turn);
		}
	    }
	    else if (skanal->search_test_ops_for(AI_SKT_GT)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->lose_gt_some_turn = 
			add_side_to_set(side2, skanal->lose_gt_some_turn);
		}
	    }
	    else {
		skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		DMprintf(
"\tCannot understand 'turn' keyword in given context.\n");
	    }
	    break;
	  default:
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    DMprintf("\tCannot understand 'turn' keyword in given context.\n");
	    break;
	}
    }
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tCannot understand 'turn' keyword in given context.\n");
	return skanal;
    }
    return skanal;
}

//! Analyze a scorekeeper 'score' form/keyword.

AI_SKAnalysis *
analyze_sk_score_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL;
    enum keywords keycode;
    Side *side2 = NULL;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(sktest, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    assert_error(parent->action_flags, 
		 "No action set for scorekeeper analysis");
    /* TODO: Handle a 'score' form. */
    /* Handle other analyses on our chain, if any. */
    if (parent->next)
      analyze_sk_score_test(side, sk, sktest, parent->next);
    /* Assign to parent scorekeeper analysis. */
    skanal = parent;
    /* Make sure that we are dealing with a keyword. */
    if (symbolp(sktest) && !boundp(sktest)) {
	keycode = (enum keywords)keyword_code(c_string(sktest));
	/* Make sure that it is the 'score' keyword. */
	if (keycode != K_SCORE) {
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    DMprintf("\tExpected 'score' keyword but found '%s'.\n",
		     c_string(sktest));
	    return skanal;
	}
    }
    /* Else we are not dealing with a keyword. */
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tExpected 'score' keyword but found something else.\n");
	return skanal;
    }
    /* Try to figure out what to note based on context. */
    if (skanal->search_test_ops_for(AI_SKT_ARITH_COMPARISONS)) {
	switch (skanal->action_flags) {
	  case AI_SKA_WIN:
	    if (skanal->search_test_ops_for(AI_SKT_EQ)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->win_eq_some_score = 
			add_side_to_set(side2, skanal->win_eq_some_score);
		}
	    }
	    else if (skanal->search_test_ops_for(AI_SKT_NE)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->win_ne_some_score = 
			add_side_to_set(side2, skanal->win_ne_some_score);
		}
	    }
	    else if (skanal->search_test_ops_for(AI_SKT_LE)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->win_le_some_score = 
			add_side_to_set(side2, skanal->win_le_some_score);
		}
	    }
	    else if (skanal->search_test_ops_for(AI_SKT_LT)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->win_lt_some_score = 
			add_side_to_set(side2, skanal->win_lt_some_score);
		}
	    }
	    else if (skanal->search_test_ops_for(AI_SKT_GE)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->win_ge_some_score = 
			add_side_to_set(side2, skanal->win_ge_some_score);
		}
	    }
	    else if (skanal->search_test_ops_for(AI_SKT_GT)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->win_gt_some_score = 
			add_side_to_set(side2, skanal->win_gt_some_score);
		}
	    }
	    else {
		skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		DMprintf(
"\tCannot understand 'score' keyword in given context.\n");
	    }
	    break;
	  case AI_SKA_LOSE:
	    if (skanal->search_test_ops_for(AI_SKT_EQ)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->lose_eq_some_score = 
			add_side_to_set(side2, skanal->lose_eq_some_score);
		}
	    }
	    else if (skanal->search_test_ops_for(AI_SKT_NE)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->lose_ne_some_score = 
			add_side_to_set(side2, skanal->lose_ne_some_score);
		}
	    }
	    else if (skanal->search_test_ops_for(AI_SKT_LE)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->lose_le_some_score = 
			add_side_to_set(side2, skanal->lose_le_some_score);
		}
	    }
	    else if (skanal->search_test_ops_for(AI_SKT_LT)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->lose_lt_some_score = 
			add_side_to_set(side2, skanal->lose_lt_some_score);
		}
	    }
	    else if (skanal->search_test_ops_for(AI_SKT_GE)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->lose_ge_some_score = 
			add_side_to_set(side2, skanal->lose_ge_some_score);
		}
	    }
	    else if (skanal->search_test_ops_for(AI_SKT_GT)) {
		for_all_sides(side2) {
		    if (!side_in_set(side2, sk->whomask))
		      continue;
		    skanal->lose_gt_some_score = 
			add_side_to_set(side2, skanal->lose_gt_some_score);
		}
	    }
	    else {
		skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		DMprintf(
"\tCannot understand 'score' keyword in given context.\n");
	    }
	    break;
	  default:
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    DMprintf("\tCannot understand 'score' keyword in given context.\n");
	    break;
	}
    }
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tCannot understand 'score' keyword in given context.\n");
	return skanal;
    }
    return skanal;
}

//! Analyze a scorekeeper arithmetic operation form.
/*! \note Mostly just let things fall through here for now. */

AI_SKAnalysis *
analyze_sk_arith_op_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL;
    Obj *keyobj = lispnil;
    enum keywords keycode;
    AI_SKANode_TestOp *tmpop = NULL;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(sktest, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    assert_error(parent->action_flags, 
		 "No action set for scorekeeper analysis");
    /* Handle other analyses on our chain, if any. */
    if (parent->next)
      analyze_sk_arith_op_test(side, sk, sktest, parent->next);
    /* Assign to parent scorekeeper analysis. */
    skanal = parent;
    /* Handle different arithmetic operations. */
    keyobj = car(sktest);
    if (symbolp(keyobj) && !boundp(keyobj)) {
	keycode = (enum keywords)keyword_code(c_string(keyobj));
	switch (keycode) {
	  case K_PLUS:
	    skanal->push_test_op(AI_SKT_PLUS);
	    skanal = analyze_sk_test(side, sk, cadr(sktest), skanal);
	    assert_warning(skanal->opstack 
			   && (AI_SKT_PLUS == skanal->opstack->op),
"Scorekeeper analysis operator stack is misaligned");
	    tmpop = skanal->pop_test_op();
	    AI_SKANode_TestOp::destroy(&tmpop);
	    break;
	  case K_MINUS:
	    skanal->push_test_op(AI_SKT_MINUS);
	    skanal = analyze_sk_test(side, sk, cadr(sktest), skanal);
	    assert_warning(skanal->opstack 
			   && (AI_SKT_MINUS == skanal->opstack->op),
"Scorekeeper analysis operator stack is misaligned");
	    tmpop = skanal->pop_test_op();
	    AI_SKANode_TestOp::destroy(&tmpop);
	    break;
	  case K_MULTIPLY:
	    skanal->push_test_op(AI_SKT_MULTIPLY);
	    skanal = analyze_sk_test(side, sk, cadr(sktest), skanal);
	    assert_warning(skanal->opstack 
			   && (AI_SKT_MULTIPLY == skanal->opstack->op),
"Scorekeeper analysis operator stack is misaligned");
	    tmpop = skanal->pop_test_op();
	    AI_SKANode_TestOp::destroy(&tmpop);
	    break;
	  case K_DIVIDE:
	    skanal->push_test_op(AI_SKT_DIVIDE);
	    skanal = analyze_sk_test(side, sk, cadr(sktest), skanal);
	    assert_warning(skanal->opstack 
			   && (AI_SKT_DIVIDE == skanal->opstack->op),
"Scorekeeper analysis operator stack is misaligned");
	    tmpop = skanal->pop_test_op();
	    AI_SKANode_TestOp::destroy(&tmpop);
	    break;
	  default:
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    DMprintf("\tExpected an arithmetic operator but found '%s'.\n",
		     c_string(keyobj));
	    break;
	} /* arithmetic operators */
    } /* bound symbol */
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tExpected an arithmetic operator but something else.\n");
    }
    return skanal;
}

//! Analyze a scorekeeper arithmetic '>' form.

AI_SKAnalysis *
analyze_sk_arith_gt_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL;
    Obj *first = lispnil, *second = lispnil;
    enum keywords keycode;
    Side *side2 = NULL;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(sktest, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    assert_error(parent->action_flags, 
		 "No action set for scorekeeper analysis");
    /* Handle other analyses on our chain, if any. */
    if (parent->next)
      analyze_sk_arith_gt_test(side, sk, sktest, parent->next);
    /* Assign to parent scorekeeper analysis. */
    skanal = parent;
    /* If test form is a list... */
    if (consp(sktest)) {
	/* Lookahead on items. */
	first = car(sktest);
	second = cadr(sktest);
	/* Resolve any symbols. */
	while (symbolp(first) && boundp(first))
	  first = eval_symbol(first);
	while (symbolp(second) && boundp(second))
	  second = eval_symbol(second);
	/* If first item is a symbol, and second item is a number... */
	if (symbolp(first) && numberp(second)) {
	    /* If symbol is not bound... */
	    if (!boundp(first)) {
		keycode = (enum keywords)keyword_code(c_string(first));
		switch (keycode) {
		  case K_TURN:
		    for_all_sides(side2) {
			if (!side_in_set(side2, sk->whomask)) 
			  continue;
			switch (skanal->action_flags) {
			  case AI_SKA_WIN:
			    skanal->win_gt_turn[side->id] = c_number(second);
			    break;
			  case AI_SKA_LOSE:
			    skanal->lose_gt_turn[side->id] = c_number(second);
			    break;
			  case AI_SKA_DRAW:
			    skanal->draw_gt_turn = c_number(second);
			    return skanal;
			  /* Other cases? */
			  default:
			    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
			    DMprintf(
"\tCannot understand action to take if turn is > %d.\n",
				     c_number(second));
			    return skanal;
			}
		    }
		    break;
		  case K_SCORE:
		    for_all_sides(side2) {
			if (!side_in_set(side2, sk->whomask)) 
			  continue;
			switch (skanal->action_flags) {
			  case AI_SKA_WIN:
			    skanal->win_gt_score[side->id] = c_number(second);
			    break;
			  case AI_SKA_LOSE:
			    skanal->lose_gt_score[side->id] = c_number(second);
			    break;
			  /* Other cases? */
			  default:
			    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
			    DMprintf(
"\tCannot understand action to take if score is > %d.\n",
				     c_number(second));
			    return skanal;
			}
		    }
		    break;
		  default:
		    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		    assert_warning(FALSE,
"Expecting a specific keyword but found something else");
		    break;
		}
	    } /* lookahead symbol is not bound */
	} /* symbol followed by number in lookahead */
	/* If first item is a list... */
	else if (consp(first)) {
	    skanal = analyze_sk_test(side, sk, first, skanal);
	} /* lookahead is list */
	/* Else we don't understand. */
	else {
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    DMprintf("\tCould not understand '>' list.\n");
	}
    } /* list */
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tExpected a '>' list but did not find one.\n");
    }
    return skanal;
}

//! Analyze a scorekeeper arithmetic '>=' form.

AI_SKAnalysis *
analyze_sk_arith_ge_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL;
    Obj *first = lispnil, *second = lispnil;
    enum keywords keycode;
    Side *side2 = NULL;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(sktest, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    assert_error(parent->action_flags, 
		 "No action set for scorekeeper analysis");
    /* Handle other analyses on our chain, if any. */
    if (parent->next)
      analyze_sk_arith_ge_test(side, sk, sktest, parent->next);
    /* Assign to parent scorekeeper analysis. */
    skanal = parent;
    /* If test form is a list... */
    if (consp(sktest)) {
	/* Lookahead on items. */
	first = car(sktest);
	second = cadr(sktest);
	/* Resolve any symbols. */
	while (symbolp(first) && boundp(first))
	  first = eval_symbol(first);
	while (symbolp(second) && boundp(second))
	  second = eval_symbol(second);
	/* If first item is a symbol, and second item is a number... */
	if (symbolp(first) && numberp(second)) {
	    /* If symbol is not bound... */
	    if (!boundp(first)) {
		keycode = (enum keywords)keyword_code(c_string(first));
		switch (keycode) {
		  case K_TURN:
		    for_all_sides(side2) {
			if (!side_in_set(side2, sk->whomask)) 
			  continue;
			switch (skanal->action_flags) {
			  case AI_SKA_WIN:
			    skanal->win_ge_turn[side->id] = c_number(second);
			    break;
			  case AI_SKA_LOSE:
			    skanal->lose_ge_turn[side->id] = c_number(second);
			    break;
			  case AI_SKA_DRAW:
			    skanal->draw_ge_turn = c_number(second);
			    return skanal;
			  /* Other cases? */
			  default:
			    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
			    DMprintf(
"\tCannot understand action to take if turn is >= %d.\n",
				     c_number(second));
			    return skanal;
			}
		    }
		    break;
		  case K_SCORE:
		    for_all_sides(side2) {
			if (!side_in_set(side2, sk->whomask)) 
			  continue;
			switch (skanal->action_flags) {
			  case AI_SKA_WIN:
			    skanal->win_ge_score[side->id] = c_number(second);
			    break;
			  case AI_SKA_LOSE:
			    skanal->lose_ge_score[side->id] = c_number(second);
			    break;
			  /* Other cases? */
			  default:
			    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
			    DMprintf(
"\tCannot understand action to take if score is >= %d.\n",
				     c_number(second));
			    return skanal;
			}
		    }
		    break;
		  default:
		    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		    assert_warning(FALSE,
"Expecting a specific keyword but found something else");
		    break;
		}
	    } /* lookahead symbol is not bound */
	} /* symbol followed by number in lookahead */
	/* If first item is a list... */
	else if (consp(first)) {
	    skanal = analyze_sk_test(side, sk, first, skanal);
	} /* lookahead is list */
	/* Else we don't understand. */
	else {
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    DMprintf("\tCould not understand '>=' list.\n");
	}
    } /* list */
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tExpected a '>=' list but did not find one.\n");
    }
    return skanal;
}

//! Analyze a scorekeeper arithmetic '<' form.

AI_SKAnalysis *
analyze_sk_arith_lt_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL;
    Obj *first = lispnil, *second = lispnil;
    enum keywords keycode;
    Side *side2 = NULL;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(sktest, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    assert_error(parent->action_flags, 
		 "No action set for scorekeeper analysis");
    /* Handle other analyses on our chain, if any. */
    if (parent->next)
      analyze_sk_arith_lt_test(side, sk, sktest, parent->next);
    /* Assign to parent scorekeeper analysis. */
    skanal = parent;
    /* If test form is a list... */
    if (consp(sktest)) {
	/* Lookahead on items. */
	first = car(sktest);
	second = cadr(sktest);
	/* Resolve any symbols. */
	while (symbolp(first) && boundp(first))
	  first = eval_symbol(first);
	while (symbolp(second) && boundp(second))
	  second = eval_symbol(second);
	/* If first item is a symbol, and second item is a number... */
	if (symbolp(first) && numberp(second)) {
	    /* If symbol is not bound... */
	    if (!boundp(first)) {
		keycode = (enum keywords)keyword_code(c_string(first));
		switch (keycode) {
		  case K_SCORE:
		    for_all_sides(side2) {
			if (!side_in_set(side2, sk->whomask)) 
			  continue;
			switch (skanal->action_flags) {
			  case AI_SKA_WIN:
			    skanal->win_lt_score[side->id] = c_number(second);
			    break;
			  case AI_SKA_LOSE:
			    skanal->lose_lt_score[side->id] = c_number(second);
			    break;
			  /* Other cases? */
			  default:
			    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
			    DMprintf(
"\tCannot understand action to take if score is < %d.\n",
				     c_number(second));
			    return skanal;
			}
		    }
		    break;
		  default:
		    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		    assert_warning(FALSE,
"Expecting a specific keyword but found something else");
		    break;
		}
	    } /* lookahead symbol is not bound */
	} /* symbol followed by number in lookahead */
	/* If first item is a list... */
	else if (consp(first)) {
	    skanal = analyze_sk_test(side, sk, first, skanal);
	} /* lookahead is list */
	/* Else we don't understand. */
	else {
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    DMprintf("\tCould not understand '<' list.\n");
	}
    } /* list */
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tExpected a '<' list but did not find one.\n");
    }
    return skanal;
}

//! Analyze a scorekeeper arithmetic '<=' form.

AI_SKAnalysis *
analyze_sk_arith_le_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL;
    Obj *first = lispnil, *second = lispnil;
    enum keywords keycode;
    Side *side2 = NULL;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(sktest, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    assert_error(parent->action_flags, 
		 "No action set for scorekeeper analysis");
    /* Handle other analyses on our chain, if any. */
    if (parent->next)
      analyze_sk_arith_le_test(side, sk, sktest, parent->next);
    /* Assign to parent scorekeeper analysis. */
    skanal = parent;
    /* If test form is a list... */
    if (consp(sktest)) {
	/* Lookahead on items. */
	first = car(sktest);
	second = cadr(sktest);
	/* Resolve any symbols. */
	while (symbolp(first) && boundp(first))
	  first = eval_symbol(first);
	while (symbolp(second) && boundp(second))
	  second = eval_symbol(second);
	/* If first item is a symbol, and second item is a number... */
	if (symbolp(first) && numberp(second)) {
	    /* If symbol is not bound... */
	    if (!boundp(first)) {
		keycode = (enum keywords)keyword_code(c_string(first));
		switch (keycode) {
		  case K_SCORE:
		    for_all_sides(side2) {
			if (!side_in_set(side2, sk->whomask)) 
			  continue;
			switch (skanal->action_flags) {
			  case AI_SKA_WIN:
			    skanal->win_le_score[side->id] = c_number(second);
			    break;
			  case AI_SKA_LOSE:
			    skanal->lose_le_score[side->id] = c_number(second);
			    break;
			  /* Other cases? */
			  default:
			    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
			    DMprintf(
"\tCannot understand action to take if score is <= %d.\n",
				     c_number(second));
			    return skanal;
			}
		    }
		    break;
		  default:
		    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		    assert_warning(FALSE,
"Expecting a specific keyword but found something else");
		    break;
		}
	    } /* lookahead symbol is not bound */
	} /* symbol followed by number in lookahead */
	/* If first item is a list... */
	else if (consp(first)) {
	    skanal = analyze_sk_test(side, sk, first, skanal);
	} /* lookahead is list */
	/* Else we don't understand. */
	else {
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    DMprintf("\tCould not understand '<=' list.\n");
	}
    } /* list */
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tExpected a '<=' list but did not find one.\n");
    }
    return skanal;
}

//! Analyze a scorekeeper arithmetic '/=' form.

AI_SKAnalysis *
analyze_sk_arith_ne_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL;
    Obj *first = lispnil, *second = lispnil;
    enum keywords keycode;
    Side *side2 = NULL;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(sktest, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    assert_error(parent->action_flags, 
		 "No action set for scorekeeper analysis");
    /* Handle other analyses on our chain, if any. */
    if (parent->next)
      analyze_sk_arith_ne_test(side, sk, sktest, parent->next);
    /* Assign to parent scorekeeper analysis. */
    skanal = parent;
    /* If test form is a list... */
    if (consp(sktest)) {
	/* Lookahead on items. */
	first = car(sktest);
	second = cadr(sktest);
	/* Resolve any symbols. */
	while (symbolp(first) && boundp(first))
	  first = eval_symbol(first);
	while (symbolp(second) && boundp(second))
	  second = eval_symbol(second);
	/* If first item is a symbol, and second item is a number... */
	if (symbolp(first) && numberp(second)) {
	    /* If symbol is not bound... */
	    if (!boundp(first)) {
		keycode = (enum keywords)keyword_code(c_string(first));
		switch (keycode) {
		  case K_SCORE:
		    for_all_sides(side2) {
			if (!side_in_set(side2, sk->whomask)) 
			  continue;
			switch (skanal->action_flags) {
			  case AI_SKA_WIN:
			    skanal->win_ne_score[side->id] = c_number(second);
			    break;
			  case AI_SKA_LOSE:
			    skanal->lose_ne_score[side->id] = c_number(second);
			    break;
			  /* Other cases? */
			  default:
			    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
			    DMprintf(
"\tCannot understand action to take if score is not equal to %d.\n",
				     c_number(second));
			    return skanal;
			}
		    }
		    break;
		  default:
		    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		    assert_warning(FALSE,
"Expecting a specific keyword but found something else");
		    break;
		}
	    } /* lookahead symbol is not bound */
	} /* symbol followed by number in lookahead */
	/* If first item is a list... */
	else if (consp(first)) {
	    skanal = analyze_sk_test(side, sk, first, skanal);
	} /* lookahead is list */
	/* Else we don't understand. */
	else {
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    DMprintf("\tCould not understand '/=' list.\n");
	}
    } /* list */
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tExpected a '/=' list but did not find one.\n");
    }
    return skanal;
}

//! Analyze a scorekeeper arithmetic '=' form.

AI_SKAnalysis *
analyze_sk_arith_eq_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL;
    Obj *first = lispnil, *second = lispnil, *tmp = lispnil;
    enum keywords keycode;
    Side *side2 = NULL;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(sktest, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    assert_error(parent->action_flags, 
		 "No action set for scorekeeper analysis");
    /* Handle other analyses on our chain, if any. */
    if (parent->next)
      analyze_sk_arith_eq_test(side, sk, sktest, parent->next);
    /* Assign to parent scorekeeper analysis. */
    skanal = parent;
    /* If test form is a list... */
    if (consp(sktest)) {
	/* Lookahead on items. */
	first = car(sktest);
	second = cadr(sktest);
	/* Resolve any symbols. */
	while (symbolp(first) && boundp(first))
	  first = eval_symbol(first);
	while (symbolp(second) && boundp(second))
	  second = eval_symbol(second);
	/* Swap ordering of first two items, if necessary. */
	if (numberp(first)) {
	    tmp = first;
	    first = second;
	    second = tmp;
	}
	/* If first item is a symbol... */
	if (symbolp(first)) {
	    /* If symbol is not bound... */
	    if (!boundp(first)) {
		keycode = (enum keywords)keyword_code(c_string(first));
		switch (keycode) {
		  case K_SCORE:
		    for_all_sides(side2) {
			if (!side_in_set(side2, sk->whomask)) 
			  continue;
			switch (skanal->action_flags) {
			  case AI_SKA_WIN:
			    skanal->win_eq_score[side2->id] = c_number(second);
			    break;
			  case AI_SKA_LOSE:
			    skanal->lose_eq_score[side2->id] = c_number(second);
			    break;
			  /* Other cases? */
			  default:
			    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
			    DMprintf(
"\tCannot understand action to take if score is equal to %d.\n",
				     c_number(second));
			    return skanal;
			} /* action cases */
		    } /* for all sides */
		    break;
		  case K_TURN:
		    for_all_sides(side2) {
			if (!side_in_set(side2, sk->whomask)) 
			  continue;
			switch (skanal->action_flags) {
			  case AI_SKA_WIN:
			    skanal->win_eq_turn[side2->id] = c_number(second);
			    break;
			  case AI_SKA_LOSE:
			    skanal->lose_eq_turn[side2->id] = c_number(second);
			    break;
			  case AI_SKA_DRAW:
			    skanal->draw_eq_turn = c_number(second);
			    break;
			  default:
			    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
			    DMprintf(
"\tCannot understand action to take if turn is equal to %d.\n",
				     c_number(second));
			    return skanal;
			}
		    }
		    break;
		  default:
		    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		    assert_warning(FALSE,
"Expecting a specific keyword but found something else");
		    break;
		}
	    } /* bound symbol */
	} /* symbol */
	/* Else if first item is a list, then recurse into it. */
	/* (Ignore other list items for now.) */
	else if (consp(first)) {
	    skanal = analyze_sk_test(side, sk, first, skanal);
	} /* list */
	/* Else we don't understand first item. */
	else {
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    DMprintf("\tCould not understand '=' list.\n");
	}
    } /* list */
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tExpected an '=' list but did not find one.\n");
    }
    return skanal;
}

//! Analyze a scorekeeper arithmetic comparison form.

AI_SKAnalysis *
analyze_sk_arith_comp_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL;
    Obj *keyobj = lispnil;
    enum keywords keycode;
    AI_SKANode_TestOp *tmpop = NULL;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(sktest, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    assert_error(parent->action_flags, 
		 "No action set for scorekeeper analysis");
    /* Handle other analyses on our chain, if any. */
    if (parent->next)
      analyze_sk_arith_comp_test(side, sk, sktest, parent->next);
    /* Assign to parent scorekeeper analysis. */
    skanal = parent;
    /* Handle different arithmetic comparisons. */
    keyobj = car(sktest);
    if (symbolp(keyobj) && !boundp(keyobj)) {
	keycode = (enum keywords)keyword_code(c_string(keyobj));
	switch (keycode) {
	  case K_EQ:
	    /* NOT of EQ is NE */
	    if (skanal->opstack && (skanal->opstack->op == AI_SKT_NOT)) {
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		skanal->push_test_op(AI_SKT_NE);
		skanal = 
		    analyze_sk_arith_ne_test(side, sk, cdr(sktest), skanal);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_NE),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		skanal->push_test_op(AI_SKT_NOT);
	    }
	    /* Else normal behavior. */
	    else {
		skanal->push_test_op(AI_SKT_EQ);
		skanal = 
		    analyze_sk_arith_eq_test(side, sk, cdr(sktest), skanal);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_EQ),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
	    }
	    break;
	  case K_NE:
	    /* NOT of NE is EQ */
	    if (skanal->opstack && (skanal->opstack->op == AI_SKT_NOT)) {
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		skanal->push_test_op(AI_SKT_EQ);
		skanal = 
		    analyze_sk_arith_eq_test(side, sk, cdr(sktest), skanal);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_EQ),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		skanal->push_test_op(AI_SKT_NOT);
	    }
	    /* Else normal behavior. */
	    else {
		skanal->push_test_op(AI_SKT_NE);
		skanal = 
		    analyze_sk_arith_ne_test(side, sk, cdr(sktest), skanal);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_NE),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
	    }
	    break;
	  case K_LE:
	    /* NOT of LE is GT */
	    if (skanal->opstack && (skanal->opstack->op == AI_SKT_NOT)) {
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		skanal->push_test_op(AI_SKT_GT);
		skanal = 
		    analyze_sk_arith_gt_test(side, sk, cdr(sktest), skanal);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_GT),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		skanal->push_test_op(AI_SKT_NOT);
	    }
	    /* Else normal behavior. */
	    else {
		skanal->push_test_op(AI_SKT_LE);
		skanal = 
		    analyze_sk_arith_le_test(side, sk, cdr(sktest), skanal);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_LE),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
	    }
	    break;
	  case K_LT:
	    /* NOT of LT is GE */
	    if (skanal->opstack && (skanal->opstack->op == AI_SKT_NOT)) {
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		skanal->push_test_op(AI_SKT_GE);
		skanal = 
		    analyze_sk_arith_ge_test(side, sk, cdr(sktest), skanal);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_GE),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		skanal->push_test_op(AI_SKT_NOT);
	    }
	    /* Else normal behavior. */
	    else {
		skanal->push_test_op(AI_SKT_LT);
		skanal = 
		    analyze_sk_arith_lt_test(side, sk, cdr(sktest), skanal);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_LT),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
	    }
	    break;
	  case K_GE:
	    /* NOT of GE is LT */
	    if (skanal->opstack && (skanal->opstack->op == AI_SKT_NOT)) {
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		skanal->push_test_op(AI_SKT_LT);
		skanal = 
		    analyze_sk_arith_lt_test(side, sk, cdr(sktest), skanal);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_LT),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		skanal->push_test_op(AI_SKT_NOT);
	    }
	    /* Else normal behavior. */
	    else {
		skanal->push_test_op(AI_SKT_GE);
		skanal = 
		    analyze_sk_arith_ge_test(side, sk, cdr(sktest), skanal);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_GE),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
	    }
	    break;
	  case K_GT:
	    /* NOT of GT is LE */
	    if (skanal->opstack && (skanal->opstack->op == AI_SKT_NOT)) {
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		skanal->push_test_op(AI_SKT_LE);
		skanal = 
		    analyze_sk_arith_le_test(side, sk, cdr(sktest), skanal);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_LE),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		skanal->push_test_op(AI_SKT_NOT);
	    }
	    /* Else normal behavior. */
	    else {
		skanal->push_test_op(AI_SKT_GT);
		skanal = 
		    analyze_sk_arith_gt_test(side, sk, cdr(sktest), skanal);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_GT),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
	    }
	    break;
	  default:
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    assert_warning(FALSE,
"Expecting arithmetic comparison but found something else");
	    break;
	} /* keycodes */
    } /* unbound symbol */
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	assert_warning(symbolp(keyobj) && !boundp(keyobj),
"Expecting arithmetic comparison but found something else");
    }
    return skanal;
}

//! Analyze a scorekeeper boolean 'or' form.

AI_SKAnalysis *
analyze_sk_bool_or_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL, *skanal2 = NULL, *skanal3 = NULL;
    Obj *rest = lispnil;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(sktest, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    assert_error(parent->action_flags, 
		 "No action set for scorekeeper analysis");
    /* Handle other analyses on our chain, if any. */
    if (parent->next)
      analyze_sk_bool_or_test(side, sk, sktest, parent->next);
    /* Assign to parent scorekeeper analysis. */
    skanal = parent;
    if (consp(sktest)) {
	for_all_list(sktest, rest) {
	    skanal = parent->clone();
	    if (!skanal3)
	      skanal3 = skanal;
	    skanal = analyze_sk_test(side, sk, car(rest), skanal);
	    if (skanal2) {
		for (; skanal2->next; skanal2 = skanal2->next);
		skanal2->next = skanal;
	    }
	    skanal2 = skanal;
	}
	AI_SKAnalysis::destroy(&parent);
	skanal = skanal3;
    } /* list */
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tExpected an 'or' list but did not find one.\n");
    }
    return skanal;
}

//! Analyze a scorekeeper boolean 'and' form.

AI_SKAnalysis *
analyze_sk_bool_and_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL;
    Obj *rest = lispnil;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(sktest, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    assert_error(parent->action_flags, 
		 "No action set for scorekeeper analysis");
    /* Handle other analyses on our chain, if any. */
    if (parent->next)
      analyze_sk_bool_and_test(side, sk, sktest, parent->next);
    /* Assign to parent scorekeeper analysis. */
    skanal = parent;
    if (consp(sktest)) {
	for_all_list(sktest, rest) 
	  skanal = analyze_sk_test(side, sk, car(rest), skanal);
    } /* list */
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tExpected an 'and' list but did not find one.\n");
    }
    return skanal;
}

//! Analyze a scorekeeper boolean comparison form.

AI_SKAnalysis *
analyze_sk_bool_comp_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL;
    Obj *keyobj = lispnil;
    enum keywords keycode;
    AI_SKANode_TestOp *bookmark = NULL, *tmpop = NULL;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(sktest, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    assert_error(parent->action_flags, 
		 "No action set for scorekeeper analysis");
    /* Handle other analyses on our chain, if any. */
    if (parent->next)
      analyze_sk_bool_comp_test(side, sk, sktest, parent->next);
    /* Assign to parent scorekeeper analysis. */
    skanal = parent;
    /* Handle different boolean comparisons. */
    keyobj = car(sktest);
    if (symbolp(keyobj) && !boundp(keyobj)) {
	keycode = (enum keywords)keyword_code(c_string(keyobj));
	switch (keycode) {
	  case K_NOT:
	    /* If two NOTs, then cancel them. */
	    if (skanal->opstack && (skanal->opstack->op == AI_SKT_NOT)) 
	      bookmark = skanal->pop_test_op();
	    /* Else, push a NOT onto opstack. */ 
	    else
	      skanal->push_test_op(AI_SKT_NOT);
	    /* Analyze argument. */
	    skanal = analyze_sk_test(side, sk, cdr(sktest), skanal);
	    /* Restore bookmark. */
	    if (bookmark) {
		AI_SKANode_TestOp::destroy(&bookmark);
		skanal->push_test_op(AI_SKT_NOT);
	    }
	    /* Check opstack alignment. */
	    else {
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_NOT),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
	    }
	    break;
	  case K_AND:
	    /* NOT of an AND is an OR of NOTs. */
	    if (skanal->opstack && (skanal->opstack->op == AI_SKT_NOT)) {
		/* Prepare operation. */
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		skanal->push_test_op(AI_SKT_OR);
		skanal->push_test_op(AI_SKT_NOT);
		/* Call the disjunction function. */
		skanal = analyze_sk_bool_or_test(side, sk, cdr(sktest), skanal);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_NOT),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_OR),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		skanal->push_test_op(AI_SKT_NOT);
	    }
	    /* Else behave normally. */
	    else {
		/* Prepare operation. */
		skanal->push_test_op(AI_SKT_AND);
		/* Call the conjunction function. */
		skanal = 
		    analyze_sk_bool_and_test(side, sk, cdr(sktest), skanal);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_AND),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
	    }
	    break;
	  case K_OR:
	    /* NOT of an OR is an AND of NOTs. */
	    if (skanal->opstack && (skanal->opstack->op == AI_SKT_NOT)) {
		/* Prepare operation. */
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		skanal->push_test_op(AI_SKT_AND);
		skanal->push_test_op(AI_SKT_NOT);
		/* Call the disjunction function. */
		skanal = 
		    analyze_sk_bool_and_test(side, sk, cdr(sktest), skanal);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_NOT),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_AND),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
		skanal->push_test_op(AI_SKT_NOT);
	    }
	    /* Else behave normally. */
	    else {
		/* Prepare operation. */
		skanal->push_test_op(AI_SKT_OR);
		/* Call the conjunction function. */
		skanal = analyze_sk_bool_or_test(side, sk, cdr(sktest), skanal);
		assert_warning(skanal->opstack 
			       && (skanal->opstack->op == AI_SKT_OR),
"Scorekeeper analysis operator stack is misaligned");
		tmpop = skanal->pop_test_op();
		AI_SKANode_TestOp::destroy(&tmpop);
	    }
	    break;
	  default:
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    assert_warning(FALSE,
"Expecting boolean comparison but found something else");
	    break;
	}
    }
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	assert_warning(symbolp(keyobj) && !boundp(keyobj),
		       "Expecting boolean comparison but found something else");
    }
    return skanal;
}

//! Analyze a scorekeeper 'when' form.
/*! \todo Implement. */

AI_SKAnalysis *
analyze_sk_when_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL;

    return skanal;
}

//! Analyze a scorekeeper test form that is always true.

AI_SKAnalysis *
analyze_sk_always_true_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(sktest, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    assert_error(parent->action_flags, 
		 "No action set for scorekeeper analysis");
    /* Handle other analyses on our chain, if any. */
    if (parent->next)
      analyze_sk_always_true_test(side, sk, sktest, parent->next);
    /* Assign to parent scorekeeper analysis. */
    skanal = parent;
    /* Examine the various possible actions. */
    if (AI_SKA_LAST_SIDE & skanal->action_flags)
      skanal->basic_flags |= AI_SK_LAST_SIDE;
    else if (AI_SKA_LAST_ALLIANCE & skanal->action_flags)
      skanal->basic_flags |= AI_SK_LAST_ALLIANCE;
    else if ((AI_SKA_WIN | AI_SKA_LOSE | AI_SKA_DRAW) & skanal->action_flags) {
	/* Try using the 'when' test. */
	if (lispnil != sk->when)
	  skanal = analyze_sk_when_test(side, sk, sk->when, skanal);
	/* Try using the 'trigger' test. */
	else if (lispnil != sk->trigger) {
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    DMprintf(
"\tEncountered scorekeeper trigger, which is not yet understood.\n");
	}
	/* If nothing works, then the action would seem unconditional. */
	else {
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    DMprintf(
"\tEncountered what seems to be unconditional scorekeeper action!\n");
	}
    }
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tCannot understand a scorekeeper form.\n");
    }
    return skanal;
}

//! Analyze a scorekeeper test form.
/*!
    \todo Handle 'append', 'remove', and 'remove-list'.
*/

AI_SKAnalysis *
analyze_sk_test(
    Side *side, Scorekeeper *sk, Obj *sktest, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL;
    Obj *keyobj = lispnil;
    enum keywords keycode;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(sktest, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    assert_error(parent->action_flags, 
		 "No action set for scorekeeper analysis");
    /* Handle other analyses on our chain, if any. */
    if (parent->next)
      analyze_sk_test(side, sk, sktest, parent->next);
    /* Assign to parent scorekeeper analysis. */
    skanal = parent;
    /* If test form is negated nil or is unconditionally true... */
    if (((lispnil == sktest) && (skanal->opstack->op == AI_SKT_NOT))
	|| (numberp(sktest) && c_number(sktest))) 
      return analyze_sk_always_true_test(side, sk, sktest, skanal);
    /* Else if test form is empty without negation, then take no action. */
    else if (lispnil == sktest);
    /* Else if test form is a list... */
    else if (consp(sktest)) {
        /* If the list has test keywords... */
        keyobj = car(sktest);
        if (symbolp(keyobj) && !boundp(keyobj)) {
            keycode = (enum keywords)keyword_code(c_string(keyobj));
            switch (keycode) {
	      case K_SUM_UPROP:
		skanal = 
		    analyze_sk_sum_uprop_test(side, sk, cdr(sktest), skanal);
		break;
	      case K_NOT: case K_AND: case K_OR:
		skanal = analyze_sk_bool_comp_test(side, sk, sktest, skanal);
		break;
	      case K_EQ: case K_NE: case K_LE: case K_LT: case K_GE: case K_GT:
		skanal = analyze_sk_arith_comp_test(side, sk, sktest, skanal);
		break;
	      case K_PLUS: case K_MINUS: case K_MULTIPLY: case K_DIVIDE:
		skanal = analyze_sk_arith_op_test(side, sk, sktest, skanal);
		break;
	      default:
		skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		DMprintf(
"\tCannot understand keyword, '%s', as part of a test form.\n",
			 c_string(keyobj));
		break;
	    }
	} /* keywords */
	/* Else if the list contains a list, then recurse into it. */
	else if (consp(keyobj)) {
	    skanal = analyze_sk_test(side, sk, keyobj, skanal);
	}
	/* Else if the list contains a number, 
	    then we might be able to understand something in an appropriate 
	    context. */
	else if (numberp(keyobj)) {
	    switch (skanal->action_flags) {
	      case AI_SKA_CHANGE_SCORE:
		skanal->basic_flags |= AI_SK_CHANGE_SCORE;
		break;
	      default:
		skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		DMprintf("\tCannot understand %d as test.\n", c_number(keyobj));
		break;
	    }
	}
	/* Else we don't understand the test. */
	else {
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    DMprintf("\tCannot understand a list of independent tests.\n");
	}
    } /* list */
    /* Else if test form is a symbol... */
    else if (symbolp(sktest)) {
	/* If symbol is bound, then substitute. */
	if (boundp(sktest))
	  skanal = analyze_sk_test(side, sk, eval_symbol(sktest), skanal);
	/* Else if symbol is not bound, then try to match a keyword. */
	else {
	    keycode = (enum keywords)keyword_code(c_string(sktest));
	    switch (keycode) {
	      case K_SCORE:
		skanal = analyze_sk_score_test(side, sk, sktest, skanal);
		break;
	      case K_TURN:
		skanal = analyze_sk_turn_test(side, sk, sktest, skanal);
		break;
	      default:
		skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		DMprintf(
"\tCannot understand keyword, '%s', as a test.\n",
			 c_string(sktest));
		break;
	    }
	} /* unbound symbol */
    } /* symbol */
    /* Else we cannot understand the test form... */
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tCannot understand a test form.\n");
    }
    return skanal;
}

//! Analyze a scorekeeper 'if' form.

AI_SKAnalysis *
analyze_sk_if_action(
    Side *side, Scorekeeper *sk, Obj *skact, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL, *skanal2 = NULL;
    Obj *sktest = lispnil;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(skact, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    /* Initially associate with parent. */
    skanal = parent;
    /* Discover if we must deal with an 'else' action or not. */
    if (lispnil != cddr(skact)) {
	skanal = parent->clone();
	skanal2 = parent->clone();
	skanal2->basic_flags |= AI_SK_DONT_TEST;
	skanal2->push_test_op(AI_SKT_NOT);
    }
    /* Analyze the 'then' action first. */
    skanal->basic_flags |= AI_SK_DONT_TEST;
    skanal = analyze_sk_action(side, sk, cadr(skact), skanal);
    /* If there is an 'else' action, then analyze it next. */
    if (skanal2)
      skanal2 = analyze_sk_action(side, sk, caddr(skact), skanal2);
    sktest = car(skact);
    /* Test the 'then' clause. */
    skanal->basic_flags &= ~AI_SK_DONT_TEST;
    skanal = analyze_sk_test(side, sk, sktest, skanal);
    /* Test the 'else' clause, if present. 
       Also link analyses together, if so. */
    if (skanal2) {
	skanal2->basic_flags &= ~AI_SK_DONT_TEST;
	skanal2 = analyze_sk_test(side, sk, sktest, skanal2);
	AI_SKAnalysis::destroy(&parent);
	/* Save start of 'then' chain. */
	parent = skanal;
	/* Scan to end of 'then' chain. */
	for (; skanal->next; skanal = skanal->next);
	/* Link first end of 'then' chain to first 'else' analysis. */
	skanal->next = skanal2;
	/* Restore start of 'then' chain. */
	skanal = parent;
    }
    return skanal;
}

//! Analyze a scorekeeper action form.
/*!
    \todo Analyze 'cond' forms.
    \todo Support 'trigger' tests.
*/

AI_SKAnalysis *
analyze_sk_action(
    Side *side, Scorekeeper *sk, Obj *skact, AI_SKAnalysis *parent)
{
    AI_SKAnalysis *skanal = NULL, *skanal2 = NULL, *skanal3 = NULL;
    Obj *keyobj = lispnil, *rest = lispnil, *sktest = NULL;
    enum keywords keycode;

    /* Sanity checks. */
    assert_error(side, "Attempted to set subgoals for a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    assert_error(skact, "Attempted to analyze a NULL scorekeeper form");
    assert_error(parent, "Attempted to access a NULL scorekeeper analysis");
    /* Initially use the parent scorekeeper analysis. */
    skanal = parent;
    /* Set 'sktest' to 't'. */
    sktest = new_number(TRUE);
    /* If action form is empty... */
    if (lispnil == skact) {
	/* If at top-level, then assume "last side wins". */
	if (!skanal->opstack)
	  skanal->action_flags |= AI_SKA_LAST_SIDE;
	/* Else we don't understand. */
	else {
	    skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	    DMprintf("\tUnexpected 'nil' action.\n");
	}
    }
    /* Else if action form is a list... */
    else if (consp(skact)) {
	/* If there is a relevant keyword... */
	keyobj = car(skact);
	if (symbolp(keyobj) && !boundp(keyobj)) {
            keycode = (enum keywords)keyword_code(c_string(keyobj));
            switch (keycode) {
	      /* Handle test forms. */
	      case K_IF:
		return analyze_sk_if_action(side, sk, cdr(skact), skanal);
	      case K_COND:
		skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		DMprintf(
"\tEncountered 'cond' form, which is not yet understood.\n");
		break;
	      /* Handle set/add score. */
	      case K_SET_SCORE: case K_ADD_SCORE:
		skanal->action_flags |= AI_SKA_CHANGE_SCORE;
		sktest = cdr(skact);
		break;
	      default:
		skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		DMprintf("\tUnexpected keyword, '%s', in action form.\n", 
			 c_string(keyobj));
		break;
	    }
	} /* keywords handler */
	/* Else assume we are dealing with a list of action forms. */
	/* This produces a disjunction of scorekeepers,
	    one scorekeeper per action form. */
	else {
	    for_all_list(skact, rest) {
		skanal = parent->clone();
		if (!skanal3)
		  skanal3 = skanal;
		skanal = analyze_sk_action(side, sk, car(rest), skanal);
		if (skanal2) {
		    for (; skanal2->next; skanal2 = skanal2->next);
		    skanal2->next = skanal;
		}
		skanal2 = skanal;
	    }
	    AI_SKAnalysis::destroy(&parent);
	    return skanal3;
	} /* list of action forms */
    } /* list */
    /* Else if action form is a symbol... */
    else if (symbolp(skact)) {
	/* Test for keywords. */
	keycode = (enum keywords)keyword_code(c_string(skact));
	switch (keycode) {
	  case K_LAST_SIDE_WINS:
	    skanal->action_flags = AI_SKA_LAST_SIDE;
	    break;
	  case K_LAST_ALLIANCE_WINS:
	    skanal->action_flags = AI_SKA_LAST_ALLIANCE;
	    break;
	  case K_WIN:
	    skanal->action_flags = AI_SKA_WIN;
	    break;
	  case K_LOSE:
	    skanal->action_flags = AI_SKA_LOSE;
	    break;
	  case K_END:
	    skanal->action_flags = AI_SKA_DRAW;
	    break;
	  /* If no recognized keyword, then try other things. */
	  default:
	    /* If the symbol is bound, then evaluate with substitution. */
	    if (boundp(skact)) 
	      return analyze_sk_action(side, sk, eval_symbol(skact), skanal);
	    /* Else, we don't understand. */
	    else {
		skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
		DMprintf("\tUnexpected keyword, '%s', in action form.\n",
			 c_string(skact));
		break;
	    }
	} /* keyword */
    } /* symbol */
    /* Else we cannot deal with the action form. */
    else {
	skanal->basic_flags |= AI_SK_DONT_UNDERSTAND;
	DMprintf("\tSomething that we cannot understand in action form.\n");
    }
    /* Now that we have established an action, run the test. */
    if (!(skanal->basic_flags & AI_SK_DONT_UNDERSTAND)
	&& !(skanal->basic_flags & AI_SK_DONT_TEST)) {
	/* Sanity check. */
	assert_error(skanal->action_flags, 
		     "No action set, yet action supposedly understood");
	/* Analyze the tests. */
	if (numberp(sktest) && c_number(sktest)) 
	  skanal = analyze_sk_always_true_test(side, sk, sktest, skanal);
	else
	  skanal = analyze_sk_test(side, sk, sktest, skanal);
    }
    return skanal;
}

//! Analyze a scorekeeper.
/*!
    \return List of 1 or more scorekeeper analyses, if successful.
    \todo Do compaction on resulting analyses.
*/

AI_SKAnalysis *
analyze_scorekeeper(Side *side, Scorekeeper *sk)
{
    AI_SKAnalysis *skanal = NULL;

    /* Sanity checks. */
    assert_error(side, "Attempted to access a NULL side");
    assert_error(sk, "Attempted to analyze a NULL scorekeeper");
    /* Allocate 1st scorekeeper analysis for this scorekeeper. */
    skanal = AI_SKAnalysis::construct();
    /* Tell someone what we are currently doing. */
    if (!empty_string(sk->title)) {
      DMprintf("Analyzing \"%s\" scorekeeper.\n", sk->title);
    } else {
      DMprintf("Analyzing scorekeeper %d.\n", sk->id);
    }
    /* Start things off by doing an action analysis of the entire sk body. */
    skanal = analyze_sk_action(side, sk, sk->body, skanal);
    return skanal;
}

//! Analyze all the scorekeepers.
/*! \todo Perform compaction and summarization of analyses. */

AI_SKAnalysis *
analyze_scorekeepers(Side *side)
{
    AI_SKAnalysis *skanal = NULL, *skanal2 = NULL, *skanalhd = NULL;
    Scorekeeper *sk = NULL;

    /* Sanity checks. */
    assert_error(side, "Attempted to access a NULL side");
    /* Iterate through scorekeepers, sewing together the analyses. */
    for_all_scorekeepers(sk) {
	skanal = analyze_scorekeeper(side, sk);
	if (!skanalhd)
	  skanalhd = skanal;
	if (skanal2) {
	    for (; skanal2->next; skanal2 = (AI_SKAnalysis *)skanal2->next);
	    skanal2->next = skanal;
	}
	else
	  skanal2 = skanal;
    }
    return skanalhd;
}
