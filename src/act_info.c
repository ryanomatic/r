/***************************************************************************
*					 MEDIEVIA CyberSpace Code and Data files		       *
*       Copyright (C) 1992, 1996 INTENSE Software(tm) and Mike Krause	   *
*							   All rights reserved				           *
***************************************************************************/
/***************************************************************************
* This program belongs to INTENSE Software, and contains trade secrets of  *
* INTENSE Software.  The program and its contents are not to be disclosed  *
* to or used by any person who has not received prior authorization from   *
* INTENSE Software.  Any such disclosure or use may subject the violator   *
* to civil and criminal penalties by law.                                  *
***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "holocode.h"
#include "trading.h"

/* extern variables */
extern struct zone_data *zone_table;
extern struct room_data *world[MAX_ROOM];	/* array of rooms  */
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct global_clan_info_struct global_clan_info;
extern int giNewsVersion;
extern char credits[MAX_STRING_LENGTH];
extern char news[MAX_STRING_LENGTH];
extern char info[MAX_STRING_LENGTH];
extern char story[MAX_STRING_LENGTH];
extern char wizlist[MAX_STRING_LENGTH];
extern ush_int Holo[MAXHOLO][MAXHOLO];
extern char *dirs[];
extern char *sexes[];
extern char *where[];
extern char *color_liquid[];
extern char *fullness[];
extern struct str_app_type str_app[];
extern char str_boot_time[];
extern char MOUNTMOVE;

/* extern functions */
extern int iMakeHoloRoom(int x, int y);
extern ListRoomFreightToChar(struct char_data *ch);
extern bool is_formed(struct char_data *ch);
extern char *get_title(struct char_data *ch);
extern void room_affect_text(struct char_data *ch);
struct time_info_data age(struct char_data *ch);
extern void page_string(struct descriptor_data *d, char *str,
			int keep_internal);
void write_filtered_text(FILE * fh, char *text);
extern void list_obj_to_char(struct obj_data *list, struct char_data *ch,
			     int mode, bool show);
extern void show_obj_to_char(struct obj_data *object, struct char_data *ch,
			     int mode, int amount);
extern bool ExamineFreight(struct char_data *stpCh, char *szpName);
extern bool LookAtFreight(struct char_data *stpCh, char *szpArgument);
extern char global_color;
void ShowTrackMessage(struct char_data *stpCh);

/* Procedures related to 'look' */

void argument_split_2(char *argument, char *first_arg, char *second_arg)
{
	int look_at, found, begin;
	found = begin = 0;

	/* Find first non blank */
	for (; *(argument + begin) == ' '; begin++) ;

	/* Find length of first word */
	for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)

		/* Make all letters lower case, AND copy them to first_arg */
		*(first_arg + look_at) = LOWER(*(argument + begin + look_at));
	*(first_arg + look_at) = '\0';
	begin += look_at;

	/* Find first non blank */
	for (; *(argument + begin) == ' '; begin++) ;

	/* Find length of second word */
	for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)

		/* Make all letters lower case, AND copy them to second_arg */
		*(second_arg + look_at) = LOWER(*(argument + begin + look_at));
	*(second_arg + look_at) = '\0';
	begin += look_at;
}

struct obj_data *get_object_in_equip_vis(struct char_data *ch,
					 char *arg,
					 struct obj_data *equipment[], int *j)
{

	for ((*j) = 0; (*j) < MAX_WEAR; (*j)++)
		if (equipment[(*j)])
			if (CAN_SEE_OBJ(ch, equipment[(*j)]))
				if (isname(arg, equipment[(*j)]->name))
					return (equipment[(*j)]);

	return (0);
}

char *find_ex_description(char *word, struct extra_descr_data *list)
{
	struct extra_descr_data *i = NULL;

	for (i = list; i; i = i->next)
		if (isname(word, i->keyword))
			return (i->description);

	return (0);
}

void show_char_to_char(struct char_data *i, struct char_data *ch, int mode)
{
	char buffer[MAX_STRING_LENGTH];
	int j, found, percent;
	struct obj_data *tmp_obj = NULL;

	char hero[32] = { 0 };
	if (get_total_level(i) == 124)
		sprintf(hero, "%s(HERO)%s", COL_WHT, COL_GRN);
	else if (GET_LEVEL(i) > 31)
		sprintf(hero, "%s(GOD)%s", COL_WHT, COL_GRN);

	if (mode == 0) {
		if (GET_LEVEL(ch) < 34)
			if (IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch, i)) {
				if (IS_AFFECTED(ch, AFF_SENSE_LIFE)) {
					global_color = 34;
					send_to_char("You sense a hidden life form in the room.\n\r", ch);
					global_color = 0;
				}
				return;
			}

		if (!(i->player.long_descr) || (GET_POS(i) != i->specials.default_pos)) {
			/* A char without long descr, or not in default pos. */
			if (!IS_NPC(i)) {
				if (i->specials.stpMount && GET_ZONE(i) != 180) {
					sprintf(buffer, "%s%s riding %s", hero, GET_NAME(i), i->specials.stpMount->player.short_descr);
				} else {
					sprintf(buffer, "%s%s", hero, GET_NAME(i));
					if (!IS_SET(ORIGINAL(ch)->specials.act, PLR_BRIEF) || i->specials.afk || (i->desc && i->desc->str) || (IS_SET(ORIGINAL(ch)->specials.act, PLR_BRIEF))) {
						strcat(buffer, " ");
						strcat(buffer, get_title(i));
					}
				}
			} else {
				strcpy(buffer, i->player.short_descr);
				(void)CAP(buffer);
			}

			switch (GET_POS(i)) {
			case POSITION_STUNNED:
				strcat(buffer, " is lying here, stunned.");
				break;
			case POSITION_INCAP:
				strcat(buffer,
				       " is lying here, incapacitated.");
				break;
			case POSITION_MORTALLYW:
				strcat(buffer,
				       " is lying here, mortally wounded.");
				break;
			case POSITION_DEAD:
				strcat(buffer, " is lying here, dead.");
				break;
			case POSITION_STANDING:
				strcat(buffer, " is here.");
				break;
			case POSITION_SITTING:
				strcat(buffer, " is sitting here.");
				break;
			case POSITION_RESTING:
				strcat(buffer, " is resting here.");
				break;
			case POSITION_SLEEPING:
				strcat(buffer, " is sleeping here.");
				break;
			case POSITION_FIGHTING:
				if (i->specials.fighting) {

					strcat(buffer, " is here, fighting ");
					if (i->specials.fighting == ch)
						strcat(buffer, " YOU!");
					else {
						if (i->in_room == i->specials.fighting->in_room)
							if (IS_NPC(i->specials.fighting))
								strcat(buffer, i->specials.fighting->player.short_descr);
							else
								strcat(buffer, GET_NAME(i->specials.fighting));
						else
							strcat(buffer, "someone who has already left.");
					}
				} else	/* NIL fighting pointer */
					strcat(buffer, " is here struggling with thin air.");
				break;
			default:
				strcat(buffer, " is floating here.");
				break;
			}

			global_color = 32;
			send_to_char(buffer, ch);
			global_color = 0;
			strcpy(buffer, "");

			if (i->specials.afk)
				strcat(buffer, " [AFK]");

			if (!IS_NPC(i) && !i->desc)
				strcat(buffer, " [LOST LINK]");

			if (IS_AFFECTED(i, AFF_INVISIBLE))
				strcat(buffer, " (invisible)");

			if (IS_AFFECTED(i, AFF_HIDE))
				strcat(buffer, " (hidden)");

			if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
				if (IS_EVIL(i)) {
					strcat(buffer, " (Red Aura)");
				}
			}

			strcat(buffer, "\n\r");
			send_to_char(buffer, ch);
		} else {	/* npc with long */

			if (IS_AFFECTED(i, AFF_INVISIBLE))
				strcpy(buffer, "*");
			else
				*buffer = '\0';

			if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
				if (IS_EVIL(i)) {
					strcat(buffer, " (Red Aura)");
				}
			}

			strcat(buffer, i->player.long_descr);
			global_color = 32;
			send_to_char(buffer, ch);
			global_color = 0;
		}

		if (IS_AFFECTED(i, AFF_SANCTUARY)) {
			global_color = 37;
			act("    $c is surrounded by a shimmering, magical field.", FALSE, i, 0, ch, TO_VICT);
			global_color = 0;
		}
		if (IS_AFFECTED(i, AFF_FIRESHIELD)) {
			global_color = 31;
			act("    $c is enshrouded in a deep red shield of fire.", FALSE, i, 0, ch, TO_VICT);
			global_color = 0;
		}
		if (IS_AFFECTED(i, AFF_MAP_CATACOMBS)) {
			global_color = 35;
			act("    $c has a bright humming purple aura about $m.",
			    FALSE, i, 0, ch, TO_VICT);
			global_color = 0;
		}
	} else if (mode == 1) {
		global_color = 32;
		if (i->player.description)
			send_to_char(i->player.description, ch);
		else {
			act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
		}
		global_color = 0;
		/* Show a character to another */

		if (GET_MAX_HIT(i) > 0)
			percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
		else
			percent = -1;	/* How could MAX_HIT be < 1?? */

		if (IS_NPC(i))
			strcpy(buffer, i->player.short_descr);
		else
			strcpy(buffer, GET_NAME(i));

		if (percent >= 100)
			strcat(buffer, " is in an excellent condition.\n\r");
		else if (percent >= 90)
			strcat(buffer, " has a few scratches.\n\r");
		else if (percent >= 83)
			strcat(buffer,
			       " has a nasty looking welt on the forehead.\n\r");
		else if (percent >= 76)
			strcat(buffer,
			       " has some small wounds and bruises.\n\r");
		else if (percent >= 69)
			strcat(buffer, " winces in pain.\n\r");
		else if (percent >= 62)
			strcat(buffer, " has some minor wounds.\n\r");
		else if (percent >= 55)
			strcat(buffer, " has quite a few wounds.\n\r");
		else if (percent >= 48)
			strcat(buffer, " grimaces in pain.\n\r");
		else if (percent >= 41)
			strcat(buffer,
			       " has some big nasty wounds and scratches.\n\r");
		else if (percent >= 36)
			strcat(buffer, " has some large, gaping wounds.\n\r");
		else if (percent >= 29)
			strcat(buffer, " looks pretty awful.\n\r");
		else if (percent >= 22)
			strcat(buffer, " screams in agony.\n\r");
		else if (percent >= 15)
			strcat(buffer, " is vomiting blood.\n\r");
		else if (percent >= 8)
			strcat(buffer, " pales visibly as Death nears.\n\r");
		else if (percent > 0)
			strcat(buffer, " barely clings to life.\n\r");
		else if (percent == 0)
			strcat(buffer, " is dead.\n\r");
		global_color = 33;
		send_to_char(buffer, ch);
		global_color = 0;
		found = FALSE;
		for (j = 0; j < MAX_WEAR; j++) {
			if (i->equipment[j]) {
				if (CAN_SEE_OBJ(ch, i->equipment[j])) {
					found = TRUE;
				}
			}
		}
		if (found) {
			act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT);
			for (j = 0; j < MAX_WEAR; j++) {
				if (i->equipment[j]) {
					if (CAN_SEE_OBJ(ch, i->equipment[j])) {
						global_color = 35;
						send_to_char(where[j], ch);
						global_color = 0;
						show_obj_to_char(i->equipment[j], ch, 1, 1);
					}
				}
			}
		}
		if ((GET_CLASS(ch) == CLASS_THIEF && ch != i) || GET_LEVEL(ch) >= 31 || IS_DEAD(i) || IS_SET(ch->player.multi_class, MULTI_CLASS_THIEF)) {
			found = FALSE;
			send_to_char("\n\rYou attempt to peek at the inventory:\n\r", ch);
			for (tmp_obj = i->carrying; tmp_obj;
			     tmp_obj = tmp_obj->next_content) {
				if (CAN_SEE_OBJ(ch, tmp_obj) && number(0, 20) < GET_LEVEL(ch)) {
					show_obj_to_char(tmp_obj, ch, 1, 1);
					found = TRUE;
				}
			}
			if (!found)
				send_to_char("You can't see anything.\n\r", ch);
		}

	} else if (mode == 2) {

		/* Lists inventory */
		act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
		list_obj_to_char(i->carrying, ch, 1, TRUE);
	}
}

void list_char_to_char(struct char_data *list, struct char_data *ch, int mode)
{
	struct char_data *i = NULL;

	if (!list) {
		log_hd("#####NO LIST IN LIST_CHAR_TOCHAR###");
		return;
	}
	if (!ch) {
		log_hd("#####NO CH IN LIST_CHAR_TO_CHAR");
		return;
	}
	global_color = 32;
	for (i = list; i; i = i->next_in_room) {
		if (ch == i)
			continue;
		if (IS_MOB(i) && i->specials.stpMount)
			continue;
		if (((IS_AFFECTED(ch, AFF_SENSE_LIFE) && !i->specials.wizInvis) || (GET_ZONE(ch) == 198) || (CAN_SEE(ch, i) && !IS_AFFECTED(i, AFF_HIDE)))) {
			show_char_to_char(i, ch, 0);
		} else {
			if ((IS_DARK(ch->in_room)) && (IS_AFFECTED(i, AFF_INFRARED))) {
				/* Monster with infra red : can't see him */
				global_color = 31;
				send_to_char("You see a pair of glowing red eyes.\n\r", ch);
				global_color = 0;
			}
		}
	}
	global_color = 0;
}

void do_look(struct char_data *ch, char *argument, int cmd)
{
	char buffer[MAX_STRING_LENGTH];
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	int keyword_no;
	int j, bits, temp;
	bool found;
	struct obj_data *tmp_object = NULL, *found_object = NULL;
	struct char_data *tmp_char;
	char *tmp_desc;
	static char *keywords[] = {
		"north",
		"east",
		"south",
		"west",
		"up",
		"down",
		"in",
		"at",
		"around",
		"",		/* Look at '' case */
		"\n"
	};

	if (!ch->desc)
		return;

	if (GET_POS(ch) < POSITION_SLEEPING)
		send_to_char("You can't see anything but stars!\n\r", ch);
	else if (GET_POS(ch) == POSITION_SLEEPING)
		send_to_char("You can't see anything, you're sleeping!\n\r", ch);
	else if (check_blind(ch)) ;
	else if (IS_DARK(ch->in_room) && !ch->specials.holyLite) {
		global_color = 33;
		send_to_char(world[ch->in_room]->name, ch);
		send_to_char("\n\r", ch);
		global_color = 0;
		send_to_char("It is pitch black...\n\r", ch);
		list_char_to_char(world[ch->in_room]->people, ch, 0);
	} else if (LookAtFreight(ch, argument)) {
		return;
	} else {
		argument_split_2(argument, arg1, arg2);
		keyword_no = search_block(arg1, keywords, FALSE);	/* Partiel Match */
		if ((keyword_no == -1) && *arg1) {
			keyword_no = 7;
			strcpy(arg2, arg1);	/* Let arg2 become the target object (arg1) */
		}
		found = FALSE;
		tmp_object = NULL;
		tmp_char = NULL;
		tmp_desc = NULL;

		switch (keyword_no) {
			/* look <dir> */
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:{

				if (EXIT(ch, keyword_no)) {
					if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_CLOSED) && IS_SET(EXIT(ch, keyword_no)->exit_info, EX_SECRET)) {
						send_to_char("Nothing special there...\r\n", ch);
						break;
					}
					if ((EXIT(ch, keyword_no)->general_description) && strlen(EXIT(ch, keyword_no)->general_description) > 1) {
						send_to_char(EXIT(ch, keyword_no)->general_description, ch);
					} else {
						send_to_char("Nothing special there...\n\r", ch);
					}
					if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_HIDDEN))
						break;
					if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_CLOSED) && (EXIT(ch, keyword_no)->keyword)) {
						sprintf(buffer, "The %s is closed.\n\r", fname(EXIT(ch, keyword_no)->keyword));
						send_to_char(buffer, ch);
					} else {
						if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_ISDOOR) && EXIT(ch, keyword_no)->keyword && strlen(EXIT(ch, keyword_no)->keyword) > 1) {
							sprintf(buffer, "The %s is open.\n\r", fname(EXIT(ch, keyword_no)->keyword));
							send_to_char(buffer, ch);
						}
					}
				} else {
					send_to_char
					    ("Nothing special there...\n\r",
					     ch);
				}
			}
			break;

			/* look 'in'    */
		case 6:{
				if (*arg2) {
					/* Item carried */
					global_color = 33;
					bits =
					    generic_find(arg2,
							 FIND_OBJ_INV |
							 FIND_OBJ_ROOM |
							 FIND_OBJ_EQUIP, ch,
							 &tmp_char,
							 &tmp_object);

					if (bits) {	/* Found something */
						if (GET_ITEM_TYPE(tmp_object) ==
						    ITEM_DRINKCON) {
							if (tmp_object->
							    obj_flags.
							    value[1] <= 0) {
								act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
							} else {
								temp =
								    ((tmp_object->obj_flags.value[1] * 3)
								     /
								     tmp_object->
								     obj_flags.
								     value[0]);
								sprintf(buffer,
									"It's %sfull of a %s liquid.\n\r",
									fullness
									[temp],
									color_liquid
									[tmp_object->obj_flags.
									 value
									 [2]]);
								send_to_char
								    (buffer,
								     ch);
							}
						} else
						    if (GET_ITEM_TYPE
							(tmp_object) ==
							ITEM_CONTAINER) {
							if (!IS_SET
							    (tmp_object->
							     obj_flags.value[1],
							     CONT_CLOSED)) {
								send_to_char(tmp_object->short_description, ch);
								switch (bits) {
								case FIND_OBJ_INV:
									send_to_char(" (carried) : \n\r", ch);
									break;
								case FIND_OBJ_ROOM:
									send_to_char(" (here) : \n\r", ch);
									break;
								case FIND_OBJ_EQUIP:
									send_to_char(" (used) : \n\r", ch);
									break;
								}
								list_obj_to_char
								    (tmp_object->
								     contains,
								     ch, 2,
								     TRUE);
							} else
								send_to_char
								    ("It is closed.\n\r",
								     ch);
						} else {
							send_to_char
							    ("That is not a container.\n\r",
							     ch);
						}
					} else {	/* wrong argument */
						send_to_char
						    ("You do not see that item here.\n\r",
						     ch);
					}
				} else {	/* no argument */
					send_to_char("Look in what?!\n\r", ch);
				}
			}
			global_color = 0;
			break;

			/* look 'at'    */
		case 7:{

				if (*arg2) {

					bits =
					    generic_find(arg2,
							 FIND_OBJ_INV |
							 FIND_OBJ_ROOM |
							 FIND_OBJ_EQUIP |
							 FIND_CHAR_ROOM, ch,
							 &tmp_char,
							 &found_object);

					if (tmp_char) {
						show_char_to_char(tmp_char, ch,
								  1);
						if (ch != tmp_char) {
							if (GET_LEVEL(ch) > 31
							    && ch->specials.
							    wizInvis == TRUE) {
								return;
							}
							global_color = 33;
							if (GET_LEVEL(ch) < 35)
								act("$n looks at you.", TRUE, ch, 0, tmp_char, TO_VICT);
							act("$n looks at $N.",
							    TRUE, ch, 0,
							    tmp_char,
							    TO_NOTVICT);
							global_color = 0;
						}
						return;
					}

					/* Search for Extra Descriptions in room and items */

					/* Extra description in room?? */
					if (!found) {
						tmp_desc =
						    find_ex_description(arg2,
									world
									[ch->
									 in_room]->
									ex_description);
						if (tmp_desc) {
							global_color = 35;
							page_string(ch->desc,
								    tmp_desc,
								    0);
							global_color = 0;
							return;	/* RETURN SINCE IT WAS ROOM DESCRIPTION */
							/* Old system was: found = TRUE; */
						}
					}

					/* Search for extra descriptions in items */

					/* Equipment Used */

					if (!found) {
						for (j = 0;
						     j < MAX_WEAR && !found;
						     j++) {
							if (ch->equipment[j]) {
								if (CAN_SEE_OBJ
								    (ch,
								     ch->
								     equipment
								     [j])) {
									tmp_desc
									    =
									    find_ex_description
									    (arg2,
									     ch->
									     equipment
									     [j]->
									     ex_description);
									if (tmp_desc) {
										global_color
										    =
										    32;
										page_string
										    (ch->
										     desc,
										     tmp_desc,
										     1);
										global_color
										    =
										    0;
										found
										    =
										    TRUE;
									}
								}
							}
						}
					}

					/* In inventory */

					if (!found) {
						for (tmp_object = ch->carrying;
						     tmp_object && !found;
						     tmp_object =
						     tmp_object->next_content) {
							if (CAN_SEE_OBJ
							    (ch, tmp_object)) {
								tmp_desc =
								    find_ex_description
								    (arg2,
								     tmp_object->
								     ex_description);
								if (tmp_desc) {
									global_color
									    =
									    33;
									page_string
									    (ch->
									     desc,
									     tmp_desc,
									     1);
									global_color
									    = 0;
									found =
									    TRUE;
								}
							}
						}
					}

					/* Object In room */

					if (!found) {
						for (tmp_object =
						     world[ch->in_room]->
						     contents;
						     tmp_object && !found;
						     tmp_object =
						     tmp_object->next_content) {
							if (CAN_SEE_OBJ
							    (ch, tmp_object)) {
								tmp_desc =
								    find_ex_description
								    (arg2,
								     tmp_object->
								     ex_description);
								if (tmp_desc) {
									global_color
									    =
									    32;
									page_string
									    (ch->
									     desc,
									     tmp_desc,
									     1);
									global_color
									    = 0;
									found =
									    TRUE;
								}
							}
						}
					}
					/* wrong argument */

					if (bits) {	/* If an object was found */
						if (!found)
							/* Show no-description */
							show_obj_to_char
							    (found_object, ch,
							     5, 1);
						else
							/* Find hum, glow etc */
							show_obj_to_char
							    (found_object, ch,
							     6, 1);
					} else if (!found) {
						send_to_char
						    ("You do not see that here.\n\r",
						     ch);
					}
				} else {
					/* no argument */

					send_to_char("Look at what?\n\r", ch);
				}
			}
			break;
		case 8:{
				if (world[ch->in_room]->holox
				    || world[ch->in_room]->holoy)
					do_areamap(ch, "\0", 9);
				else
					do_look(ch, "\0", 15);
			}
			break;

			/* look ''      */
		case 9:{
				if (!IS_NPC(ch) && IS_FLYING(ch)) {
					do_areamap(ch, "", 9);
					return;
				}
				if (ch->specials.ll_set) {
					do_stat_room(ch, "", 9);
					return;
				}
				global_color = 33;
				send_to_char(world[ch->in_room]->name, ch);
				send_to_char("\n\r", ch);
				global_color = 35;
				if (!IS_SET
				    (ORIGINAL(ch)->specials.act, PLR_BRIEF))
					send_to_char(world[ch->in_room]->
						     description, ch);
				global_color = 0;
				room_affect_text(ch);
				/* Make sure CMD == 16 so only runs when you walk in the room */
				if (cmd == 16 && ch->specials.hunting)
					if (ch->desc
					    && ch->desc->connected ==
					    CON_PLAYING)
						ShowTrackMessage(ch);

				ListRoomFreightToChar(ch);
				list_obj_to_char(world[ch->in_room]->contents,
						 ch, 0, FALSE);
				list_char_to_char(world[ch->in_room]->people,
						  ch, 0);
			}
			break;

			/* wrong arg    */
		case -1:
			send_to_char("Sorry, I didn't understand that!\n\r",
				     ch);
			break;
		}
	}
}

/* end of look */

void do_read(struct char_data *ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH * 2];

	/* This is just for now - To be changed later.! */
	sprintf(buf, "at %s", argument);
	do_look(ch, buf, 15);
}

void do_examine(struct char_data *ch, char *argument, int cmd)
{
	char name[MAX_INPUT_LENGTH * 2], buf[100];
	int bits;
	struct char_data *tmp_char = NULL;
	struct obj_data *tmp_object = NULL;

	one_argument(argument, name);

	if (!*name) {
		send_to_char("Examine what?\n\r", ch);
		return;
	}

	if (ExamineFreight(ch, name))
		return;

	sprintf(buf, "at %s", argument);
	do_look(ch, buf, 15);

	bits = generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM |
			    FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

	if (tmp_object) {
		if ((GET_ITEM_TYPE(tmp_object) == ITEM_DRINKCON) ||
		    (GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER)) {
			send_to_char("When you look inside, you see:\n\r", ch);
			sprintf(buf, "in %s", argument);
			do_look(ch, buf, 15);
		}
	}
}

void do_exits(struct char_data *ch, char *argument, int cmd)
{
	int door;
	char buf[MAX_STRING_LENGTH];
	char *exits[] = {
		"North",
		"East ",
		"South",
		"West ",
		"Up   ",
		"Down "
	};

	*buf = '\0';

	if (check_blind(ch))
		return;
	global_color = 34;
	for (door = 0; door <= 5; door++)
		if (EXIT(ch, door)) {
			/* the (!CLOSED || !SECRET) means that it will evalulate
			 *  false if BOTH those are true and thus skip a CLOSED
			 *   and secret door -raster 
			 */
			if ((EXIT(ch, door)->to_room != NOWHERE &&
			     !IS_SET(EXIT(ch, door)->exit_info, EX_ILLUSION) &&
			     ((!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) ||
			      (!IS_SET(EXIT(ch, door)->exit_info, EX_SECRET) &&
			       !IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN))))
			    || GET_LEVEL(ch) > 33) {
				sprintf(buf + strlen(buf), "%s ", exits[door]);
				if (GET_LEVEL(ch) > 31)
					sprintf(buf + strlen(buf), " [%d]",
						world[EXIT(ch, door)->to_room]->
						number);
				if (IS_SET
				    (EXIT(ch, door)->exit_info, EX_CLOSED)) {
					if (IS_SET
					    (EXIT(ch, door)->exit_info,
					     EX_HIDDEN))
						sprintf(buf + strlen(buf),
							" - %s\r\n",
							"[Hidden]");
					else if (IS_SET
						 (EXIT(ch, door)->exit_info,
						  EX_SECRET))
						sprintf(buf + strlen(buf),
							" - %s\r\n",
							"[Secret]");
					else
						sprintf(buf + strlen(buf),
							" - %s\r\n",
							"[Closed]");
				} else
				    if (IS_SET
					(EXIT(ch, door)->exit_info,
					 EX_ILLUSION))
					sprintf(buf + strlen(buf), " - %s\r\n",
						"[Illusion]");
				else if (IS_DARK(EXIT(ch, door)->to_room)
					 && !ch->specials.holyLite
					 && world[ch->in_room]->zone != 198)
					sprintf(buf + strlen(buf), " - %s\n\r",
						"Darkness..");
				else
					sprintf(buf + strlen(buf), " - %s\n\r",
						world[EXIT(ch, door)->to_room]->
						name);
			}
		}

	send_to_char("Obvious exits:\n\r", ch);

	if (*buf)
		send_to_char(buf, ch);
	else
		send_to_char("None.\n\r", ch);
	global_color = 0;
}

void ShowTrackMessage(struct char_data *stpCh)
{
	int iDir = 0, iLoop = 0;
	struct char_data *i;
	char *szaTrackMessages[] = {
		"You discover tracks on the ground, indicating your prey left north.\r\n",
		"You discover tracks on the ground, indicating your prey left east.\r\n",
		"You discover tracks on the ground, indicating your prey left south.\r\n",
		"You discover tracks on the ground, indicating your prey left west.\r\n",
		"You discover tracks on the ground, indicating your prey left up.\r\n",
		"You discover tracks on the ground, indicating your prey left down.\r\n",
		"You search the area but find no trace of your prey.\r\n",
		"Your prey is in the immediate area with you!\r\n"
	};

	int choose_scent(struct char_data *ch, struct char_data *hunted);

	if (IS_NPC(stpCh))
		return;

	if (!stpCh->specials.hunting) {
		log_hd
		    ("### SHOWING TRACKING MESSAGES WHEN NO ONE BEING TRACKED.");
		return;
	}

	for (i = world[stpCh->in_room]->people; i; i = i->next_in_room)
		if (i == stpCh->specials.hunting) {
			iDir = 7;
			break;
		}

	if (!iDir)
		if (number(1, 101) > stpCh->skills[SKILL_TRACK].learned) {
			if (number(1, 100) > 25)
				iDir = 6;
			else {
				while (1) {
					if (iLoop > 5) {
						iDir = 6;
						break;
					}
					iDir = number(0, 5);
					if (EXIT(stpCh, iDir))
						break;
					iLoop++;
				}
			}
		} else
			iDir = choose_scent(stpCh, stpCh->specials.hunting);

	global_color = 37;
	send_to_char(szaTrackMessages[iDir], stpCh);
	global_color = 0;
	act("$n searchs the area for $s prey.", FALSE, stpCh, 0, 0, TO_ROOM);

	WAIT_STATE(stpCh, 10);

}

void do_map(struct char_data *ch, char *argument, int cmd)
{
	do_areamap(ch, "", 9);
}

void do_glance(struct char_data *ch, char *argument, int cmd)
{
	struct char_data* victim;
	char buffer[MAX_INPUT_LENGTH] = { 0 };
	one_argument(argument, buffer);

	if (strlen(buffer) == 0) {
		send_to_char("Glance at whom?\n", ch);
		return;
	}

	if (!(victim = get_char_room_vis(ch, buffer))) {
		send_to_char("You do not see that here.\n", ch);
		return;
	}

	sprintf(buffer, GET_NAME(victim));

	int percent = 100 * GET_HIT(victim) / GET_MAX_HIT(victim);
	if (percent >= 100)
		strcat(buffer, " is in an excellent condition.\n\r");
	else if (percent >= 90)
		strcat(buffer, " has a few scratches.\n\r");
	else if (percent >= 83)
		strcat(buffer, " has a nasty looking welt on the forehead.\n\r");
	else if (percent >= 76)
		strcat(buffer, " has some small wounds and bruises.\n\r");
	else if (percent >= 69)
		strcat(buffer, " winces in pain.\n\r");
	else if (percent >= 62)
		strcat(buffer, " has some minor wounds.\n\r");
	else if (percent >= 55)
		strcat(buffer, " has quite a few wounds.\n\r");
	else if (percent >= 48)
		strcat(buffer, " grimaces in pain.\n\r");
	else if (percent >= 41)
		strcat(buffer, " has some big nasty wounds and scratches.\n\r");
	else if (percent >= 36)
		strcat(buffer, " has some large, gaping wounds.\n\r");
	else if (percent >= 29)
		strcat(buffer, " looks pretty awful.\n\r");
	else if (percent >= 22)
		strcat(buffer, " screams in agony.\n\r");
	else if (percent >= 15)
		strcat(buffer, " is vomiting blood.\n\r");
	else if (percent >= 8)
		strcat(buffer, " pales visibly as Death nears.\n\r");
	else if (percent > 0)
		strcat(buffer, " barely clings to life.\n\r");
	else
		strcat(buffer, " is dead.\n\r");

	global_color = 33;
	send_to_char(buffer, ch);
	global_color = 0;
}

void do_stats(struct char_data *ch, char *argument, int cmd)
{
	int xp_to_level = exp_table[GET_LEVEL(ch) + 1] - GET_EXP(ch);
	char xp[16];

	if (GET_LEVEL(ch) < 31) {
		sprintf(xp, "%d", xp_to_level);
	}
	else {
		sprintf(xp, "None needed");
	}

	const char* fmt = " %s%4d/%4dhp  %4d/%4dm  %4d/%4dmv  %dac  %dhr %ddr  %4da  %10sxp\n";
	char buf[256];
	sprintf(buf, fmt, COL_BLU,
		GET_HIT(ch), GET_MAX_HIT(ch),
		GET_MANA(ch), GET_MAX_MANA(ch),
		GET_MOVE(ch), GET_MAX_MOVE(ch),
		GET_AC(ch), GET_HITROLL(ch) + str_app[STRENGTH_APPLY_INDEX(ch)].tohit,
		GET_DAMROLL(ch) + str_app[STRENGTH_APPLY_INDEX(ch)].todam,
		GET_ALIGNMENT(ch), xp);
	send_to_char(buf, ch);

	send_to_char(COL_NRM, ch);
}

void do_score(struct char_data *ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	struct affected_type *aff = NULL;
	extern char *apply_types[];
	extern char *spells[];
	extern char *pc_class_types[];
	int iX, iY, iMes, iClock, iAngle, iDistance;
	extern int iHoloWhere(int iX1, int iY1, int iX2, int iY2,
			      int *ipDistan);
	extern char *szaSurveyDists[];

	global_color = 34;
	sprintf(buf, "%s %s\n\r", GET_NAME(ch), get_title(ch));
	send_to_char(buf, ch);

	global_color = 0;
	sprintf(buf, "Level %-2d %-20s     ",
		GET_LEVEL(ch), pc_class_types[(int)GET_CLASS(ch)]);
	send_to_char(buf, ch);
	global_color = 34;
	send_to_char("Age: ", ch);
	global_color = 0;
	sprintf(buf, "%d years old", GET_AGE(ch));

	if ((age(ch).month == 0) && (age(ch).day == 0))
		strcat(buf, " It's your birthday today.\n\r");
	else
		strcat(buf, "\n\r");
	send_to_char(buf, ch);

	global_color = 34;
	send_to_char("Hit Points: ", ch);
	global_color = 0;
	sprintf(buf, "%d", GET_HIT(ch));
	send_to_char(buf, ch);
	global_color = 34;
	send_to_char("(", ch);
	global_color = 0;
	sprintf(buf, "%d", GET_MAX_HIT(ch));
	send_to_char(buf, ch);
	global_color = 34;
	send_to_char(")     Mana: ", ch);
	global_color = 0;
	sprintf(buf, "%d", GET_MANA(ch));
	send_to_char(buf, ch);
	global_color = 34;
	send_to_char("(", ch);
	global_color = 0;
	sprintf(buf, "%d", GET_MAX_MANA(ch));
	send_to_char(buf, ch);
	global_color = 34;
	send_to_char(")     Movement: ", ch);
	global_color = 0;
	sprintf(buf, "%d", GET_MOVE(ch));
	send_to_char(buf, ch);
	global_color = 34;
	send_to_char("(", ch);
	global_color = 0;
	sprintf(buf, "%d", GET_MAX_MOVE(ch));
	send_to_char(buf, ch);
	global_color = 34;
	send_to_char(")\n\r", ch);

	send_to_char("Str: ", ch);
	global_color = 0;
	sprintf(buf, "%-2d                           ", GET_STR(ch));
	send_to_char(buf, ch);
	global_color = 34;
	send_to_char("Armor Class: ", ch);
	global_color = 0;
	sprintf(buf, "%d\n\r", GET_AC(ch));
	send_to_char(buf, ch);
	global_color = 34;
	send_to_char("Int: ", ch);
	global_color = 0;
	sprintf(buf, "%-2d\n\r", GET_INT(ch));
	send_to_char(buf, ch);
	global_color = 34;
		send_to_char("Wis: ", ch);
		global_color = 0;
		sprintf(buf, "%-2d", GET_WIS(ch));
		send_to_char(buf, ch);
		global_color = 34;
		send_to_char("                           Hitroll: ", ch);
		global_color = 0;
		sprintf(buf, "%d\n\r",
			GET_HITROLL(ch) +
			str_app[STRENGTH_APPLY_INDEX(ch)].tohit);
		send_to_char(buf, ch);
	global_color = 34;
		send_to_char("Dex: ", ch);
		global_color = 0;
		sprintf(buf, "%-2d", GET_DEX(ch));
		send_to_char(buf, ch);
		global_color = 34;
		send_to_char("                           Damroll: ", ch);
		global_color = 0;
		sprintf(buf, "%d\n\r",
			GET_DAMROLL(ch) +
			str_app[STRENGTH_APPLY_INDEX(ch)].todam);
		send_to_char(buf, ch);
	global_color = 34;
	send_to_char("Con: ", ch);
	global_color = 0;
	sprintf(buf, "%-2d\n\r", GET_CON(ch));
	send_to_char(buf, ch);

	global_color = 34;
	send_to_char("Sta: ", ch);
	global_color = 0;
	sprintf(buf, "%-2d\n\r", GET_STA(ch));
	send_to_char(buf, ch);

	global_color = 34;
	send_to_char("Experience Points: ", ch);
	global_color = 0;
	sprintf(buf, "%-10d", GET_EXP(ch));
	send_to_char(buf, ch);
	global_color = 34;
	send_to_char("     Gold Coins: ", ch);
	global_color = 0;
	sprintf(buf, "%d\n\r", GET_GOLD(ch));
	send_to_char(buf, ch);
	global_color = 34;
	if (GET_LEVEL(ch) < 31) {
		send_to_char("XP to Level: ", ch);
		global_color = 0;
		sprintf(buf, "%-10d",
			exp_table[GET_LEVEL(ch) + 1] - GET_EXP(ch));
		send_to_char(buf, ch);
		global_color = 34;
		send_to_char("           Practice Sessions: ", ch);
		global_color = 0;
		sprintf(buf, "%d\n\r", ch->specials.practices);
		send_to_char(buf, ch);
	} else {
		global_color = 34;
		send_to_char("XP to Level: ", ch);
		global_color = 0;
		send_to_char("None Needed          ", ch);
		global_color = 34;
		send_to_char("Practice Sessions: ", ch);
		global_color = 0;
		sprintf(buf, "%d\n\r", ch->specials.practices);
		send_to_char(buf, ch);
	}

	sprintf(buf, "You are carrying %d", IS_CARRYING_N(ch));
	send_to_char(buf, ch);
	global_color = 34;
	send_to_char("/", ch);
	global_color = 0;
	sprintf(buf, "%d items with a weight of %d",
		CAN_CARRY_N(ch), IS_CARRYING_W(ch));
	send_to_char(buf, ch);
	global_color = 34;
	send_to_char("/", ch);
	global_color = 0;
	sprintf(buf, "%d stones\n\r", CAN_CARRY_W(ch));
	send_to_char(buf, ch);

	if ((GET_ALIGNMENT(ch) < 1001) && (GET_ALIGNMENT(ch) > 900))
		send_to_char("You are a saint\n\r", ch);
	else if ((GET_ALIGNMENT(ch) < 901) && (GET_ALIGNMENT(ch) > 700))
		send_to_char("Goodness exudes from your body\n\r", ch);
	else if ((GET_ALIGNMENT(ch) < 701) && (GET_ALIGNMENT(ch) > 500))
		send_to_char("You are very good\n\r", ch);
	else if ((GET_ALIGNMENT(ch) < 501) && (GET_ALIGNMENT(ch) > 350))
		send_to_char("You are good\n\r", ch);
	else if ((GET_ALIGNMENT(ch) < 351) && (GET_ALIGNMENT(ch) > 300))
		send_to_char("You are almost good\n\r", ch);
	else if ((GET_ALIGNMENT(ch) < 301) && (GET_ALIGNMENT(ch) > 100))
		send_to_char("You are neutral with tendencies toward good\n\r",
			     ch);
	else if ((GET_ALIGNMENT(ch) < 101) && (GET_ALIGNMENT(ch) > -101))
		send_to_char("You are neutral\n\r", ch);
	else if ((GET_ALIGNMENT(ch) < -100) && (GET_ALIGNMENT(ch) > -301))
		send_to_char("You are neutral with tendencies toward evil\n\r",
			     ch);
	else if ((GET_ALIGNMENT(ch) < -300) && (GET_ALIGNMENT(ch) > -351))
		send_to_char("You are almost evil\n\r", ch);
	else if ((GET_ALIGNMENT(ch) < -350) && (GET_ALIGNMENT(ch) > -501))
		send_to_char("You are evil\n\r", ch);
	else if ((GET_ALIGNMENT(ch) < -500) && (GET_ALIGNMENT(ch) > -701))
		send_to_char("You are very evil\n\r", ch);
	else if ((GET_ALIGNMENT(ch) < -700) && (GET_ALIGNMENT(ch) > -901))
		send_to_char("Evil exudes from your body\n\r", ch);
	else if ((GET_ALIGNMENT(ch) < -900) && (GET_ALIGNMENT(ch) > -1001))
		send_to_char("You are prime evil itself\n\r", ch);

	send_to_char("Monster Kills", ch);
	global_color = 34;
	send_to_char(": ", ch);
	global_color = 0;
	sprintf(log_buf, "%-5d              ", ORIGINAL(ch)->specials.numkills);
	send_to_char(log_buf, ch);

	switch (GET_POS(ch)) {
	case POSITION_DEAD:
		send_to_char("You are DEAD!\n\r", ch);
		break;
	case POSITION_MORTALLYW:
		send_to_char("You are mortally wounded!\n\r", ch);
		break;
	case POSITION_INCAP:
		send_to_char("You are incapacitated, slowly fading away\n\r",
			     ch);
		break;
	case POSITION_STUNNED:
		send_to_char("You are stunned! You can't move\n\r", ch);
		break;
	case POSITION_SLEEPING:
		send_to_char("You are sleeping\n\r", ch);
		break;
	case POSITION_RESTING:
		send_to_char("You are resting.\n\r", ch);
		break;
	case POSITION_SITTING:
		send_to_char("You are sitting\n\r", ch);
		break;
	case POSITION_FIGHTING:
		if (ch->specials.fighting)
			act("You are fighting $N\n\r", FALSE, ch, 0,
			    ch->specials.fighting, TO_CHAR);
		else
			send_to_char("You are fighting thin air\n\r", ch);
		break;
	case POSITION_STANDING:
		send_to_char("You are standing\n\r", ch);
		break;
	default:
		send_to_char("You are floating\n\r", ch);
		break;
	}

	send_to_char("Player Kills", ch);
	global_color = 34;
	send_to_char(": ", ch);
	global_color = 0;
	sprintf(log_buf, "%-5d", ORIGINAL(ch)->specials.numpkills);
	send_to_char(log_buf, ch);

	if (is_formed(ch))
		send_to_char("               You are in a formation\n\r", ch);
	else
		send_to_char("\n\r", ch);

	send_to_char("Deaths", ch);
	global_color = 34;
	send_to_char(": ", ch);
	global_color = 0;
	sprintf(log_buf, "%-5d                     Hours Played",
		ch->specials.death_counter);
	send_to_char(log_buf, ch);
	global_color = 34;
	send_to_char(": ", ch);
	global_color = 0;
	sprintf(buf, "%ld\n\r",
		(ch->player.time.played + time(0) -
		 ch->player.time.logon) / 3600);
	send_to_char(buf, ch);
	/* Freight Where */

	if (!IS_NPC(ch) && ch->p && ch->p->stpFreight && !IS_FLYING(ch)) {
		if (world[ch->p->stpFreight->iLocationRoom]->zone != 197) {
			sprintf(buf,
				"Freight Location %s:%s Somewhere in %s.\n\r",
				BLU(ch), NRM(ch),
				zone_table[world
					   [ch->p->stpFreight->iLocationRoom]->
					   zone].name);
			send_to_char(buf, ch);
		} else {
			sprintf(buf, "Freight Location %s:%s ", BLU(ch),
				NRM(ch));
			iX = ch->p->stpFreight->iLocationX;
			iY = ch->p->stpFreight->iLocationY;
			iAngle =
			    iHoloWhere(HOLOX(ch), HOLOY(ch), iX, iY,
				       &iDistance);
			if (iAngle >= 360)
				iClock = 12;
			else if (iAngle >= 330)
				iClock = 11;
			else if (iAngle >= 300)
				iClock = 10;
			else if (iAngle >= 270)
				iClock = 9;
			else if (iAngle >= 240)
				iClock = 8;
			else if (iAngle >= 210)
				iClock = 7;
			else if (iAngle >= 180)
				iClock = 6;
			else if (iAngle >= 150)
				iClock = 5;
			else if (iAngle >= 120)
				iClock = 4;
			else if (iAngle >= 90)
				iClock = 3;
			else if (iAngle >= 60)
				iClock = 2;
			else if (iAngle >= 30)
				iClock = 1;
			else if (iAngle >= 0)
				iClock = 12;
			else
				iClock = -1;
			if (iDistance > 200)
				iMes = 0;
			else if (iDistance > 150)
				iMes = 1;
			else if (iDistance > 100)
				iMes = 2;
			else if (iDistance > 75)
				iMes = 3;
			else if (iDistance > 50)
				iMes = 4;
			else if (iDistance > 25)
				iMes = 5;
			else if (iDistance > 15)
				iMes = 6;
			else if (iDistance > 10)
				iMes = 7;
			else if (iDistance > 5)
				iMes = 8;
			else if (iDistance > 1)
				iMes = 9;
			else
				iMes = 10;

			send_to_char(buf, ch);
			if (iAngle == -1)
				sprintf(buf,
					"Your freight is right with you.\r\n");
			else
				sprintf(buf,
					"Your freight is at %d o'Clock %s.\r\n",
					iClock, szaSurveyDists[(int)iMes]);

			send_to_char(buf, ch);
		}
	}

	if (IS_DEAD(ch)) {
		if (ORIGINAL(ch)->specials.death_timer > 20)
			sprintf(log_buf,
				"Fight a necromancer for your soul and regain your life.\n\r");
		else if (ORIGINAL(ch)->specials.death_timer > 0)
			sprintf(log_buf,
				"You will remain undead for at least %d minutes.\n\r",
				ORIGINAL(ch)->specials.death_timer);
		else
			sprintf(log_buf,
				"Pray at the holy place and the god of the Magic Code will take pity on you.\n\r");
		send_to_char(log_buf, ch);
	}

	if (GET_COND(ch, DRUNK) > 10)
		send_to_char("You are intoxicated.\n\r", ch);
	if (!GET_COND(ch, THIRST))
		send_to_char("You are thirsty.\n\r", ch);
	if (!GET_COND(ch, FULL))
		send_to_char("You are hungry.\n\r", ch);

	if (ch->affected) {
		global_color = 0;
		send_to_char("You are affected by:\n\r", ch);
		for (aff = ch->affected; aff; aff = aff->next) {
			if (aff->type != SKILL_SNEAK) {
				sprintf(buf, "  %-20s: ",
					spells[aff->type - 1]);
				global_color = 34;
				send_to_char(buf, ch);
				global_color = 0;
					sprintf(buf,
						"Modifies %s by %d points, Duration - %3d hours\n\r",
						apply_types[(int)aff->location],
						aff->modifier, aff->duration);
					send_to_char(buf, ch);
			}
		}
	}

	if (ch->specials.birthday == (age(ch).year - 1) && !IS_NPC(ch)) {
		ch->specials.birthday = age(ch).year;
		send_to_char("\n\r\n\rHAPPY BIRTHDAY!!!!!!\n\r", ch);
	}
/*   Old System for displaying armor class */
/*    send_to_char("Armor Class:  ",ch);
    if (GET_LEVEL(ch)>=20)
      {
	sprintf(buf, "%d.\n\r", GET_AC(ch));
	send_to_char(buf, ch);
      }
    else
      {
	if ((GET_AC(ch)<101) && (GET_AC(ch)>90))
	  send_to_char("You are naked.  Better get some clothes.\n\r",ch);
	else
	if ((GET_AC(ch)<91) && (GET_AC(ch)>70))
	  send_to_char("At least you are wearing clothes.\n\r",ch);
	else
	if ((GET_AC(ch)<71) && (GET_AC(ch)>50))
	  send_to_char("You feel slightly armored.\n\r",ch);
	else
	if ((GET_AC(ch)<51) && (GET_AC(ch)>40))
	  send_to_char("You feel partially armored.\n\r",ch);
	else
	if ((GET_AC(ch)<41) && (GET_AC(ch)>20))
	  send_to_char("You feel armored.\n\r",ch);
	else
	if ((GET_AC(ch)<21) && (GET_AC(ch)>10))
	  send_to_char("You feel heavily armored.\n\r",ch);
	else
	    if ((GET_AC(ch)<11) && (GET_AC(ch)>-10))
	  send_to_char("You are very heavily armored.\n\r",ch);
	else
	    if ((GET_AC(ch)<-9) && (GET_AC(ch)>-30))
	  send_to_char("You are superbly armored.\n\r",ch);
	else
	if ((GET_AC(ch)<-29) && (GET_AC(ch)>-50))
	  send_to_char("Your entire body is covered with armor.\n\r",ch);
	else
	if ((GET_AC(ch)<-49) && (GET_AC(ch)>-70))
	  send_to_char("You feel invincible!\n\r",ch);
	else
	    if ((GET_AC(ch)<-69) && (GET_AC(ch)>-90))
	  send_to_char("You have the gods on your side.\n\r",ch);
	else
	if ((GET_AC(ch)<-89) && (GET_AC(ch)>-120))
	  send_to_char("You are wearing divine armor.  May I have some?\n\r",
		ch);
	else
	if ((GET_AC(ch)<-119) && (GET_AC(ch)>-140))
	  send_to_char("You are a walking juggernaut.\n\r",ch);
	else
	if (GET_AC(ch)<-139)
	  send_to_char("Nothing can touch you now!\n\r",ch);
      }
*/

}

void do_time(struct char_data *ch, char *argument, int cmd)
{
	char buf[100], *suf;
	int weekday, day;
	long ct;
	extern struct time_info_data time_info;
	extern const char *weekdays[];
	extern const char *month_name[];
	/* 35 days in a month */
	weekday = ((35 * time_info.month) + time_info.day + 1) % 7;
	global_color = 36;
	sprintf(buf, "It is %d o'clock %s, on %s.\n\r",
		((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
		((time_info.hours >= 12) ? "pm" : "am"), weekdays[weekday]);
	send_to_char("\n\r              GAMES TIMES\n\r", ch);
	send_to_char(buf, ch);

	day = time_info.day + 1;	/* day in [1..35] */

	if (day == 1)
		suf = "st";
	else if (day == 2)
		suf = "nd";
	else if (day == 3)
		suf = "rd";
	else if (day < 20)
		suf = "th";
	else if ((day % 10) == 1)
		suf = "st";
	else if ((day % 10) == 2)
		suf = "nd";
	else if ((day % 10) == 3)
		suf = "rd";
	else
		suf = "th";

	sprintf(buf, "The %d%s Day of the %s, Year %d.\n\r",
		day, suf, month_name[time_info.month], time_info.year);

	send_to_char(buf, ch);
	ct = time(0);
	sprintf(buf,
		"\n\r              SYSTEM TIMES\n\r   Medievia start time was %s\r",
		str_boot_time);
	send_to_char(buf, ch);
	sprintf(buf, "The current system time is %s\n\r", ctime(&ct));
	send_to_char(buf, ch);
	global_color = 0;
}

void do_weather(struct char_data *ch, char *argument, int cmd)
{
	extern struct weather_data weather_info;
	char buf[256];
/*
    static char *sky_look[4]= {
    "cloudless",
    "cloudy",
    "rainy",
    "lit by flashes of lightning"};

    if (OUTSIDE(ch)) {
	sprintf(buf, 
	"The sky is %s and %s.\n\r",
	    sky_look[weather_info.sky],
	    (weather_info.change >=0 ? "you feel a warm wind from south" :
	 "your foot tells you bad weather is due"));
	send_to_char(buf,ch);
    } else
	send_to_char("You have no feeling about the weather at all.\n\r", ch);
*/
	if (OUTSIDE(ch)) {
		switch (weather_info.sky) {
		case SKY_CLOUDLESS:
			sprintf(buf, "%s\r\n", (weather_info.change >= 0 ?
						"The sky is clear without a cloud in sight."
						:
						"The sky is clear, but there are clouds on the horizon."));
			break;
		case SKY_CLOUDY:
			sprintf(buf, "%s\r\n", (weather_info.change >= 0 ?
						"The sky is cloudy with clear skys in the distance."
						:
						"The sky is cloudy and you see dark storm clouds approaching."));
			break;
		case SKY_RAINING:
			sprintf(buf, "%s\r\n", (weather_info.change >= 0 ?
						"It is raining, but it appears to be tapering off."
						:
						"A steady rain falls from the skys and you hear thunder in the distance."));
			break;
		case SKY_LIGHTNING:
			sprintf(buf, "%s\r\n", (weather_info.change >= 0 ?
						"Rain pours from the sky, but the lightning seems to be moving away."
						:
						"Lightning flashes all around you and the sky is dark for miles."));
			break;
		default:
			sprintf(buf, "Error in act_info.c!\r\n");
		}
		send_to_char(buf, ch);
	} else
		send_to_char
		    ("You have no feeling about the weather at all.\r\n", ch);
}

void do_help(struct char_data *ch, char *argument, int cmd)
{
	extern int top_of_helpt;
	extern struct help_index_element *help_index;
	extern FILE *help_fl;
	extern char help[MAX_STRING_LENGTH];

	int chk, bot, top, mid;
	char buf[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];

	if (!ch->desc)
		return;

	for (; isspace(*argument); argument++) ;

	if (*argument) {
		if (!help_index) {
			send_to_char("No help available.\n\r", ch);
			return;
		}
		bot = 0;
		top = top_of_helpt;

		for (;;) {
			mid = (bot + top) / 2;

			if (!(chk = str_cmp(argument, help_index[mid].keyword))) {
				fseek(help_fl, help_index[mid].pos, 0);
				*buffer = '\0';
				for (;;) {
					fgets(buf, 80, help_fl);
					if (*buf == '#')
						break;
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
				global_color = 33;
				page_string(ch->desc, buffer, 1);
				global_color = 0;
				return;
			} else if (bot >= top) {
				send_to_char
				    ("There is no help on that word.\n\r", ch);
				return;
			} else if (chk > 0)
				bot = ++mid;
			else
				top = --mid;
		}
		return;
	}

	global_color = 32;
	send_to_char(help, ch);
	global_color = 0;
}

char *get_status(struct char_data *ch)
{
	strcpy(log_buf, "");
	if (GET_POS(ch) == POSITION_SLEEPING)
		strcat(log_buf, "[ASLEEP]");
	if (ch->in_room && ch->specials.home_number == ch->in_room)
		strcat(log_buf, "(At Home)");
	/*if(ch->desc&&ch->desc->str)
	   strcat(log_buf,"[*EDITING*]"); */
	if (ch->desc && IS_SET(ch->specials.plr_flags, PLR_WRITING))
		strcat(log_buf, "[*EDITING*]");
	if (world[(ch)->in_room]->zone == 198)
		strcat(log_buf, "(Catacombs)");
	if (ch->specials.afk)
		strcat(log_buf, "[*AFK*]");
	if (GET_POS(ch) == POSITION_FIGHTING)
		strcat(log_buf, "{FIGHTING}");
	if (ch->desc && ch->desc->connected == CON_SOCIAL_ZONE)
		strcat(log_buf, "(MedLink)");
	if (ch->in_room == 0)
		strcat(log_buf, "(IDLE)");
	if (is_formed(ch))
		strcat(log_buf, "<IN FORMATION>");
	if (IS_HOVERING(ch))
		strcat(log_buf, "<RECENTLY DECEASED>");
	if (IS_UNDEAD(ch))
		strcat(log_buf, "<UNDEAD CORPSE>");
	return (log_buf);
}

void make_finger_info(char *str)
{
	FILE *fh;

	if (!(fh = med_open("/tmp/finger.txt", "w"))) {
		log_hd("## couldn't open finger.txt file");
		return;
	}
	open_files++;
	fputs
	    ("       MEDIEVIA MUD list of players currently online(port 4000)\n       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n",
	     fh);
	write_filtered_text(fh, str);
	med_close(fh);
	open_files--;
}

void do_who(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *whoname = NULL;
	struct descriptor_data *d = NULL;
	char buf[256];
	char tmp[256];
	char temp[256];
	char num1[10], num2[10];
	char arg[MAX_INPUT_LENGTH];
	char name[MAX_INPUT_LENGTH];
	char sex;
	int ct, cc, cm, cw;
	int levelone, leveltwo, c, x;
	int chzone;
	int count = 0;
	levelone = 0;
	leveltwo = 40;
	chzone = -1;
	sex = -1;
	ct = cc = cm = cw = 0;
	half_chop(argument, arg, name);

	if (!*arg) {
		levelone = 0;
		leveltwo = 40;
	} else {
		if ((!strcmp(arg, "-n")) && name[0]) {
			if (!(whoname = get_char(name))) {
				sprintf(buf,
					"%s is not in Medievia at the moment.\n\r",
					name);
				send_to_char(buf, ch);
				return;
			}
			if (IS_PLAYER(whoname, "Starblade")) {
				sprintf(buf,
					"%s is not in Medievia at the moment.\n\r",
					name);
				send_to_char(buf, ch);
				return;
			}
			if ((GET_LEVEL(whoname) > 32)) {
				sprintf(buf,
					"%s is not in Medievia at the moment.\n\r",
					name);
				send_to_char(buf, ch);
				return;
			}
			if (!IS_NPC(whoname)) {
				sprintf(buf,
					"[TL-%d :: LV-%d] [PKs-%d :: Clan-%d] %s\n\r",
					get_total_level(whoname),
					GET_LEVEL(whoname),
					whoname->specials.numpkills,
					whoname->specials.clan,
					GET_NAME(whoname));
				send_to_char(buf, ch);
				return;
			} else {
				sprintf(buf,
					"[TL-MOB :: LV-%d] %s is online.\n\r",
					GET_LEVEL(whoname), name);
				send_to_char(buf, ch);
				return;
			}
		}
		if (!strcmp(arg, "-z")) {
			chzone = world[ch->in_room]->zone;
			levelone = 0;
			leveltwo = 40;
		} else if (!strcmp(arg, "-f")) {
			sex = SEX_FEMALE;
			levelone = 0;
			leveltwo = 40;
		} else if (!strcmp(arg, "-m")) {
			sex = SEX_MALE;
			levelone = 0;
			leveltwo = 40;
		} else if (!strcmp(arg, "-n")) {
			sex = SEX_NEUTRAL;
			levelone = 0;
			leveltwo = 40;
		} else {
			c = -1;
			do {
				c++;
				num1[c] = arg[c];
			} while (arg[c] != 0 && arg[c] != '-' && arg[c] != '+');
			if (arg[c] == 0)
				arg[0] = 0;
			else {
				num1[c + 1] = 0;
				if (arg[c] == '+') {
					levelone = atoi(num1);
					leveltwo = 40;
				} else if (arg[c] == '-' && arg[c + 1] == 0) {
					levelone = 0;
					leveltwo = atoi(num1);
				} else {
					x = -1;
					do {
						x++;
						c++;
						num2[x] = arg[c];
					} while (arg[c] != 0);
					levelone = atoi(num1);
					leveltwo = atoi(num2);
					if (abs(levelone) > 100
					    || abs(leveltwo) > 100)
						arg[0] = 0;
					if (abs(levelone) > abs(leveltwo))
						arg[0] = 0;
				}
			}
		}
	}
	if (!*arg) {
		levelone = 0;
		leveltwo = 40;
	}
	ch->specials.setup_page = 1;	/*so all text goes to this buffer instead ofch */
	page_setup[0] = MED_NULL;
	global_color = 1;
	sprintf(log_buf,
		"  TL Lv CC  S   PKs   Clan Name\r\n-----------------------------------------------------------------\r\n");
	send_to_char(log_buf, ch);
	global_color = 0;
	for (d = descriptor_list; d; d = d->next) {
		if (((d->connected == CON_PLAYING)
		     || (d->connected == CON_HOVERING)
		     || (d->connected == CON_UNDEAD)
		     || (d->connected == CON_SOCIAL_ZONE)
		    )
		    && (GET_LEVEL(d->character) >= levelone)
		    && (GET_LEVEL(d->character) <= leveltwo)) {
			if (chzone == -1
			    || chzone == world[d->character->in_room]->zone) {
				if (sex != -1 && GET_SEX(d->character) != sex)
					continue;
				if (IS_PLAYER(d->character, "Starblade"))
					continue;

				if (IS_DEAD(d->character))
					sprintf(temp, "DED");
				else
					sprintf(temp, "MOB");

/* juice
 * add test here to make sure ch can see level 35's
 * or else they aren't counted since they don't show
 * up on who list
 */
				if ((GET_CLASS(d->character) == CLASS_WARRIOR)
				    && !((GET_LEVEL(d->character) >= 33)
					 && (!CAN_SEE(ch, d->character))
				    )
				    ) {
					sprintf(temp, "WAR");
					cw++;
				} else
					
				    if ((GET_CLASS(d->character) == CLASS_THIEF)
					&& !((GET_LEVEL(d->character) >= 33)
					     && (!CAN_SEE(ch, d->character))
					)
				    ) {
					sprintf(temp, "THI");
					ct++;
				} else
				    if ((GET_CLASS(d->character) ==
					 CLASS_CLERIC)
					&& !((GET_LEVEL(d->character) >= 33)
					     && (!CAN_SEE(ch, d->character))
					)
				    ) {
					sprintf(temp, "CLE");
					cc++;
				} else
				    if ((GET_CLASS(d->character) ==
					 CLASS_MAGIC_USER)
					&& !((GET_LEVEL(d->character) >= 33)
					     && (!CAN_SEE(ch, d->character))
					)
				    ) {
					sprintf(temp, "MAG");
					cm++;
				}
				if (GET_LEVEL(d->character) == 35)
					sprintf(temp, "GOD");
				else if (GET_LEVEL(d->character) == 34)
					sprintf(temp, "SUP");
				else if (GET_LEVEL(d->character) == 33)
					sprintf(temp, "DEI");
				else if (GET_LEVEL(d->character) == 32)
					sprintf(temp, "IMM");
				else if (get_total_level(d->character) == 124)
					sprintf(temp, "HER");

				if (GET_SEX(d->character) == SEX_MALE)
					strcat(temp, "-M");
				else if (GET_SEX(d->character) == SEX_FEMALE)
					strcat(temp, "-F");
				else if (GET_SEX(d->character) == SEX_NEUTRAL)
					strcat(temp, "-N");
				/*if level 35 guy is invisible make him MAG (SNEAKY) */

				if (GET_LEVEL(d->character) > 32
				    && IS_AFFECTED(d->character, AFF_INVISIBLE)
				    && GET_LEVEL(ch) < GET_LEVEL(d->character))
					continue;

				sprintf(buf, "[%3d %2d %s]",
					get_total_level(d->character),
					GET_LEVEL(d->character), temp);
				global_color = 31;
				if (GET_LEVEL(d->character) < 31)
					global_color = 33;
				if (GET_LEVEL(d->character) < 20)
					global_color = 32;
				if (GET_LEVEL(d->character) < 10)
					global_color = 35;
				send_to_char(buf, ch);
				global_color = 0;
				temp[0] = '\0';
				tmp[0] = '\0';
				if (IS_SET
				    (d->character->specials.affected_by,
				     AFF_KILLER))
					sprintf(temp, "(KILLER) ");
				if (IS_SET
				    (d->character->specials.affected_by,
				     AFF_THIEF))
					sprintf(tmp, "(THIEF)");

				if (ORIGINAL(d->character)->specials.numpkills)
					sprintf(buf, " [%4d] ",
/*			ORIGINAL(d->character)->specials.numkills, */
						ORIGINAL(d->character)->
						specials.numpkills);
				else
					sprintf(buf, " [    ] ");
				global_color = 33;
				send_to_char(buf, ch);
				if ((d->character->specials.clan)
				    && (GET_LEVEL(d->character) < 33))
					sprintf(buf, "[%2d] ",
						d->character->specials.clan);
				else
					sprintf(buf, "[  ] ");
				send_to_char(buf, ch);
				global_color = 0;

				if (IS_UNDEAD(d->character)) {
					sprintf(buf, "%s %s %s %s\n\r",
						GET_NAME(d->character),
						get_status(d->character),
						temp, tmp);
				} else {
					sprintf(buf, "%s %s %s %s\n\r",
						GET_NAME(ORIGINAL
							 (d->character)),
						get_status(ORIGINAL
							   (d->character)),
						temp, tmp);
				}	/*finger thing */
				if (IS_AFFECTED(d->character, AFF_INVISIBLE)
				    && cmd != 1219)
					sprintf(buf,
						"Someone (INVIS) %s %s\n\r",
						temp, tmp);
				if (IS_AFFECTED(d->character, AFF_HIDE)
				    && cmd != 1219)
					sprintf(buf,
						"Someone (HIDDEN) %s %s\n\r",
						temp, tmp);
				if (IS_AFFECTED(d->character, AFF_INVISIBLE)
				    && GET_LEVEL(ch) == 35)
					sprintf(buf, "%s [INVIS] %s %s %s \n\r",
						GET_NAME(ORIGINAL
							 (d->character)),
						IS_DEAD(d->
							character) ?
						get_status(d->character)
						:
						get_status(ORIGINAL
							   (d->character)),
						temp, tmp);
				if (IS_AFFECTED(d->character, AFF_HIDE)
				    && GET_LEVEL(ch) == 35)
					sprintf(buf,
						"%s [HIDDEN] %s %s %s \n\r",
						GET_NAME(ORIGINAL
							 (d->character)),
						IS_DEAD(d->character)
						? get_status(d->character)
						:
						get_status(ORIGINAL
							   (d->character)),
						temp, tmp);

				global_color = 36;
				send_to_char(buf, ch);
				global_color = 0;
				count++;
			}
		}
	}
	global_color = 1;
	sprintf(buf,
		"\n\r[%d] Total Players [%d]MAG [%d]THI [%d]CLE [%d]WAR\n\r",
		count, cm, ct, cc, cw);
	send_to_char(buf, ch);
	global_color = 0;
	ch->specials.setup_page = 0;
	if (cmd == 1219) {	/*makes finger.txt for finger @ourmachine */
		make_finger_info(page_setup);
		return;
	}
	page_string(ch->desc, page_setup, 1);
}

void do_sockets(struct char_data *ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	char findhost[300], *p;
	int num_can_see = 0, x;
	struct descriptor_data *d = NULL;
	struct char_data *vict = NULL;

	if (IS_NPC(ch)) {
		send_to_char("Monsters don't care who's logged in.\n\r", ch);
		return;
	}
	one_argument(argument, buf);
	if (buf[0]) {
		vict = get_char(buf);
		if (!vict) {
			send_to_char("Could find no such person.\n\r", ch);
			return;
		}
		if (!vict->desc)
			return;
		strcpy(findhost, vict->desc->host);
		x = 0;
		p = findhost;
		for (x = 0; x < 3;) {
			if (p[0] == '.')
				x++;
			if (!p[0])
				return;
			p++;
		}
		p[0] = MED_NULL;
	}
	ch->specials.setup_page = 1;
	page_setup[0] = MED_NULL;
	global_color = 1;
	send_to_char("Socket Stats:\n\r------------\n\r", ch);
	for (d = descriptor_list; d; d = d->next) {
		if (vict)
			if (!strstr(d->host, findhost))
				continue;
		if (d->character == NULL)
			continue;
		if (d->character->player.name == NULL)
			continue;
		if (!CAN_SEE(ch, d->character))
			continue;
		if (IS_PLAYER(d->character, "Starblade"))
			continue;
		else
			num_can_see++;
		sprintf(buf, "%3d : %-30s / %-16s --", d->descriptor, d->host,
			d->original ? d->original->player.name : d->character->
			player.name);
		global_color = 32;
		send_to_char(buf, ch);
		global_color = 35;
		switch (d->connected) {
		case CON_SHOW_TITLE:
			send_to_char("CON_SELECT_TITLE\n\r", ch);
			break;
		case 20:
			send_to_char("CON_SHOW_TITLE\n\r", ch);
			break;
		case CON_PLAYING:
			send_to_char("CON_PLAYING\n\r", ch);
			break;
		case CON_GET_NAME:
			send_to_char("CON_GET_NAME\n\r", ch);
			break;
		case CON_GET_OLD_PASSWORD:
			send_to_char("CON_GET_OLD_PASSWORD\n\r", ch);
			break;
		case CON_CONFIRM_NEW_NAME:
			send_to_char("CON_CONFIRM_NEW_NAME\n\r", ch);
			break;
		case CON_GET_NEW_PASSWORD:
			send_to_char("CON_GET_NEW_PASSWORD\n\r", ch);
			break;
		case CON_CONFIRM_NEW_PASSWORD:
			send_to_char("CON_CONFIRM_NEW_PASSWORD\n\r", ch);
			break;
		case CON_GET_NEW_SEX:
			send_to_char("CON_GET_NEW_SEX\n\r", ch);
			break;
		case CON_GET_NEW_CLASS:
			send_to_char("CON_GET_NEW_CLASS\n\r", ch);
			break;
		case CON_READ_MOTD:
			send_to_char("CON_READ_MOTD\n\r", ch);
			break;
		case CON_SELECT_MENU:
			send_to_char("CON_SELECT_MENU\n\r", ch);
			break;
		case CON_RESET_PASSWORD:
			send_to_char("CON_RESET_PASSWORD\n\r", ch);
			break;
		case CON_CONFIRM_RESET_PASSWORD:
			send_to_char("CON_CONFIRM_RESET_PASSWORD\n\r", ch);
			break;
		case CON_EXDSCR:
			send_to_char("CON_EXDSCR\n\r", ch);
			break;
		case CON_HOVERING:
			send_to_char("CON_HOVERING\n\r", ch);
			break;
		case CON_UNDEAD:
			send_to_char("CON_UNDEAD\n\r", ch);
			break;
		case CON_SOCIAL_ZONE:
			send_to_char("CON_SOCIAL_ZONE\n\r", ch);
			break;
		default:
			sprintf(log_buf, "(%d)***UNKNOWN***\n\r", d->connected);
			send_to_char(log_buf, ch);
			break;
		}
	}
	sprintf(buf, "\n\rThere are %d visible users.\n\r", num_can_see);
	global_color = 1;
	send_to_char(buf, ch);
	global_color = 0;
	ch->specials.setup_page = 0;
	page_string(ch->desc, page_setup, 1);
}

void do_inventory(struct char_data *ch, char *argument, int cmd)
{
	global_color = 32;
	send_to_char("You are carrying:\n\r", ch);
	list_obj_to_char(ch->carrying, ch, 1, TRUE);
	global_color = 0;
}

void do_equipment(struct char_data *ch, char *argument, int cmd)
{
	int j;
	bool found;
	global_color = 32;
	send_to_char("You are using:\n\r", ch);
	found = FALSE;
	for (j = 0; j < MAX_WEAR; j++) {
		if (ch->equipment[j]) {
			if (CAN_SEE_OBJ(ch, ch->equipment[j])) {
				global_color = 32;
				send_to_char(where[j], ch);
				show_obj_to_char(ch->equipment[j], ch, 1, 1);
				found = TRUE;
			} else {
				global_color = 32;
				send_to_char(where[j], ch);
				global_color = 0;
				send_to_char("Something.\n\r", ch);
				found = TRUE;
			}
		} else {
			global_color = 32;
			send_to_char(where[j], ch);
			global_color = 0;
			send_to_char("Nothing.\n", ch);
		}
	}
	global_color = 0;
}

void do_credits(struct char_data *ch, char *argument, int cmd)
{
	global_color = 33;
	page_string(ch->desc, credits, 0);
	global_color = 0;
}

void do_story(struct char_data *ch, char *argument, int cmd)
{
	global_color = 33;
	page_string(ch->desc, story, 0);
	global_color = 0;
}

void do_news(struct char_data *ch, char *argument, int cmd)
{
	char szNews[MAX_STRING_LENGTH + 1], szBuf[MAX_INPUT_LENGTH * 2];
	char szFilename[MAX_INPUT_LENGTH * 2];
	int iNewsVersion;
	FILE *fp;

	if (!ch->p)
		return;
	one_argument(argument, szBuf);
	if (!szBuf || !szBuf[0]) {
		ch->p->iNewsVersion++;
		if (ch->p->iNewsVersion > giNewsVersion)
			ch->p->iNewsVersion = giNewsVersion;
		iNewsVersion = ch->p->iNewsVersion;
	} else {
		if (szBuf[0] == 'n' && GET_LEVEL(ch) >= 33) {
			if (!(fp = med_open("../news/news.dat", "r"))) {
				send_to_char("ERROR READING NEWS.DAT!\n\r", ch);
				return;
			}
			fscanf(fp, " %d ", &giNewsVersion);
			med_close(fp);
			send_to_char
			    ("NEWS NEW NEWS NEW NEWS NEW NEWS!!! :)  \n\r", ch);
			return;
		}
		iNewsVersion = atoi(szBuf);
	}
	sprintf(szFilename, "../news/news.%d.txt", iNewsVersion);
	if (!(fp = med_open(szFilename, "r"))) {
		sprintf(szBuf, "There is no News File Issue [%d].\n\r",
			iNewsVersion);
		send_to_char(szBuf, ch);
		return;
	}
	med_close(fp);
	file_to_string(szFilename, szNews);
	global_color = 31;
	sprintf(szBuf, "News File Issue [%s%d%s].\n\r", WHT(ch), iNewsVersion,
		RED(ch));
	send_to_char(szBuf, ch);
	global_color = 33;
	page_string(ch->desc, szNews, TRUE);
	global_color = 0;
}

void do_info(struct char_data *ch, char *argument, int cmd)
{
	global_color = 33;
	page_string(ch->desc, info, 0);
	global_color = 0;
}

void do_wizlist(struct char_data *ch, char *argument, int cmd)
{
	global_color = 33;
	page_string(ch->desc, wizlist, 0);
	global_color = 0;
}

void do_affects(struct char_data *ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];
	extern char *apply_types[];
	extern char *spells[];
	struct affected_type *aff = NULL;

	if (ch->affected) {
		global_color = 0;
		send_to_char("You are affected by:\n\r", ch);
		for (aff = ch->affected; aff; aff = aff->next) {
			if (aff->type != SKILL_SNEAK) {
				sprintf(buf, "  %-20s: ", spells[aff->type - 1]);
				global_color = 34;
				send_to_char(buf, ch);
				global_color = 0;
				sprintf(buf,
					"Modifies %s by %d points, Duration - %3d hours\n\r",
					apply_types[(int)aff->location],
					aff->modifier, aff->duration);
				send_to_char(buf, ch);
			}
		}
	}

}

void do_recall(struct char_data *ch, char *argument, int cmd)
{
	if (IS_FLYING(ch)) {
		global_color = 0;
		send_to_char("You cannot recall while flying!\n", ch);
		return;
	}

	send_to_char("You close your eyes and picture yourself back in the City of Medievia.\n\n", ch);
        act("$n disappears.", TRUE, ch, 0, 0, TO_ROOM);
	MOUNTMOVE = TRUE;
	char_from_room(ch);
	char_to_room(ch, 1);
	MOUNTMOVE = FALSE;
        act("$n appears in the room with a bright flash of white light.", TRUE, ch, 0, 0, TO_ROOM);
	do_look(ch, "", 0);
}

void do_playerinfo(struct char_data *ch, char *argument, int cmd)
{
	extern char *pc_class_types[];
	char name[MAX_INPUT_LENGTH];
	char strsave[80];
	one_argument(argument, name);

	if (strlen(name) == 0) {
		send_to_char("Which player?\n", ch);
		return;
	}

	name[0] = UPPER(name[0]);

	sprintf(strsave, "%s/%c/%s", SAVE_DIR, LOWER(name[0]), name);

	FILE *f = med_open(strsave, "rb");
	if (f == NULL) {
		send_to_char("That player does not exist.\n", ch);
		return;
	}

	struct char_file_u cfile;
	int bytes_read = fread(&cfile, sizeof(cfile), 1, f);
	med_close(f);

	struct char_data *c;
	CREATE(c, struct char_data, 1);
	reset_char(c);
	clear_pData(c);
	GET_NAME(c) = str_dup(cfile.name);
	c->formation[0][1] = c;
	c->master = c;
	store_to_char(&cfile, c);

	char buf[256];
	char classes[8];
	memset(classes, 0, sizeof(classes));

	if (get_total_level(c) == 124) {
		sprintf(classes, "(CMTW)");
	}
	else if (get_total_level(c) > 31) {
		strcat(classes, "(");
		if (IS_SET(c->player.multi_class, MULTI_CLASS_CLERIC))
			strcat(classes, "C");
		if (IS_SET(c->player.multi_class, MULTI_CLASS_MAGIC_USER))
			strcat(classes, "M");
		if (IS_SET(c->player.multi_class, MULTI_CLASS_THIEF))
			strcat(classes, "T");
		if (IS_SET(c->player.multi_class, MULTI_CLASS_WARRIOR))
			strcat(classes, "W");
		strcat(classes, ")");
	}

	char clan_name[64];
	if (global_clan_info.clan_name[c->specials.clan] && strlen(global_clan_info.clan_name[c->specials.clan]) > 1)
		sprintf(clan_name, global_clan_info.clan_name[c->specials.clan]);
	else
		sprintf(clan_name, "None");

	const char *fmt =
	    "%sName: %s%s\n"
	    "%sClan: %s%s (%d)\n"
	    "%sLevel: %s%d (%d)\n"
	    "%sClass: %s%s %s\n"
	    "%sSex: %s%s\n"
	    "%sPKs: %s%d\n";

	sprintf(buf, fmt,
		COL_BLU, COL_CYN, c->player.name,
		COL_BLU, COL_CYN, clan_name, c->specials.clan,
		COL_BLU, COL_CYN, c->player.level, get_total_level(c),
		COL_BLU, COL_CYN, pc_class_types[c->player.class], classes,
		COL_BLU, COL_CYN, sexes[GET_SEX(c)],
		COL_BLU, COL_CYN, c->specials.numpkills);

	send_to_char(buf, ch);

	if (get_player(c->player.name)) {
		sprintf(buf, "%s%s is on right now!\n", COL_WHT,
			c->player.name);
		send_to_char(buf, ch);
	}

	free_char(c);
	send_to_char(COL_NRM, ch);
}

void do_where(struct char_data *ch, char *argument, int cmd)
{
	char name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], buf2[256];
	register struct char_data *i = NULL;
	register struct obj_data *k = NULL;
	struct descriptor_data *d = NULL;
	int zonenumber;
	int count = 0;
	extern int iHoloChDistance(struct char_data *ch,
				   struct char_data *vict);

	buf[0] = '\0';
	one_argument(argument, name);

	if (!*name) {
		if (GET_LEVEL(ch) < 32) {
			ch->specials.setup_page = 1;
			page_setup[0] = MED_NULL;
			zonenumber = world[ch->in_room]->zone;
			sprintf(buf, "%s%s",
				WHT(ch),
				"Players in your vicinity:\n\r-------------------------\n\r");

			for (d = descriptor_list; d; d = d->next) {
				if (d->character
				    && ((d->connected == CON_PLAYING)
					|| (d->connected == CON_HOVERING)
					|| (d->connected == CON_UNDEAD)
					|| (d->connected == CON_SOCIAL_ZONE))
				    && (d->character->in_room != NOWHERE)
				    && CAN_SEE(ch, d->character)
				    && d->character != ch) {
					if (IS_PLAYER
					    (d->character, "Starblade"))
						continue;
/*
	      if  (world[d->character->in_room]->zone == zonenumber) {
*/
					if (iHoloChDistance(ch, d->character) <
					    50) {
						if (IS_SET
						    (world
						     [d->character->in_room]->
						     room_flags, GODPROOF)
						    && GET_LEVEL(ch) < 35)
/*			&&!IS_PLAYER(ch,"Vryce")
			&&!IS_PLAYER(ch,"Io")
			&&!IS_PLAYER(ch,"Firm"))
*/
							sprintf(buf,
								"%s%s[%s%-20s%s] %s- Somewhere\n\r",
								buf, BLU(ch),
								YEL(ch),
								GET_NAME(d->
									 character),
								BLU(ch),
								GRN(ch));
						else if (IS_AFFECTED
							 (d->character,
							  AFF_HIDE))
							sprintf(buf,
								"%s%s[%s%-20s%s] %s- Hidden\n\r",
								buf, BLU(ch),
								YEL(ch),
								"Someone",
								BLU(ch),
								GRN(ch));
						else if (IS_AFFECTED
							 (d->character,
							  AFF_SNEAK))
							sprintf(buf,
								"%s%s[%s%-20s%s] %s- Lurking about\n\r",
								buf, BLU(ch),
								YEL(ch),
								GET_NAME(d->
									 character),
								BLU(ch),
								GRN(ch));
						else
							sprintf(buf,
								"%s%s[%s%-20s%s] %s- %s\n\r",
								buf, BLU(ch),
								YEL(ch),
								GET_NAME(d->
									 character),
								BLU(ch),
								GRN(ch),
								world[d->
								      character->
								      in_room]->
								name);
					}	/* if in zone */
				}	/* if can see */
			}	/* for */
			global_color = 0;
			ch->specials.setup_page = 0;
			page_string(ch->desc, buf, 1);
			return;
		} /* if GET_LEVEL(ch)<32 */
		else {
			ch->specials.setup_page = 1;
			page_setup[0] = MED_NULL;
			strcpy(buf, "Players:\n\r--------\n\r");
			global_color = 1;
			send_to_char(buf, ch);
			global_color = 33;
			for (d = descriptor_list; d; d = d->next) {
				if (d->character
				    && ((d->connected == CON_PLAYING)
					|| (d->connected == CON_HOVERING)
					|| (d->connected == CON_UNDEAD)
					|| (d->connected == CON_SOCIAL_ZONE))
				    && (d->character->in_room != NOWHERE)
				    && CAN_SEE(ch, d->character)) {
					if (GET_LEVEL(d->character) >
					    GET_LEVEL(ch)
					    && IS_AFFECTED(d->character,
							   AFF_INVISIBLE))
						continue;
					count++;
					if (IS_PLAYER
					    (d->character, "Starblade"))
						continue;
					if (d->original && !IS_UNDEAD(ch))	/* If switched */
						sprintf(buf,
							"%-20s - %s [%d] In body of %s\n\r",
							d->original->player.
							name,
							world[d->character->
							      in_room]->name,
							world[d->character->
							      in_room]->number,
							fname(d->character->
							      player.name));
					else {
						if (IS_SET
						    (world
						     [d->character->in_room]->
						     room_flags, GODPROOF)
						    && GET_LEVEL(ch) < 35)
/*				&&!IS_PLAYER(ch,"Vryce")
				&&!IS_PLAYER(ch,"Shalafi")
				&&!IS_PLAYER(ch,"Io"))
*/
							sprintf(buf,
								"%-20s - Somewhere\n\r",
								GET_NAME(d->
									 character));
						else
							sprintf(buf,
								"%-20s - %s [%d]\n\r",
								d->character->
								player.name,
								world[d->
								      character->
								      in_room]->
								name,
								world[d->
								      character->
								      in_room]->
								number);
					}
					send_to_char(buf, ch);
				}
			}
			sprintf(buf, "\n\r[%d] Total Players\n\r", count);
			send_to_char(buf, ch);
			global_color = 0;
			ch->specials.setup_page = 0;
			page_string(ch->desc, page_setup, 1);
			return;
		}
	}

	*buf = '\0';
	ch->specials.setup_page = 1;
	page_setup[0] = MED_NULL;
	for (i = character_list; i; i = i->next)
		if (isname(name, i->player.name) && CAN_SEE(ch, i)) {
			if ((i->in_room != NOWHERE) && ((GET_LEVEL(ch) > 31) ||
							(world[i->in_room]->
							 zone ==
							 world[ch->in_room]->
							 zone
							 && !IS_AFFECTED(i,
									 AFF_HIDE))))
			{

				if (IS_NPC(i)) {
					if (GET_LEVEL(ch) > 31)
						sprintf(buf, "%-20s - %s ",
							i->player.short_descr,
							world[i->in_room]->
							name);
					else
						sprintf(buf,
							"%-20s - Close by ",
							i->player.short_descr);
				} else
				    if (IS_SET
					(world[i->in_room]->room_flags,
					 GODPROOF)
					&& GET_LEVEL(ch) < 35)
/*			&&!IS_PLAYER(ch,"Vryce")
			&&!IS_PLAYER(ch,"Io"))
*/
					sprintf(buf, "%-20s - Somewhere ",
						GET_NAME(i));
				else if (IS_AFFECTED(i, AFF_SNEAK)
					 && !IS_PLAYER(ch, "Vryce")
					 && !IS_PLAYER(ch, "Firm")
					 && !IS_PLAYER(ch, "Io")
					 && !IS_PLAYER(ch, "Shalafi")
					 && i != ch)
					sprintf(buf, "%-20s - Lurking about",
						i->player.name);
				else
					sprintf(buf, "%-20s - %s ",
						i->player.name,
						world[i->in_room]->name);
				if (!IS_SET
				    (world[i->in_room]->room_flags, GODPROOF)
				    && GET_LEVEL(ch) > 31)
					sprintf(buf2, "[%d]\n\r",
						world[i->in_room]->number);
				else
					strcpy(buf2, "\n\r");

				strcat(buf, buf2);
				send_to_char(buf, ch);

				if (GET_LEVEL(ch) < 32)
					break;
			}
		}

	if (GET_LEVEL(ch) > 31) {
		for (k = object_list; k; k = k->next) {
			if (isname(name, k->name) && CAN_SEE_OBJ(ch, k) &&
			    (k->in_room != NOWHERE)) {
				sprintf(buf, "%-30s- %s [%d]\n\r",
					k->short_description,
					world[k->in_room]->name,
					world[k->in_room]->number);
				send_to_char(buf, ch);
			}
		}
	}

	if (!*buf)
		send_to_char("Couldn't find any such thing.\n\r", ch);
	ch->specials.setup_page = 0;
	page_string(ch->desc, page_setup, 1);
	global_color = 0;
}

void do_levels(struct char_data *ch, char *argument, int cmd)
{
	int i;
	char buf[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int class;

	if (IS_NPC(ch)) {
		send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
		return;
	}
	class = GET_CLASS(ch);

	one_argument(argument, arg);

	if (*arg) {
		if (!str_cmp(arg, "magic"))
			class = 1;
		else if (!str_cmp(arg, "cleric"))
			class = 2;
		else if (!str_cmp(arg, "thief"))
			class = 3;
		else if (!str_cmp(arg, "fighter"))
			class = 4;
	}
	global_color = 34;
	for (i = 1; i <= 31; i++) {
		sprintf(buf, "[%2d] %10d %s\n\r",
			i,
			exp_table[i],
			title_table[class - 1][i][GET_SEX(ch) ==
						  SEX_FEMALE ? 1 : 0]
		    );
		send_to_char(buf, ch);
	}
	global_color = 0;
}

void do_consider(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim = NULL;
	char name[256];
	int diff;

	one_argument(argument, name);

	if (!(victim = get_char_room_vis(ch, name))) {
		send_to_char("Consider killing who?\n\r", ch);
		return;
	}

	if (victim == ch) {
		send_to_char("Easy! Very easy indeed!\n\r", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		send_to_char
		    ("Would you like to borrow a cross and a shovel?\n\r", ch);
		return;
	}

	diff = (GET_LEVEL(victim) - GET_LEVEL(ch));

	if (diff <= -10)
		send_to_char("Now where did that chicken go?\n\r", ch);
	else if (diff <= -5)
		send_to_char("You could do it with a needle!\n\r", ch);
	else if (diff <= -2)
		send_to_char("Easy.\n\r", ch);
	else if (diff <= -1)
		send_to_char("Fairly easy.\n\r", ch);
	else if (diff == 0)
		send_to_char("The perfect match!\n\r", ch);
	else if (diff <= 1)
		send_to_char("You would need some luck!\n\r", ch);
	else if (diff <= 2)
		send_to_char("You would need a lot of luck!\n\r", ch);
	else if (diff <= 3)
		send_to_char
		    ("You would need a lot of luck and great equipment!\n\r",
		     ch);
	else if (diff <= 5)
		send_to_char("Do you feel lucky, punk?\n\r", ch);
	else if (diff <= 10)
		send_to_char("Are you mad!?\n\r", ch);
	else if (diff <= 15)
		send_to_char("You ARE mad!\n\r", ch);
	else if (diff <= 20)
		send_to_char
		    ("Why don't you just lie down and pretend you are dead instead?\n\r",
		     ch);
	else if (diff <= 25)
		send_to_char("Death will thank you for your gift.\n\r", ch);
	else if (diff <= 30)
		send_to_char("What do you want your epitaph to say?\n\r", ch);
	else if (diff <= 35)
		send_to_char
		    ("What ever kills you WILL NOT make you stronger!\n\r", ch);
	else
		send_to_char
		    ("Here lies one dead and very dumb MEDIEVIA player.\n\r",
		     ch);
}
